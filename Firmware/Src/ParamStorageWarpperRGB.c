#include "ParamStorage.h"
#include "ParamStorageKeys.h"
#include "ParamStorageWarpper.h"
#include "RGBControl.h"

void RGB_Control_Load_Params(void)
{
    EE_Read(SK_RGB_CONFIG_HID_CHANNEL_MAP, RGB_Hid_Channel_Lamp_Map, RGB_CONTROL_HID_CHANNELS_COUNT * 2 * sizeof(uint16_t));
    EE_Read(SK_RGB_CONFIG_PHY_CHANNEL_MAP, RGB_Phy_Channel_Lamp_Map, RGB_CONTROL_PHY_CHANNELS_COUNT * 2 * sizeof(uint16_t));
}

void RGB_Control_Save_Params(void)
{
    EE_Write(SK_RGB_CONFIG_HID_CHANNEL_MAP, RGB_Hid_Channel_Lamp_Map, RGB_CONTROL_HID_CHANNELS_COUNT * 2 * sizeof(uint16_t));
    EE_Write(SK_RGB_CONFIG_PHY_CHANNEL_MAP, RGB_Phy_Channel_Lamp_Map, RGB_CONTROL_PHY_CHANNELS_COUNT * 2 * sizeof(uint16_t));
}
