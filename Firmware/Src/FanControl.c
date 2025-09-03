/*
 * Copyright (c) 2025 mr258876
 * SPDX-License-Identifier: MIT
 */

#include "FanControl.h"
#include "ParamStorageWarpper.h"
#include "HostCommWarpper.h"
#include <string.h>
#include "stm32f10x_tim.h"
#include "stm32f10x_adc.h"

SYSTEM_FAN_HALL_TYPE Fan_Hall_Count[SYSTEM_FAN_COUNT];
SYSTEM_FAN_RPM_TYPE Fan_RPM_Count[SYSTEM_FAN_COUNT];
SYSTEM_FAN_LEVEL_TYPE Fan_Control_Levels[SYSTEM_FAN_COUNT];
SYSTEM_TEMP_LEVEL_TYPE Fan_Control_Temperature[SYSTEM_TEMP_SENSOR_COUNT];
FanCurvePointValue Fan_Control_Curves[SYSTEM_FAN_COUNT][SYSTEM_MAX_CURVE_POINTS];
FanCurveCfgValue Fan_Control_Curve_Cfgs[SYSTEM_FAN_COUNT];

static void Fan_Control_Set_Level_TIM2(uint8_t ch, uint16_t level_x10);
static void Fan_Control_Set_Level_TIM3(uint8_t ch, uint16_t level_x10);
static void Fan_Control_Calc_Fan_Level(void);
static void Fan_Control_Update_Fan_Level(void);
static void Fan_Control_Update_PRM(void);
static SYSTEM_TEMP_LEVEL_TYPE Fan_Control_Read_Internal_Temperature(void);

void Fan_Control_Initialize(void)
{
	for (size_t i = 0; i < SYSTEM_FAN_COUNT; i++)
	{
		Fan_Hall_Count[i] = 0;
		Fan_RPM_Count[i] = 0;
		Fan_Control_Levels[i] = 500;

		{
			FanCurveCfgValue val = {SYSTEM_MAX_CURVE_POINTS, 0};
			Fan_Control_Curve_Cfgs[i] = val;
		}
		for (size_t j = 0; j < SYSTEM_MAX_CURVE_POINTS; j++)
		{
			FanCurvePointValue val = {250 * j, 250 * j};
			Fan_Control_Curves[i][j] = val;
		}
	}

	for (size_t i = 0; i < SYSTEM_TEMP_SENSOR_COUNT; i++)
	{
		Fan_Control_Temperature[i] = 25;
	}

	Fan_Control_Load_Params();
}

SYSTEM_FAN_RPM_TYPE Fan_Control_Get_RPM(uint8_t fan_id)
{
	return (fan_id < SYSTEM_FAN_COUNT) ? Fan_RPM_Count[fan_id] : 0;
}

void Fan_Control_Set_Level(uint8_t fan_id, uint16_t level)
{
	if ((level & 0x7FFF) > 1000)
		level = (level & FAN_LEVEL_OVERRIDE_BY_HOST) | 1000;

	Fan_Control_Levels[fan_id] = level;
}

static void Fan_Control_Calc_Fan_Level(void)
{
	for (size_t i = 0; i < SYSTEM_FAN_COUNT; i++)
	{
		if (Fan_Control_Levels[i] & FAN_LEVEL_OVERRIDE_BY_HOST)
			continue;

		const int16_t current_temp = Fan_Control_Temperature[Fan_Control_Curve_Cfgs[i].TempSensorId];
		for (size_t j = 0; j < Fan_Control_Curve_Cfgs[i].CurvePointCountTotal; j++)
		{
			if (current_temp < (Fan_Control_Curves[i][j].CurvePointTemp))
			{
				if (j == 0) // Temperature lower than the first point
				{
					Fan_Control_Levels[i] = (Fan_Control_Curves[i][j].CurvePointPWM);
					break;
				}

				const int32_t d_pwm = Fan_Control_Curves[i][j].CurvePointPWM - Fan_Control_Curves[i][j - 1].CurvePointPWM;
				const int32_t d_temp = Fan_Control_Curves[i][j].CurvePointTemp - Fan_Control_Curves[i][j - 1].CurvePointTemp;

				if (d_temp <= 0)
					Fan_Control_Levels[i] = Fan_Control_Curves[i][j - 1].CurvePointPWM;

				Fan_Control_Levels[i] = d_pwm * (current_temp - Fan_Control_Curves[i][j - 1].CurvePointTemp) / d_temp + Fan_Control_Curves[i][j - 1].CurvePointPWM;
				break;
			}

			if (j == Fan_Control_Curve_Cfgs[i].CurvePointCountTotal - 1) // Temperature higher than the last point
			{
				Fan_Control_Levels[i] = (Fan_Control_Curves[i][j].CurvePointPWM);
				break;
			}
		}
	}
}

