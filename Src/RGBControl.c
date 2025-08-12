#include "RGBControl.h"

extern const LampAttributes RGB_Lamp_Attributes[] =
    {
        {
            0x00,              // Lamp ID 0
            1,                 // PositionXInMicrometers
            1,                 // PositionYInMicrometers
            1,                 // PositionZInMicrometers
            4000,              // UpdateLatencyInMicroseconds
            LampPurposeAccent, // Lamp purpose: bit5 -> LampPurposePresentation, bit4 -> LampPurposeIllumination, bit3 -> LampPurposeStatus (Unread msg etc.),
                               //     bit2 - > LampPurposeBranding (Logo etc.), bit1 -> LampPurposeAccent (CaseFan, etc.), bit0 -> LampPurposeControl (Keys on board, etc.)
            0xFF,              // RedLevelCount
            0xFF,              // GreenLevelCount
            0xFF,              // BlueLevelCount
            1,                 // IntensityLevelCount, just set to 1 if at least 2 channels are adjustable
            1,                 // IsProgrammable
            0,                 // InputBinding
        },
};
