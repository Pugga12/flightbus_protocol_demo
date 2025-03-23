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

struct ring_buf ringbuf;
static bool rx_throttled;


static void interrupt_handler(const struct device *dev, void *user_data) {
    ARG_UNUSED(user_data);

    while (uart_irq_update(dev) && uart_irq_is_pending(dev)) {
        if (!rx_throttled && uart_irq_rx_ready(dev)) {
            int recv_len, rb_len;
            uint8_t buffer[64];

            size_t len = MIN(ring_buf_space_get(&ringbuf),
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

            rb_len = ring_buf_put(&ringbuf, buffer, recv_len);
            if (rb_len < recv_len) {
                LOG_ERR("Drop %u bytes", recv_len - rb_len);
            }

            LOG_DBG("tty fifo -> ringbuf %d bytes", rb_len);
            if (rb_len) {
                uart_irq_tx_enable(dev);
            }
        }

        if (uart_irq_tx_ready(dev)) {
            uint8_t buffer[64];
            int rb_len, send_len;

            rb_len = ring_buf_get(&ringbuf, buffer, sizeof(buffer));
            if (!rb_len) {
                LOG_DBG("Ring buffer empty, disable TX IRQ");
                uart_irq_tx_disable(dev);
                continue;
            }

            if (rx_throttled) {
                uart_irq_rx_enable(dev);
                rx_throttled = false;
            }

            send_len = uart_fifo_fill(dev, buffer, rb_len);
            if (send_len < rb_len) {
                LOG_ERR("Drop %d bytes", rb_len - send_len);
            }

            LOG_DBG("ringbuf -> tty fifo %d bytes", send_len);
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

    ring_buf_init(&ringbuf, sizeof(ring_buffer), ring_buffer);
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