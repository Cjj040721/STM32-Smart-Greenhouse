/* ==================== bsp_tftlcd.c ==================== */
#include "bsp_tftlcd.h"
#include "bsp_delay_us.h"
#include "font.h"
#include <stdarg.h>

/*
 * FSMC Bank1 NE4 地址映射：
 *   基地址 = 0x6C000000 (Bank1 NE4)
 *   RS(A10) = 0 → 命令，RS(A10) = 1 → 数据
 *   A10 在 16-bit 模式下对应地址位 A11（左移 1 位）
 *   命令地址 = 0x6C000000
 *   数据地址 = 0x6C000000 + (1 << 11) = 0x6C000800
 */
#define LCD_CMD  (*(volatile uint16_t *)0x6C000000)
#define LCD_DATA (*(volatile uint16_t *)0x6C000800)

uint16_t g_lcd_id = 0;

/* ---------- 底层读写 ---------- */
static void LCD_WriteReg(uint16_t reg, uint16_t val)
{
    LCD_CMD  = reg;
    LCD_DATA = val;
}

static uint16_t LCD_ReadReg(uint16_t reg)
{
    LCD_CMD = reg;
    return LCD_DATA;
}

static void LCD_WriteCmd(uint16_t cmd)
{
    LCD_CMD = cmd;
}

static void LCD_WriteData(uint16_t data)
{
    LCD_DATA = data;
}

/* ---------- 背光控制 ---------- */
void LCD_BacklightOn(void)
{
    HAL_GPIO_WritePin(LCD_BL_PORT, LCD_BL_PIN, GPIO_PIN_SET);
}

void LCD_BacklightOff(void)
{
    HAL_GPIO_WritePin(LCD_BL_PORT, LCD_BL_PIN, GPIO_PIN_RESET);
}

/* ---------- 读取 LCD 控制器 ID ---------- */
static void LCD_ReadID(void)
{
    /* 1. 尝试读 0xD3 (ILI9341) */
    LCD_WriteCmd(0xD3);
    g_lcd_id = LCD_DATA;   /* dummy */
    g_lcd_id = LCD_DATA;
    g_lcd_id = LCD_DATA;
    g_lcd_id <<= 8;
    g_lcd_id |= LCD_DATA;

    /* 2. 尝试读 ILI9481 */
    if (g_lcd_id != 0x9341) {
        LCD_WriteCmd(0xBF);
        g_lcd_id = LCD_DATA;
        g_lcd_id = LCD_DATA;
        g_lcd_id = LCD_DATA;
        g_lcd_id <<= 8;
        g_lcd_id |= LCD_DATA;
    }

    /* 3. 尝试读 NT35510 (0x5510) */
    if (g_lcd_id != 0x9341 && g_lcd_id != 0x9481) {
        LCD_WriteCmd(0xD400);
        g_lcd_id = LCD_DATA;   // dummy 读
        g_lcd_id = LCD_DATA;   // 读回 0x00
        g_lcd_id = LCD_DATA;   // 读回 0x55
        g_lcd_id <<= 8;
        g_lcd_id |= LCD_DATA;  // 读回 0x10，组合为 0x5510
    }
}


/* ---------- 设置显示窗口 ---------- */
static void LCD_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    if (g_lcd_id == 0x5510) {
        /* NT35510：每个参数字节都有独立寄存器地址 */
        LCD_WriteCmd(0x2A00); LCD_WriteData(x0 >> 8);
        LCD_WriteCmd(0x2A01); LCD_WriteData(x0 & 0xFF);
        LCD_WriteCmd(0x2A02); LCD_WriteData(x1 >> 8);
        LCD_WriteCmd(0x2A03); LCD_WriteData(x1 & 0xFF);

        LCD_WriteCmd(0x2B00); LCD_WriteData(y0 >> 8);
        LCD_WriteCmd(0x2B01); LCD_WriteData(y0 & 0xFF);
        LCD_WriteCmd(0x2B02); LCD_WriteData(y1 >> 8);
        LCD_WriteCmd(0x2B03); LCD_WriteData(y1 & 0xFF);

        LCD_WriteCmd(0x2C00);   /* 开始写 GRAM */
    } else {
        /* ILI9341 / ILI9481 等标准格式 */
        LCD_WriteCmd(0x2A);
        LCD_WriteData(x0 >> 8); LCD_WriteData(x0 & 0xFF);
        LCD_WriteData(x1 >> 8); LCD_WriteData(x1 & 0xFF);

        LCD_WriteCmd(0x2B);
        LCD_WriteData(y0 >> 8); LCD_WriteData(y0 & 0xFF);
        LCD_WriteData(y1 >> 8); LCD_WriteData(y1 & 0xFF);

        LCD_WriteCmd(0x2C);
    }
}


