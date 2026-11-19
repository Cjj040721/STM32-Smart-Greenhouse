/* ==================== bsp_tftlcd.h ==================== */
#ifndef __BSP_TFTLCD_H
#define __BSP_TFTLCD_H

#include "config.h"

/* ---------- LCD 控制器 ID ---------- */
extern uint16_t g_lcd_id;  /* 运行时读取，如 0x9341 / 0x9481 / 0x5310 等 */

/* ---------- 基本操作 ---------- */
void    LCD_Init(void);
void    LCD_Clear(uint16_t color);
void    LCD_SetCursor(uint16_t x, uint16_t y);
void    LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color);
uint16_t LCD_ReadPoint(uint16_t x, uint16_t y);

/* ---------- 图形绘制 ---------- */
void LCD_Fill(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void LCD_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void LCD_DrawRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void LCD_FillRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void LCD_DrawCircle(uint16_t x, uint16_t y, uint8_t r, uint16_t color);

/* ---------- 文字显示 ---------- */
void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t ch, uint8_t size, uint16_t color, uint16_t bg);
void LCD_ShowString(uint16_t x, uint16_t y, const char *str, uint8_t size, uint16_t color, uint16_t bg);
void LCD_ShowNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size, uint16_t color, uint16_t bg);
void LCD_Printf(uint16_t x, uint16_t y, uint8_t size, uint16_t color, uint16_t bg, const char *fmt, ...);

/* ---------- 背光 ---------- */
void LCD_BacklightOn(void);
void LCD_BacklightOff(void);

#endif
