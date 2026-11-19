/* ==================== bsp_delay_us.c ==================== */
#include "bsp_delay_us.h"

static TIM_HandleTypeDef *pHTim = NULL;

/* CubeMX 配置 TIM6：Prescaler = 71, Period = 0xFFFF
   → 计数频率 = 72MHz / (71+1) = 1 MHz，即 1 tick = 1μs */
void Delay_us_Init(TIM_HandleTypeDef *htim)
{
    pHTim = htim;
    HAL_TIM_Base_Start(pHTim);
}

void Delay_us(uint16_t us)
{
    __HAL_TIM_SET_COUNTER(pHTim, 0);
    while (__HAL_TIM_GET_COUNTER(pHTim) < us);
}
