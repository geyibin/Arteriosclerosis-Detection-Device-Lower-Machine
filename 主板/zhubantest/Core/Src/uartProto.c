#include "uartProto.h"
#include "masterCan.h"
#include <stdio.h>

extern UART_HandleTypeDef huart1;

#define UART_PROTO_MAX_LEN   8

typedef enum
{
    UART_WAIT_AA = 0,
    UART_WAIT_55,
    UART_WAIT_CMD,
    UART_WAIT_LEN,
    UART_WAIT_DATA,
    UART_WAIT_CS
} UartProto_State_t;

static UartProto_State_t g_uart_state = UART_WAIT_AA;
static uint8_t g_uart_rx_byte = 0;
static uint8_t g_uart_cmd = 0;
static uint8_t g_uart_len = 0;
static uint8_t g_uart_data[UART_PROTO_MAX_LEN];
static uint8_t g_uart_index = 0;

static uint8_t UartProto_Checksum(uint8_t cmd, uint8_t len, uint8_t *data)
{
    uint16_t sum = 0;
    uint8_t i;

    sum += cmd;
    sum += len;

    for (i = 0; i < len; i++)
    {
        sum += data[i];
    }

    return (uint8_t)(sum & 0xFF);
}

static void UartProto_Reset(void)
{
    g_uart_state = UART_WAIT_AA;
    g_uart_cmd = 0;
    g_uart_len = 0;
    g_uart_index = 0;
}

static void UartProto_HandleFrame(uint8_t cmd, uint8_t len, uint8_t *data)
{
    HAL_StatusTypeDef ret;
    uint8_t node_id;
    uint8_t ctrl;

    printf("UART_PROTO_V2 ENTER: cmd=%02X len=%02X d0=%02X d1=%02X\r\n",
           cmd, len, data[0], data[1]);

    if (cmd == 0x01)
    {
        if (len != 2)
        {
            printf("UART_PROTO_V2 len error: len=%02X\r\n", len);
            return;
        }

        node_id = data[0];
        ctrl    = data[1];

        ret = MasterCAN_SendCtrlByte(node_id, ctrl);
        if (ret == HAL_OK)
        {
            printf("UART_PROTO_V2 ok, node=%d ctrl=%02X\r\n", node_id, ctrl);
        }
        else
        {
            printf("UART_PROTO_V2 fail, node=%d ctrl=%02X\r\n", node_id, ctrl);
        }
    }
    else
    {
        printf("UART_PROTO_V2 unknown cmd=%02X\r\n", cmd);
    }
}

void UartProto_Init(void)
{
    UartProto_Reset();
    HAL_UART_Receive_IT(&huart1, &g_uart_rx_byte, 1);
    printf("UART proto init ok\r\n");
}

void UartProto_RxCpltCallback(UART_HandleTypeDef *huart)
{
    uint8_t cs;

    if (huart->Instance != USART1)
    {
        return;
    }

    switch (g_uart_state)
    {
        case UART_WAIT_AA:
            if (g_uart_rx_byte == 0xAA)
            {
                g_uart_state = UART_WAIT_55;
            }
            break;

        case UART_WAIT_55:
            if (g_uart_rx_byte == 0x55)
            {
                g_uart_state = UART_WAIT_CMD;
            }
            else
            {
                UartProto_Reset();
            }
            break;

        case UART_WAIT_CMD:
            g_uart_cmd = g_uart_rx_byte;
            g_uart_state = UART_WAIT_LEN;
            break;

        case UART_WAIT_LEN:
            g_uart_len = g_uart_rx_byte;
            if (g_uart_len > UART_PROTO_MAX_LEN)
            {
                printf("UART len too long\r\n");
                UartProto_Reset();
            }
            else if (g_uart_len == 0)
            {
                g_uart_state = UART_WAIT_CS;
            }
            else
            {
                g_uart_index = 0;
                g_uart_state = UART_WAIT_DATA;
            }
            break;

        case UART_WAIT_DATA:
            g_uart_data[g_uart_index++] = g_uart_rx_byte;
            if (g_uart_index >= g_uart_len)
            {
                g_uart_state = UART_WAIT_CS;
            }
            break;

        case UART_WAIT_CS:
            cs = UartProto_Checksum(g_uart_cmd, g_uart_len, g_uart_data);
            if (cs == g_uart_rx_byte)
            {
                UartProto_HandleFrame(g_uart_cmd, g_uart_len, g_uart_data);
            }
            else
            {
                printf("UART checksum error, calc=%02X recv=%02X\r\n", cs, g_uart_rx_byte);
            }
            UartProto_Reset();
            break;

        default:
            UartProto_Reset();
            break;
    }

    HAL_UART_Receive_IT(&huart1, &g_uart_rx_byte, 1);
}