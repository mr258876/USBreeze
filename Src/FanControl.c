#include "FanControl.h"
#include <string.h>
#include "stm32f10x_tim.h"

SYSTEM_FAN_HALL_TYPE Fan_Hall_Count[SYSTEM_FAN_COUNT];
SYSTEM_FAN_RPM_TYPE Fan_RPM_Count[SYSTEM_FAN_COUNT];

static void Fan_Contorol_Set_Level_TIM2(uint8_t ch, uint16_t level_x10);

void Fan_Control_Initialize(void)
{
		memset(Fan_Hall_Count, 0, SYSTEM_FAN_COUNT * SYSTEM_FAN_HALL_SIZE);
		memset(Fan_RPM_Count, 0, SYSTEM_FAN_COUNT * SYSTEM_FAN_RPM_SIZE);
}

SYSTEM_FAN_RPM_TYPE Fan_Control_Get_RPM(uint8_t fan_id)
{
		return (fan_id < SYSTEM_FAN_COUNT) ? Fan_RPM_Count[fan_id] : 0;
}

void Fan_Control_Set_Level(uint8_t fan_id, uint16_t level_x10)
{
		if (fan_id < 4) Fan_Contorol_Set_Level_TIM2(fan_id, level_x10);
}

static void Fan_Contorol_Set_Level_TIM2(uint8_t ch, uint16_t level_x10)
{		
		uint16_t compare = level_x10;
		if (level_x10 > 999) compare = 999;
		
		switch (ch)
		{
				case 0:
						TIM_SetCompare4(TIM2, compare);
						break;
				case 1:
						TIM_SetCompare3(TIM2, compare);
						break;
				case 2:
						TIM_SetCompare2(TIM2, compare);
						break;
				case 3:
						TIM_SetCompare1(TIM2, compare);
						break;
		}
}
