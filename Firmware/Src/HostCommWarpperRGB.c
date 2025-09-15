/*
 * Copyright (c) 2025 mr258876
 * SPDX-License-Identifier: MIT
 */

#include "HostCommWarpper.h"

#include "RGBControl.h"
#include <string.h>
#include "rl_usb.h"
#include "cmsis_os.h"

#define LAMP_NOT_PROGRAMMABLE 0x00
#define LAMP_IS_PROGRAMMABLE 0x01

enum LampArrayKind
{
    LampArrayKindKeyboard = 1,
    LampArrayKindMouse = 2,
    LampArrayKindGameController = 3,
    LampArrayKindPeripheral = 4,
    LampArrayKindScene = 5,
    LampArrayKindNotification = 6,
    LampArrayKindChassis = 7,
    LampArrayKindWearable = 8,
    LampArrayKindFurniture = 9,
    LampArrayKindArt = 10,
};

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

typedef __packed struct
{
    uint16_t LampCount;
    uint32_t BoundingBoxWidthInMicrometers;
    uint32_t BoundingBoxHeightInMicrometers;
    uint32_t BoundingBoxDepthInMicrometers;
    uint32_t LampArrayKind;
    uint32_t MinUpdateIntervalInMicroseconds;
} LampArrayAttributesReport;

typedef __packed struct
{
    uint16_t LampId;
} LampAttributesRequestReport;

typedef __packed struct
{
    LampAttributes Attributes;
} LampAttributesResponseReport;

typedef __packed struct
{
    uint8_t LampCount;
    uint8_t LampUpdateFlags;
    uint16_t LampIds[RGB_LAMP_MULTI_UPDATE_LAMP_COUNT];
    LampArrayColor UpdateColors[RGB_LAMP_MULTI_UPDATE_LAMP_COUNT];
} LampMultiUpdateReport;

typedef __packed struct
{
    uint8_t LampUpdateFlags;
    uint16_t LampIdStart;
    uint16_t LampIdEnd;
    LampArrayColor UpdateColor;
} LampRangeUpdateReport;

typedef __packed struct
{
    uint8_t AutonomousMode;
} LampArrayControlReport;

static const LampAttributes RGB_Lamp_Attributes_Template = {
    0x00,                      // Lamp ID 0
    1,                         // PositionXInMicrometers
    1,                         // PositionYInMicrometers
    1,                         // PositionZInMicrometers
    RGB_LAMP_TOTAL_COUNT * 36, // UpdateLatencyInMicroseconds <- assume 1.5us for 1bit in WS281X
    LampPurposeAccent,         // Lamp purpose: bit5 -> LampPurposePresentation, bit4 -> LampPurposeIllumination, bit3 -> LampPurposeStatus (Unread msg etc.),
                               //     bit2 - > LampPurposeBranding (Logo etc.), bit1 -> LampPurposeAccent (CaseFan, etc.), bit0 -> LampPurposeControl (Keys on board, etc.)
    0xFF,                      // RedLevelCount
    0xFF,                      // GreenLevelCount
    0xFF,                      // BlueLevelCount
    1,                         // IntensityLevelCount, just set to 1 if at least 2 channels are adjustable
    1,                         // IsProgrammable <- No command will be sent if set to 0. LMAO XD
    0,                         // InputBinding
};
static const uint16_t RGB_Lamp_Count_By_Instances[] = {RGB_LAMP_INSTANCE_0_LAMP_COUNT, RGB_LAMP_INSTANCE_1_LAMP_COUNT, RGB_LAMP_INSTANCE_2_LAMP_COUNT};
static uint16_t RGB_Attributes_Request_Report_Lamp_ID[] = {0, 0, 0};

static inline uint16_t RGB_Hid_Instance_Get_Lamp_Paddings(uint8_t instance)
{
    if (instance >= RGB_LAMP_INSTANCES_COUNT || instance >= RGB_CONTROL_HID_CHANNELS_COUNT) return 0;

    return RGB_Hid_Channel_Lamp_Map[instance][0];
}

int32_t RGB_Control_Get_Attr_Report(uint8_t instance, uint8_t *buf)
{
    LampArrayAttributesReport *data = (LampArrayAttributesReport *)buf;

    data->LampCount = RGB_Lamp_Count_By_Instances[instance];
#if RGB_CUSTOM_LAMP_POSITIONS
    data->BoundingBoxWidthInMicrometers = RGB_BOUNDING_BOX_WIDTH_X;
    data->BoundingBoxHeightInMicrometers = RGB_BOUNDING_BOX_HEIGHT_Z;
    data->BoundingBoxDepthInMicrometers = RGB_BOUNDING_BOX_DEPTH_Y;
#else
    data->BoundingBoxWidthInMicrometers = RGB_Lamp_Count_By_Instances[instance] * 1000;
    data->BoundingBoxHeightInMicrometers = 1000;
    data->BoundingBoxDepthInMicrometers = 1000;
#endif
    data->LampArrayKind = LampArrayKindChassis;
    // data->LampArrayKind = LampArrayKindPeripheral;
    data->MinUpdateIntervalInMicroseconds = RGB_MIN_UPDATE_INTERVAL;

    return sizeof(LampArrayAttributesReport);
}

