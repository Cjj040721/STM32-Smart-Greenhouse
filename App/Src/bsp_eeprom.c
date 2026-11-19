/* ==================== bsp_eeprom.c ==================== */
#include "bsp_eeprom.h"

static I2C_HandleTypeDef *pHi2c = NULL;
static uint16_t g_write_addr = EEPROM_DATA_START;

void EEPROM_Init(I2C_HandleTypeDef *hi2c) { pHi2c = hi2c; }

void EEPROM_WriteByte(uint8_t addr, uint8_t data)
{
    uint8_t buf[2] = { addr, data };
    HAL_I2C_Master_Transmit(pHi2c, EEPROM_I2C_ADDR, buf, 2, 100);
    HAL_Delay(5);
}

uint8_t EEPROM_ReadByte(uint8_t addr)
{
    uint8_t data = 0;
    HAL_I2C_Master_Transmit(pHi2c, EEPROM_I2C_ADDR, &addr, 1, 100);
    HAL_I2C_Master_Receive(pHi2c, EEPROM_I2C_ADDR, &data, 1, 100);
    return data;
}

/* 6 字节/条：temp×10(2) + hum×10(2) + soil>>4(1) + light>>4(1) */
void EEPROM_LogRecord(SensorData_t *d)
{
    int16_t t10 = (int16_t)(d->temperature * 10.0f);
    int16_t h10 = (int16_t)(d->humidity    * 10.0f);

    EEPROM_WriteByte(g_write_addr++, (uint8_t)(t10 >> 8));
    EEPROM_WriteByte(g_write_addr++, (uint8_t)(t10 & 0xFF));
    EEPROM_WriteByte(g_write_addr++, (uint8_t)(h10 >> 8));
    EEPROM_WriteByte(g_write_addr++, (uint8_t)(h10 & 0xFF));
    EEPROM_WriteByte(g_write_addr++, (uint8_t)(d->soil  >> 4));
    EEPROM_WriteByte(g_write_addr++, (uint8_t)(d->light >> 4));

    if (g_write_addr >= EEPROM_SIZE)
        g_write_addr = EEPROM_DATA_START;
}

void EEPROM_DumpAll(void (*send_fn)(const char *))
{
    char buf[48];
    send_fn("=== EEPROM Data Dump ===\r\n");
    send_fn("Temp(C),Hum(%),Soil,Light\r\n");

    for (uint16_t addr = EEPROM_DATA_START;
         addr + RECORD_SIZE <= EEPROM_SIZE;
         addr += RECORD_SIZE)
    {
        int16_t  t10 = (int16_t)((EEPROM_ReadByte(addr) << 8) | EEPROM_ReadByte(addr + 1));
        int16_t  h10 = (int16_t)((EEPROM_ReadByte(addr + 2) << 8) | EEPROM_ReadByte(addr + 3));
        uint16_t s   = (uint16_t)EEPROM_ReadByte(addr + 4) << 4;
        uint16_t l   = (uint16_t)EEPROM_ReadByte(addr + 5) << 4;

        if (t10 == 0 && h10 == 0 && s == 0 && l == 0) continue;

        snprintf(buf, sizeof(buf), "%.1f,%.1f,%u,%u\r\n",
                 t10 / 10.0f, h10 / 10.0f, s, l);
        send_fn(buf);
    }
    send_fn("=== END ===\r\n");
}
