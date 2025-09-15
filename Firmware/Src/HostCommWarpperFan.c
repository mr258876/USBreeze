/*
 * Copyright (c) 2025 mr258876
 * SPDX-License-Identifier: MIT
 */

#include "HostCommWarpper.h"

#include "FanControl.h"

#include "rl_usb.h"

typedef __packed struct
{
    uint16_t DeviceFwBcd;
    uint32_t DeviceSerial;
    uint8_t FanCount;
    uint8_t FanCapabilities;
    uint16_t FanPwmMaxPercent;
    uint8_t TempSensorCount;
    uint8_t TempSensorCapabilities;
    uint8_t FanCurvePointsMax;
    uint16_t FanRpmPollMinMs;
} FanInfoReport;

typedef __packed struct
{
    uint8_t FanRpmIdOffset;
    uint8_t FanRpmsInPacket;
    uint32_t FanRpmValues[FAN_RPM_REPORT_DATA_CNT];
} FanRpmReport;

typedef __packed struct
{
    uint8_t TempSensorIdOffset;
    uint8_t TempSensorsInPacket;
    int16_t TempSensorValues[FAN_TEMP_REPORT_DATA_CNT];
} FanTempSensorReport;

typedef __packed struct
{
    uint8_t FanControlIdOffset;
    uint8_t FanControlsInPacket;
    uint16_t FanControlValues[FAN_CONTROL_REPORT_DATA_CNT];
} FanControlValueReport;

typedef __packed struct
{
    uint8_t FanId;
    uint8_t OpFlags;
    FanCurveCfgValue CurveCfg;
} FanCurveCfgReport;

typedef __packed struct
{
    uint8_t FanId;
    uint8_t CurvePointIdOffset;
    uint8_t CurvePointsInPacket;
    FanCurvePointValue CurvePoints[FAN_CURVE_POINT_DATA_CNT];
} FanCurvePointReport;

static uint8_t Fan_PWM_Report_Offset = 0;
static uint8_t Fan_Temp_Sensor_Report_Offset = 0;
static uint8_t Fan_Control_Value_Report_Offset = 0;
static uint8_t Fan_Curve_Cfg_Report_Fan_Id = 0;
static uint8_t Fan_Curve_Point_Report_Fan_Id = 0;
static uint8_t Fan_Curve_Point_Report_Point_Id_Offset = 0;

int32_t Fan_Control_Get_Info_Report(uint8_t *buf)
{
    FanInfoReport *_buf = (FanInfoReport *)buf;

    _buf->FanCount = SYSTEM_FAN_COUNT;
    _buf->FanPwmMaxPercent = FAN_PWM_MAX_VALUE;
    _buf->TempSensorCount = SYSTEM_TEMP_SENSOR_COUNT;
    _buf->TempSensorCapabilities = 1;

    return sizeof(FanInfoReport);
}

int32_t Fan_Control_Get_RPM_Report(uint8_t *buf)
{
    FanRpmReport *_buf = (FanRpmReport *)buf;

    _buf->FanRpmIdOffset = Fan_PWM_Report_Offset;
    _buf->FanRpmsInPacket = (Fan_PWM_Report_Offset + FAN_RPM_REPORT_DATA_CNT > SYSTEM_FAN_COUNT) ? (SYSTEM_FAN_COUNT - Fan_PWM_Report_Offset) : FAN_RPM_REPORT_DATA_CNT;

    for (int i = 0; i < _buf->FanRpmsInPacket; ++i)
    {
        _buf->FanRpmValues[i] = Fan_RPM_Count[Fan_PWM_Report_Offset + i];
    }

    if (Fan_PWM_Report_Offset + _buf->FanRpmsInPacket >= SYSTEM_FAN_COUNT)
        Fan_PWM_Report_Offset = 0;
    else
        Fan_PWM_Report_Offset += _buf->FanRpmsInPacket;

    return sizeof(FanRpmReport);
}

