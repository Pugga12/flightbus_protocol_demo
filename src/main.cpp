//
// Created by adama on 3/23/25.
//
#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/device.h>
const device *const uart_dev = DEVICE_DT_GET_ONE(zephyr_cdc_acm_uart);
extern "C" {
    int start_cdc_acm(const device *dev);
    uint8_t* read(size_t len, const device *dev);
    void write(const uint8_t *data, size_t len, const device *dev);
};

int main(void) {
    int ret;

    if (!device_is_ready(uart_dev)) {
        printk("UART init failed.");
        return -1;
    }

    ret = start_cdc_acm(uart_dev);
    if (ret != 0) {
        printk("Failed to start CDC ACM interface.");
        return -1;
    }

    return 0;
}