/* ---------- 初始化 ---------- */
void LCD_Init(void)
{
    LCD_BacklightOn();
    HAL_Delay(50);

    LCD_ReadID();
    g_lcd_id = 0x5510;
    if (g_lcd_id == 0x9341) {
        /* ILI9341 初始化序列（保留原来的） */
        LCD_WriteCmd(0xCF);
        LCD_WriteData(0x00); LCD_WriteData(0xC1); LCD_WriteData(0x30);
        LCD_WriteCmd(0xED);
        LCD_WriteData(0x64); LCD_WriteData(0x03); LCD_WriteData(0x12); LCD_WriteData(0x81);
        LCD_WriteCmd(0xE8);
        LCD_WriteData(0x85); LCD_WriteData(0x10); LCD_WriteData(0x7A);
        LCD_WriteCmd(0xCB);
        LCD_WriteData(0x39); LCD_WriteData(0x2C); LCD_WriteData(0x00);
        LCD_WriteData(0x34); LCD_WriteData(0x02);
        LCD_WriteCmd(0xF7); LCD_WriteData(0x20);
        LCD_WriteCmd(0xEA); LCD_WriteData(0x00); LCD_WriteData(0x00);
        LCD_WriteCmd(0xC0); LCD_WriteData(0x1B);
        LCD_WriteCmd(0xC1); LCD_WriteData(0x01);
        LCD_WriteCmd(0xC5); LCD_WriteData(0x30); LCD_WriteData(0x30);
        LCD_WriteCmd(0xC7); LCD_WriteData(0xB7);
        LCD_WriteCmd(0x36); LCD_WriteData(0x48);
        LCD_WriteCmd(0x3A); LCD_WriteData(0x55);
        LCD_WriteCmd(0xB1); LCD_WriteData(0x00); LCD_WriteData(0x1A);
        LCD_WriteCmd(0xB6); LCD_WriteData(0x0A); LCD_WriteData(0xA2);
        LCD_WriteCmd(0xF2); LCD_WriteData(0x00);
        LCD_WriteCmd(0x26); LCD_WriteData(0x01);
        LCD_WriteCmd(0x11);
        HAL_Delay(120);
        LCD_WriteCmd(0x29);

    } else if (g_lcd_id == 0x5510) {
        /* NT35510 初始化序列 */
        LCD_WriteCmd(0xF000); LCD_WriteData(0x55);
        LCD_WriteCmd(0xF001); LCD_WriteData(0xAA);
        LCD_WriteCmd(0xF002); LCD_WriteData(0x52);
        LCD_WriteCmd(0xF003); LCD_WriteData(0x08);
        LCD_WriteCmd(0xF004); LCD_WriteData(0x01);

        /* AVDD 设置 */
        LCD_WriteCmd(0xB000); LCD_WriteData(0x0D);
        LCD_WriteCmd(0xB001); LCD_WriteData(0x0D);
        LCD_WriteCmd(0xB002); LCD_WriteData(0x0D);

        /* AVDD 比率 */
        LCD_WriteCmd(0xB600); LCD_WriteData(0x34);
        LCD_WriteCmd(0xB601); LCD_WriteData(0x34);
        LCD_WriteCmd(0xB602); LCD_WriteData(0x34);

        /* AVEE 设置 */
        LCD_WriteCmd(0xB100); LCD_WriteData(0x0D);
        LCD_WriteCmd(0xB101); LCD_WriteData(0x0D);
        LCD_WriteCmd(0xB102); LCD_WriteData(0x0D);

        /* AVEE 比率 */
        LCD_WriteCmd(0xB700); LCD_WriteData(0x34);
        LCD_WriteCmd(0xB701); LCD_WriteData(0x34);
        LCD_WriteCmd(0xB702); LCD_WriteData(0x34);

        /* VCL 设置 */
        LCD_WriteCmd(0xB200); LCD_WriteData(0x00);
        LCD_WriteCmd(0xB800); LCD_WriteData(0x24);
        LCD_WriteCmd(0xB801); LCD_WriteData(0x24);
        LCD_WriteCmd(0xB802); LCD_WriteData(0x24);

        /* VGH 设置 */
        LCD_WriteCmd(0xBF00); LCD_WriteData(0x01);
        LCD_WriteCmd(0xB300); LCD_WriteData(0x0F);
        LCD_WriteCmd(0xB301); LCD_WriteData(0x0F);
        LCD_WriteCmd(0xB302); LCD_WriteData(0x0F);
        LCD_WriteCmd(0xB900); LCD_WriteData(0x34);
        LCD_WriteCmd(0xB901); LCD_WriteData(0x34);
        LCD_WriteCmd(0xB902); LCD_WriteData(0x34);

        /* VGL 设置 */
        LCD_WriteCmd(0xB500); LCD_WriteData(0x08);
        LCD_WriteCmd(0xB501); LCD_WriteData(0x08);
        LCD_WriteCmd(0xB502); LCD_WriteData(0x08);
        LCD_WriteCmd(0xC200); LCD_WriteData(0x03);
        LCD_WriteCmd(0xBA00); LCD_WriteData(0x24);
        LCD_WriteCmd(0xBA01); LCD_WriteData(0x24);
        LCD_WriteCmd(0xBA02); LCD_WriteData(0x24);

        /* VGMP/VGSP 设置 */
        LCD_WriteCmd(0xBC00); LCD_WriteData(0x00);
        LCD_WriteCmd(0xBC01); LCD_WriteData(0x78);
        LCD_WriteCmd(0xBC02); LCD_WriteData(0x00);
        LCD_WriteCmd(0xBD00); LCD_WriteData(0x00);
        LCD_WriteCmd(0xBD01); LCD_WriteData(0x78);
        LCD_WriteCmd(0xBD02); LCD_WriteData(0x00);

        /* VCOM 设置 */
        LCD_WriteCmd(0xBE00); LCD_WriteData(0x00);
        LCD_WriteCmd(0xBE01); LCD_WriteData(0x64);

        /* Gamma 设置 */
        LCD_WriteCmd(0xD100); LCD_WriteData(0x00);
        LCD_WriteCmd(0xD101); LCD_WriteData(0x33);
        LCD_WriteCmd(0xD102); LCD_WriteData(0x00);
        LCD_WriteCmd(0xD103); LCD_WriteData(0x33);
        LCD_WriteCmd(0xD104); LCD_WriteData(0x00);
        LCD_WriteCmd(0xD105); LCD_WriteData(0x3A);
        LCD_WriteCmd(0xD106); LCD_WriteData(0x00);
        LCD_WriteCmd(0xD107); LCD_WriteData(0x72);
        LCD_WriteCmd(0xD108); LCD_WriteData(0x00);
        LCD_WriteCmd(0xD109); LCD_WriteData(0x8F);
        LCD_WriteCmd(0xD10A); LCD_WriteData(0x00);
        LCD_WriteCmd(0xD10B); LCD_WriteData(0xB0);
        LCD_WriteCmd(0xD10C); LCD_WriteData(0x00);
        LCD_WriteCmd(0xD10D); LCD_WriteData(0xCA);
        LCD_WriteCmd(0xD10E); LCD_WriteData(0x00);
        LCD_WriteCmd(0xD10F); LCD_WriteData(0xE5);
        LCD_WriteCmd(0xD110); LCD_WriteData(0x01);
        LCD_WriteCmd(0xD111); LCD_WriteData(0x00);
        LCD_WriteCmd(0xD112); LCD_WriteData(0x01);
        LCD_WriteCmd(0xD113); LCD_WriteData(0x2E);
        LCD_WriteCmd(0xD114); LCD_WriteData(0x01);
        LCD_WriteCmd(0xD115); LCD_WriteData(0x54);
        LCD_WriteCmd(0xD116); LCD_WriteData(0x01);
        LCD_WriteCmd(0xD117); LCD_WriteData(0x8B);
        LCD_WriteCmd(0xD118); LCD_WriteData(0x01);
        LCD_WriteCmd(0xD119); LCD_WriteData(0xB7);
        LCD_WriteCmd(0xD11A); LCD_WriteData(0x01);
        LCD_WriteCmd(0xD11B); LCD_WriteData(0xB8);
        LCD_WriteCmd(0xD11C); LCD_WriteData(0x01);
        LCD_WriteCmd(0xD11D); LCD_WriteData(0xDC);
        LCD_WriteCmd(0xD11E); LCD_WriteData(0x02);
        LCD_WriteCmd(0xD11F); LCD_WriteData(0x00);
        LCD_WriteCmd(0xD120); LCD_WriteData(0x02);
        LCD_WriteCmd(0xD121); LCD_WriteData(0x16);
        LCD_WriteCmd(0xD122); LCD_WriteData(0x02);
        LCD_WriteCmd(0xD123); LCD_WriteData(0x30);
        LCD_WriteCmd(0xD124); LCD_WriteData(0x02);
        LCD_WriteCmd(0xD125); LCD_WriteData(0x55);
        LCD_WriteCmd(0xD126); LCD_WriteData(0x02);
        LCD_WriteCmd(0xD127); LCD_WriteData(0x73);
        LCD_WriteCmd(0xD128); LCD_WriteData(0x02);
        LCD_WriteCmd(0xD129); LCD_WriteData(0x9C);
        LCD_WriteCmd(0xD12A); LCD_WriteData(0x02);
        LCD_WriteCmd(0xD12B); LCD_WriteData(0x9C);
        LCD_WriteCmd(0xD12C); LCD_WriteData(0x02);
        LCD_WriteCmd(0xD12D); LCD_WriteData(0xC4);
        LCD_WriteCmd(0xD12E); LCD_WriteData(0x02);
        LCD_WriteCmd(0xD12F); LCD_WriteData(0xF0);
        LCD_WriteCmd(0xD130); LCD_WriteData(0x03);
        LCD_WriteCmd(0xD131); LCD_WriteData(0x0E);
        LCD_WriteCmd(0xD132); LCD_WriteData(0x03);
        LCD_WriteCmd(0xD133); LCD_WriteData(0x54);

        /* G/B Gamma 与 R 相同，简化处理 */
        LCD_WriteCmd(0xD200); LCD_WriteData(0x00); LCD_WriteCmd(0xD201); LCD_WriteData(0x33);
        LCD_WriteCmd(0xD202); LCD_WriteData(0x00); LCD_WriteCmd(0xD203); LCD_WriteData(0x33);
        LCD_WriteCmd(0xD204); LCD_WriteData(0x00); LCD_WriteCmd(0xD205); LCD_WriteData(0x3A);
        LCD_WriteCmd(0xD206); LCD_WriteData(0x00); LCD_WriteCmd(0xD207); LCD_WriteData(0x72);
        LCD_WriteCmd(0xD208); LCD_WriteData(0x00); LCD_WriteCmd(0xD209); LCD_WriteData(0x8F);
        LCD_WriteCmd(0xD20A); LCD_WriteData(0x00); LCD_WriteCmd(0xD20B); LCD_WriteData(0xB0);
        LCD_WriteCmd(0xD20C); LCD_WriteData(0x00); LCD_WriteCmd(0xD20D); LCD_WriteData(0xCA);
        LCD_WriteCmd(0xD20E); LCD_WriteData(0x00); LCD_WriteCmd(0xD20F); LCD_WriteData(0xE5);
        LCD_WriteCmd(0xD210); LCD_WriteData(0x01); LCD_WriteCmd(0xD211); LCD_WriteData(0x00);
        LCD_WriteCmd(0xD212); LCD_WriteData(0x01); LCD_WriteCmd(0xD213); LCD_WriteData(0x2E);
        LCD_WriteCmd(0xD214); LCD_WriteData(0x01); LCD_WriteCmd(0xD215); LCD_WriteData(0x54);
        LCD_WriteCmd(0xD216); LCD_WriteData(0x01); LCD_WriteCmd(0xD217); LCD_WriteData(0x8B);
        LCD_WriteCmd(0xD218); LCD_WriteData(0x01); LCD_WriteCmd(0xD219); LCD_WriteData(0xB7);
        LCD_WriteCmd(0xD21A); LCD_WriteData(0x01); LCD_WriteCmd(0xD21B); LCD_WriteData(0xB8);
        LCD_WriteCmd(0xD21C); LCD_WriteData(0x01); LCD_WriteCmd(0xD21D); LCD_WriteData(0xDC);
        LCD_WriteCmd(0xD21E); LCD_WriteData(0x02); LCD_WriteCmd(0xD21F); LCD_WriteData(0x00);
        LCD_WriteCmd(0xD220); LCD_WriteData(0x02); LCD_WriteCmd(0xD221); LCD_WriteData(0x16);
        LCD_WriteCmd(0xD222); LCD_WriteData(0x02); LCD_WriteCmd(0xD223); LCD_WriteData(0x30);
        LCD_WriteCmd(0xD224); LCD_WriteData(0x02); LCD_WriteCmd(0xD225); LCD_WriteData(0x55);
        LCD_WriteCmd(0xD226); LCD_WriteData(0x02); LCD_WriteCmd(0xD227); LCD_WriteData(0x73);
        LCD_WriteCmd(0xD228); LCD_WriteData(0x02); LCD_WriteCmd(0xD229); LCD_WriteData(0x9C);
        LCD_WriteCmd(0xD22A); LCD_WriteData(0x02); LCD_WriteCmd(0xD22B); LCD_WriteData(0x9C);
        LCD_WriteCmd(0xD22C); LCD_WriteData(0x02); LCD_WriteCmd(0xD22D); LCD_WriteData(0xC4);
        LCD_WriteCmd(0xD22E); LCD_WriteData(0x02); LCD_WriteCmd(0xD22F); LCD_WriteData(0xF0);
        LCD_WriteCmd(0xD230); LCD_WriteData(0x03); LCD_WriteCmd(0xD231); LCD_WriteData(0x0E);
        LCD_WriteCmd(0xD232); LCD_WriteData(0x03); LCD_WriteCmd(0xD233); LCD_WriteData(0x54);

        LCD_WriteCmd(0xD300); LCD_WriteData(0x00); LCD_WriteCmd(0xD301); LCD_WriteData(0x33);
        LCD_WriteCmd(0xD302); LCD_WriteData(0x00); LCD_WriteCmd(0xD303); LCD_WriteData(0x33);
        LCD_WriteCmd(0xD304); LCD_WriteData(0x00); LCD_WriteCmd(0xD305); LCD_WriteData(0x3A);
        LCD_WriteCmd(0xD306); LCD_WriteData(0x00); LCD_WriteCmd(0xD307); LCD_WriteData(0x72);
        LCD_WriteCmd(0xD308); LCD_WriteData(0x00); LCD_WriteCmd(0xD309); LCD_WriteData(0x8F);
        LCD_WriteCmd(0xD30A); LCD_WriteData(0x00); LCD_WriteCmd(0xD30B); LCD_WriteData(0xB0);
        LCD_WriteCmd(0xD30C); LCD_WriteData(0x00); LCD_WriteCmd(0xD30D); LCD_WriteData(0xCA);
        LCD_WriteCmd(0xD30E); LCD_WriteData(0x00); LCD_WriteCmd(0xD30F); LCD_WriteData(0xE5);
        LCD_WriteCmd(0xD310); LCD_WriteData(0x01); LCD_WriteCmd(0xD311); LCD_WriteData(0x00);
        LCD_WriteCmd(0xD312); LCD_WriteData(0x01); LCD_WriteCmd(0xD313); LCD_WriteData(0x2E);
        LCD_WriteCmd(0xD314); LCD_WriteData(0x01); LCD_WriteCmd(0xD315); LCD_WriteData(0x54);
        LCD_WriteCmd(0xD316); LCD_WriteData(0x01); LCD_WriteCmd(0xD317); LCD_WriteData(0x8B);
        LCD_WriteCmd(0xD318); LCD_WriteData(0x01); LCD_WriteCmd(0xD319); LCD_WriteData(0xB7);
        LCD_WriteCmd(0xD31A); LCD_WriteData(0x01); LCD_WriteCmd(0xD31B); LCD_WriteData(0xB8);
        LCD_WriteCmd(0xD31C); LCD_WriteData(0x01); LCD_WriteCmd(0xD31D); LCD_WriteData(0xDC);
        LCD_WriteCmd(0xD31E); LCD_WriteData(0x02); LCD_WriteCmd(0xD31F); LCD_WriteData(0x00);
        LCD_WriteCmd(0xD320); LCD_WriteData(0x02); LCD_WriteCmd(0xD321); LCD_WriteData(0x16);
        LCD_WriteCmd(0xD322); LCD_WriteData(0x02); LCD_WriteCmd(0xD323); LCD_WriteData(0x30);
        LCD_WriteCmd(0xD324); LCD_WriteData(0x02); LCD_WriteCmd(0xD325); LCD_WriteData(0x55);
        LCD_WriteCmd(0xD326); LCD_WriteData(0x02); LCD_WriteCmd(0xD327); LCD_WriteData(0x73);
        LCD_WriteCmd(0xD328); LCD_WriteData(0x02); LCD_WriteCmd(0xD329); LCD_WriteData(0x9C);
        LCD_WriteCmd(0xD32A); LCD_WriteData(0x02); LCD_WriteCmd(0xD32B); LCD_WriteData(0x9C);
        LCD_WriteCmd(0xD32C); LCD_WriteData(0x02); LCD_WriteCmd(0xD32D); LCD_WriteData(0xC4);
        LCD_WriteCmd(0xD32E); LCD_WriteData(0x02); LCD_WriteCmd(0xD32F); LCD_WriteData(0xF0);
        LCD_WriteCmd(0xD330); LCD_WriteData(0x03); LCD_WriteCmd(0xD331); LCD_WriteData(0x0E);
        LCD_WriteCmd(0xD332); LCD_WriteData(0x03); LCD_WriteCmd(0xD333); LCD_WriteData(0x54);

        LCD_WriteCmd(0xD400); LCD_WriteData(0x00); LCD_WriteCmd(0xD401); LCD_WriteData(0x33);
        LCD_WriteCmd(0xD402); LCD_WriteData(0x00); LCD_WriteCmd(0xD403); LCD_WriteData(0x33);
        LCD_WriteCmd(0xD404); LCD_WriteData(0x00); LCD_WriteCmd(0xD405); LCD_WriteData(0x3A);
        LCD_WriteCmd(0xD406); LCD_WriteData(0x00); LCD_WriteCmd(0xD407); LCD_WriteData(0x72);
        LCD_WriteCmd(0xD408); LCD_WriteData(0x00); LCD_WriteCmd(0xD409); LCD_WriteData(0x8F);
        LCD_WriteCmd(0xD40A); LCD_WriteData(0x00); LCD_WriteCmd(0xD40B); LCD_WriteData(0xB0);
        LCD_WriteCmd(0xD40C); LCD_WriteData(0x00); LCD_WriteCmd(0xD40D); LCD_WriteData(0xCA);
        LCD_WriteCmd(0xD40E); LCD_WriteData(0x00); LCD_WriteCmd(0xD40F); LCD_WriteData(0xE5);
        LCD_WriteCmd(0xD410); LCD_WriteData(0x01); LCD_WriteCmd(0xD411); LCD_WriteData(0x00);
        LCD_WriteCmd(0xD412); LCD_WriteData(0x01); LCD_WriteCmd(0xD413); LCD_WriteData(0x2E);
        LCD_WriteCmd(0xD414); LCD_WriteData(0x01); LCD_WriteCmd(0xD415); LCD_WriteData(0x54);
        LCD_WriteCmd(0xD416); LCD_WriteData(0x01); LCD_WriteCmd(0xD417); LCD_WriteData(0x8B);
        LCD_WriteCmd(0xD418); LCD_WriteData(0x01); LCD_WriteCmd(0xD419); LCD_WriteData(0xB7);
        LCD_WriteCmd(0xD41A); LCD_WriteData(0x01); LCD_WriteCmd(0xD41B); LCD_WriteData(0xB8);
        LCD_WriteCmd(0xD41C); LCD_WriteData(0x01); LCD_WriteCmd(0xD41D); LCD_WriteData(0xDC);
        LCD_WriteCmd(0xD41E); LCD_WriteData(0x02); LCD_WriteCmd(0xD41F); LCD_WriteData(0x00);
        LCD_WriteCmd(0xD420); LCD_WriteData(0x02); LCD_WriteCmd(0xD421); LCD_WriteData(0x16);
        LCD_WriteCmd(0xD422); LCD_WriteData(0x02); LCD_WriteCmd(0xD423); LCD_WriteData(0x30);
        LCD_WriteCmd(0xD424); LCD_WriteData(0x02); LCD_WriteCmd(0xD425); LCD_WriteData(0x55);
        LCD_WriteCmd(0xD426); LCD_WriteData(0x02); LCD_WriteCmd(0xD427); LCD_WriteData(0x73);
        LCD_WriteCmd(0xD428); LCD_WriteData(0x02); LCD_WriteCmd(0xD429); LCD_WriteData(0x9C);
        LCD_WriteCmd(0xD42A); LCD_WriteData(0x02); LCD_WriteCmd(0xD42B); LCD_WriteData(0x9C);
        LCD_WriteCmd(0xD42C); LCD_WriteData(0x02); LCD_WriteCmd(0xD42D); LCD_WriteData(0xC4);
        LCD_WriteCmd(0xD42E); LCD_WriteData(0x02); LCD_WriteCmd(0xD42F); LCD_WriteData(0xF0);
        LCD_WriteCmd(0xD430); LCD_WriteData(0x03); LCD_WriteCmd(0xD431); LCD_WriteData(0x0E);
        LCD_WriteCmd(0xD432); LCD_WriteData(0x03); LCD_WriteCmd(0xD433); LCD_WriteData(0x54);

        LCD_WriteCmd(0xD500); LCD_WriteData(0x00); LCD_WriteCmd(0xD501); LCD_WriteData(0x33);
        LCD_WriteCmd(0xD502); LCD_WriteData(0x00); LCD_WriteCmd(0xD503); LCD_WriteData(0x33);
        LCD_WriteCmd(0xD504); LCD_WriteData(0x00); LCD_WriteCmd(0xD505); LCD_WriteData(0x3A);
        LCD_WriteCmd(0xD506); LCD_WriteData(0x00); LCD_WriteCmd(0xD507); LCD_WriteData(0x72);
        LCD_WriteCmd(0xD508); LCD_WriteData(0x00); LCD_WriteCmd(0xD509); LCD_WriteData(0x8F);
        LCD_WriteCmd(0xD50A); LCD_WriteData(0x00); LCD_WriteCmd(0xD50B); LCD_WriteData(0xB0);
        LCD_WriteCmd(0xD50C); LCD_WriteData(0x00); LCD_WriteCmd(0xD50D); LCD_WriteData(0xCA);
        LCD_WriteCmd(0xD50E); LCD_WriteData(0x00); LCD_WriteCmd(0xD50F); LCD_WriteData(0xE5);
        LCD_WriteCmd(0xD510); LCD_WriteData(0x01); LCD_WriteCmd(0xD511); LCD_WriteData(0x00);
        LCD_WriteCmd(0xD512); LCD_WriteData(0x01); LCD_WriteCmd(0xD513); LCD_WriteData(0x2E);
        LCD_WriteCmd(0xD514); LCD_WriteData(0x01); LCD_WriteCmd(0xD515); LCD_WriteData(0x54);
        LCD_WriteCmd(0xD516); LCD_WriteData(0x01); LCD_WriteCmd(0xD517); LCD_WriteData(0x8B);
        LCD_WriteCmd(0xD518); LCD_WriteData(0x01); LCD_WriteCmd(0xD519); LCD_WriteData(0xB7);
        LCD_WriteCmd(0xD51A); LCD_WriteData(0x01); LCD_WriteCmd(0xD51B); LCD_WriteData(0xB8);
        LCD_WriteCmd(0xD51C); LCD_WriteData(0x01); LCD_WriteCmd(0xD51D); LCD_WriteData(0xDC);
        LCD_WriteCmd(0xD51E); LCD_WriteData(0x02); LCD_WriteCmd(0xD51F); LCD_WriteData(0x00);
        LCD_WriteCmd(0xD520); LCD_WriteData(0x02); LCD_WriteCmd(0xD521); LCD_WriteData(0x16);
        LCD_WriteCmd(0xD522); LCD_WriteData(0x02); LCD_WriteCmd(0xD523); LCD_WriteData(0x30);
        LCD_WriteCmd(0xD524); LCD_WriteData(0x02); LCD_WriteCmd(0xD525); LCD_WriteData(0x55);
        LCD_WriteCmd(0xD526); LCD_WriteData(0x02); LCD_WriteCmd(0xD527); LCD_WriteData(0x73);
        LCD_WriteCmd(0xD528); LCD_WriteData(0x02); LCD_WriteCmd(0xD529); LCD_WriteData(0x9C);
        LCD_WriteCmd(0xD52A); LCD_WriteData(0x02); LCD_WriteCmd(0xD52B); LCD_WriteData(0x9C);
        LCD_WriteCmd(0xD52C); LCD_WriteData(0x02); LCD_WriteCmd(0xD52D); LCD_WriteData(0xC4);
        LCD_WriteCmd(0xD52E); LCD_WriteData(0x02); LCD_WriteCmd(0xD52F); LCD_WriteData(0xF0);
        LCD_WriteCmd(0xD530); LCD_WriteData(0x03); LCD_WriteCmd(0xD531); LCD_WriteData(0x0E);
        LCD_WriteCmd(0xD532); LCD_WriteData(0x03); LCD_WriteCmd(0xD533); LCD_WriteData(0x54);

        LCD_WriteCmd(0xD600); LCD_WriteData(0x00); LCD_WriteCmd(0xD601); LCD_WriteData(0x33);
        LCD_WriteCmd(0xD602); LCD_WriteData(0x00); LCD_WriteCmd(0xD603); LCD_WriteData(0x33);
        LCD_WriteCmd(0xD604); LCD_WriteData(0x00); LCD_WriteCmd(0xD605); LCD_WriteData(0x3A);
        LCD_WriteCmd(0xD606); LCD_WriteData(0x00); LCD_WriteCmd(0xD607); LCD_WriteData(0x72);
        LCD_WriteCmd(0xD608); LCD_WriteData(0x00); LCD_WriteCmd(0xD609); LCD_WriteData(0x8F);
        LCD_WriteCmd(0xD60A); LCD_WriteData(0x00); LCD_WriteCmd(0xD60B); LCD_WriteData(0xB0);
        LCD_WriteCmd(0xD60C); LCD_WriteData(0x00); LCD_WriteCmd(0xD60D); LCD_WriteData(0xCA);
        LCD_WriteCmd(0xD60E); LCD_WriteData(0x00); LCD_WriteCmd(0xD60F); LCD_WriteData(0xE5);
        LCD_WriteCmd(0xD610); LCD_WriteData(0x01); LCD_WriteCmd(0xD611); LCD_WriteData(0x00);
        LCD_WriteCmd(0xD612); LCD_WriteData(0x01); LCD_WriteCmd(0xD613); LCD_WriteData(0x2E);
        LCD_WriteCmd(0xD614); LCD_WriteData(0x01); LCD_WriteCmd(0xD615); LCD_WriteData(0x54);
        LCD_WriteCmd(0xD616); LCD_WriteData(0x01); LCD_WriteCmd(0xD617); LCD_WriteData(0x8B);
        LCD_WriteCmd(0xD618); LCD_WriteData(0x01); LCD_WriteCmd(0xD619); LCD_WriteData(0xB7);
        LCD_WriteCmd(0xD61A); LCD_WriteData(0x01); LCD_WriteCmd(0xD61B); LCD_WriteData(0xB8);
        LCD_WriteCmd(0xD61C); LCD_WriteData(0x01); LCD_WriteCmd(0xD61D); LCD_WriteData(0xDC);
        LCD_WriteCmd(0xD61E); LCD_WriteData(0x02); LCD_WriteCmd(0xD61F); LCD_WriteData(0x00);
        LCD_WriteCmd(0xD620); LCD_WriteData(0x02); LCD_WriteCmd(0xD621); LCD_WriteData(0x16);
        LCD_WriteCmd(0xD622); LCD_WriteData(0x02); LCD_WriteCmd(0xD623); LCD_WriteData(0x30);
        LCD_WriteCmd(0xD624); LCD_WriteData(0x02); LCD_WriteCmd(0xD625); LCD_WriteData(0x55);
        LCD_WriteCmd(0xD626); LCD_WriteData(0x02); LCD_WriteCmd(0xD627); LCD_WriteData(0x73);
        LCD_WriteCmd(0xD628); LCD_WriteData(0x02); LCD_WriteCmd(0xD629); LCD_WriteData(0x9C);
        LCD_WriteCmd(0xD62A); LCD_WriteData(0x02); LCD_WriteCmd(0xD62B); LCD_WriteData(0x9C);
        LCD_WriteCmd(0xD62C); LCD_WriteData(0x02); LCD_WriteCmd(0xD62D); LCD_WriteData(0xC4);
        LCD_WriteCmd(0xD62E); LCD_WriteData(0x02); LCD_WriteCmd(0xD62F); LCD_WriteData(0xF0);
        LCD_WriteCmd(0xD630); LCD_WriteData(0x03); LCD_WriteCmd(0xD631); LCD_WriteData(0x0E);
        LCD_WriteCmd(0xD632); LCD_WriteData(0x03); LCD_WriteCmd(0xD633); LCD_WriteData(0x54);

        /* Page 0 设置 */
        LCD_WriteCmd(0xF000); LCD_WriteData(0x55);
        LCD_WriteCmd(0xF001); LCD_WriteData(0xAA);
        LCD_WriteCmd(0xF002); LCD_WriteData(0x52);
        LCD_WriteCmd(0xF003); LCD_WriteData(0x08);
        LCD_WriteCmd(0xF004); LCD_WriteData(0x00);

        LCD_WriteCmd(0xB100); LCD_WriteData(0xCC);
        LCD_WriteCmd(0xB101); LCD_WriteData(0x00);

        LCD_WriteCmd(0xB600); LCD_WriteData(0x05);

        LCD_WriteCmd(0xB700); LCD_WriteData(0x70);
        LCD_WriteCmd(0xB701); LCD_WriteData(0x70);

        LCD_WriteCmd(0xB800); LCD_WriteData(0x01);
        LCD_WriteCmd(0xB801); LCD_WriteData(0x03);
        LCD_WriteCmd(0xB802); LCD_WriteData(0x03);
        LCD_WriteCmd(0xB803); LCD_WriteData(0x03);

        LCD_WriteCmd(0xBC00); LCD_WriteData(0x02);
        LCD_WriteCmd(0xBC01); LCD_WriteData(0x01);
        LCD_WriteCmd(0xBC02); LCD_WriteData(0x06);

        LCD_WriteCmd(0xCC00); LCD_WriteData(0x03);
        LCD_WriteCmd(0xCC01); LCD_WriteData(0x00);

        /* 内存访问控制：竖屏 */
        LCD_WriteCmd(0x3600); LCD_WriteData(0x00);

        /* 像素格式：16-bit RGB565 */
        LCD_WriteCmd(0x3A00); LCD_WriteData(0x55);

        LCD_WriteCmd(0x1100);   /* Sleep Out */
        HAL_Delay(120);
        LCD_WriteCmd(0x2900);   /* Display ON */
        HAL_Delay(10);
    }

    LCD_Clear(COLOR_BG);
}








