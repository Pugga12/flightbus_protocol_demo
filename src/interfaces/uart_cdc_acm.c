//
// Created by adama on 3/23/25.
//
#include "uart_cdc_acm.h"
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/ring_buffer.h>
#include <zephyr/usb/usb_device.h>
LOG_MODULE_REGISTER(flightbus_cdc_acm_interface);

#define RING_BUF_SIZE 1024
uint8_t ring_buffer[RING_BUF_SIZE];
uint8_t write_buffer[RING_BUF_SIZE];

struct ring_buf readbuf;
struct ring_buf writebuf;
static bool rx_throttled;


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

int start_cdc_acm(const struct device *dev) {
    int ret;

    // initialize the usb port
    ret = usb_enable(NULL);
    if (ret != 0) {
        LOG_ERR("Failed to enable USB");
        return -1;
    }

    ring_buf_init(&readbuf, sizeof(ring_buffer), ring_buffer);
    ring_buf_init(&writebuf, sizeof(write_buffer), write_buffer);
    LOG_INF("USB initialized! Waiting for DTR");
    // wait for DTR signal
    waitForDTR(dev);
    LOG_INF("DTR set");

    // wait 100ms for the host to finish setting up
    k_msleep(100);

    uart_irq_callback_set(dev, interrupt_handler);
    uart_irq_rx_enable(dev);

    return 0;
}

// write to the cdc-acm port
void write(const uint8_t *data, size_t len, const struct device *dev) {
    ring_buf_put(&writebuf, data, len);
    uart_irq_tx_enable(dev);
}

// read a specified amount of bytes from the cdc-acm port
uint8_t* read(const size_t len, const struct device *dev) {
    uint8_t *data = k_malloc(len);
    if (!data) {
        return NULL;  // Memory allocation failed
    }
    ring_buf_get(&readbuf, data, len);
    if (ring_buf_item_space_get(&readbuf) < RING_BUF_SIZE && rx_throttled) {
        // recieve ringbuff is no longer full, unthrottle
        rx_throttled = false;
        uart_irq_rx_enable(dev);
    }
    return data;
}