//
// Created by adama on 3/30/25.
//

#ifndef UART_MSG_PREPROCESSOR_H
#define UART_MSG_PREPROCESSOR_H
#include <zephyr/kernel.h>
#ifdef __cplusplus
    extern "C" {
#endif
        void start_uart_pre_processor(struct device *dev);
#ifdef __cplusplus
        }
#endif
#endif //UART_MSG_PREPROCESSOR_H
