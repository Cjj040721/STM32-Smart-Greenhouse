/* ==================== bsp_actuator.h ==================== */
#ifndef __BSP_ACTUATOR_H
#define __BSP_ACTUATOR_H

#include "config.h"

void Actuator_Init(void);
void Pump_On(void);
void Pump_Off(void);
void Fan_On(void);
void Fan_Off(void);
void GrowLight_On(void);
void GrowLight_Off(void);
void Beep_On(void);
void Beep_Off(void);
void Alarm_Beep(uint8_t times);

#define ACT_PUMP   0x01
#define ACT_FAN    0x02
#define ACT_LIGHT  0x04
uint8_t Actuator_GetStatus(void);

#endif
