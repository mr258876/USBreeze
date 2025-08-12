#include "HostCommWarpper.h"

#include "FanControl.h"

#include "rl_usb.h"


void Fan_Control_Notify_Host_RPM(void)
{
    USBD_HID_GetReportTrigger(FAN_HID_INSTANCE, FAN_CONTROL_RPM_INPUT_REPORT_ID, (uint8_t *)Fan_RPM_Count, sizeof(SYSTEM_FAN_RPM_TYPE) * SYSTEM_FAN_COUNT);
}

int32_t Fan_Control_Copy_RPM_To_Buffer(uint8_t *buf)
{
    // Fan RPMs, Input for PC
    for (int i = 0; i < SYSTEM_FAN_COUNT; ++i)
    {
        uint32_t v = Fan_RPM_Count[i];
        buf[i * 4 + 0] = (uint8_t)(v & 0xFF);
        buf[i * 4 + 1] = (uint8_t)((v >> 8) & 0xFF);
        buf[i * 4 + 2] = (uint8_t)((v >> 16) & 0xFF);
        buf[i * 4 + 3] = (uint8_t)((v >> 24) & 0xFF);
    }
    return SYSTEM_FAN_COUNT * sizeof(SYSTEM_FAN_RPM_TYPE);
}

int32_t Fan_Control_Copy_Fan_Level_To_Buffer(uint8_t *buf)
{
    // Fan Levels, Input for PC
    for (int i = 0; i < SYSTEM_FAN_COUNT; ++i)
    {
        buf[i] = Fan_Control_Levels[i];
    }
    return SYSTEM_FAN_COUNT;
}

// bool Fan_Control_Set_RPM_From_Host(const uint8_t *buf, int32_t len)
// {
//     if (len != sizeof(SYSTEM_FAN_RPM_TYPE) * SYSTEM_FAN_COUNT)
//         return false;

//     // Fan RPMs, Output of PC
//     for (int i = 0; i < SYSTEM_FAN_COUNT; ++i)
//     {
//         Fan_Control_Set_RPM(i, buf[i * 4] + (buf[i * 4 + 1] << 8));
//     }
//     return true;
// }

bool Fan_Control_Set_Fan_Level_From_Host(const uint8_t *buf, int32_t len)
{
    if (len != SYSTEM_FAN_COUNT)
        return false;

    // Fan Levels, Output of PC
    for (int i = 0; i < SYSTEM_FAN_COUNT; ++i)
    {
        Fan_Control_Set_Level(i, buf[i]);
    }
    return true;
}
