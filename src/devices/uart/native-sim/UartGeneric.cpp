//
// Created by adama on 03/04/25.
//
#include "UartGeneric.hpp"
#include <zephyr/logging/log.h>
#include "i_uart_generic.h"
LOG_MODULE_REGISTER(uart_generic_impl);

// constructs a new CDC ACM Wrapper
UartGeneric::UartGeneric(const struct device *device, const bool dtrWait){
    this->mDtrWait = dtrWait;
    this->mDeviceType = "Generic UART Device";
    this->mDevice = device;
    this->mDeviceName = this->mDevice->name;
}

// reads a specified number of bytes from the device and fills the passed in buffer
// returns -EIO if device not ready, number of bytes read if successful
ssize_t UartGeneric::read(uint8_t *buffer, const size_t len) const {
    if (mDeviceReady) {
        return ::read(buffer, len, mDevice);
    }
    LOG_ERR("Attemped to read from device %s, which is currently not ready", mDeviceName);
    return -EIO;
}

// writes a specified number of bytes from the passed in data buffer to the device
// returns -EIO if device not ready, number of bytes read if successful
ssize_t UartGeneric::write(const uint8_t *data, const size_t len) const {
    if (mDeviceReady) {
        return ::write(data, len, mDevice);
    }
    LOG_ERR("Attemped to write to device %s, which is currently not ready", mDeviceName);
    return -EIO;
}

// returns the number of bytes available to read from the device
// returns -EIO if device not ready or number of bytes if successful
ssize_t UartGeneric::getAvailableRXBufferSize() {
    if (mDeviceReady) {
        return static_cast<ssize_t>(::getAvailableRXBufferSize());
    }
    LOG_ERR("Attemped to get available RX buffer size from device %s, which is currently not ready", mDeviceName);
    return -EIO;
}

// start the device
bool UartGeneric::start() {
    LOG_INF("Attempting to start device '%s' of device type '%s'", mDeviceName, mDeviceType);
    bool deviceAvail = device_is_ready(mDevice);
    if (!deviceAvail) {
        LOG_ERR("Device '%s' not ready", mDeviceName);
        return false;
    }
    ::start(this->mDevice, this->mDtrWait);
    LOG_INF("'%s' device '%s' ready", mDeviceType, mDeviceName);
    mDeviceReady = true;
    return true;
}

UartGeneric::~UartGeneric() {
    LOG_INF("Attempting to stop device '%s'", mDeviceName);
    UartGeneric::stop();
};

// stop the device (currently does nothing)
bool UartGeneric::stop() {
    ::shutdown(mDevice);
    return true;
}

// get device name
const char * UartGeneric::getDeviceName() const {
    return mDeviceName;
}
