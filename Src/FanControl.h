#ifndef _FAN_CONTROL_H
#define _FAN_CONTROL_H

#define SYSTEM_FAN_COUNT			4
#define SYSTEM_FAN_PPR				2		// <- Count only one edge
#define SYSTEM_FAN_HALL_TYPE	uint16_t
#define SYSTEM_FAN_HALL_SIZE	sizeof(uint16_t)
#define SYSTEM_FAN_RPM_TYPE	uint32_t
#define SYSTEM_FAN_RPM_SIZE	sizeof(uint32_t)

#include <stdint.h>

extern SYSTEM_FAN_HALL_TYPE Fan_Hall_Count[SYSTEM_FAN_COUNT];
extern SYSTEM_FAN_RPM_TYPE Fan_RPM_Count[SYSTEM_FAN_COUNT];

void Fan_Control_Initialize(void);
SYSTEM_FAN_RPM_TYPE Fan_Control_Get_RPM(uint8_t fan_id);
void Fan_Control_Set_Level(uint8_t fan_id, uint16_t level_x10);

#endif
