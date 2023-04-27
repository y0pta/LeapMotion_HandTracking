//
// Created by Liza on 4/27/2023.
//

#include "Utils.h"

std::string toStr(eLeapDeviceStatus status)
{
    std::string str;
    const char* string_view[] = {
            stringify(eLeapDeviceStatus_Streaming),
            stringify(eLeapDeviceStatus_Paused),
            stringify(eLeapDeviceStatus_Robust),
            stringify(eLeapDeviceStatus_Smudged),
            stringify(eLeapDeviceStatus_LowResource),
            stringify(eLeapDeviceStatus_UnknownFailure),
            stringify(eLeapDeviceStatus_BadCalibration),
            stringify(eLeapDeviceStatus_BadFirmware),
            stringify(eLeapDeviceStatus_BadTransport),
            stringify(eLeapDeviceStatus_BadControl),
    };
    return string_view[status];
}