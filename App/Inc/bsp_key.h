/* ==================== bsp_key.h ==================== */
#ifndef __BSP_KEY_H
#define __BSP_KEY_H

#include "config.h"

#define KEY_NONE     0
#define KEY_MODE     1
#define KEY_SELECT   2
#define KEY_UP       3
#define KEY_DOWN     4

uint8_t Key_Scan(void);

#endif
