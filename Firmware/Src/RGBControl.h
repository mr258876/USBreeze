/* 
 * Copyright (c) 2025 mr258876
 * SPDX-License-Identifier: MIT
 */

#ifndef _RGB_CONTROL_H
#define _RGB_CONTROL_H

#include <stdint.h>
#include "cmsis_os.h"

#define RGB_LAMP_TOTAL_COUNT        256
#define RGB_LAMPARRAY_KIND          7       // 07 -> LampArrayKindChassis. Referer: Page 330, https://www.usb.org/sites/default/files/hut1_4.pdf
#define RGB_MIN_UPDATE_INTERVAL     RGB_LAMP_TOTAL_COUNT * 36    // In Microseconds, assume 1.5us for 1bit in WS281X

#define RGB_CUSTOM_LAMP_POSITIONS   0       // See RGBLampPositions.c

#if RGB_CUSTOM_LAMP_POSITIONS
#define RGB_BOUNDING_BOX_WIDTH_X    10000   // In Micrometers
#define RGB_BOUNDING_BOX_DEPTH_Y    10000   // In Micrometers
#define RGB_BOUNDING_BOX_HEIGHT_Z   10000   // In Micrometers
#endif

#define RGB_CONTROL_CHANNELS_COUNT          3
#define RGB_CONTROL_CHANNEL_A_LAMP_COUNT    128
#define RGB_CONTROL_CHANNEL_B_LAMP_COUNT    64
#define RGB_CONTROL_CHANNEL_C_LAMP_COUNT    64

#define RGB_CHANNELS_PER_LAMP       3

#define RGB_WS2812_PORT             GPIOA
#define RGB_WS2812_PIN              GPIO_Pin_8

#define RGB_WS2812_BITS_PER_LED     24
#define RGB_WS2812_BUFFER_SIZE      RGB_WS2812_BITS_PER_LED * RGB_CONTROL_CHANNELS_COUNT * 2 // <- Ping-pong buffer, contains data of 2 lamps
#define RGB_WS2812_ARR              90      // Autoreload value of TIM1
#define RGB_WS2812_T0H              30		// 1/3 high for a 0bit
#define RGB_WS2812_T1H              60		// 2/3 high for a 1bit
#define RGB_WS2812_RESET_CYCLES     200     // 100bits LOW to reset

extern volatile uint16_t RGB_WS2812_Buffer[];    // WS2812 buffer
extern volatile uint8_t RGB_Lamp_Colors[RGB_LAMP_TOTAL_COUNT * RGB_CHANNELS_PER_LAMP];

typedef __packed struct
{
    uint16_t PositionXInMillimeters;
    uint16_t PositionYInMillimeters;
    uint16_t PositionZInMillimeters;
} LampPosition;

#if RGB_CUSTOM_LAMP_POSITIONS == true
extern const LampPosition RGB_Lamp_Positions[]; // ID count MUST match the size of RGB_LAMP_COUNT
#endif

extern osMessageQId RGB_Update_Msg_Queue;

void RGB_Control_thread(const void * dummy);

void RGB_Control_Fill_Half_Buffer(int half_idx);
void RGB_Control_WS2812B_Reset(void);

void RGB_Control_Set_Autonomous_Mode(uint8_t channel, int autonomous_on);
uint8_t RGB_Control_get_Autonomous_Mode(uint8_t channel);

#endif
