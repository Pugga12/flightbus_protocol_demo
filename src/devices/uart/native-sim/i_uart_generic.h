//
// Created by adama on 28/04/25.
//
#pragma once
#include <zephyr/device.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

    void start(const struct device *dev, bool dtrWait);
    ssize_t read(uint8_t *buffer, size_t len, const struct device *dev);
    ssize_t write(const uint8_t *data, size_t len, const struct device *dev);
    size_t getAvailableRXBufferSize();
    void shutdown(const struct device *dev);

#ifdef __cplusplus
}
#endif