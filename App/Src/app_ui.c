#include "app_ui.h"
#include "bsp_tftlcd.h"
#include "bsp_key.h"
#include "bsp_actuator.h"

/* ---------- 布局常量（480×272 屏幕） ---------- */
/*
 *  ┌──────────────────────────────────────────────┐
 *  │  SmartGreenhouse v2.0-TFT        [AUTO]  │  ← 标题栏 y=0~28
 *  ├────────────┬────────────┬────────────────────┤
 *  │  Temp      │  Humidity  │                    │
 *  │  25.3 °C   │  62.0 %    │   温度实时曲线     │  ← 数据区 y=30~160
 *  ├────────────┼────────────┤                    │
 *  │  Soil      │  Light     │                    │
 *  │  2048      │  1520      │                    │
 *  ├────────────┴────────────┼────────────────────┤
 *  │  PUMP: OFF  FAN: OFF    │  LIGHT: OFF        │  ← 状态栏 y=165~200
 *  ├─────────────────────────┴────────────────────┤
 *  │  阈值: T>28.0  H<40.0  S<1500  L<800        │  ← 底栏 y=205~240
 *  └──────────────────────────────────────────────┘
 */

#define TITLE_Y       0
#define TITLE_H       28
#define DATA_Y        30
#define DATA_H        132
#define STATUS_Y      165
#define STATUS_H      36
#define BOTTOM_Y      205
#define BOTTOM_H      35
#define CURVE_X       240
#define CURVE_Y       32
#define CURVE_W       235
#define CURVE_H       128

/* 温度曲线历史数据 */
#define CURVE_POINTS  CURVE_W
static uint16_t g_curve_buf[CURVE_POINTS];
static uint16_t g_curve_idx = 0;
static uint8_t  g_curve_started = 0;

/* ---------- 绘制主仪表盘框架 ---------- */
void UI_DrawDashboard(void)
{
    LCD_Clear(COLOR_BG);

    /* 标题栏 */
    LCD_FillRect(0, TITLE_Y, LCD_W - 1, TITLE_Y + TITLE_H, COLOR_PANEL_BG);
    LCD_ShowString(8, 6, "SmartGreenhouse " FW_VERSION, 16, COLOR_CYAN, COLOR_PANEL_BG);

    /* 数据面板分隔线 */
    LCD_DrawLine(0,   DATA_Y,       LCD_W - 1, DATA_Y,       COLOR_GRAY);
    LCD_DrawLine(0,   STATUS_Y,     LCD_W - 1, STATUS_Y,     COLOR_GRAY);
    LCD_DrawLine(0,   BOTTOM_Y,     LCD_W - 1, BOTTOM_Y,     COLOR_GRAY);
    LCD_DrawLine(120, DATA_Y,       120,       STATUS_Y,     COLOR_GRAY);  /* 竖线 */
    LCD_DrawLine(CURVE_X - 4, DATA_Y, CURVE_X - 4, STATUS_Y, COLOR_GRAY);

    /* 数据标签 */
    LCD_ShowString(10,  DATA_Y + 5,  "Temp",     16, COLOR_GRAY,   COLOR_BG);
    LCD_ShowString(130, DATA_Y + 5,  "Humidity", 16, COLOR_GRAY,   COLOR_BG);
    LCD_ShowString(10,  DATA_Y + 68, "Soil",     16, COLOR_GRAY,   COLOR_BG);
    LCD_ShowString(130, DATA_Y + 68, "Light",    16, COLOR_GRAY,   COLOR_BG);

    /* 曲线框 */
    LCD_DrawRect(CURVE_X - 1, CURVE_Y - 1,
                 CURVE_X + CURVE_W, CURVE_Y + CURVE_H, COLOR_GRAY);
    LCD_ShowString(CURVE_X + 70, CURVE_Y - 1, "Temp Curve", 16, COLOR_GRAY, COLOR_BG);

    /* 底部阈值标签 */
    LCD_ShowString(8, BOTTOM_Y + 10, "Thresholds:", 16, COLOR_GRAY, COLOR_BG);
}

