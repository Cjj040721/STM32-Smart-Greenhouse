/* ==================== bsp_dht22.h ==================== */
#ifndef __BSP_DHT22_H
#define __BSP_DHT22_H

#include "config.h"

uint8_t DHT22_Read(float *temperature, float *humidity);

#endif
