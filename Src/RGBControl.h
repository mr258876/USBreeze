#ifndef _RGB_CONTROL_H
#define _RGB_CONTROL_H

#include <stdint.h>

#define RGB_LAMP_COUNT              1
#define RGB_LAMPARRAY_KIND          7       // 07 -> LampArrayKindChassis. Referer: Page 330, https://www.usb.org/sites/default/files/hut1_4.pdf
#define RGB_MIN_UPDATE_INTERVAL     50000   // In Microseconds

enum LampPurposeKind
{
    LampPurposeControl = 1,
    LampPurposeAccent = 2,
    LampPurposeBranding = 4,
    LampPurposeStatus = 8,
    LampPurposeIllumination = 16,
    LampPurposePresentation = 32,
};

typedef __packed struct
{
    uint8_t RedChannel;
    uint8_t GreenChannel;
    uint8_t BlueChannel;
    uint8_t IntensityChannel;
} LampArrayColor;

typedef __packed struct
{
    uint16_t LampId;
    uint32_t PositionXInMicrometers;
    uint32_t PositionYInMicrometers;
    uint32_t PositionZInMicrometers;
    uint32_t UpdateLatencyInMicroseconds;
    uint32_t LampPurposes;
    uint8_t RedLevelCount;
    uint8_t GreenLevelCount;
    uint8_t BlueLevelCount;
    uint8_t IntensityLevelCount;
    uint8_t IsProgrammable;
    uint8_t LampKey;
} LampAttributes;

extern const LampAttributes RGB_Lamp_Attributes[]; // ID count MUST match the size of RGB_LAMP_COUNT

#endif