/* ---------- 刷新传感器数据（大字号） ---------- */
/* ---------- 刷新传感器数据（大字号） ---------- */
/* ---------- 刷新传感器数据（大字号） ---------- */
void UI_UpdateSensorData(SensorData_t *d, Thresholds_t *thr)
{
    char buf[32];

    uint16_t tc = (d->temperature > thr->temp_high) ? COLOR_RED : COLOR_GREEN;
    uint16_t hc = (d->humidity    < thr->hum_low)   ? COLOR_RED : COLOR_GREEN;
    uint16_t sc = (d->soil        > thr->soil_dry)  ? COLOR_RED : COLOR_GREEN;
    uint16_t lc = (d->light       > thr->light_low) ? COLOR_RED : COLOR_GREEN;

    /* 【关键修复】：在打印新数字前，先用背景色画一个实心矩形，把旧数字擦除！ */
    /* 参数分别是：起点X, 起点Y, 终点X, 终点Y, 颜色 */
    LCD_FillRect(10,  DATA_Y + 26, 125, DATA_Y + 50, COLOR_BG); // 擦除温度区域
    LCD_FillRect(130, DATA_Y + 26, 235, DATA_Y + 50, COLOR_BG); // 擦除湿度区域
    LCD_FillRect(10,  DATA_Y + 90, 125, DATA_Y + 114, COLOR_BG); // 擦除土壤区域
    LCD_FillRect(130, DATA_Y + 90, 235, DATA_Y + 114, COLOR_BG); // 擦除光照区域
    LCD_FillRect(108, BOTTOM_Y + 10, 450, BOTTOM_Y + 26, COLOR_BG); // 擦除底部阈值区

    /* 1. 打印温度 */
    sprintf(buf, "%5.1f C", d->temperature);
    LCD_ShowString(10, DATA_Y + 26, buf, 16, tc, COLOR_BG);

    /* 2. 打印湿度 */
    sprintf(buf, "%5.1f %%", d->humidity);
    LCD_ShowString(130, DATA_Y + 26, buf, 16, hc, COLOR_BG);

    /* 3. 打印土壤 */
    sprintf(buf, " %4u", d->soil);
    LCD_ShowString(10, DATA_Y + 90, buf, 16, sc, COLOR_BG);

    /* 4. 打印光照 */
    sprintf(buf, " %4u", d->light);
    LCD_ShowString(130, DATA_Y + 90, buf, 16, lc, COLOR_BG);

    /* 5. 刷新底部阈值 */
    sprintf(buf, "T>%.0f H<%.0f S>%u L>%u",
            thr->temp_high, thr->hum_low, thr->soil_dry, thr->light_low);
    LCD_ShowString(108, BOTTOM_Y + 10, buf, 16, COLOR_YELLOW, COLOR_BG);
}



/* ---------- 刷新执行器状态 ---------- */
/* ---------- 刷新执行器状态 ---------- */
/* ---------- 刷新执行器状态 ---------- */
void UI_UpdateActuatorStatus(uint8_t act_status)
{
    uint16_t y = STATUS_Y + 10;
    char buf[20];

    /* 【关键修复】：擦除旧的 ON/OFF 状态文字 */
    LCD_FillRect(10,  y, 110, y + 16, COLOR_BG);
    LCD_FillRect(120, y, 220, y + 16, COLOR_BG);
    LCD_FillRect(230, y, 350, y + 16, COLOR_BG);

    sprintf(buf, "PUMP:%s", (act_status & ACT_PUMP) ? " ON" : "OFF");
    LCD_ShowString(10,  y, buf, 16, (act_status & ACT_PUMP) ? COLOR_CYAN : COLOR_GRAY, COLOR_BG);

    sprintf(buf, "FAN:%s", (act_status & ACT_FAN) ? " ON" : "OFF");
    LCD_ShowString(120, y, buf, 16, (act_status & ACT_FAN) ? COLOR_CYAN : COLOR_GRAY, COLOR_BG);

    sprintf(buf, "LIGHT:%s", (act_status & ACT_LIGHT) ? " ON" : "OFF");
    LCD_ShowString(230, y, buf, 16, (act_status & ACT_LIGHT) ? COLOR_YELLOW : COLOR_GRAY, COLOR_BG);
}


