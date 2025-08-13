/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.6.0
  * @date    20-September-2021
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2011 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"

#include "RGBControl.h"

/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
#ifndef RTE_CMSIS_RTOS_RTX
void SVC_Handler(void)
{
}
#endif

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
#ifndef RTE_CMSIS_RTOS_RTX
void PendSV_Handler(void)
{
}
#endif

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
#ifndef RTE_CMSIS_RTOS_RTX
void SysTick_Handler(void)
{
}
#endif
/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @brief  This function handles GPIO8-GP1O9 interrupt request.
  * @param  None
  * @retval None
  */
#include "FanControl.h"
void EXTI9_5_IRQHandler(void)// PB8-PB9
{ 
		if(EXTI_GetITStatus(EXTI_Line8)!=RESET){
        EXTI_ClearITPendingBit(EXTI_Line8);
        Fan_Hall_Count[3]++;
    }
		if(EXTI_GetITStatus(EXTI_Line9)!=RESET){
        EXTI_ClearITPendingBit(EXTI_Line9);
        Fan_Hall_Count[2]++;
    }
}

/**
  * @brief  This function handles GPIO10-GP1O15 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI15_10_IRQHandler(void)// PB10-PB15
{ 
		if(EXTI_GetITStatus(EXTI_Line10)!=RESET){
        EXTI_ClearITPendingBit(EXTI_Line10);
        Fan_Hall_Count[1]++;
    }
		if(EXTI_GetITStatus(EXTI_Line11)!=RESET){
        EXTI_ClearITPendingBit(EXTI_Line11);
        Fan_Hall_Count[0]++;
    }
    if(EXTI_GetITStatus(EXTI_Line12)!=RESET){
        EXTI_ClearITPendingBit(EXTI_Line12);
        Fan_Hall_Count[7]++;
    }
    if(EXTI_GetITStatus(EXTI_Line13)!=RESET){
        EXTI_ClearITPendingBit(EXTI_Line13);
        Fan_Hall_Count[6]++;
    }
		if(EXTI_GetITStatus(EXTI_Line14)!=RESET){
        EXTI_ClearITPendingBit(EXTI_Line14);
        Fan_Hall_Count[5]++;
    }
		if(EXTI_GetITStatus(EXTI_Line15)!=RESET){
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
    if (DMA_GetITStatus(DMA1_IT_HT5)) {     // Former half buffer sent. Start reload
        DMA_ClearITPendingBit(DMA1_IT_HT5);
        RGB_Control_Fill_Half_Buffer(0);
    }
    if (DMA_GetITStatus(DMA1_IT_TC5)) {     // Latter half buffer sent. Start reload
        DMA_ClearITPendingBit(DMA1_IT_TC5);
        RGB_Control_Fill_Half_Buffer(1);
    }
}
// void DMA1_Channel2_IRQHandler(void)
// {
//     uint32_t isr = DMA1->ISR;

//     if (isr & (DMA_ISR_HTIF2 | DMA_ISR_TCIF2)) {
//         // 先一次性清掉 HT/TC，避免重复进来
//         DMA1->IFCR = DMA_IFCR_CHTIF2 | DMA_IFCR_CTCIF2;

//         // 追赶循环：最多补两半区（防止我们落后 DMA 一半区以上）
//         for (int k = 0; k < 2; ++k) {
//             // CNDTR = 本轮剩余传输数（0..48）
//             uint16_t ndtr = DMA1_Channel2->CNDTR;

//             // 若 ndtr > 24，DMA 仍在用“前半区”（0..23），我们就去写“后半区”（1）
//             // 否则 DMA 在用“后半区”（24..47），我们就去写“前半区”（0）
//             int safe_half = (ndtr > RGB_WS2812_BITS_PER_LED) ? 1 : 0;
//             RGB_Control_Fill_Half_Buffer(safe_half);

//             // 如果真的什么都不用再填了，可提前退出
//             // if (RGB_Lamps_Encoded >= RGB_Lamps_To_Update &&
//             //     RGB_Encoded_Reset_Bits >= RGB_WS2812_RESET_CYCLES) {
//             //     break;
//             // }
//         }
//     }
// }

/**
  * @}
  */ 
