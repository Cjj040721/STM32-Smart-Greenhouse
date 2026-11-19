/* ==================== app_comm.h ==================== */
#ifndef __APP_COMM_H
#define __APP_COMM_H

#include "config.h"

void Comm_Init(UART_HandleTypeDef *huart);
void Comm_SendString(const char *str);
void Comm_SendSensorCSV(SensorData_t *d);
void Comm_PollCommand(SystemMode_t *mode);

#endif