/* ---------- 刷新模式标签 ---------- */
void UI_UpdateModeLabel(SystemMode_t mode)
{
    const char *labels[] = { " AUTO ", "MANUAL", " SET  ", " LOG  " };
    uint16_t colors[]    = { COLOR_GREEN, COLOR_ORANGE, COLOR_YELLOW, COLOR_CYAN };
    uint8_t  idx = (uint8_t)mode;
    if (idx > 3) idx = 0;

    LCD_FillRect(400, 4, 475, 24, COLOR_PANEL_BG);
    LCD_Printf(404, 6, 16, colors[idx], COLOR_PANEL_BG, "[%s]", labels[idx]);
}

/* ---------- 实时温度曲线 ---------- */
void UI_PushTempCurve(float temp)
{
    /* 将温度映射到曲线高度（0°C ~ 50°C → CURVE_H ~ 0） */
    int16_t py = CURVE_H - (int16_t)(temp * CURVE_H / 50.0f);
    if (py < 0) py = 0;
    if (py >= CURVE_H) py = CURVE_H - 1;

    /* 新数据点入队 */
    g_curve_buf[g_curve_idx] = (uint16_t)py;
    g_curve_idx = (g_curve_idx + 1) % CURVE_POINTS;
    if (!g_curve_started && g_curve_idx > 1) g_curve_started = 1;

    /* 清空曲线区域 */
    LCD_FillRect(CURVE_X, CURVE_Y, CURVE_X + CURVE_W - 1, CURVE_Y + CURVE_H - 1, COLOR_BG);

    /* 绘制网格线 */
    for (int i = 0; i < CURVE_H; i += (CURVE_H / 5)) {
        for (int j = 0; j < CURVE_W; j += 8) {
            LCD_DrawPoint(CURVE_X + j, CURVE_Y + i, COLOR_GRAY);
        }
    }

    /* 绘制曲线 */
    if (g_curve_started) {
        uint16_t total = (g_curve_idx < CURVE_POINTS && !g_curve_started)
                         ? g_curve_idx : CURVE_POINTS;
        for (uint16_t i = 1; i < total; i++) {
            uint16_t i0 = (g_curve_idx + i - 1) % CURVE_POINTS;
            uint16_t i1 = (g_curve_idx + i)     % CURVE_POINTS;
            LCD_DrawLine(
                CURVE_X + i - 1, CURVE_Y + g_curve_buf[i0],
                CURVE_X + i,     CURVE_Y + g_curve_buf[i1],
                COLOR_CYAN);
        }
    }
}

/* ---------- 报警 ---------- */
void UI_ShowAlarm(const char *msg)
{
    LCD_FillRect(0, LCD_H - 40, LCD_W - 1, LCD_H - 1, COLOR_RED);
    LCD_ShowString(10, LCD_H - 30, msg, 24, COLOR_WHITE, COLOR_RED);
}

void UI_ClearAlarm(void)
{
    LCD_FillRect(0, LCD_H - 40, LCD_W - 1, LCD_H - 1, COLOR_BG);
}

/* ---------- 设置模式索引 ---------- */
static uint8_t g_setting_idx = 0;

