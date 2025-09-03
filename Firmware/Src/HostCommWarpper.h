/*
 * Copyright (c) 2025 mr258876
 * SPDX-License-Identifier: MIT
 */

#ifndef _HOST_COMM_WARPPER
#define _HOST_COMM_WARPPER

#include <stdint.h>
#include <stdbool.h>

#define FAN_HID_INSTANCE        0
#define RGB_HID_INSTANCE_A      1
#define RGB_HID_INSTANCE_B      2
#define RGB_HID_INSTANCE_C      3

/*-------------------- F A N --------------------*/

#define FAN_INFO_REPORT_ID          1
#define FAN_PWM_REPORT_ID           2
#define FAN_TEMP_SENSOR_REPORT_ID   3
#define FAN_HOST_OVERRIDE_REPORT    4
#define FAN_CURVE_CFG_REPORT        5
#define FAN_CURVE_POINTS_REPORT     6

#define FAN_RPM_REPORT_DATA_CNT     4
#define FAN_TEMP_REPORT_DATA_CNT    8
#define FAN_CONTROL_REPORT_DATA_CNT 8
#define FAN_CURVE_POINT_DATA_CNT    4

int32_t Fan_Control_Get_Info_Report(uint8_t *buf);
int32_t Fan_Control_Get_RPM_Report(uint8_t *buf);
int32_t Fan_Control_Get_Temp_Report(uint8_t *buf);
int32_t Fan_Control_Get_Control_Report(uint8_t *buf);
int32_t Fan_Control_Get_Curve_Cfg_Report(uint8_t *buf);
int32_t Fan_Control_Get_Curve_Point_Report(uint8_t *buf);
bool Fan_Control_Set_RPM_Report(const uint8_t *buf, int32_t len);
bool Fan_Control_Set_Temp_Report(const uint8_t *buf, int32_t len);
bool Fan_Control_Set_Control_Report(const uint8_t *buf, int32_t len);
bool Fan_Control_Set_Curve_Cfg_Report(const uint8_t *buf, int32_t len);
bool Fan_Control_Set_Curve_Point_Report(const uint8_t *buf, int32_t len);

/*-------------------- R G B --------------------*/

#define RGB_LAMP_ARRAY_ATTRIBUTES_REPORT_ID     1
#define RGB_LAMP_ATTRIBUTES_REQUEST_REPORT_ID   2
#define RGB_LAMP_ATTRIBUTES_RESPONSE_REPORT_ID  3
#define RGB_LAMP_MULTI_UPDATE_REPORT_ID         4
#define RGB_LAMP_RANGE_UPDATE_REPORT_ID         5
#define RGB_LAMP_ARRAY_CONTROL_REPORT_ID        6

#define RGB_LAMP_MULTI_UPDATE_LAMP_COUNT        10

#define RGB_LAMP_INSTANCES_COUNT                3
#define RGB_LAMP_INSTANCE_0_LAMP_COUNT          128
#define RGB_LAMP_INSTANCE_1_LAMP_COUNT          64
#define RGB_LAMP_INSTANCE_2_LAMP_COUNT          64

int32_t RGB_Control_Get_Attr_Report(uint8_t instance, uint8_t *buf);
int32_t RGB_Control_Get_Attributes_Response(uint8_t instance, uint8_t *buf);
bool RGB_Control_Set_Attr_Request_Lamp_ID(uint8_t instance, const uint8_t *buf, int32_t len);
bool RGB_Control_Set_Multi_Update(uint8_t instance, const uint8_t *buf, int32_t len);
bool RGB_Control_Set_Range_Update(uint8_t instance, const uint8_t *buf, int32_t len);
bool RGB_Control_Set_Control_Mode(uint8_t instance, const uint8_t *buf, int32_t len);

#endif
