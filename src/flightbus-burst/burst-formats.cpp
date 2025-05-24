//
// Created by adama on 5/22/25.
//

#include "burst-formats.hpp"

AutopilotMode::AutopilotMode(uint16_t bits) {
    headingHold         = ((bits >> 0)  & 1) != 0;
    LNAV                = ((bits >> 1)  & 1) != 0;
    VNAV                = ((bits >> 2)  & 1) != 0;
    speedHold           = ((bits >> 3)  & 1) != 0;
    localizerMode       = ((bits >> 4)  & 1) != 0;
    approachMode        = ((bits >> 5)  & 1) != 0;
    flightDirector      = ((bits >> 6)  & 1) != 0;
    altitudeHold        = ((bits >> 7)  & 1) != 0;
    levelChangeMode     = ((bits >> 8)  & 1) != 0;
    verticalSpeedHold   = ((bits >> 9)  & 1) != 0;
    n1Hold              = ((bits >> 10) & 1) != 0;
    autothrottleEnabled = ((bits >> 11) & 1) != 0;
}
