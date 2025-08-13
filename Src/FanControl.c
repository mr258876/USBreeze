#include "FanControl.h"
#include "HostCommWarpper.h"
#include <string.h>
#include "stm32f10x_tim.h"

SYSTEM_FAN_HALL_TYPE Fan_Hall_Count[SYSTEM_FAN_COUNT];
SYSTEM_FAN_RPM_TYPE Fan_RPM_Count[SYSTEM_FAN_COUNT];
SYSTEM_FAN_LEVEL_TYPE Fan_Control_Levels[SYSTEM_FAN_COUNT];

static void Fan_Control_Set_Level_TIM2(uint8_t ch, uint16_t level_x10);
static void Fan_Control_Set_Level_TIM3(uint8_t ch, uint16_t level_x10);
static void Fan_Control_Update_PRM(void);

void Fan_Control_Initialize(void)
{
	memset(Fan_Hall_Count, 0, SYSTEM_FAN_COUNT * sizeof(SYSTEM_FAN_HALL_TYPE));
	memset(Fan_RPM_Count, 0, SYSTEM_FAN_COUNT * sizeof(SYSTEM_FAN_RPM_TYPE));
	memset(Fan_Control_Levels, 0, SYSTEM_FAN_COUNT * sizeof(SYSTEM_FAN_LEVEL_TYPE));
}

SYSTEM_FAN_RPM_TYPE Fan_Control_Get_RPM(uint8_t fan_id)
{
	return (fan_id < SYSTEM_FAN_COUNT) ? Fan_RPM_Count[fan_id] : 0;
}

void Fan_Control_Set_Level(uint8_t fan_id, uint8_t level)
{
	if (level > 100)
		level = 100;

	if (fan_id < 4)
		Fan_Control_Set_Level_TIM2(fan_id, level * 10);
	else if (fan_id < 8)
		Fan_Control_Set_Level_TIM3(fan_id - 4, level * 10);
	else
		return;

	Fan_Control_Levels[fan_id] = level;
}

static void Fan_Control_Set_Level_TIM2(uint8_t ch, uint16_t level_x10)
{
	if (level_x10 > 999)
		level_x10 = 999;

	switch (ch)
	{
	case 0:
		TIM_SetCompare4(TIM2, level_x10);
		break;
	case 1:
		TIM_SetCompare3(TIM2, level_x10);
		break;
	case 2:
		TIM_SetCompare2(TIM2, level_x10);
		break;
	case 3:
		TIM_SetCompare1(TIM2, level_x10);
		break;
	}
}

static void Fan_Control_Set_Level_TIM3(uint8_t ch, uint16_t level_x10)
{
	if (level_x10 > 999)
		level_x10 = 999;

	switch (ch)
	{
	case 0:
		TIM_SetCompare4(TIM3, level_x10);
		break;
	case 1:
		TIM_SetCompare3(TIM3, level_x10);
		break;
	case 2:
		TIM_SetCompare2(TIM3, level_x10);
		break;
	case 3:
		TIM_SetCompare1(TIM3, level_x10);
		break;
	}
}

static void Fan_Control_Update_PRM(void)
{
	for (int i = 0; i < SYSTEM_FAN_COUNT; i++)
	{
		Fan_RPM_Count[i] = (Fan_Hall_Count[i] * 60 * 2) / SYSTEM_FAN_PPR; // sampled in 0.5s
		Fan_Hall_Count[i] = 0;
	}
}

#include "cmsis_os.h"
#include "rl_usb.h"
void Fan_Control_thread(const void *dummy)
{

	while (1)
	{
		Fan_Control_Update_PRM();

		Fan_Control_Notify_Host_RPM();

		osDelay(500);
	}
}
