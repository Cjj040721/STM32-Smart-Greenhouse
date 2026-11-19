/* ==================== bsp_delay_us.h ==================== */
#ifndef __BSP_DELAY_US_H
#define __BSP_DELAY_US_H
#include "stm32f1xx_hal.h"

void Delay_us_Init(TIM_HandleTypeDef *htim);
void Delay_us(uint16_t us);

#endif