/* ---------- 基本绘图 ---------- */
void LCD_Clear(uint16_t color)
{
    LCD_SetWindow(0, 0, LCD_W - 1, LCD_H - 1);
    for (uint32_t i = 0; i < (uint32_t)LCD_W * LCD_H; i++) {
        LCD_WriteData(color);
    }
}

void LCD_SetCursor(uint16_t x, uint16_t y)
{
    LCD_SetWindow(x, y, x, y);
}

void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
    LCD_SetWindow(x, y, x, y);
    LCD_WriteData(color);
}

void LCD_Fill(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    LCD_SetWindow(x0, y0, x1, y1);
    uint32_t n = (uint32_t)(x1 - x0 + 1) * (y1 - y0 + 1);
    for (uint32_t i = 0; i < n; i++) {
        LCD_WriteData(color);
    }
}

/* 别名 */
void LCD_FillRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    LCD_Fill(x0, y0, x1, y1, color);
}

void LCD_DrawRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    LCD_DrawLine(x0, y0, x1, y0, color);
    LCD_DrawLine(x0, y1, x1, y1, color);
    LCD_DrawLine(x0, y0, x0, y1, color);
    LCD_DrawLine(x1, y0, x1, y1, color);
}

void LCD_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int dy = (y1 > y0) ? (y1 - y0) : (y0 - y1);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        LCD_DrawPoint(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 <  dx) { err += dx; y0 += sy; }
    }
}

