#ifndef _HOST_COMMUNICATION_H
#define _HOST_COMMUNICATION_H

#include "cmsis_os.h"
#include <stdint.h>

#define PROTO_MAX	512	// Max length of a packet

extern osMessageQId  MsgInBox;

void Host_Communication_Thread(void const* dummy_args);

extern uint8_t  board_get_fan_count(void);            				// Fan count
extern void     board_get_uid(uint32_t uid[3]);       				// STM32 96-bit UID
extern uint8_t  board_set_pwm(uint8_t ch, uint16_t duty_x10); // 0=OK, others=Err
extern uint32_t board_get_rpm(uint8_t ch);            				// get RPM

#endif
