/* ==================== bsp_key.c ==================== */
#include "bsp_key.h"

uint8_t Key_Scan(void)
{
    static uint8_t key_locked = 0;

    if (key_locked == 0) {
        if (HAL_GPIO_ReadPin(KEY0_PORT, KEY0_PIN) == GPIO_PIN_RESET) {
            HAL_Delay(10);
            if (HAL_GPIO_ReadPin(KEY0_PORT, KEY0_PIN) == GPIO_PIN_RESET)
                { key_locked = 1; return KEY_MODE; }
        }
        if (HAL_GPIO_ReadPin(KEY1_PORT, KEY1_PIN) == GPIO_PIN_RESET) {
            HAL_Delay(10);
            if (HAL_GPIO_ReadPin(KEY1_PORT, KEY1_PIN) == GPIO_PIN_RESET)
                { key_locked = 1; return KEY_SELECT; }
        }
        if (HAL_GPIO_ReadPin(KEY2_PORT, KEY2_PIN) == GPIO_PIN_RESET) {
            HAL_Delay(10);
            if (HAL_GPIO_ReadPin(KEY2_PORT, KEY2_PIN) == GPIO_PIN_RESET)
                { key_locked = 1; return KEY_UP; }
        }
        if (HAL_GPIO_ReadPin(KEYUP_PORT, KEYUP_PIN) == GPIO_PIN_SET) {
            HAL_Delay(10);
            if (HAL_GPIO_ReadPin(KEYUP_PORT, KEYUP_PIN) == GPIO_PIN_SET)
                { key_locked = 1; return KEY_DOWN; }
        }
    } else {
        if (HAL_GPIO_ReadPin(KEY0_PORT, KEY0_PIN) == GPIO_PIN_SET &&
            HAL_GPIO_ReadPin(KEY1_PORT, KEY1_PIN) == GPIO_PIN_SET &&
            HAL_GPIO_ReadPin(KEY2_PORT, KEY2_PIN) == GPIO_PIN_SET &&
            HAL_GPIO_ReadPin(KEYUP_PORT, KEYUP_PIN) == GPIO_PIN_RESET)
            key_locked = 0;
    }
    return KEY_NONE;
}