void LCD_DrawCircle(uint16_t xc, uint16_t yc, uint8_t r, uint16_t color)
{
    int x = 0, y = r, d = 1 - r;
    while (x <= y) {
        LCD_DrawPoint(xc + x, yc + y, color);
        LCD_DrawPoint(xc - x, yc + y, color);
        LCD_DrawPoint(xc + x, yc - y, color);
        LCD_DrawPoint(xc - x, yc - y, color);
        LCD_DrawPoint(xc + y, yc + x, color);
        LCD_DrawPoint(xc - y, yc + x, color);
        LCD_DrawPoint(xc + y, yc - x, color);
        LCD_DrawPoint(xc - y, yc - x, color);
        if (d < 0) { d += 2 * x + 3; }
        else       { d += 2 * (x - y) + 5; y--; }
        x++;
    }
}

/* ---------- 字符显示（内嵌 ASCII 字库） ---------- */

/*
 * 最小 8×16 ASCII 字库（空格 ~ 波浪号）
 * 实际项目中通常放在单独的 font.h / font.c 中
 * 这里为简洁只给出框架，完整字库可从正点原子例程复制
 */
extern const unsigned char asc2_1608[][16]; /* 8×16 字库数组，正点原子例程自带 */
extern const unsigned char asc2_2412[][36]; /* 12×24 字库数组 */

