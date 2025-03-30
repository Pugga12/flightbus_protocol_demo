//
// Created by adama on 3/25/25.
//

#include "protobuf_util.h"
#include <pb_encode.h>
#include <pb_decode.h>
#include <zephyr/logging/log.h>
#include "src/interfaces/protobufs/flightbus_message.pb.h"
#include <sys/types.h>
#include <zephyr/sys/crc.h>
LOG_MODULE_REGISTER(protobuf_util);

static uint16_t convertU8toU16(uint8_t lsb, uint8_t msb) {
    return (static_cast<uint16_t>(msb) << 8) | static_cast<uint16_t>(lsb);
}

static ssize_t validateAndStripHeader(uint8_t *inputBuffer, uint8_t *outputBuffer, size_t buffer_size) {
    if (!inputBuffer) {
        LOG_ERR("Invalid input buffer");
        return -EINVAL;
    }
    if (!outputBuffer) {
        LOG_ERR("Invalid output buffer");
        return -EINVAL;
    }

    const size_t pktLength = inputBuffer[0];
    if (pktLength != buffer_size - 3) {
        LOG_ERR("Invalid message size");
        return -EINVAL;
    }
    const uint16_t crc16 = convertU8toU16(inputBuffer[1], inputBuffer[2]);
    memcpy(outputBuffer, inputBuffer + 3, buffer_size - 3);
    const uint16_t calculatedCrc = crc16_ccitt(0x0000, outputBuffer, pktLength);
    if (calculatedCrc != crc16) {
        LOG_ERR("CRC Error!");
        return -EBADMSG;
    }
    return static_cast<ssize_t>(pktLength);
}

ssize_t encode_message(FB_MESSAGE_P2P *msg, uint8_t *buffer, size_t buffer_size) {
    bool status;
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, buffer_size);
    status = pb_encode(&stream, FB_MESSAGE_P2P_fields, &msg);
    if (!status) {
        LOG_ERR("Encoding failed: %s", PB_GET_ERROR(&stream));
        return -1;
    }

    return static_cast<ssize_t>(stream.bytes_written);
}

bool decode_message(FB_MESSAGE_P2P *msg, const uint8_t *buffer, size_t buffer_size) {

}