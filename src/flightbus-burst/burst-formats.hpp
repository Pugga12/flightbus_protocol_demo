//
// Created by adama on 5/22/25.
//
#pragma once
#include <sys/types.h>
#include <vector>

enum PacketType {
    STREAM_BURST = 0,
    REQUEST_STREAM = 1,
    REQUEST_USER_SIMVAR = 2,
};

enum PredefinedStreamType {
    NAVIGATION = 0,
    CONTROL_SURFACES = 1,
    USER_SIMVARS = 2
};

struct FbStdTOCEntry{
    uint8_t streamId;
    PredefinedStreamType streamType;
    uint16_t offset;
    uint16_t length;
    uint16_t crc16;
};

struct FbHeader {
    uint8_t busId;
    uint8_t senderId;
    uint8_t receiverId;
    PacketType type;
    uint8_t amountOfStreams;
    uint8_t systemReserved = 0;
    std::vector<FbStdTOCEntry> streams;
};

struct AutopilotMode {
    bool headingHold = false;
    bool LNAV = false;
    bool VNAV = false;
    bool speedHold = false;
    bool localizerMode = false;
    bool approachMode = false;
    bool flightDirector = false;
    bool altitudeHold = false;
    bool levelChangeMode = false;
    bool verticalSpeedHold = false;
    bool n1Hold = false;
    bool autothrottleEnabled = false;

    AutopilotMode() = default;
    // Constructor: Unpacks from uint16_t
    explicit AutopilotMode(uint16_t bits);
};

struct FbNavigationStream{
    uint8_t featureFlags;
    float latitude;
    float longitude;
    int16_t altitude;
    AutopilotMode apMode;
    uint16_t currentHdg;
    uint16_t selectedHdg;
    uint16_t currentIAS;
    uint16_t selectedIAS;
    int16_t currentVS;
    int16_t selectedVS;
};

struct FbControlSurfacesStream {
    uint8_t featureFlags;
    bool spoilersArmed;
    int16_t spoilerPos;
    uint16_t spoilerDeflectionR;
    uint16_t spoilerDeflectionL;
    int16_t rudderPos;
    float rudderTrim;
    int16_t aileronPos;
    float aileronTrim;
    int16_t elevatorPos;
    float elevatorTrim;
};