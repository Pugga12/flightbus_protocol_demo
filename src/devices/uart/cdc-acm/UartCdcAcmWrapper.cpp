//
// Created by adama on 03/04/25.
//
#include "UartCdcAcmWrapper.hpp"
#include <zephyr/logging/log.h>
#include "uart_cdc_acm.h"
LOG_MODULE_REGISTER(uart_cdc_acm_wrapper);

// constructs a new CDC ACM Wrapper
UartCdcAcmWrapper::UartCdcAcmWrapper(const struct device *device, const bool dtrWait){
    this->mDtrWait = dtrWait;
    this->mDeviceType = "UART_CDC_ACM";
    this->mDevice = device;
    this->mDeviceName = this->mDevice->name;
}

// reads a specified number of bytes from the device and fills the passed in buffer
// returns -EIO if device not ready, number of bytes read if successful
ssize_t UartCdcAcmWrapper::read(uint8_t *buffer, const size_t len) const {
    if (mDeviceReady) {
        return ::read(buffer, len, mDevice);
    }
    LOG_ERR("Attemped to read from device %s, which is currently not ready", mDeviceName);
    return -EIO;
}

// writes a specified number of bytes from the passed in data buffer to the device
// returns -EIO if device not ready, number of bytes read if successful
ssize_t UartCdcAcmWrapper::write(const uint8_t *data, const size_t len) const {
    if (mDeviceReady) {
        return ::write(data, len, mDevice);
    }
    LOG_ERR("Attemped to write to device %s, which is currently not ready", mDeviceName);
    return -EIO;
}

// returns the number of bytes available to read from the device
// returns -EIO if device not ready or number of bytes if successful
ssize_t UartCdcAcmWrapper::getAvailableRXBufferSize() {
    if (mDeviceReady) {
        return static_cast<ssize_t>(::getAvailableRXBufferSize());
    }
    LOG_ERR("Attemped to get available RX buffer size from device %s, which is currently not ready", mDeviceName);
    return -EIO;
}

// start the device
bool UartCdcAcmWrapper::start() {
    LOG_INF("Attempting to start device '%s' of device type '%s'", mDeviceName, mDeviceType);
    bool deviceAvail = device_is_ready(mDevice);
    if (!deviceAvail) {
        LOG_ERR("Device '%s' not ready", mDeviceName);
        return false;
    }
    deviceAvail = ::start(this->mDevice, this->mDtrWait);
    if (!deviceAvail) {
        LOG_ERR("Failed to start UART CDC ACM device '%s'", mDeviceName);
        return false;
    }
    LOG_INF("'%s' device '%s' ready", mDeviceType, mDeviceName);
    mDeviceReady = true;
    return true;
}

UartCdcAcmWrapper::~UartCdcAcmWrapper() {
    LOG_INF("Attempting to stop device '%s'", mDeviceName);
    UartCdcAcmWrapper::stop();
};

// stop the device (currently does nothing)
bool UartCdcAcmWrapper::stop() {
    return ::shutdown(mDevice);
}

// get device name
const char * UartCdcAcmWrapper::getDeviceName() const {
    return mDeviceName;
}
