/* ======================== config.h ======================== */
#ifndef __CONFIG_H
#define __CONFIG_H

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

/* ---------- 版本 ---------- */
#define FW_VERSION          "v2.0-TFT"

/* ---------- 采样周期 ---------- */
#define SAMPLE_INTERVAL_MS  200U   /* 每 200 ms 采样一次 */

/* ---------- EEPROM ---------- */
#define EEPROM_I2C_ADDR     0xA0
#define EEPROM_DATA_START   0x10
#define EEPROM_SIZE         256      /* AT24C02 = 256 Bytes */
#define RECORD_SIZE         6        /* 每条记录 6 字节 */

/* ---------- 引脚宏 ---------- */
/* 继电器（低电平触发）——已避开 FSMC 占用的 PD 组 */
#define PUMP_PORT           GPIOC
#define PUMP_PIN            GPIO_PIN_0
#define FAN_PORT            GPIOC
#define FAN_PIN             GPIO_PIN_1
#define LIGHT_PORT          GPIOC
#define LIGHT_PIN           GPIO_PIN_2

/* 蜂鸣器（高电平触发） */
#define BEEP_PORT           GPIOB
#define BEEP_PIN            GPIO_PIN_8

/* LED */
#define LED0_PORT           GPIOB
#define LED0_PIN            GPIO_PIN_5
#define LED1_PORT           GPIOE
#define LED1_PIN            GPIO_PIN_5

/* DHT22 */
#define DHT22_PORT          GPIOG
#define DHT22_PIN           GPIO_PIN_11

/* 按键 */
#define KEY0_PORT           GPIOE
#define KEY0_PIN            GPIO_PIN_4
#define KEY1_PORT           GPIOE
#define KEY1_PIN            GPIO_PIN_3
#define KEY2_PORT           GPIOE
#define KEY2_PIN            GPIO_PIN_2
#define KEYUP_PORT          GPIOA
#define KEYUP_PIN           GPIO_PIN_0

/* LCD 背光 */
#define LCD_BL_PORT         GPIOB
#define LCD_BL_PIN          GPIO_PIN_0

/* ---------- ADC 通道 ---------- */
#define ADC_CH_SOIL         ADC_CHANNEL_1   /* PA1 */
#define ADC_CH_LIGHT        ADC_CHANNEL_2   /* PA2 */

/* ---------- 数据结构 ---------- */
typedef struct {
    float    temperature;   /* °C，来自 DHT22 */
    float    humidity;      /* %RH，来自 DHT22 */
    uint16_t soil;          /* 0~4095，ADC 原始值 */
    uint16_t light;         /* 0~4095，ADC 原始值 */
    uint32_t timestamp;     /* RTC 秒戳或采样计数 */
} SensorData_t;

typedef struct {
    float    temp_high;     /* 温度上限（℃），超过则开风扇 */
    float    hum_low;       /* 湿度下限（%），低于则开水泵 */
    uint16_t soil_dry;      /* 土壤干燥阈值（当前 ADC 极性下，高于则浇水） */
    uint16_t light_low;     /* 光照不足阈值（当前 ADC 极性下，高于则补光） */
} Thresholds_t;

typedef enum {
    MODE_AUTO = 0,
    MODE_MANUAL,
    MODE_SETTING,
    MODE_VIEW_LOG
} SystemMode_t;

/* ---------- 默认阈值 ---------- */
#define DEFAULT_TEMP_HIGH   25.0f
#define DEFAULT_HUM_LOW     30.0f
#define DEFAULT_SOIL_DRY    2800U
#define DEFAULT_LIGHT_LOW   3000U

/* ---------- TFTLCD 颜色定义（RGB565） ---------- */
#define COLOR_WHITE         0xFFFF
#define COLOR_BLACK         0x0000
#define COLOR_RED           0xF800
#define COLOR_GREEN         0x07E0
#define COLOR_BLUE          0x001F
#define COLOR_YELLOW        0xFFE0
#define COLOR_CYAN          0x07FF
#define COLOR_ORANGE        0xFD20
#define COLOR_GRAY          0x8410
#define COLOR_DARKGREEN     0x03E0
#define COLOR_LIGHTBLUE     0xAD7F
#define COLOR_BG            0x0000   /* 深色背景 */
#define COLOR_PANEL_BG      0x18E3   /* 面板底色（深灰蓝） */

/* ---------- TFTLCD 屏幕尺寸 ---------- */
#define LCD_W               480
#define LCD_H               800

#endif /* __CONFIG_H */