void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t ch, uint8_t size,
                  uint16_t color, uint16_t bg)
{
    uint8_t csize, t, temp;
    uint16_t x0 = x;
    const unsigned char *pfont;

    ch = ch - ' ';  /* 得到偏移后的值 */

    if (size == 16) {
        csize = 16;
        pfont = asc2_1608[ch];
    } else if (size == 24) {
        csize = 36;
        pfont = asc2_2412[ch];
    } else {
        return;
    }

    uint8_t char_w = size / 2;  /* 8 or 12 */

    for (t = 0; t < csize; t++) {
        temp = pfont[t];
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (temp & 0x80)
                LCD_DrawPoint(x, y, color);
            else
                LCD_DrawPoint(x, y, bg);
            temp <<= 1;
            x++;
            if ((x - x0) >= char_w) {
                x = x0;
                y++;
                break;
            }
        }
    }
}

void LCD_ShowString(uint16_t x, uint16_t y, const char *str,
                    uint8_t size, uint16_t color, uint16_t bg)
{
    uint8_t char_w = size / 2;
    while (*str) {
        LCD_ShowChar(x, y, *str++, size, color, bg);
        x += char_w;
        if (x + char_w > LCD_W) { x = 0; y += size; }
    }
}

void LCD_ShowNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len,
                 uint8_t size, uint16_t color, uint16_t bg)
{
    char buf[12];
    snprintf(buf, sizeof(buf), "%*lu", len, (unsigned long)num);
    LCD_ShowString(x, y, buf, size, color, bg);
}

/* 格式化打印（最方便的接口） */
void LCD_Printf(uint16_t x, uint16_t y, uint8_t size,
                uint16_t color, uint16_t bg, const char *fmt, ...)
{
    char buf[64];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    LCD_ShowString(x, y, buf, size, color, bg);
}
