//
// Created by adama on 3/30/25.
//
#include "uart_msg_processor.hpp"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "protobufs/protobuf_util.hpp"
#include "uart_cdc_acm.h"
#include <zephyr/device.h>

LOG_MODULE_DECLARE(flightbus_cdc_acm);
#define UART_THREADS_STACK_SIZE 3072
#define WORK_QUEUE_THREAD_PRIORITY K_PRIO_COOP(8)
#define PREPROCESSOR_THREAD_PRIORITY K_PRIO_COOP(7)

K_THREAD_STACK_DEFINE(work_stack_area, UART_THREADS_STACK_SIZE);
K_THREAD_STACK_DEFINE(uart_preprocessor_stack_area, UART_THREADS_STACK_SIZE);
struct k_work_q uart_work_q;
struct uart_work {
    struct k_work work;
    uint8_t message_input_buffer[CONFIG_FLIGHTBUS_UART_MAX_MSG_SIZE];
    size_t messageSize;
};
static struct uart_work uart_work;
static struct k_mutex uart_work_mutex;
struct k_thread preprocessor_thread;

void process_uart_work(struct k_work *work_item) {
    struct uart_work *uart_work_container = CONTAINER_OF(work_item, struct uart_work, work);
    FB_MESSAGE_P2P msg = FB_MESSAGE_P2P_init_zero;
    decode_message(&msg, uart_work_container->message_input_buffer, uart_work_container->messageSize);
}

void preprocess_uart_message(void *dev, void *, void *) {
    // Cast the device pointer to the correct type
    struct device *device = static_cast<struct device*>(dev);
    while (true) {
        k_sleep(K_MSEC(250));
        if (getAvailableRXBufferSize() == 0) {
            continue;
        }
        uint8_t input_buffer[CONFIG_FLIGHTBUS_UART_MAX_MSG_SIZE];
        const ssize_t msg_len = read(input_buffer, sizeof(input_buffer), device);
        if (msg_len > 3) {
            k_mutex_lock(&uart_work_mutex, K_FOREVER);
            uint8_t strippedBuffer[msg_len];
            ssize_t strippedBufferLen = validateAndStripHeader(input_buffer, strippedBuffer, msg_len);
            if (strippedBufferLen <= 0) {
                continue;
                k_mutex_unlock(&uart_work_mutex);
            }
            memcpy(uart_work.message_input_buffer, strippedBuffer, strippedBufferLen);
            uart_work.messageSize = strippedBufferLen;
            k_work_submit(&uart_work.work);
            k_mutex_unlock(&uart_work_mutex);
        }
    }
}

void start_uart_pre_processor(device *dev) {
    k_work_queue_init(&uart_work_q);
    k_work_queue_start(
        &uart_work_q,
        work_stack_area,
        K_THREAD_STACK_SIZEOF(work_stack_area),
        WORK_QUEUE_THREAD_PRIORITY,
        NULL
    );
    k_work_init(&uart_work.work, process_uart_work);
    k_mutex_init(&uart_work_mutex);
    k_thread_create(
        &preprocessor_thread,
        uart_preprocessor_stack_area,
        K_THREAD_STACK_SIZEOF(uart_preprocessor_stack_area),
        preprocess_uart_message,
        static_cast<void*>(dev),
        NULL,
        NULL,
        PREPROCESSOR_THREAD_PRIORITY,
        0,
        K_MSEC(250)
    );
}