/* ==================== app_ui.h ==================== */
#ifndef __APP_UI_H
#define __APP_UI_H

#include "config.h"

/* 绘制主界面框架（只需开机调用一次） */
void UI_DrawDashboard(void);

/* 刷新传感器数据区域（每次采样后调用） */
void UI_UpdateSensorData(SensorData_t *d, Thresholds_t *thr);

/* 刷新执行器状态指示 */
void UI_UpdateActuatorStatus(uint8_t act_status);

/* 刷新模式标签 */
void UI_UpdateModeLabel(SystemMode_t mode);

/* 添加一个数据点到实时温度曲线 */
void UI_PushTempCurve(float temp);

/* 显示报警遮罩（屏幕变红闪烁） */
void UI_ShowAlarm(const char *msg);
void UI_ClearAlarm(void);

/* 按键处理（含设置菜单） */
void UI_Process(SystemMode_t *mode, Thresholds_t *thr, SensorData_t *data);

#endif