int32_t Fan_Control_Get_Temp_Report(uint8_t *buf)
{
    FanTempSensorReport *_buf = (FanTempSensorReport *)buf;

    _buf->TempSensorIdOffset = Fan_Temp_Sensor_Report_Offset;
    _buf->TempSensorsInPacket = (Fan_Temp_Sensor_Report_Offset + FAN_TEMP_REPORT_DATA_CNT > SYSTEM_TEMP_SENSOR_COUNT) ? (SYSTEM_TEMP_SENSOR_COUNT - Fan_Temp_Sensor_Report_Offset) : FAN_TEMP_REPORT_DATA_CNT;

    for (int i = 0; i < _buf->TempSensorsInPacket; ++i)
    {
        _buf->TempSensorValues[i] = Fan_Control_Temperature[Fan_Temp_Sensor_Report_Offset + i];
    }

    if (Fan_Temp_Sensor_Report_Offset + _buf->TempSensorsInPacket >= SYSTEM_TEMP_SENSOR_COUNT)
        Fan_Temp_Sensor_Report_Offset = 0;
    else
        Fan_Temp_Sensor_Report_Offset += _buf->TempSensorsInPacket;

    return sizeof(FanTempSensorReport);
}

int32_t Fan_Control_Get_Control_Report(uint8_t *buf)
{
    FanControlValueReport *_buf = (FanControlValueReport *)buf;

    _buf->FanControlIdOffset = Fan_Control_Value_Report_Offset;
    _buf->FanControlsInPacket = (Fan_Control_Value_Report_Offset + FAN_CONTROL_REPORT_DATA_CNT > SYSTEM_FAN_COUNT) ? (SYSTEM_FAN_COUNT - Fan_Control_Value_Report_Offset) : FAN_CONTROL_REPORT_DATA_CNT;

    for (int i = 0; i < _buf->FanControlsInPacket; ++i)
    {
        _buf->FanControlValues[i] = Fan_Control_Levels[Fan_Control_Value_Report_Offset + i];
    }

    if (Fan_Control_Value_Report_Offset + _buf->FanControlsInPacket >= SYSTEM_FAN_COUNT)
        Fan_Control_Value_Report_Offset = 0;
    else
        Fan_Control_Value_Report_Offset += _buf->FanControlsInPacket;

    return sizeof(FanControlValueReport);
}

int32_t Fan_Control_Get_Curve_Cfg_Report(uint8_t *buf)
{
    FanCurveCfgReport *_buf = (FanCurveCfgReport *)buf;

    _buf->FanId = Fan_Curve_Cfg_Report_Fan_Id;
    _buf->OpFlags = 0;
    _buf->CurveCfg = Fan_Control_Curve_Cfgs[Fan_Curve_Cfg_Report_Fan_Id];

    Fan_Curve_Cfg_Report_Fan_Id += 1;
    if (Fan_Curve_Cfg_Report_Fan_Id >= SYSTEM_FAN_COUNT)
        Fan_Curve_Cfg_Report_Fan_Id = 0;

    return sizeof(FanCurveCfgReport);
}

int32_t Fan_Control_Get_Curve_Point_Report(uint8_t *buf)
{
    FanCurvePointReport *_buf = (FanCurvePointReport *)buf;

    _buf->FanId = Fan_Curve_Point_Report_Fan_Id;
    _buf->CurvePointIdOffset = Fan_Curve_Point_Report_Point_Id_Offset;
    _buf->CurvePointsInPacket = (Fan_Curve_Point_Report_Point_Id_Offset + FAN_CURVE_POINT_DATA_CNT > SYSTEM_MAX_CURVE_POINTS) ? (SYSTEM_MAX_CURVE_POINTS - Fan_Curve_Point_Report_Point_Id_Offset) : FAN_CURVE_POINT_DATA_CNT;

    for (size_t i = 0; i < _buf->CurvePointsInPacket; i++)
    {
        _buf->CurvePoints[i] = Fan_Control_Curves[Fan_Curve_Point_Report_Fan_Id][Fan_Curve_Point_Report_Point_Id_Offset + i];
    }

    if (Fan_Curve_Point_Report_Point_Id_Offset + _buf->CurvePointsInPacket >= SYSTEM_FAN_COUNT)
    {
        Fan_Curve_Point_Report_Point_Id_Offset = 0;
        Fan_Curve_Point_Report_Fan_Id += 1;
    }
    else
    {
        Fan_Curve_Point_Report_Point_Id_Offset += _buf->CurvePointsInPacket;
    }

    if (Fan_Curve_Point_Report_Fan_Id >= SYSTEM_FAN_COUNT)
        Fan_Curve_Point_Report_Fan_Id = 0;

    return sizeof(FanCurvePointReport);
}

