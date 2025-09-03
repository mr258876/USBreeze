/* 
 * Copyright (c) 2025 mr258876
 * SPDX-License-Identifier: MIT
 */

#ifndef _FAN_CONTROL_H
#define _FAN_CONTROL_H

#define SYSTEM_FAN_COUNT			8
#define SYSTEM_FAN_PPR				2		// <- Count only one edge
#define SYSTEM_TEMP_SENSOR_COUNT    1
#define SYSTEM_MAX_CURVE_POINTS     4
#define SYSTEM_FAN_HALL_TYPE    uint16_t
#define SYSTEM_FAN_RPM_TYPE     uint32_t
#define SYSTEM_FAN_LEVEL_TYPE   uint16_t
#define SYSTEM_TEMP_LEVEL_TYPE  int16_t

#define SYSTEM_UPDATE_INTERVAL_MS   500

#define FAN_LEVEL_OVERRIDE_BY_HOST  0x8000

#define FAN_PWM_MIN_VALUE           0
#define FAN_PWM_MAX_VALUE           1000

#include <stdint.h>

typedef __packed struct
{
    int16_t CurvePointTemp;
    int16_t CurvePointPWM;
} FanCurvePointValue;

typedef __packed struct
{
    uint8_t CurvePointCountTotal;
    uint8_t TempSensorId;
} FanCurveCfgValue;

extern SYSTEM_FAN_HALL_TYPE Fan_Hall_Count[SYSTEM_FAN_COUNT];
extern SYSTEM_FAN_RPM_TYPE Fan_RPM_Count[SYSTEM_FAN_COUNT];
extern SYSTEM_FAN_LEVEL_TYPE Fan_Control_Levels[SYSTEM_FAN_COUNT];
extern SYSTEM_TEMP_LEVEL_TYPE Fan_Control_Temperature[SYSTEM_TEMP_SENSOR_COUNT];
extern FanCurvePointValue Fan_Control_Curves[SYSTEM_FAN_COUNT][SYSTEM_MAX_CURVE_POINTS];
extern FanCurveCfgValue Fan_Control_Curve_Cfgs[SYSTEM_FAN_COUNT];

void Fan_Control_Loop(void);

void Fan_Control_Initialize(void);
SYSTEM_FAN_RPM_TYPE Fan_Control_Get_RPM(uint8_t fan_id);
void Fan_Control_Set_Level(uint8_t fan_id, uint16_t level);
void Fan_Control_Save_Settings_Flash(void);

#endif
