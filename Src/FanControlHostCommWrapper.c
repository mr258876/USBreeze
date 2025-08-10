#include "FanControl.h"
#include "HostCommunication.h"

// Get Board ID
void board_get_uid(uint32_t uid[3])
{
}

// Fan count
uint8_t board_get_fan_count(void)
{
	return 4;
}

// Set Level
uint8_t board_set_pwm(uint8_t ch, uint16_t duty_x10)
{
	if (ch >= SYSTEM_FAN_COUNT) { return 1; }
	Fan_Control_Set_Level(ch, duty_x10);
	return 0;
}

// Get RPM
uint32_t board_get_rpm(uint8_t ch)
{
	return Fan_Control_Get_RPM(ch);
}
