/* 
 * Copyright (c) 2025 mr258876
 * SPDX-License-Identifier: MIT
 */

#include "bsp.h"
#include "rl_usb.h"

//#include "EventRecorder.h"

#include "FanControl.h"
#include "RGBControl.h"
#include "ParamStorage.h"

#include "stm32f10x_iwdg.h"

//osThreadDef(Fan_Control_thread, osPriorityNormal, 1, 0);
osThreadDef(RGB_Control_thread, osPriorityNormal, 1, 0);

/*
 * main: initialize and start the system
 */
int main(void)
{
	//EventRecorderInitialize(EventRecordAll, 1); // Initialize and start

	osKernelInitialize(); // initialize CMSIS-RTOS

	// initialize peripherals here
	BSP_Initialize();
	
	EE_Init();

	Fan_Control_Initialize();

	GPIO_WriteBit(GPIOD, GPIO_Pin_2, 1);
	
	if (USBD_Initialize(0) != usbOK)
	{
		// USB Init Failed
		// GPIO_WriteBit(GPIOA, GPIO_Pin_8, 0);
	}
	NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 3);
	NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
	USBD_Connect(0);

	//if (osThreadCreate(osThread(Fan_Control_thread), NULL) == NULL)
	//{
		// Thread Create Failed
		// pass
	//}

	if (osThreadCreate(osThread(RGB_Control_thread), NULL) == NULL)
	{
		// Thread Create Failed
		 GPIO_WriteBit(GPIOD, GPIO_Pin_2, 0);
		// pass
	}

	// create 'thread' functions that start executing,
	// example: tid_name = osThreadCreate (osThread(name), NULL);

	osKernelStart(); // start thread execution
	
	uint32_t last_fan_update_tick = osKernelSysTick();
	while (1)
	{
		IWDG_ReloadCounter();
		if (osKernelSysTick() - last_fan_update_tick >= osKernelSysTickMicroSec(1000 * SYSTEM_UPDATE_INTERVAL_MS))
		{
		  Fan_Control_Loop();
			last_fan_update_tick = osKernelSysTick();
		}
		osDelay(25);
	}
}

