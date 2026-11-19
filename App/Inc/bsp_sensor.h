/* ==================== bsp_sensor.h ==================== */
#ifndef __BSP_SENSOR_H
#define __BSP_SENSOR_H

#include "config.h"

void     Sensor_Init(ADC_HandleTypeDef *hadc);
uint16_t Sensor_ReadSoil(void);
uint16_t Sensor_ReadLight(void);
void     Sensor_ReadAll(SensorData_t *data);

#endif
