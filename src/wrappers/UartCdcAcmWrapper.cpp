//
// Created by adama on 03/04/25.
//
#include "UartCdcAcmWrapper.hpp"
#include <zephyr/logging/log.h>
#include "../interfaces/uart_cdc_acm.h"
LOG_MODULE_REGISTER(uart_cdc_acm_wrapper);

UartCdcAcmWrapper::UartCdcAcmWrapper(const struct device *device, const bool dtrWait) {
    this->device = device;
    this->dtrWait = dtrWait;
    deviceReady = device_is_ready(device);
    deviceName = this->device->name;
    if (!deviceReady) {
        LOG_ERR("Device '%s' not ready", deviceName);
        return;
    }
    deviceReady = start(this->device, this->dtrWait);
    if (!deviceReady) {
        LOG_ERR("Failed to start UART CDC ACM device '%s'", deviceName);
    }
    LOG_INF("UART CDC ACM device '%s' ready", deviceName);
}

ssize_t UartCdcAcmWrapper::read(uint8_t *buffer, const size_t len) const {
    return ::read(buffer, len, device);
}

ssize_t UartCdcAcmWrapper::write(const uint8_t *data, const size_t len) const {
    return ::write(data, len, device);
}

size_t UartCdcAcmWrapper::getAvailableRXBufferSize() {
    return ::getAvailableRXBufferSize();
}

bool UartCdcAcmWrapper::retryDeviceStartup() const {
    bool retry = false;
    if (deviceReady) {
        LOG_INF("Retrying device startup!");
        retry = start(device, dtrWait);
        if (!retry) {
            LOG_ERR("Retry of device startup %s failed", deviceName);
        }
    } else {
        LOG_ERR("Attempted to start already initialized device %s!", deviceName);
        retry = true;
    }
    return retry;
}
