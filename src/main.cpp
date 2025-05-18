//
// Created by adama on 3/23/25.
//
#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/device.h>
#include "devices/uart/uart_msg_processor.hpp"
#ifdef CONFIG_FLIGHTBUS_CDC_ACM
#include <devices/uart/cdc-acm/UartCdcAcm.hpp>
#elif defined(CONFIG_FLIGHTBUS_NATIVE_UART)
#include <devices/uart/native-sim/UartGeneric.hpp>
#endif

int main(void) {
    const struct device *uart_dev = nullptr;
    static UartBase *uart_wrapper = nullptr;

    #ifdef CONFIG_FLIGHTBUS_CDC_ACM
    uart_dev = DEVICE_DT_GET_ONE(zephyr_cdc_acm_uart);
    static UartCdcAcm uart_cdc_acm(uart_dev, false);
    uart_cdc_acm.start();
    uart_wrapper = &uart_cdc_acm;
    #elif defined(CONFIG_FLIGHTBUS_NATIVE_UART)
    uart_dev = DEVICE_DT_GET(DT_NODELABEL(uart1));
    static UartGeneric uart_generic(uart_dev, false);
    uart_generic.start();
    uart_wrapper = &uart_generic;
    #endif

    if (uart_wrapper) {
        start_uart_pre_processor(uart_wrapper);
    }

    return 0;
}