bool Fan_Control_Set_RPM_Report(const uint8_t *buf, int32_t len)
{
    if (len != sizeof(FanRpmReport))
        return false;

    FanRpmReport *_buf = (FanRpmReport *)buf;

    if (_buf->FanRpmIdOffset >= SYSTEM_FAN_COUNT || _buf->FanRpmIdOffset + _buf->FanRpmsInPacket > SYSTEM_FAN_COUNT || _buf->FanRpmsInPacket > FAN_RPM_REPORT_DATA_CNT)
        return false;

    Fan_PWM_Report_Offset = _buf->FanRpmIdOffset;

    return true;
}

bool Fan_Control_Set_Temp_Report(const uint8_t *buf, int32_t len)
{
    if (len != sizeof(FanTempSensorReport))
        return false;

    FanTempSensorReport *_buf = (FanTempSensorReport *)buf;

    if (_buf->TempSensorIdOffset >= SYSTEM_TEMP_SENSOR_COUNT || _buf->TempSensorIdOffset + _buf->TempSensorsInPacket > SYSTEM_TEMP_SENSOR_COUNT || _buf->TempSensorsInPacket > FAN_TEMP_REPORT_DATA_CNT)
        return false;

    Fan_Temp_Sensor_Report_Offset = _buf->TempSensorIdOffset;

    return true;
}

bool Fan_Control_Set_Control_Report(const uint8_t *buf, int32_t len)
{
    if (len != sizeof(FanControlValueReport))
        return false;

    FanControlValueReport *_buf = (FanControlValueReport *)buf;

    if (_buf->FanControlIdOffset >= SYSTEM_FAN_COUNT || _buf->FanControlIdOffset + _buf->FanControlsInPacket > SYSTEM_FAN_COUNT || _buf->FanControlsInPacket > FAN_CONTROL_REPORT_DATA_CNT)
        return false;

    Fan_Control_Value_Report_Offset = _buf->FanControlIdOffset;

    for (size_t i = 0; i < _buf->FanControlsInPacket; i++)
    {
        Fan_Control_Set_Level(_buf->FanControlIdOffset + i, _buf->FanControlValues[i]);
    }

    return true;
}

bool Fan_Control_Set_Curve_Cfg_Report(const uint8_t *buf, int32_t len)
{
    if (len != sizeof(FanCurveCfgReport))
        return false;

    FanCurveCfgReport *_buf = (FanCurveCfgReport *)buf;

    if (_buf->FanId >= SYSTEM_FAN_COUNT)
        return false;

    Fan_Curve_Cfg_Report_Fan_Id = _buf->FanId;

    if ((_buf->OpFlags) & 1)
    {
        Fan_Control_Curve_Cfgs[Fan_Curve_Cfg_Report_Fan_Id] = _buf->CurveCfg;
    }

    if ((_buf->OpFlags >> 1) & 1)
    {
        Fan_Control_Save_Settings_Flash();
    }

    return true;
}

bool Fan_Control_Set_Curve_Point_Report(const uint8_t *buf, int32_t len)
{
    if (len != sizeof(FanCurvePointReport))
        return false;

    FanCurvePointReport *_buf = (FanCurvePointReport *)buf;

    if (_buf->FanId >= SYSTEM_FAN_COUNT)
        return false;
    if (_buf->CurvePointIdOffset >= SYSTEM_MAX_CURVE_POINTS || _buf->CurvePointIdOffset + _buf->CurvePointsInPacket > SYSTEM_MAX_CURVE_POINTS || _buf->CurvePointsInPacket > FAN_CURVE_POINT_DATA_CNT)
        return false;

    Fan_Curve_Point_Report_Fan_Id = _buf->FanId;
    Fan_Curve_Point_Report_Point_Id_Offset = _buf->CurvePointIdOffset;

    for (size_t i = 0; i < _buf->CurvePointsInPacket; i++)
    {
        Fan_Control_Curves[Fan_Curve_Point_Report_Fan_Id][Fan_Curve_Point_Report_Point_Id_Offset + i] = _buf->CurvePoints[i];
    }

    return true;
}
