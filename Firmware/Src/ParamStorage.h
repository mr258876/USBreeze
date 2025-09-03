#ifndef _PARAM_STORAGE_H_
#define _PARAM_STORAGE_H_

#include <stdint.h>
#include <stdbool.h>

void EE_Init(void);
uint16_t EE_Read(uint16_t key, void *buf, uint16_t maxlen);
bool EE_Write(uint16_t key, const void *data, uint16_t len);

#endif
