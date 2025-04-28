//
// Created by adama on 03/04/25.
//

#pragma once
#include <sys/types.h>
#include <zephyr/device.h>
#include "devices/uart/UartBase.hpp"

class UartCdcAcmWrapper : public UartBase {
    bool dtrWait = false;
    public:
        explicit UartCdcAcmWrapper(const struct device *device, bool dtrWait);
        ssize_t read(uint8_t *buffer, size_t len) const override;
        ssize_t write(const uint8_t *data, size_t len) const override;
        ssize_t getAvailableRXBufferSize() override;
        bool start() override;
        ~UartCdcAcmWrapper() override;
        bool stop() override;
        const char *getDeviceName() const override;
};
