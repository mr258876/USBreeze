/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions
#include "FanControl.h"
#include "bsp.h"
#include "rl_usb.h"

#include "EventRecorder.h"

osThreadDef(Fan_Control_thread, osPriorityNormal, 1, 256);

/*
 * main: initialize and start the system
 */
int main (void) {
		EventRecorderInitialize(EventRecordAll, 1); // Initialize and start
	
		osKernelInitialize ();                    // initialize CMSIS-RTOS
		
		// initialize peripherals here
		Fan_Control_Initialize();
	
		BSP_Initialize();
		
		// GPIO_WriteBit(GPIOA, GPIO_Pin_8, 1);
		GPIO_WriteBit(GPIOD, GPIO_Pin_2, 1);
		
		if (USBD_Initialize(0) != usbOK)
		{
				// USB Init Failed
				//GPIO_WriteBit(GPIOA, GPIO_Pin_8, 0);
		}
		USBD_Connect(0);
		
		if (osThreadCreate(osThread(Fan_Control_thread), NULL) == NULL)
		{
				// Thread Create Failed
				// pass
		}
		
		// create 'thread' functions that start executing,
		// example: tid_name = osThreadCreate (osThread(name), NULL);

		osKernelStart ();                         // start thread execution 
	

		
		while (1)
		{
				GPIO_WriteBit(GPIOD , GPIO_Pin_2, !GPIO_ReadOutputDataBit(GPIOD , GPIO_Pin_2));
				osDelay(1000);
		}
}
