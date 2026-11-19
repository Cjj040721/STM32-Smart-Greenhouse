/* ==================== bsp_dht22.c ==================== */
#include "bsp_dht22.h"
#include "bsp_delay_us.h"

static void DHT22_SetOutput(void)
{
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin   = DHT22_PIN;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT22_PORT, &gpio);
}

static void DHT22_SetInput(void)
{
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin  = DHT22_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DHT22_PORT, &gpio);
}

static uint8_t DHT22_ReadPin(void)
{
    return (uint8_t)HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN);
}

uint8_t DHT22_Read(float *temperature, float *humidity)
{
    uint8_t data[5] = {0};
    uint16_t timeout;
    uint8_t i, j;

    /* 1) 主机起始信号 */
    DHT22_SetOutput();
    HAL_GPIO_WritePin(DHT22_PORT, DHT22_PIN, GPIO_PIN_RESET);
    HAL_Delay(2);
    HAL_GPIO_WritePin(DHT22_PORT, DHT22_PIN, GPIO_PIN_SET);
    Delay_us(30);

    /* 2) 切换输入，等传感器回应 */
    DHT22_SetInput();

    timeout = 200;
    while (DHT22_ReadPin() && --timeout) Delay_us(1);
    if (!timeout) return 0;

    timeout = 200;
    while (!DHT22_ReadPin() && --timeout) Delay_us(1);
    if (!timeout) return 0;

    timeout = 200;
    while (DHT22_ReadPin() && --timeout) Delay_us(1);
    if (!timeout) return 0;

    /* 3) 读取 40 bit */
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 8; j++) {
            timeout = 200;
            while (!DHT22_ReadPin() && --timeout) Delay_us(1);
            Delay_us(40);
            data[i] <<= 1;
            if (DHT22_ReadPin()) data[i] |= 0x01;
            timeout = 200;
            while (DHT22_ReadPin() && --timeout) Delay_us(1);
        }
    }

    /* 4) 校验和 */
    if (((data[0] + data[1] + data[2] + data[3]) & 0xFF) != data[4])
        return 0;

    /* 5) 解析 */
    *humidity    = ((uint16_t)(data[0] << 8) | data[1]) / 10.0f;
    *temperature = ((uint16_t)((data[2] & 0x7F) << 8) | data[3]) / 10.0f;
    if (data[2] & 0x80) *temperature = -(*temperature);

    return 1;
}
