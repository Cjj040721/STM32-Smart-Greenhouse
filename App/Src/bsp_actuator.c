/* ==================== bsp_actuator.c ==================== */
#include "bsp_actuator.h"

static uint8_t g_act_status = 0;

void Actuator_Init(void)
{
    /*
     * 根据硬件测试结果调整初始化电平：
     * 1. 水泵为低电平触发，初始写高电平(SET)关闭
     * 2. 风扇和灯为高电平触发，初始写低电平(RESET)关闭
     */
    HAL_GPIO_WritePin(PUMP_PORT,  PUMP_PIN,  GPIO_PIN_SET);   // 水泵初始关闭
    HAL_GPIO_WritePin(FAN_PORT,   FAN_PIN,   GPIO_PIN_RESET); // 风扇初始关闭 (修改了这里)
    HAL_GPIO_WritePin(LIGHT_PORT, LIGHT_PIN, GPIO_PIN_RESET); // 灯初始关闭 (修改了这里)

    HAL_GPIO_WritePin(BEEP_PORT,  BEEP_PIN,  GPIO_PIN_RESET); // 蜂鸣器初始关闭
    g_act_status = 0;
}

/* ============ 水泵控制（低电平触发，保持不变） ============ */
void Pump_On(void)  { HAL_GPIO_WritePin(PUMP_PORT, PUMP_PIN, GPIO_PIN_RESET); g_act_status |= ACT_PUMP; }
void Pump_Off(void) { HAL_GPIO_WritePin(PUMP_PORT, PUMP_PIN, GPIO_PIN_SET);   g_act_status &= ~ACT_PUMP; }

/* ============ 风扇控制（高电平触发，已反转逻辑） ============ */
void Fan_On(void)   { HAL_GPIO_WritePin(FAN_PORT,  FAN_PIN,  GPIO_PIN_SET);   g_act_status |= ACT_FAN; }
void Fan_Off(void)  { HAL_GPIO_WritePin(FAN_PORT,  FAN_PIN,  GPIO_PIN_RESET); g_act_status &= ~ACT_FAN; }

/* ============ 补光灯控制（高电平触发，已反转逻辑） ============ */
void GrowLight_On(void)  { HAL_GPIO_WritePin(LIGHT_PORT, LIGHT_PIN, GPIO_PIN_SET);   g_act_status |= ACT_LIGHT; }
void GrowLight_Off(void) { HAL_GPIO_WritePin(LIGHT_PORT, LIGHT_PIN, GPIO_PIN_RESET); g_act_status &= ~ACT_LIGHT; }

/* ============ 蜂鸣器控制 ============ */
void Beep_On(void)  { HAL_GPIO_WritePin(BEEP_PORT, BEEP_PIN, GPIO_PIN_SET); }
void Beep_Off(void) { HAL_GPIO_WritePin(BEEP_PORT, BEEP_PIN, GPIO_PIN_RESET); }

void Alarm_Beep(uint8_t times)
{
    /* 注意：这里的 HAL_Delay 在报警时会稍微卡顿主循环。
       如果后续发现报警时按键不灵敏，可以考虑把这里的延时去掉，改为非阻塞控制 */
    for (uint8_t i = 0; i < times; i++) {
        Beep_On(); HAL_Delay(150);
        Beep_Off(); HAL_Delay(150);
    }
}

uint8_t Actuator_GetStatus(void) { return g_act_status; }
