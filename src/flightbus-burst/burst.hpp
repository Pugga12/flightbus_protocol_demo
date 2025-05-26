//
// Created by adama on 5/25/25.
//
#pragma once
#include <sys/types.h>
#include "burst-formats.hpp"
bool decodeHeader(const uint8_t* data, FbHeader* header);
uint16_t readUint16LE(const uint8_t lsb, const uint8_t msb);