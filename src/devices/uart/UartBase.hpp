//
// Created by adama on 28/04/25.
//
#pragma once
#include <cstdint>
#include <sys/types.h>

class UartBase {
    public:
        virtual ~UartBase() = default; // <-- Make destructor virtual for inheritance safety

        virtual bool start() = 0;
        virtual bool stop() = 0;
        virtual const char* getDeviceName() const = 0;
        virtual ssize_t read(uint8_t *buffer, size_t len) const = 0;
        virtual ssize_t write(const uint8_t *data, size_t len) const = 0;
        virtual ssize_t getAvailableRXBufferSize() = 0;

protected:
    const struct device *mDevice = nullptr;
    bool mDeviceReady = false;
    const char* mDeviceType = "Default UART Device";
    const char* mDeviceName = "Default UART Device";
};
