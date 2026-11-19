/*
 * main.c
 *
 *  Created on: 2026年4月20日
 *      Author: wangyubo
 */
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "rtc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "config.h"
#include "bsp_delay_us.h"
#include "bsp_dht22.h"
#include "bsp_sensor.h"
#include "bsp_actuator.h"
#include "bsp_tftlcd.h"
#include "bsp_eeprom.h"
#include "bsp_key.h"
#include "app_comm.h"
#include "app_control.h"
#include "app_ui.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* ========== 全局变量 ========== */
SensorData_t  g_sensor_data = {0};
Thresholds_t  g_thresholds  = {
    DEFAULT_TEMP_HIGH, DEFAULT_HUM_LOW,
    DEFAULT_SOIL_DRY,  DEFAULT_LIGHT_LOW
};
SystemMode_t  g_mode = MODE_AUTO;

volatile uint32_t g_tick_ms     = 0;
volatile uint8_t  g_sample_flag = 0;

/* 声明外部外设句柄，解决未定义报错问题 */
extern ADC_HandleTypeDef   hadc1;
extern I2C_HandleTypeDef   hi2c1;
extern UART_HandleTypeDef  huart1;
extern TIM_HandleTypeDef   htim6;
extern TIM_HandleTypeDef   htim7;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
#include "app_ui.h" // 确保包含了相关的头文件

void Hardware_SelfTest(void)
{
    /* 1. 屏幕与蜂鸣器测试 */
    LCD_Clear(COLOR_BLUE);
    LCD_ShowString(10, 10, "Hardware Test Start...", 24, COLOR_WHITE, COLOR_BLUE);
    Comm_SendString("\r\n--- Hardware Test Start ---\r\n");
    Beep_On();
    HAL_Delay(200);
    Beep_Off();

    /* 2. 继电器执行器测试 (请注意听继电器是否有“吧嗒”声，或看模块指示灯) */
    LCD_ShowString(10, 50, "1. Testing Pump...", 24, COLOR_WHITE, COLOR_BLUE);
    Pump_On(); HAL_Delay(1000); Pump_Off();

    LCD_ShowString(10, 80, "2. Testing Fan...", 24, COLOR_WHITE, COLOR_BLUE);
    Fan_On(); HAL_Delay(1000); Fan_Off();

    LCD_ShowString(10, 110, "3. Testing Light...", 24, COLOR_WHITE, COLOR_BLUE);
    GrowLight_On(); HAL_Delay(1000); GrowLight_Off();

    /* 3. 传感器读取测试 */
    SensorData_t test_data = {0};
    LCD_ShowString(10, 150, "Reading Sensors...", 24, COLOR_YELLOW, COLOR_BLUE);
    HAL_Delay(1000); // 稍微延时，给 DHT22 和 ADC 一点稳定时间
    Sensor_ReadAll(&test_data);

    /* 4. 清屏并显示传感器测试结果 */
    LCD_Clear(COLOR_BLACK);
    LCD_ShowString(10, 10, "Sensor Result:", 24, COLOR_WHITE, COLOR_BLACK);
    LCD_Printf(10, 50, 24, COLOR_GREEN, COLOR_BLACK,  "Temp : %.1f C", test_data.temperature);
    LCD_Printf(10, 90, 24, COLOR_GREEN, COLOR_BLACK,  "Hum  : %.1f %%", test_data.humidity);
    LCD_Printf(10, 130, 24, COLOR_YELLOW, COLOR_BLACK,"Soil : %u (ADC)", test_data.soil);
    LCD_Printf(10, 170, 24, COLOR_YELLOW, COLOR_BLACK,"Light: %u (ADC)", test_data.light);

    /* 将结果也打印到电脑串口 */
    Comm_SendString("Sensor Test Data:\r\n");
    Comm_SendSensorCSV(&test_data);

    LCD_ShowString(10, 230, "Test Finished! Booting...", 16, COLOR_GRAY, COLOR_BLACK);
    HAL_Delay(5000); // 画面停留 5 秒钟供您观察接线数据是否正常
}

int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_FSMC_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_RTC_Init();
  MX_TIM6_Init();
  MX_TIM7_Init();
  MX_USART1_UART_Init();

  /* USER CODE BEGIN 2 */

  /* 应用层初始化 */
  Delay_us_Init(&htim6);
  Sensor_Init(&hadc1);
  Actuator_Init();
  EEPROM_Init(&hi2c1);
  Comm_Init(&huart1);
  LCD_Init();
  UI_DrawDashboard();
  UI_UpdateModeLabel(g_mode);
  /* ★ TFTLCD 初始化 + 绘制仪表盘 ★ */
  /* 临时调试：打印 LCD ID */
  char id_buf[32];
  snprintf(id_buf, sizeof(id_buf), "LCD ID: 0x%04X\r\n", g_lcd_id);
  Comm_SendString(id_buf);  // 通过串口打印出来


  /* 启动心跳定时器 */
  HAL_TIM_Base_Start_IT(&htim7);

  /* 开机首次采样 */
  Sensor_ReadAll(&g_sensor_data);
  UI_UpdateSensorData(&g_sensor_data, &g_thresholds);
  UI_UpdateActuatorStatus(Actuator_GetStatus());
  UI_PushTempCurve(g_sensor_data.temperature);
  Comm_SendSensorCSV(&g_sensor_data);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    /* 1) 按键 + UI */
    UI_Process(&g_mode, &g_thresholds, &g_sensor_data);

    /* 2) 串口命令 */
    Comm_PollCommand(&g_mode);

    /* 3) 定时采样 */
    if (g_sample_flag) {
        g_sample_flag = 0;

        Sensor_ReadAll(&g_sensor_data);

        /* 刷新 TFTLCD 各区域 */
        /* 刷新 TFTLCD 各区域 */
        if (g_mode != MODE_SETTING) {
            UI_UpdateSensorData(&g_sensor_data, &g_thresholds);
            UI_UpdateActuatorStatus(Actuator_GetStatus());
        }
        UI_PushTempCurve(g_sensor_data.temperature);


        /* 串口上报 + EEPROM 记录 */
        Comm_SendSensorCSV(&g_sensor_data);
        EEPROM_LogRecord(&g_sensor_data);

        /* LED0 翻转 */
        HAL_GPIO_TogglePin(LED0_PORT, LED0_PIN);

        /* 自动控制 */
        if (g_mode == MODE_AUTO) {
            Control_Auto(&g_sensor_data, &g_thresholds);
            UI_UpdateActuatorStatus(Actuator_GetStatus());
        }

        /* 报警检测 */
        if (Control_CheckAlarm(&g_sensor_data, &g_thresholds)) {
            HAL_GPIO_WritePin(LED1_PORT, LED1_PIN, GPIO_PIN_RESET);
            Alarm_Beep(3);
            UI_ShowAlarm("! ALARM: Critical !");
            Comm_SendString("[ALARM] Critical!\r\n");
        } else {
            HAL_GPIO_WritePin(LED1_PORT, LED1_PIN, GPIO_PIN_SET);
            UI_ClearAlarm();
        }
    }

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_ADC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enables the Clock Security System
  */
  HAL_RCC_EnableCSS();
}

/* USER CODE BEGIN 4 */
/* ========== TIM7 回调：1ms ========== */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim7) {
        g_tick_ms++;
        if (g_tick_ms >= SAMPLE_INTERVAL_MS) {
            g_tick_ms = 0;
            g_sample_flag = 1;
        }
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
