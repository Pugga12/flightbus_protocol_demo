//
// Created by adama on 28/04/25.
//
#include "i_uart_generic.h"
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/ring_buffer.h>
#include <sys/types.h>
LOG_MODULE_DECLARE(uart_internal);

#define RING_BUF_SIZE 1024
uint8_t *read_buffer = NULL;
uint8_t *write_buffer = NULL;

struct ring_buf readbuf;
struct ring_buf writebuf;
static bool rx_throttled;
struct k_mutex read_mutex;
struct k_mutex write_mutex;

static bool allocateBuffers() {
    read_buffer = k_malloc(RING_BUF_SIZE);
    write_buffer = k_malloc(RING_BUF_SIZE);
    if (!read_buffer || !write_buffer) return false;
    ring_buf_init(&readbuf, sizeof(read_buffer), read_buffer);
    ring_buf_init(&writebuf, sizeof(write_buffer), write_buffer);
    return true;
}

static void free_buffers(void) {
    if (read_buffer) {
        k_free(read_buffer);
        read_buffer = NULL;
    }
    if (write_buffer) {
        k_free(write_buffer);
        write_buffer = NULL;
    }
}

static void interrupt_handler(const struct device *dev, void *user_data) {
    ARG_UNUSED(user_data);

    while (uart_irq_update(dev) && uart_irq_is_pending(dev)) {
        if (!rx_throttled && uart_irq_rx_ready(dev)) {
            int recv_len, rb_len;
            uint8_t buffer[64];

            size_t len = MIN(ring_buf_space_get(&readbuf),
                     sizeof(buffer));

            if (len == 0) {
                // Throttle because ring buffer is full
                uart_irq_rx_disable(dev);
                rx_throttled = true;
                continue;
            }

            recv_len = uart_fifo_read(dev, buffer, len);
            if (recv_len < 0) {
                LOG_ERR("Failed to read UART FIFO");
                recv_len = 0;
            }

            rb_len = ring_buf_put(&readbuf, buffer, recv_len);
            if (rb_len < recv_len) {
                LOG_ERR("Drop %u bytes", recv_len - rb_len);
            }

            LOG_DBG("tty fifo -> read buffer %d bytes", rb_len);
        }

        if (uart_irq_tx_ready(dev)) {
            uint8_t buffer[64];

            if (ring_buf_get(&writebuf, buffer, sizeof(buffer)) > 0) {
                int sendlen = uart_fifo_fill(dev, buffer, sizeof(buffer));
                LOG_DBG("write buffer -> write buffer %d bytes", sendlen);
            } else {
                uart_irq_tx_disable(dev);
            }
        }
    }
}

static void waitForDTR(const struct device *dev) {
    while (true) {
        uint32_t dtr = 0U;

        uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
        if (dtr) {
            break;
        } else {
            /* Give CPU resources to low priority threads. */
            k_sleep(K_MSEC(100));
        }
    }
}

void start(const struct device *dev, bool dtrWait) {
    k_mutex_init(&read_mutex);
    k_mutex_init(&write_mutex);
    allocateBuffers();
    // wait for dtr signal (if requested)
    if (dtrWait) {
        LOG_INF("Waiting for DTR");
        waitForDTR(dev);
        LOG_INF("DTR set");
    }
    uart_irq_callback_user_data_set(dev, interrupt_handler, NULL);
    uart_irq_rx_enable(dev);
}

void shutdown(const struct device *dev) {
    // disable interrupts
    uart_irq_rx_disable(dev);
    uart_irq_tx_disable(dev);
    uart_irq_callback_user_data_set(dev, NULL, NULL);
    rx_throttled = false;

    k_mutex_lock(&read_mutex, K_FOREVER);
    ring_buf_reset(&readbuf);
    k_mutex_unlock(&read_mutex);

    k_mutex_lock(&write_mutex, K_FOREVER);
    ring_buf_reset(&writebuf);
    k_mutex_unlock(&write_mutex);

    free_buffers();
    LOG_INF("Successfully shut down generic UART");
}

// read a specified amount of bytes to the uart port
ssize_t write(const uint8_t *data, const size_t len, const struct device *dev) {
    if (!data || len == 0) {
        return -EINVAL;  // Invalid argument
    }

    k_mutex_lock(&write_mutex, K_FOREVER);  // Lock before accessing ring buffer

    ssize_t bytes_written = (ssize_t)ring_buf_put(&writebuf, data, len);
    if (bytes_written == 0) {
        k_mutex_unlock(&write_mutex);
        return -EAGAIN;  // ring buffer full, try again later
    }

    if (bytes_written < len) {
        LOG_ERR("Incomplete Write: Written %u/%u bytes to TX buffer", bytes_written, len);
    }

    // If data was added to the ring buffer, enable TX interrupt
    uart_irq_tx_enable(dev);

    k_mutex_unlock(&write_mutex);  // Unlock after writing
    return bytes_written;
}

// read a specified amount of bytes from the uart port
ssize_t read(uint8_t *buffer, const size_t len, const struct device *dev) {
    if (!buffer || len <= 0) {
        return -EINVAL;
    }

    // lock the mutex, then read the ringbuf
    k_mutex_lock(&read_mutex, K_FOREVER);
    ssize_t bytes_read = (ssize_t)ring_buf_get(&readbuf, buffer, len);
    if (bytes_read == 0) {
        // no data unavailable, unlock and return an error
        k_mutex_unlock(&read_mutex);
        return -EAGAIN;
    }

    if (bytes_read < len) {
        LOG_ERR("Incomplete Read: Read %u/%u bytes from RX buffer", bytes_read, len);
    }

    if (ring_buf_item_space_get(&readbuf) < RING_BUF_SIZE && rx_throttled) {
        rx_throttled = false;
        uart_irq_rx_enable(dev);
    }

    k_mutex_unlock(&read_mutex);
    return bytes_read;
}

size_t getAvailableRXBufferSize() {
    k_mutex_lock(&read_mutex, K_FOREVER);
    const size_t available_bytes = ring_buf_size_get(&readbuf);
    k_mutex_unlock(&read_mutex);
    return available_bytes;
}
