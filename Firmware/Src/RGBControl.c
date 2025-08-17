/* 
 * Copyright (c) 2025 mr258876
 * SPDX-License-Identifier: MIT
 */

#include "RGBControl.h"
#include "stm32f10x.h"

#include "cmsis_os.h"

volatile uint16_t RGB_WS2812_Buffer[RGB_WS2812_BUFFER_SIZE];
volatile uint8_t RGB_Lamp_Colors[RGB_LAMP_TOTAL_COUNT * RGB_CHANNELS_PER_LAMP];

static const uint16_t RGB_WS2812_NIBBLE_LUT[16][4] = {
    /*0x0*/ {RGB_WS2812_T0H, RGB_WS2812_T0H, RGB_WS2812_T0H, RGB_WS2812_T0H},
    /*0x1*/ {RGB_WS2812_T0H, RGB_WS2812_T0H, RGB_WS2812_T0H, RGB_WS2812_T1H},
    /*0x2*/ {RGB_WS2812_T0H, RGB_WS2812_T0H, RGB_WS2812_T1H, RGB_WS2812_T0H},
    /*0x3*/ {RGB_WS2812_T0H, RGB_WS2812_T0H, RGB_WS2812_T1H, RGB_WS2812_T1H},
    /*0x4*/ {RGB_WS2812_T0H, RGB_WS2812_T1H, RGB_WS2812_T0H, RGB_WS2812_T0H},
    /*0x5*/ {RGB_WS2812_T0H, RGB_WS2812_T1H, RGB_WS2812_T0H, RGB_WS2812_T1H},
    /*0x6*/ {RGB_WS2812_T0H, RGB_WS2812_T1H, RGB_WS2812_T1H, RGB_WS2812_T0H},
    /*0x7*/ {RGB_WS2812_T0H, RGB_WS2812_T1H, RGB_WS2812_T1H, RGB_WS2812_T1H},
    /*0x8*/ {RGB_WS2812_T1H, RGB_WS2812_T0H, RGB_WS2812_T0H, RGB_WS2812_T0H},
    /*0x9*/ {RGB_WS2812_T1H, RGB_WS2812_T0H, RGB_WS2812_T0H, RGB_WS2812_T1H},
    /*0xA*/ {RGB_WS2812_T1H, RGB_WS2812_T0H, RGB_WS2812_T1H, RGB_WS2812_T0H},
    /*0xB*/ {RGB_WS2812_T1H, RGB_WS2812_T0H, RGB_WS2812_T1H, RGB_WS2812_T1H},
    /*0xC*/ {RGB_WS2812_T1H, RGB_WS2812_T1H, RGB_WS2812_T0H, RGB_WS2812_T0H},
    /*0xD*/ {RGB_WS2812_T1H, RGB_WS2812_T1H, RGB_WS2812_T0H, RGB_WS2812_T1H},
    /*0xE*/ {RGB_WS2812_T1H, RGB_WS2812_T1H, RGB_WS2812_T1H, RGB_WS2812_T0H},
    /*0xF*/ {RGB_WS2812_T1H, RGB_WS2812_T1H, RGB_WS2812_T1H, RGB_WS2812_T1H},
};

osMessageQDef(RGB_Update_Msg_Queue, 4, uint8_t); // Define message queue
osMessageQId RGB_Update_Msg_Queue;

/* Data send status */
static const volatile uint8_t *RGB_Color_Source = NULL;
static int RGB_Lamps_To_Update = 0;
static int RGB_Lamps_Encoded = 0;      // LEDs in send buffer
static int RGB_Encoded_Reset_Bits = 0; // Encoded reset bit count
static int RGB_Update_Busy = 0;

static int RGB_Autonomous_Mode = 1;

static void RGB_Control_Encode_RGB(uint8_t r, uint8_t g, uint8_t b, volatile uint16_t *dst);
static void RGB_Control_Show_RGB_Blocking_From_Array(const volatile uint8_t *rgb_bytes, int n);

static inline void RGB_Control_Encode_LUT(uint8_t v, volatile uint16_t *dst)
{
    const uint16_t *hi = RGB_WS2812_NIBBLE_LUT[v >> 4];
    const uint16_t *lo = RGB_WS2812_NIBBLE_LUT[v & 0x0F];

    // 用直接赋值避免 volatile + memcpy 的告警/优化问题
    dst[0] = hi[0];
    dst[1] = hi[1];
    dst[2] = hi[2];
    dst[3] = hi[3];
    dst[4] = lo[0];
    dst[5] = lo[1];
    dst[6] = lo[2];
    dst[7] = lo[3];
}

