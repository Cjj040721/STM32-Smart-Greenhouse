/* ==================== app_control.c ==================== */
#include "app_control.h"
#include "bsp_actuator.h"

#define ALARM_TEMP_OFFSET   5.0f
#define ALARM_HUM_OFFSET    15.0f

void Control_Auto(SensorData_t *data, Thresholds_t *thr)
{
    /* 1. 温度控制风扇 */
    if (data->temperature > thr->temp_high)
        Fan_On();
    else
        Fan_Off();

    /* 2. 空气湿度过低或土壤干燥时控制水泵 */
    if (data->humidity < thr->hum_low || data->soil > thr->soil_dry) {
        Pump_On();
    } else {
        Pump_Off();
    }

    /* 3. 光照控制补光灯 */
    if (data->light > thr->light_low)
        GrowLight_On();
    else
        GrowLight_Off();
}

uint8_t Control_CheckAlarm(SensorData_t *data, Thresholds_t *thr)
{
    if (data->temperature > thr->temp_high + ALARM_TEMP_OFFSET) return 1;
    if (data->humidity    < thr->hum_low   - ALARM_HUM_OFFSET)  return 1;
    return 0;
}
