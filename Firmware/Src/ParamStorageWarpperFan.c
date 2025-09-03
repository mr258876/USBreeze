#include "ParamStorage.h"
#include "ParamStorageKeys.h"
#include "ParamStorageWarpper.h"
#include "FanControl.h"

void Fan_Control_Load_Params(void)
{
    EE_Read(SK_FAN_CONTROL_CURVES_ARRAY, Fan_Control_Curves, SYSTEM_FAN_COUNT * SYSTEM_MAX_CURVE_POINTS * sizeof(FanCurvePointValue));
    EE_Read(SK_FAN_CONTROL_CURVE_POINTS_ARRAY, Fan_Control_Curve_Cfgs, SYSTEM_FAN_COUNT * sizeof(FanCurveCfgValue));
}

void Fan_Control_Save_Params(void)
{
    EE_Write(SK_FAN_CONTROL_CURVES_ARRAY, Fan_Control_Curves, SYSTEM_FAN_COUNT * SYSTEM_MAX_CURVE_POINTS * sizeof(FanCurvePointValue));
    EE_Write(SK_FAN_CONTROL_CURVE_POINTS_ARRAY, Fan_Control_Curve_Cfgs, SYSTEM_FAN_COUNT * sizeof(FanCurveCfgValue));
}
