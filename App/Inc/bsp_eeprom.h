/* ==================== bsp_eeprom.h ==================== */
#ifndef __BSP_EEPROM_H
#define __BSP_EEPROM_H

#include "config.h"

void    EEPROM_Init(I2C_HandleTypeDef *hi2c);
void    EEPROM_WriteByte(uint8_t addr, uint8_t data);
uint8_t EEPROM_ReadByte(uint8_t addr);
void    EEPROM_LogRecord(SensorData_t *d);
void    EEPROM_DumpAll(void (*send_fn)(const char *));

#endif
