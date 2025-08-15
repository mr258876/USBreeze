/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

#define osObjectsPublic // define objects in main module
#include "osObjects.h"	// RTOS object definitions
#include "bsp.h"
#include "rl_usb.h"

#include "EventRecorder.h"

#include "FanControl.h"
#include "RGBControl.h"

//osThreadDef(Fan_Control_thread, osPriorityNormal, 1, 0);
osThreadDef(RGB_Control_thread, osPriorityNormal, 1, 0);

/*
 * main: initialize and start the system
 */
int main(void)
{
	EventRecorderInitialize(EventRecordAll, 1); // Initialize and start

	osKernelInitialize(); // initialize CMSIS-RTOS

	// initialize peripherals here
	Fan_Control_Initialize();

	BSP_Initialize();

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

	Fan_Control_thread(NULL);
}
