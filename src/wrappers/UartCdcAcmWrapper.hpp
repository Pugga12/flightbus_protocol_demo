//
// Created by adama on 03/04/25.
//

#pragma once
#include <sys/types.h>
#include <zephyr/device.h>

class UartCdcAcmWrapper {
    const struct device *device = nullptr;
    bool deviceReady = false;
public:
    const char* deviceName = "UART CDC ACM (default)";
    bool dtrWait = false;
    explicit UartCdcAcmWrapper(const struct device *device, bool dtrWait);
    ssize_t read(uint8_t *buffer, size_t len) const;
    ssize_t write(const uint8_t *data, size_t len) const;
    static size_t getAvailableRXBufferSize();
    bool retryDeviceStartup() const;
};
