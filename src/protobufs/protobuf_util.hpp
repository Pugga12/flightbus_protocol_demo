//
// Created by adama on 3/25/25.
//
#pragma once

#include <sys/types.h>
#include "src/protobufs/flightbus_message.pb.h"
#ifdef __cplusplus
extern "C" {
#endif
    ssize_t encode_message(FB_MESSAGE_P2P *msg, uint8_t *buffer, size_t buffer_size);
    bool decode_message(FB_MESSAGE_P2P *msg, const uint8_t *buffer, const size_t buffer_size);
    ssize_t validateAndStripHeader(const uint8_t *inputBuffer, uint8_t *outputBuffer, size_t buffer_size);
#ifdef __cplusplus
}
#endif
