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

static uint16_t RGB_Attributes_Request_Report_Lamp_ID = 0;
static const LampAttributes RGB_Lamp_Attributes_Template = {
    0x00,              // Lamp ID 0
    1,                 // PositionXInMicrometers
    1,                 // PositionYInMicrometers
    1,                 // PositionZInMicrometers
    4,                 // UpdateLatencyInMicroseconds
    LampPurposeAccent, // Lamp purpose: bit5 -> LampPurposePresentation, bit4 -> LampPurposeIllumination, bit3 -> LampPurposeStatus (Unread msg etc.),
                       //     bit2 - > LampPurposeBranding (Logo etc.), bit1 -> LampPurposeAccent (CaseFan, etc.), bit0 -> LampPurposeControl (Keys on board, etc.)
    0xFF,              // RedLevelCount
    0xFF,              // GreenLevelCount
    0xFF,              // BlueLevelCount
    1,                 // IntensityLevelCount, just set to 1 if at least 2 channels are adjustable
    1,                 // IsProgrammable <- No command will be sent if set to 0. LMAO XD
    0,                 // InputBinding
};

int32_t RGB_Control_Get_Attr_Report(uint8_t *buf)
{
    LampArrayAttributesReport *data = (LampArrayAttributesReport *)buf;

    data->LampCount = RGB_LAMP_COUNT;
    data->BoundingBoxWidthInMicrometers = RGB_BOUNDING_BOX_WIDTH_X * 1000;
    data->BoundingBoxHeightInMicrometers = RGB_BOUNDING_BOX_HEIGHT_Z * 1000;
    data->BoundingBoxDepthInMicrometers = RGB_BOUNDING_BOX_DEPTH_Y * 1000;
    data->LampArrayKind = LampArrayKindChassis;
    // data->LampArrayKind = LampArrayKindPeripheral;
    data->MinUpdateIntervalInMicroseconds = RGB_MIN_UPDATE_INTERVAL;

    return sizeof(LampArrayAttributesReport);
}

int32_t RGB_Control_Get_Attributes_Response(uint8_t *buf)
{
    /*
        Send attributes of lamp ID #n.
        Referer: Page 337, https://www.usb.org/sites/default/files/hut1_4.pdf
    */

    if (RGB_Attributes_Request_Report_Lamp_ID >= RGB_LAMP_COUNT)
    {
        // Make sure we are not reading sth else
        RGB_Attributes_Request_Report_Lamp_ID = RGB_LAMP_COUNT - 1;
    }

    memcpy(buf, &RGB_Lamp_Attributes_Template, sizeof(LampAttributes));

    LampAttributes *_buf = (LampAttributes *)buf;
    _buf->LampId = RGB_Attributes_Request_Report_Lamp_ID;
    // _buf->PositionXInMicrometers = RGB_Lamp_Positions[RGB_Attributes_Request_Report_Lamp_ID].PositionXInMillimeters;
    // _buf->PositionYInMicrometers = RGB_Lamp_Positions[RGB_Attributes_Request_Report_Lamp_ID].PositionYInMillimeters;
    // _buf->PositionZInMicrometers = RGB_Lamp_Positions[RGB_Attributes_Request_Report_Lamp_ID].PositionZInMillimeters;
    _buf->PositionXInMicrometers = RGB_Attributes_Request_Report_Lamp_ID;
    _buf->PositionYInMicrometers = RGB_Attributes_Request_Report_Lamp_ID;
    _buf->PositionZInMicrometers = RGB_Attributes_Request_Report_Lamp_ID;

    RGB_Attributes_Request_Report_Lamp_ID++;
    if (RGB_Attributes_Request_Report_Lamp_ID >= RGB_LAMP_COUNT)
    {
        // Reached the end of lamps, return to 0 and next response will be the last response.
        RGB_Attributes_Request_Report_Lamp_ID = 0;
    }

    return sizeof(LampAttributesResponseReport);
}

bool RGB_Control_Set_Attr_Request_Lamp_ID(const uint8_t *buf, int32_t len)
{
    if (len != sizeof(LampAttributesRequestReport))
        return false;

    RGB_Attributes_Request_Report_Lamp_ID = ((LampAttributesRequestReport *)buf)->LampId;

    if (RGB_Attributes_Request_Report_Lamp_ID >= RGB_LAMP_COUNT)
    {
        RGB_Attributes_Request_Report_Lamp_ID = RGB_LAMP_COUNT - 1;
    }

    return true;
}

bool RGB_Control_Set_Multi_Update(const uint8_t *buf, int32_t len)
{
    if (len != sizeof(LampMultiUpdateReport))
        return false;

    LampMultiUpdateReport *_buf = (LampMultiUpdateReport *)buf;
    if (_buf->LampCount > RGB_LAMP_MULTI_UPDATE_LAMP_COUNT)
        return false;

    for (int i = 0; i < _buf->LampCount; i++)
    {
        if (_buf->LampIds[i] > RGB_LAMP_COUNT)
            return false;

        RGB_Lamp_Colors[_buf->LampIds[i] * 3] = _buf->UpdateColors[i].RedChannel;
        RGB_Lamp_Colors[_buf->LampIds[i] * 3 + 1] = _buf->UpdateColors[i].GreenChannel;
        RGB_Lamp_Colors[_buf->LampIds[i] * 3 + 2] = _buf->UpdateColors[i].BlueChannel;
    }

    if (_buf->LampUpdateFlags & 1)
    {
        osMessagePut(RGB_Update_Msg_Queue, 1, osWaitForever);
    }

    return true;
}

bool RGB_Control_Set_Range_Update(const uint8_t *buf, int32_t len)
{
    if (len != sizeof(LampRangeUpdateReport))
        return false;

    LampRangeUpdateReport *_buf = (LampRangeUpdateReport *)buf;

    if (_buf->LampIdStart > _buf->LampIdEnd)
        return false;
    if (_buf->LampIdStart > RGB_LAMP_COUNT || _buf->LampIdStart >= RGB_LAMP_COUNT)
        return false;

    for (int i = _buf->LampIdStart; i <= _buf->LampIdEnd; i++)
    {
        RGB_Lamp_Colors[i * 3] = _buf->UpdateColor.RedChannel;
        RGB_Lamp_Colors[i * 3 + 1] = _buf->UpdateColor.GreenChannel;
        RGB_Lamp_Colors[i * 3 + 2] = _buf->UpdateColor.BlueChannel;
    }

    if (_buf->LampUpdateFlags & 1)
    {
        osMessagePut(RGB_Update_Msg_Queue, 1, osWaitForever);
    }

    return true;
}

bool RGB_Control_Set_Control_Mode(const uint8_t *buf, int32_t len)
{
    if (len != sizeof(LampArrayControlReport))
        return false;

    LampArrayControlReport *_buf = (LampArrayControlReport *)buf;
    RGB_Control_Set_Autonomous_Mode(_buf->AutonomousMode);

    return true;
}