static void Fan_Control_Update_Fan_Level(void)
{
	for (size_t fan_id = 0; fan_id < SYSTEM_FAN_COUNT; fan_id++)
	{
		if (fan_id < 4)
			Fan_Control_Set_Level_TIM2(fan_id, Fan_Control_Levels[fan_id] & 0x7FFF);
		else if (fan_id < 8)
			Fan_Control_Set_Level_TIM3(fan_id - 4, Fan_Control_Levels[fan_id] & 0x7FFF);
	}
}

static void Fan_Control_Set_Level_TIM2(uint8_t ch, uint16_t level_x10)
{
	if (level_x10 >= 1000)
		level_x10 = 1000;

	level_x10 = 1000 - level_x10; // control singal is reversed

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
	if (level_x10 > 1000)
		level_x10 = 1000;

	level_x10 = 1000 - level_x10; // control singal is reversed

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
		Fan_RPM_Count[i] = (Fan_Hall_Count[i] * 60 * (1000 / SYSTEM_UPDATE_INTERVAL_MS)) / SYSTEM_FAN_PPR; // sampled in 0.5s
		Fan_Hall_Count[i] = 0;
	}
}

static uint16_t ADC1_ReadOnce(uint8_t channel)
{
	ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_239Cycles5);
	ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET)
		;
	return ADC_GetConversionValue(ADC1);
}

static uint16_t ADC1_ReadAverage(uint8_t channel, uint8_t times)
{
	uint32_t sum = 0;
	for (uint8_t i = 0; i < times; i++)
		sum += ADC1_ReadOnce(channel);
	return (uint16_t)(sum / times);
}

#define ADC_AVG_SAMPLES 10
#define V25_MV 1430			// 25°C temp sensor voltage (mV)
#define AVG_SLOPE_10X 43	// 4.3 mV/°C
#define VREFINT_TYP_MV 1200 // internal typical voltage (mV)

static uint16_t Get_VDDA_mV(void)
{
	uint16_t adc_vref = ADC1_ReadAverage(ADC_Channel_17, ADC_AVG_SAMPLES);
	if (adc_vref == 0)
		return 3300;
	// (1200 * 4096 + adc_vref/2) / adc_vref
	uint32_t num = (uint32_t)VREFINT_TYP_MV * 4096u + (adc_vref >> 1);
	return (uint16_t)(num / adc_vref);
}

SYSTEM_TEMP_LEVEL_TYPE Fan_Control_Read_Internal_Temperature(void)
{
	uint16_t vdda_mV = Get_VDDA_mV();
	uint16_t adc_ts = ADC1_ReadAverage(ADC_Channel_16, ADC_AVG_SAMPLES);

	// Vsense(mV) = adc * VDDA / 4096, +2048 to round
	uint32_t vsense_mV = ((uint32_t)adc_ts * vdda_mV + 2048u) >> 12; // /4096

	// delta = V25 - Vsense (mV)
	int32_t delta_mV = (int32_t)V25_MV - (int32_t)vsense_mV;

	// T(0.1°C) = delta*100/43 + 250, + (AVG_SLOPE_10X/2) to round
	int32_t t_x10 = (delta_mV * 100 + (AVG_SLOPE_10X / 2)) / AVG_SLOPE_10X + 250;
	return (int16_t)t_x10;
}

void Fan_Control_Save_Settings_Flash(void)
{
	Fan_Control_Save_Params();
}

void Fan_Control_Loop(void)
{
	Fan_Control_Update_PRM();

	Fan_Control_Temperature[0] = Fan_Control_Read_Internal_Temperature();

	Fan_Control_Calc_Fan_Level();

	Fan_Control_Update_Fan_Level();

	GPIO_WriteBit(GPIOD, GPIO_Pin_2, !GPIO_ReadOutputDataBit(GPIOD, GPIO_Pin_2));
}
