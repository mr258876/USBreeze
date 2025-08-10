#include "bsp.h"

static void GPIO_Initialize(void);
static void GPIO_EXTI_Initialize(void);
static void TIM2_Initialize(void);
static void TIM3_Initialize(void);

void BSP_Initialize(void)
{
		GPIO_Initialize();
		GPIO_EXTI_Initialize();
		TIM2_Initialize();
		TIM3_Initialize();
		//RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
}


static void GPIO_Initialize(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
	
		/* Enable GPIO Periph */
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
		/* GPIO A8 LED */
		/*GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);*/
	
		/* GPIO D2 LED */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOD, &GPIO_InitStructure);
	
		/* GPIO A0-A3 for TIM2 CH1-4 PWM Out*/
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		GPIO_ResetBits(GPIOA, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3);
		
		/* GPIO B12-B15 for PWM In*/
		GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		GPIO_ResetBits(GPIOB, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
}

static void TIM2_Initialize(void)
{
		TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
		TIM_OCInitTypeDef TIM_OCInitStructure;
		
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
		TIM_InternalClockConfig(TIM2);
		
		/* TIM2 Config */
		TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Period = 1000 - 1;		// PWM Freq 1kHz	TimeOut = ((Prescaler + 1) * (Period + 1)) / TimeClockFreq
    TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;		// Timer Freq 1MHz
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 		
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
		
		/* TIM2 PWM Config */
		TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
		TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
		TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
		TIM_OCInitStructure.TIM_Pulse = 500;
		
		TIM_OC1Init(TIM2, &TIM_OCInitStructure);
		TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
		TIM_OC2Init(TIM2, &TIM_OCInitStructure);
		TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);
		TIM_OC3Init(TIM2, &TIM_OCInitStructure);
		TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);
		TIM_OC4Init(TIM2, &TIM_OCInitStructure);
		TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);
		
		TIM_Cmd(TIM2, ENABLE);
}

static void TIM3_Initialize(void)
{
		TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
		NVIC_InitTypeDef NVIC_InitStructure;
		
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
		/* TIM3 Config, Calc RPM Every 500ms */
		TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Prescaler   = 7200-1;   // 72MHz/7200=10kHz
    TIM_TimeBaseStructure.TIM_Period      = 5000-1;   // TimeOut = ((Prescaler + 1) * (Period + 1)) / TimeClockFreq = 0.5s
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);
		
		/* Set TIM3 Interrupt */
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
		
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
    NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    TIM_Cmd(TIM3,ENABLE);
}

static void GPIO_EXTI_Initialize(void)
{
		EXTI_InitTypeDef EXTI_InitStructure;
		NVIC_InitTypeDef NVIC_InitStructure;
		
		/* Config Interrupts for B12-B15 */
		GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource12);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource13);
		GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource14);
		GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource15);
		
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStructure.EXTI_Trigger=EXTI_Trigger_Falling;
		EXTI_InitStructure.EXTI_LineCmd=ENABLE;
		
		EXTI_InitStructure.EXTI_Line=EXTI_Line12;
		EXTI_Init(&EXTI_InitStructure);
		EXTI_InitStructure.EXTI_Line=EXTI_Line13;
		EXTI_Init(&EXTI_InitStructure);
		EXTI_InitStructure.EXTI_Line=EXTI_Line14;
		EXTI_Init(&EXTI_InitStructure);
		EXTI_InitStructure.EXTI_Line=EXTI_Line15;
		EXTI_Init(&EXTI_InitStructure);
	
    NVIC_InitStructure.NVIC_IRQChannel=EXTI15_10_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
		NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
		NVIC_Init(&NVIC_InitStructure);
}
