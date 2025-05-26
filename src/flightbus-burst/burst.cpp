//
// Created by adama on 5/25/25.
//
#include "burst.hpp"
#include "burst-formats.hpp"
#include "devices/uart/native-sim/i_uart_generic.h"
#include "zephyr/kernel.h"
#include "zephyr/logging/log.h"
LOG_MODULE_REGISTER(flightbus_burst);
const static uint8_t MAGIC_BYTES[5] = {0x0A, 0x14, 0x46, 0x42, 0x42};

uint16_t readUint16LE(const uint8_t lsb, const uint8_t msb) {
    return (static_cast<uint16_t>(msb) << 8) | static_cast<uint16_t>(lsb);
}

static bool checkMagic(const uint8_t* data) {
    return memcmp(data, MAGIC_BYTES, sizeof(MAGIC_BYTES)) == 0;
}


static bool decodeTOCEntries(const uint8_t *data, FbStdTOCEntry *entries, uint8_t amount) {
    for (uint8_t i = 0; i < amount; i++) {
        const uint8_t *entry_ptr = data + (i * sizeof(FbStdTOCEntry));
        FbStdTOCEntry entry;
        entry.streamId = entry_ptr[0];
        entry.streamType = static_cast<PredefinedStreamType>(entry_ptr[1]);
        entry.offset = readUint16LE(entry_ptr[2], entry_ptr[3]);
        entry.length = readUint16LE(entry_ptr[4], entry_ptr[5]);
        entry.crc16 = readUint16LE(entry_ptr[6], entry_ptr[7]);
        entries[i] = entry;
    }
}

bool decodeHeader(const uint8_t *data, FbHeader *header, uint8_t length) {
    uint8_t magic[4];
    memcpy(magic, data, 4);
    header->busId = data[4];
    header->senderId = data[5];
    header->receiverId = data[6];
    const auto packetType = static_cast<PacketType>(data[7]);
    if (packetType <= REQUEST_USER_SIMVAR) {
        header->type = packetType;
    } else {
        LOG_ERR("Invalid packet type");
        return false;
    }
    header->amountOfStreams = data[8];
    uint8_t tocLength = header->amountOfStreams * sizeof(FbStdTOCEntry);
    uint8_t remainingLength = length - 8;
    if (remainingLength < tocLength) {
        LOG_ERR("TOC length invalid");
        return false;
    }
    auto *tocEntries = static_cast<FbStdTOCEntry *>(k_malloc(header->amountOfStreams * sizeof(FbStdTOCEntry)));
    if (!tocEntries) {
        LOG_ERR("Failed to allocate memory for TOC entries");
    }
    decodeTOCEntries(data + 9, tocEntries, header->amountOfStreams);
    memcpy(&header->tableOfContents, tocEntries, header->amountOfStreams * sizeof(FbStdTOCEntry));
    k_free(tocEntries);
    return true;
}
