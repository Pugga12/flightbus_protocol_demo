//
// Created by adama on 3/23/25.
//
#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/device.h>
#include "devices/uart/uart_msg_processor.hpp"
#include <devices/uart/cdc-acm/UartCdcAcm.hpp>

int main(void) {
    const struct device *uart_dev = nullptr;
    #ifdef CONFIG_FLIGHTBUS_CDC_ACM
    uart_dev = DEVICE_DT_GET_ONE(zephyr_cdc_acm_uart);
    static UartCdcAcm uart_wrapper(uart_dev, false);
    uart_wrapper.start();
    start_uart_pre_processor(&uart_wrapper);
    #endif
    return 0;
}
