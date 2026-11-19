/* ==================== app_control.h ==================== */
#ifndef __APP_CONTROL_H
#define __APP_CONTROL_H

#include "config.h"

void    Control_Auto(SensorData_t *data, Thresholds_t *thr);
uint8_t Control_CheckAlarm(SensorData_t *data, Thresholds_t *thr);

#endif
