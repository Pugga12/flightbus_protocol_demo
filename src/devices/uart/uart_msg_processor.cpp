//
// Created by adama on 3/30/25.
//
#include "uart_msg_processor.hpp"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "protobufs/protobuf_util.hpp"
#include <zephyr/device.h>

LOG_MODULE_REGISTER(flightbus_data_processor);
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
static struct k_mutex uart_work_mutex;
struct k_thread preprocessor_thread;

// work queue handler
static void process_uart_work(struct k_work *work_item) {
    auto* uart_work_container = CONTAINER_OF(work_item, struct uart_work, work);
    FB_MESSAGE_P2P msg = FB_MESSAGE_P2P_init_zero;
    decode_message(&msg, uart_work_container->message_input_buffer, uart_work_container->messageSize);
}

// handles the actual logic of preprocessing uart messages
static void preprocess_uart_message(UartBase *uartWrapper) {
    while (true) {
        k_sleep(K_MSEC(500));
        if (uartWrapper->getAvailableRXBufferSize() == 0) {
            continue;
        }
        uint8_t input_buffer[CONFIG_FLIGHTBUS_UART_MAX_MSG_SIZE];
        const ssize_t read_size = uartWrapper->read(input_buffer, CONFIG_FLIGHTBUS_UART_MAX_MSG_SIZE);
        if (read_size >= 3) {
            k_mutex_lock(&uart_work_mutex, K_FOREVER);
            uint8_t message_length = input_buffer[2];
            uint8_t strippedBuffer[read_size];
            ssize_t strippedBufferLen = validateAndStripHeader(input_buffer, strippedBuffer, read_size);
            if (strippedBufferLen <= 0) {
                k_mutex_unlock(&uart_work_mutex);
                continue;
            }
            auto* work_item = new struct uart_work;
            k_work_init(&work_item->work, process_uart_work);
            memcpy(work_item->message_input_buffer, strippedBuffer, strippedBufferLen);
            work_item->messageSize = strippedBufferLen;
            k_work_submit_to_queue(&uart_work_q, &work_item->work);
            k_mutex_unlock(&uart_work_mutex);
        }
    }
}

// the thread entry point for the uart preprocessor
// also performs graceful termination of the preprocessor and work queue
static void preprocessor_entry_point(void *dev, void *, void *) {
    auto* uartWrapper = static_cast<UartBase*>(dev);
    preprocess_uart_message(uartWrapper);
}

// thread setup
void start_uart_pre_processor(UartBase* uart) {
    LOG_INF("Starting data processor on uart device '%s'", uart->getDeviceName());
    k_work_queue_init(&uart_work_q);
    k_work_queue_start(
        &uart_work_q,
        work_stack_area,
        K_THREAD_STACK_SIZEOF(work_stack_area),
        WORK_QUEUE_THREAD_PRIORITY,
        NULL
    );
    auto p1 = static_cast<void *>(uart);
    k_mutex_init(&uart_work_mutex);
    k_thread_create(
        &preprocessor_thread,
        uart_preprocessor_stack_area,
        K_THREAD_STACK_SIZEOF(uart_preprocessor_stack_area),
        preprocessor_entry_point,
        p1,
        NULL,
        NULL,
        PREPROCESSOR_THREAD_PRIORITY,
        0,
        K_MSEC(250)
    );
}