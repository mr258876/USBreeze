#include "HostCommWarpper.h"

#include "RGBControl.h"
#include <string.h>
#include "rl_usb.h"

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

int32_t RGB_Control_Get_Attr_Report(uint8_t *buf)
{
    LampArrayAttributesReport *data = (LampArrayAttributesReport *)buf;

    data->LampCount = RGB_LAMP_COUNT;
    data->BoundingBoxWidthInMicrometers = 1;
    data->BoundingBoxHeightInMicrometers = 1;
    data->BoundingBoxDepthInMicrometers = 1;
    data->LampArrayKind = LampArrayKindChassis;
    data->MinUpdateIntervalInMicroseconds = 5000;

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

    memcpy(buf, RGB_Lamp_Attributes + (RGB_Attributes_Request_Report_Lamp_ID * sizeof(LampAttributes)), sizeof(LampAttributes));

    RGB_Attributes_Request_Report_Lamp_ID += 1;
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

bool RGB_Control_Set_Control_Mode(const uint8_t *buf, int32_t len)
{
    if (len != sizeof(LampArrayControlReport))
        return false;

    /* TODO */

    return true;
}
