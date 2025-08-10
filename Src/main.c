/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions
#include "FanControl.h"
#include "bsp.h"
#include "rl_usb.h"
#include "Hostcommunication.h"

//osThreadDef(Host_Communication_Thread, osPriorityNormal, 1, 512U);

/*
 * main: initialize and start the system
 */
int main (void) {
		osKernelInitialize ();                    // initialize CMSIS-RTOS
		
		// initialize peripherals here
		Fan_Control_Initialize();
	
		BSP_Initialize();
		
		//GPIO_WriteBit(GPIOA, GPIO_Pin_8, 1);
		GPIO_WriteBit(GPIOD, GPIO_Pin_2, 1);
		
		//if (osThreadCreate(osThread(Host_Communication_Thread), NULL) == NULL)
		{
				// Thread Create Failed
				// pass
		}
		
		if (USBD_Initialize(0) != usbOK)
		{
				// USB Init Failed
				//GPIO_WriteBit(GPIOA, GPIO_Pin_8, 0);
		}
		USBD_Connect(0);
		
		// create 'thread' functions that start executing,
		// example: tid_name = osThreadCreate (osThread(name), NULL);

		osKernelStart ();                         // start thread execution 
	

		
		while (1)
		{
				GPIO_WriteBit(GPIOD , GPIO_Pin_2, !GPIO_ReadOutputDataBit(GPIOD , GPIO_Pin_2));
				osDelay(1000);
		}
}
