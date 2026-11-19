/* ==================== bsp_sensor.c ==================== */
#include "bsp_sensor.h"
#include "bsp_dht22.h"

static ADC_HandleTypeDef *pHadc = NULL;

void Sensor_Init(ADC_HandleTypeDef *hadc)
{
    pHadc = hadc;
}

static uint16_t ADC_ReadChannel(uint32_t channel)
{
    ADC_ChannelConfTypeDef cfg = {0};
    cfg.Channel      = channel;
    cfg.Rank         = ADC_REGULAR_RANK_1;
    cfg.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    HAL_ADC_ConfigChannel(pHadc, &cfg);

    HAL_ADC_Start(pHadc);
    HAL_ADC_PollForConversion(pHadc, 10);
    uint16_t val = (uint16_t)HAL_ADC_GetValue(pHadc);
    HAL_ADC_Stop(pHadc);
    return val;
}

uint16_t Sensor_ReadSoil(void)  { return ADC_ReadChannel(ADC_CH_SOIL);  }
uint16_t Sensor_ReadLight(void) { return ADC_ReadChannel(ADC_CH_LIGHT); }

void Sensor_ReadAll(SensorData_t *data)
{
    DHT22_Read(&data->temperature, &data->humidity);
    data->soil  = Sensor_ReadSoil();
    data->light = Sensor_ReadLight();
    data->timestamp++;
}
