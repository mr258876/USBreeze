/*
 * Copyright (c) 2025 mr258876
 * SPDX-License-Identifier: MIT
 */

#include "stm32f10x_it.h"

#include "RGBControl.h"

/**
 * @brief  This function handles GPIO8-GP1O9 interrupt request.
 * @param  None
 * @retval None
 */
#include "FanControl.h"
void EXTI9_5_IRQHandler(void) // PB8-PB9
{
    if (EXTI_GetITStatus(EXTI_Line8) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line8);
        Fan_Hall_Count[3]++;
    }
    if (EXTI_GetITStatus(EXTI_Line9) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line9);
        Fan_Hall_Count[2]++;
    }
}

/**
 * @brief  This function handles GPIO10-GP1O15 interrupt request.
 * @param  None
 * @retval None
 */
void EXTI15_10_IRQHandler(void) // PB10-PB15
{
    if (EXTI_GetITStatus(EXTI_Line10) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line10);
        Fan_Hall_Count[1]++;
    }
    if (EXTI_GetITStatus(EXTI_Line11) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line11);
        Fan_Hall_Count[0]++;
    }
    if (EXTI_GetITStatus(EXTI_Line12) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line12);
        Fan_Hall_Count[7]++;
    }
    if (EXTI_GetITStatus(EXTI_Line13) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line13);
        Fan_Hall_Count[6]++;
    }
    if (EXTI_GetITStatus(EXTI_Line14) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line14);
        Fan_Hall_Count[5]++;
    }
    if (EXTI_GetITStatus(EXTI_Line15) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line15);
        Fan_Hall_Count[4]++;
    }
}

/**
 * @brief  This function handles DMA1 CH5 interrupt request for WS2812 Control.
 * @param  None
 * @retval None
 */
void DMA1_Channel5_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA1_IT_HT5))
    { // Former half buffer sent. Start reload
        DMA_ClearITPendingBit(DMA1_IT_HT5);
        RGB_Control_Fill_Half_Buffer(0);
    }
    if (DMA_GetITStatus(DMA1_IT_TC5))
    { // Latter half buffer sent. Start reload
        DMA_ClearITPendingBit(DMA1_IT_TC5);
        RGB_Control_Fill_Half_Buffer(1);
    }
}

/**
 * @}
 */
