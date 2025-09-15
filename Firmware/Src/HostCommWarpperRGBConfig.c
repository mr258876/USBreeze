/*
 * Copyright (c) 2025 mr258876
 * SPDX-License-Identifier: MIT
 */

#include "HostCommWarpper.h"

#include "RGBControl.h"
#include <string.h>

typedef __packed struct
{
    uint8_t RgbHidChannelCount;
    uint8_t RgbPhyChannelCount;
    uint16_t RgbLedCapacity;
    uint8_t RgbDebugDataFormat;
} RgbInfoReport;

typedef __packed struct
{
    uint8_t RgbHidChannelFlag;  // operational flags, bit0: update; bit1: write to flash
    uint8_t RgbHidChannelId;
    uint16_t RgbHidChannelStartId;
    uint16_t RgbHidChannelLedCount;
} RgbHidChannelMapReport;

typedef __packed struct
{
    uint8_t RgbPhyChannelFlag;  // operational flags, bit0: update; bit1: write to flash
    uint8_t RgbPhyChannelId;
    uint16_t RgbPhyChannelStartId;
    uint16_t RgbPhyChannelLedCount;
} RgbPhyChannelMapReport;

static uint8_t RGB_Config_Hid_Channel_Map_Report_Offset = 0;
static uint8_t RGB_Config_Phy_Channel_Map_Report_Offset = 0;


int32_t RGB_Config_Get_Info_Report(uint8_t *buf)
{
    RgbInfoReport *_buf = (RgbInfoReport*)buf;

    _buf->RgbHidChannelCount = RGB_CONTROL_HID_CHANNELS_COUNT;
    _buf->RgbPhyChannelCount = RGB_CONTROL_PHY_CHANNELS_COUNT;
    _buf->RgbLedCapacity = RGB_LAMP_TOTAL_COUNT;
    _buf->RgbDebugDataFormat = 1;

    return sizeof(RgbInfoReport);
}

int32_t RGB_Config_Get_Hid_Channel_Map_Report(uint8_t *buf)
{
    RgbHidChannelMapReport *_buf = (RgbHidChannelMapReport*)buf;

    _buf->RgbHidChannelId = RGB_Config_Hid_Channel_Map_Report_Offset;
    _buf->RgbHidChannelStartId = RGB_Hid_Channel_Lamp_Map[RGB_Config_Hid_Channel_Map_Report_Offset][0];
    _buf->RgbHidChannelLedCount = RGB_Hid_Channel_Lamp_Map[RGB_Config_Hid_Channel_Map_Report_Offset][1];

    if (RGB_Config_Hid_Channel_Map_Report_Offset + 1 >= RGB_CONTROL_HID_CHANNELS_COUNT)
        RGB_Config_Hid_Channel_Map_Report_Offset = 0;
    else
        RGB_Config_Hid_Channel_Map_Report_Offset += 1;

    return sizeof(RgbHidChannelMapReport);
}

int32_t RGB_Config_Get_Phy_Channel_Map_Report(uint8_t *buf)
{
    RgbPhyChannelMapReport *_buf = (RgbPhyChannelMapReport*)buf;

    _buf->RgbPhyChannelId = RGB_Config_Phy_Channel_Map_Report_Offset;
    _buf->RgbPhyChannelStartId = RGB_Phy_Channel_Lamp_Map[RGB_Config_Phy_Channel_Map_Report_Offset][0];
    _buf->RgbPhyChannelLedCount = RGB_Phy_Channel_Lamp_Map[RGB_Config_Phy_Channel_Map_Report_Offset][1];

    if (RGB_Config_Phy_Channel_Map_Report_Offset + 1 >= RGB_CONTROL_PHY_CHANNELS_COUNT)
        RGB_Config_Phy_Channel_Map_Report_Offset = 0;
    else
        RGB_Config_Phy_Channel_Map_Report_Offset += 1;

    return sizeof(RgbPhyChannelMapReport);
}

bool RGB_Config_Set_Hid_Channel_Map_Report(const uint8_t *buf, int32_t len)
{
    if (len != sizeof(RgbHidChannelMapReport))
        return false;
    
    RgbHidChannelMapReport *_buf = (RgbHidChannelMapReport*)buf;
    if (_buf->RgbHidChannelId >= RGB_CONTROL_HID_CHANNELS_COUNT)
        return false;

    RGB_Config_Hid_Channel_Map_Report_Offset = _buf->RgbHidChannelId;

    if ((_buf->RgbHidChannelFlag) & 1)
    {
        RGB_Hid_Channel_Lamp_Map[RGB_Config_Hid_Channel_Map_Report_Offset][0] = _buf->RgbHidChannelStartId;
        RGB_Hid_Channel_Lamp_Map[RGB_Config_Hid_Channel_Map_Report_Offset][1] = _buf->RgbHidChannelLedCount;
    }

    if ((_buf->RgbHidChannelFlag >> 1) & 1)
    {
        RGB_Control_Save_Settings_Flash();
    }

    return true;
}

bool RGB_Config_Set_Phy_Channel_Map_Report(const uint8_t *buf, int32_t len)
{
    if (len != sizeof(RgbPhyChannelMapReport))
        return false;

    RgbPhyChannelMapReport *_buf = (RgbPhyChannelMapReport*)buf;
    if (_buf->RgbPhyChannelId >= RGB_CONTROL_PHY_CHANNELS_COUNT)
        return false;

    RGB_Config_Phy_Channel_Map_Report_Offset = _buf->RgbPhyChannelId;

    if ((_buf->RgbPhyChannelFlag) & 1)
    {
        RGB_Phy_Channel_Lamp_Map[RGB_Config_Phy_Channel_Map_Report_Offset][0] = _buf->RgbPhyChannelStartId;
        RGB_Phy_Channel_Lamp_Map[RGB_Config_Phy_Channel_Map_Report_Offset][1] = _buf->RgbPhyChannelLedCount;
    }

    if ((_buf->RgbPhyChannelFlag >> 1) & 1)
    {
        RGB_Control_Save_Settings_Flash();
    }

    return true;
}