int32_t RGB_Control_Get_Attributes_Response(uint8_t instance, uint8_t *buf)
{
    /*
        Send attributes of lamp ID #n.
        Referer: Page 337, https://www.usb.org/sites/default/files/hut1_4.pdf
    */

    if (RGB_Attributes_Request_Report_Lamp_ID[instance] >= RGB_Lamp_Count_By_Instances[instance])
    {
        // Make sure we are not reading sth else
        RGB_Attributes_Request_Report_Lamp_ID[instance] = RGB_Lamp_Count_By_Instances[instance] - 1;
    }

    memcpy(buf, &RGB_Lamp_Attributes_Template, sizeof(LampAttributes));

    LampAttributes *_buf = (LampAttributes *)buf;
    _buf->LampId = RGB_Attributes_Request_Report_Lamp_ID[instance];
#if RGB_CUSTOM_LAMP_POSITIONS
    _buf->PositionXInMicrometers = RGB_Lamp_Positions[RGB_Hid_Instance_Get_Lamp_Paddings(instance) + RGB_Attributes_Request_Report_Lamp_ID[instance]][0] * 1000;
    _buf->PositionYInMicrometers = RGB_Lamp_Positions[RGB_Hid_Instance_Get_Lamp_Paddings(instance) + RGB_Attributes_Request_Report_Lamp_ID[instance]][1] * 1000;
    _buf->PositionZInMicrometers = RGB_Lamp_Positions[RGB_Hid_Instance_Get_Lamp_Paddings(instance) + RGB_Attributes_Request_Report_Lamp_ID[instance]][2] * 1000;
#else
    _buf->PositionXInMicrometers = RGB_Attributes_Request_Report_Lamp_ID[instance] * 1000;
    _buf->PositionYInMicrometers = 1000;
    _buf->PositionZInMicrometers = 1000;
#endif

    RGB_Attributes_Request_Report_Lamp_ID[instance]++;
    if (RGB_Attributes_Request_Report_Lamp_ID[instance] >= RGB_Lamp_Count_By_Instances[instance])
    {
        // Reached the end of lamps, return to 0 and next response will be the last response.
        RGB_Attributes_Request_Report_Lamp_ID[instance] = 0;
    }

    return sizeof(LampAttributesResponseReport);
}

bool RGB_Control_Set_Attr_Request_Lamp_ID(uint8_t instance, const uint8_t *buf, int32_t len)
{
    if (len != sizeof(LampAttributesRequestReport))
        return false;

    RGB_Attributes_Request_Report_Lamp_ID[instance] = ((LampAttributesRequestReport *)buf)->LampId;

    if (RGB_Attributes_Request_Report_Lamp_ID[instance] >= RGB_Lamp_Count_By_Instances[instance])
    {
        RGB_Attributes_Request_Report_Lamp_ID[instance] = RGB_Lamp_Count_By_Instances[instance] - 1;
    }

    return true;
}

bool RGB_Control_Set_Multi_Update(uint8_t instance, const uint8_t *buf, int32_t len)
{
    if (len != sizeof(LampMultiUpdateReport))
        return false;

    LampMultiUpdateReport *_buf = (LampMultiUpdateReport *)buf;
    if (_buf->LampCount > RGB_LAMP_MULTI_UPDATE_LAMP_COUNT)
        return false;

    for (int i = 0; i < _buf->LampCount; i++)
    {
        if (_buf->LampIds[i] >= RGB_Lamp_Count_By_Instances[instance])
            return false;

        RGB_Lamp_Colors[RGB_Hid_Instance_Get_Lamp_Paddings(instance) + _buf->LampIds[i] * 3] = _buf->UpdateColors[i].RedChannel;
        RGB_Lamp_Colors[RGB_Hid_Instance_Get_Lamp_Paddings(instance) + _buf->LampIds[i] * 3 + 1] = _buf->UpdateColors[i].GreenChannel;
        RGB_Lamp_Colors[RGB_Hid_Instance_Get_Lamp_Paddings(instance) + _buf->LampIds[i] * 3 + 2] = _buf->UpdateColors[i].BlueChannel;
    }

    if (_buf->LampUpdateFlags & 1)
    {
        osMessagePut(RGB_Update_Msg_Queue, 1, 0);
    }

    return true;
}

bool RGB_Control_Set_Range_Update(uint8_t instance, const uint8_t *buf, int32_t len)
{
    if (len != sizeof(LampRangeUpdateReport))
        return false;

    LampRangeUpdateReport *_buf = (LampRangeUpdateReport *)buf;

    if (_buf->LampIdStart > _buf->LampIdEnd)
        return false;
    if (_buf->LampIdStart > RGB_Lamp_Count_By_Instances[instance] || _buf->LampIdStart >= RGB_Lamp_Count_By_Instances[instance])
        return false;

    for (int i = _buf->LampIdStart; i <= _buf->LampIdEnd; i++)
    {
        RGB_Lamp_Colors[RGB_Hid_Instance_Get_Lamp_Paddings(instance) + i * 3] = _buf->UpdateColor.RedChannel;
        RGB_Lamp_Colors[RGB_Hid_Instance_Get_Lamp_Paddings(instance) + i * 3 + 1] = _buf->UpdateColor.GreenChannel;
        RGB_Lamp_Colors[RGB_Hid_Instance_Get_Lamp_Paddings(instance) + i * 3 + 2] = _buf->UpdateColor.BlueChannel;
    }

    if (_buf->LampUpdateFlags & 1)
    {
        osMessagePut(RGB_Update_Msg_Queue, 1, 0);
    }

    return true;
}

bool RGB_Control_Set_Control_Mode(uint8_t instance, const uint8_t *buf, int32_t len)
{
    if (len != sizeof(LampArrayControlReport))
        return false;

    LampArrayControlReport *_buf = (LampArrayControlReport *)buf;
    RGB_Control_Set_Autonomous_Mode(instance, _buf->AutonomousMode);

    return true;
}
