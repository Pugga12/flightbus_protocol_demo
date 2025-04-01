//
// Created by adama on 3/23/25.
//
#ifndef UART_CDC_ACM_H
#define UART_CDC_ACM_H
#include <zephyr/device.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

    int start_cdc_acm(const struct device *dev);
    ssize_t read(uint8_t *buffer, size_t len, const struct device *dev);
    ssize_t write(const uint8_t *data, size_t len, const struct device *dev);
    size_t getAvailableRXBufferSize();

#ifdef __cplusplus
}
#endif

#endif //UART_CDC_ACM_H
