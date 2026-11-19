/* ==================== app_comm.c ==================== */
#include "app_comm.h"
#include "bsp_eeprom.h"

static UART_HandleTypeDef *pHuart = NULL;

#define RX_BUF_SIZE  64
static uint8_t  g_rx_buf[RX_BUF_SIZE];
static uint8_t  g_rx_byte;
static volatile uint8_t g_rx_head = 0;

void Comm_Init(UART_HandleTypeDef *huart)
{
    pHuart = huart;
    HAL_UART_Receive_IT(pHuart, &g_rx_byte, 1);

    Comm_SendString("\r\n=============================\r\n");
    Comm_SendString("  SmartGreenhouse " FW_VERSION "\r\n");
    Comm_SendString("  STM32F103ZET6 + TFTLCD\r\n");
    Comm_SendString("=============================\r\n");
}

void Comm_SendString(const char *str)
{
    HAL_UART_Transmit(pHuart, (uint8_t *)str, strlen(str), 200);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == pHuart) {
        if (g_rx_head < RX_BUF_SIZE - 1)
            g_rx_buf[g_rx_head++] = g_rx_byte;
        HAL_UART_Receive_IT(pHuart, &g_rx_byte, 1);
    }
}

void Comm_SendSensorCSV(SensorData_t *d)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "%.1f,%.1f,%u,%u\r\n",
             d->temperature, d->humidity, d->soil, d->light);
    Comm_SendString(buf);
}

void Comm_PollCommand(SystemMode_t *mode)
{
    if (g_rx_head == 0) return;

    uint8_t got_line = 0;
    for (uint8_t i = 0; i < g_rx_head; i++) {
        if (g_rx_buf[i] == '\r' || g_rx_buf[i] == '\n') {
            g_rx_buf[i] = '\0';
            got_line = 1;
            break;
        }
    }
    if (!got_line) return;

    if (strcmp((char *)g_rx_buf, "AUTO") == 0) {
        *mode = MODE_AUTO;
        Comm_SendString("[OK] MODE=AUTO\r\n");
    } else if (strcmp((char *)g_rx_buf, "MANUAL") == 0) {
        *mode = MODE_MANUAL;
        Comm_SendString("[OK] MODE=MANUAL\r\n");
    } else if (strcmp((char *)g_rx_buf, "DUMP") == 0) {
        EEPROM_DumpAll(Comm_SendString);
    } else {
        Comm_SendString("[ERR] Unknown cmd\r\n");
    }

    g_rx_head = 0;
    memset(g_rx_buf, 0, RX_BUF_SIZE);
}
