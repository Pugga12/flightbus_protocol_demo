//
// Created by adama on 03/04/25.
//
#include "UartCdcAcmWrapper.hpp"
#include <zephyr/logging/log.h>
#include "uart_cdc_acm.h"
LOG_MODULE_REGISTER(uart_cdc_acm_wrapper);

UartCdcAcmWrapper::UartCdcAcmWrapper(const struct device *device, const bool dtrWait){
    this->dtrWait = dtrWait;
    this->deviceType = "UART_CDC_ACM";
    this->device = device;
    this->deviceName = this->device->name;
}

ssize_t UartCdcAcmWrapper::read(uint8_t *buffer, const size_t len) const {
    if (deviceReady) {
        return ::read(buffer, len, device);
    }
    LOG_ERR("Attemped to read from device %s, which is currently not ready", deviceName);
    return -EIO;
}

ssize_t UartCdcAcmWrapper::write(const uint8_t *data, const size_t len) const {
    if (deviceReady) {
        return ::write(data, len, device);
    }
    LOG_ERR("Attemped to write to device %s, which is currently not ready", deviceName);
    return -EIO;
}

ssize_t UartCdcAcmWrapper::getAvailableRXBufferSize() {
    if (deviceReady) {
        return static_cast<ssize_t>(::getAvailableRXBufferSize());
    }
    LOG_ERR("Attemped to get available RX buffer size from device %s, which is currently not ready", deviceName);
    return -EIO;
}

bool UartCdcAcmWrapper::start() {
    LOG_INF("Attempting to start device '%s' of device type '%s'", deviceName, deviceType);
    bool deviceAvail = device_is_ready(device);
    if (!deviceAvail) {
        LOG_ERR("Device '%s' not ready", deviceName);
        return false;
    }
    deviceAvail = ::start(this->device, this->dtrWait);
    if (!deviceAvail) {
        LOG_ERR("Failed to start UART CDC ACM device '%s'", deviceName);
        return false;
    }
    LOG_INF("'%s' device '%s' ready", deviceType, deviceName);
    deviceReady = true;
    return true;
}

UartCdcAcmWrapper::~UartCdcAcmWrapper() = default;

bool UartCdcAcmWrapper::stop() {
    return true;
}

const char * UartCdcAcmWrapper::getDeviceName() const {
    return deviceName;
}
