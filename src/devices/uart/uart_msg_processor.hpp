//
// Created by adama on 3/30/25.
//
#pragma once
#include "cdc-acm/UartCdcAcmWrapper.hpp"
#ifdef __cplusplus
    extern "C" {
#endif
        void start_uart_pre_processor(UartCdcAcmWrapper* uart);
#ifdef __cplusplus
        }
#endif