static void RGB_Control_Encode_RGB(uint8_t r, uint8_t g, uint8_t b, volatile uint16_t *dst24)
{
    RGB_Control_Encode_LUT(g, &dst24[0]);  // G
    RGB_Control_Encode_LUT(r, &dst24[8]);  // R
    RGB_Control_Encode_LUT(b, &dst24[16]); // B
}

void RGB_Control_Fill_Half_Buffer(int half_idx)
{
    volatile uint16_t *dst = &RGB_WS2812_Buffer[half_idx * RGB_WS2812_BITS_PER_LED];

    if (RGB_Lamps_Encoded < RGB_Lamps_To_Update)
    {
        const volatile uint8_t *p = &RGB_Color_Source[RGB_Lamps_Encoded * 3]; // RGBRGB...
        RGB_Control_Encode_RGB(p[0], p[1], p[2], dst);                        // p[0]=R, p[1]=G, p[2]=B
        RGB_Lamps_Encoded++;
    }
    else
    {
        // A full-zero bit, for reset
        for (int i = 0; i < RGB_WS2812_BITS_PER_LED; i++)
            dst[i] = 0;
        RGB_Encoded_Reset_Bits += RGB_WS2812_BITS_PER_LED;
    }
}

void RGB_Control_WS2812B_Reset(void)
{
    TIM_Cmd(TIM1, DISABLE);
    GPIO_ResetBits(RGB_WS2812_PORT, RGB_WS2812_PIN);
    osDelay(1);
}

static void RGB_Control_Show_RGB_Blocking_From_Array(const volatile uint8_t *rgb_bytes, int n)
{
    while (RGB_Update_Busy)
    { /* Wait until last update finish */
        osDelay(1);
    }

    RGB_Update_Busy = 1;
    RGB_Color_Source = rgb_bytes;
    RGB_Lamps_To_Update = n;
    RGB_Lamps_Encoded = 0;
    RGB_Encoded_Reset_Bits = 0;

    for (size_t i = 0; i < RGB_WS2812_BUFFER_SIZE; i++)
    {
        // Send 2 empty bytes first
        // No preloading since timing issues
        RGB_WS2812_Buffer[i] = 0;
    }

    // Starting sequence
    DMA_ClearFlag(DMA1_FLAG_GL5);
    DMA_SetCurrDataCounter(DMA1_Channel5, RGB_WS2812_BITS_PER_LED * sizeof(uint16_t));
    TIM_SetCompare1(TIM1, RGB_WS2812_Buffer[0]);
    TIM_SetCounter(TIM1, 0);
    TIM_GenerateEvent(TIM1, TIM_EventSource_Update);

    DMA_Cmd(DMA1_Channel5, ENABLE);
    TIM_DMACmd(TIM1, TIM_DMA_Update, ENABLE); // MUST use update & DMA1 CH5 or serious timing issues occurs
    TIM_Cmd(TIM1, ENABLE);

    // wait until all LEDs sent (or scheduled to send) and reached reset slot
    while (RGB_Lamps_Encoded < RGB_Lamps_To_Update || RGB_Encoded_Reset_Bits < RGB_WS2812_RESET_CYCLES)
    {
        osThreadYield();
    }

    // Wait until the last reset bit sent
    uint32_t isr0 = DMA1->ISR;
    for (;;)
    {
        uint32_t isr1 = DMA1->ISR;
        if ((isr1 ^ isr0) & (DMA_ISR_HTIF5 | DMA_ISR_TCIF5))
            break;
    }

    // Off sequence
    TIM_Cmd(TIM1, DISABLE);
    TIM_DMACmd(TIM1, TIM_DMA_Update, DISABLE);
    DMA_Cmd(DMA1_Channel5, DISABLE);

    RGB_Update_Busy = 0;
}

void RGB_Control_thread(const void *dummy)
{
    osEvent evt;

    RGB_Update_Msg_Queue = osMessageCreate(osMessageQ(RGB_Update_Msg_Queue), NULL); // create msg queue

    RGB_Control_WS2812B_Reset();

    while (1)
    {
        evt = osMessageGet(RGB_Update_Msg_Queue, osWaitForever);
        if (evt.status == osEventMessage)
        {
            RGB_Control_Show_RGB_Blocking_From_Array(RGB_Lamp_Colors, RGB_LAMP_TOTAL_COUNT);
        }
    }
}

void RGB_Control_Set_Autonomous_Mode(uint8_t channel, int autonomous_on) { RGB_Autonomous_Mode = autonomous_on; }
uint8_t RGB_Control_get_Autonomous_Mode(uint8_t channel) { return RGB_Autonomous_Mode; }
