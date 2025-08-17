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

#define FAN_CONTROL_RPM_INPUT_REPORT_ID     1
#define FAN_CONTROL_RPM_OUTPUT_REPORT_ID    1
#define FAN_CONTROL_LEVEL_REPORT_ID         1

void Fan_Control_Notify_Host_RPM(void);

int32_t Fan_Control_Copy_RPM_To_Buffer(uint8_t *buf);
int32_t Fan_Control_Copy_Fan_Level_To_Buffer(uint8_t *buf);
// bool Fan_Control_Set_RPM_From_Host(const uint8_t *buf, int32_t len);
bool Fan_Control_Set_Fan_Level_From_Host(const uint8_t *buf, int32_t len);

/*-------------------- R G B --------------------*/

#define RGB_LAMP_ARRAY_ATTRIBUTES_REPORT_ID     1
#define RGB_LAMP_ATTRIBUTES_REQUEST_REPORT_ID   2
#define RGB_LAMP_ATTRIBUTES_RESPONSE_REPORT_ID  3
#define RGB_LAMP_MULTI_UPDATE_REPORT_ID         4
#define RGB_LAMP_RANGE_UPDATE_REPORT_ID         5
#define RGB_LAMP_ARRAY_CONTROL_REPORT_ID        6

#define RGB_LAMP_MULTI_UPDATE_LAMP_COUNT        10

int32_t RGB_Control_Get_Attr_Report(uint8_t instance, uint8_t *buf);
int32_t RGB_Control_Get_Attributes_Response(uint8_t instance, uint8_t *buf);
bool RGB_Control_Set_Attr_Request_Lamp_ID(uint8_t instance, const uint8_t *buf, int32_t len);
bool RGB_Control_Set_Multi_Update(uint8_t instance, const uint8_t *buf, int32_t len);
bool RGB_Control_Set_Range_Update(uint8_t instance, const uint8_t *buf, int32_t len);
bool RGB_Control_Set_Control_Mode(uint8_t instance, const uint8_t *buf, int32_t len);

#endif
