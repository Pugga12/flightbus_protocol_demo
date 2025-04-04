//
// Created by adama on 3/30/25.
//
#pragma once
#include <wrappers/UartCdcAcmWrapper.hpp>
#include <zephyr/kernel.h>
#ifdef __cplusplus
    extern "C" {
#endif
        void start_uart_pre_processor(UartCdcAcmWrapper* uart);
#ifdef __cplusplus
        }
#endif