/* ---------- 按键处理主函数 ---------- */
/* ---------- 按键处理主函数 ---------- */
void UI_Process(SystemMode_t *mode, Thresholds_t *thr, SensorData_t *data)
{
    uint8_t key = Key_Scan();
    if (key == KEY_NONE) return;

    switch (*mode) {
        /* ================= 1. 自动模式 ================= */
        case MODE_AUTO:
            if (key == KEY_MODE) {
                *mode = MODE_MANUAL;
                UI_UpdateModeLabel(*mode);
            }
            break;

        /* ================= 2. 手动模式 ================= */
        case MODE_MANUAL:
            if (key == KEY_MODE) {
                Pump_Off(); Fan_Off(); GrowLight_Off();
                *mode = MODE_SETTING; // 切换到设置模式
                UI_UpdateModeLabel(*mode);
            }
            if (key == KEY_UP)     { if(Actuator_GetStatus() & ACT_PUMP) Pump_Off(); else Pump_On(); }
            if (key == KEY_DOWN)   { if(Actuator_GetStatus() & ACT_FAN) Fan_Off(); else Fan_On(); }
            if (key == KEY_SELECT) { if(Actuator_GetStatus() & ACT_LIGHT) GrowLight_Off(); else GrowLight_On(); }
            UI_UpdateActuatorStatus(Actuator_GetStatus());
            break;

        /* ================= 3. 设置模式 ================= */
        case MODE_SETTING:
            /* 1. 退出设置模式 */
            if (key == KEY_MODE) {
                *mode = MODE_AUTO; // 退出回到自动模式
                UI_UpdateModeLabel(*mode);

                /* 【修复 1：退出时清理屏幕】擦除右下角的 "Edit..." 提示框 */
                LCD_FillRect(CURVE_X, STATUS_Y + 2, 480 - 1, BOTTOM_Y - 2, COLOR_BG);
            }
            /* 2. 切换要修改的参数 */
            else if (key == KEY_SELECT) {
                g_setting_idx = (g_setting_idx + 1) % 4;
            }
            /* 3. 增加数值 */
            else if (key == KEY_UP) {
                switch (g_setting_idx) {
                    case 0: thr->temp_high += 0.5f; break;
                    case 1: thr->hum_low   += 1.0f; break;
                    case 2: thr->soil_dry  += 50;   break;
                    case 3: thr->light_low += 50;   break;
                }
                /* 【修复 2：实时刷新】修改后立即刷新底部阈值栏 */
                UI_UpdateSensorData(data, thr);
            }
            /* 4. 减小数值 */
            else if (key == KEY_DOWN) {
                switch (g_setting_idx) {
                    case 0: thr->temp_high -= 0.5f; break;
                    case 1: thr->hum_low   -= 1.0f; break;
                    case 2: thr->soil_dry  -= 50;   break;
                    case 3: thr->light_low -= 50;   break;
                }
                /* 【修复 2：实时刷新】修改后立即刷新底部阈值栏 */
                UI_UpdateSensorData(data, thr);
            }

            /* 高亮显示当前编辑项 */
            if (*mode == MODE_SETTING) {
                const char *names[] = {"TempHigh", "HumLow ", "SoilDry ", "LightLow"};
                char val_buf[16];
                switch (g_setting_idx) {
                    case 0: snprintf(val_buf, 16, "%.1f C", thr->temp_high); break;
                    case 1: snprintf(val_buf, 16, "%.1f %%",  thr->hum_low); break;
                    case 2: snprintf(val_buf, 16, "%u",       thr->soil_dry); break;
                    case 3: snprintf(val_buf, 16, "%u",       thr->light_low); break;
                }
                LCD_FillRect(CURVE_X, STATUS_Y + 2, 480 - 1, BOTTOM_Y - 2, COLOR_PANEL_BG);
                LCD_Printf(CURVE_X + 5, STATUS_Y + 10, 16, COLOR_YELLOW, COLOR_PANEL_BG,
                           "Edit: %s = %s", names[g_setting_idx], val_buf);
            }
            break;

        /* ================= 其他模式 (消除警告) ================= */
        case MODE_VIEW_LOG:
        default:
            if (key == KEY_MODE) {
                *mode = MODE_AUTO;
                UI_UpdateModeLabel(*mode);
            }
            break;
    }
}
