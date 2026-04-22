#include "userCan.h"
#include <string.h>
#include <stdio.h>

extern CAN_HandleTypeDef hcan;

static UserCAN_RxCmd_t g_userCanRxCmd;
static uint8_t g_send_request_flag = 0;

/* =========================
 * 工具函数
 * ========================= */
static int16_t UserCAN_FloatToX100(float value)
{
    float temp;

    if (value > 327.67f)  value = 327.67f;
    if (value < -327.68f) value = -327.68f;

    temp = value * 100.0f;

    if (temp >= 0.0f)
        temp += 0.5f;
    else
        temp -= 0.5f;

    return (int16_t)temp;
}

static void UserCAN_ParseCtrlByte(uint8_t ctrl, UserCAN_RxCmd_t *cmd)
{
    if (cmd == NULL)
        return;

    cmd->request_send = (ctrl & UCAN_REQ_SEND_BIT) ? 1U : 0U;
    cmd->pump_run     = (ctrl & UCAN_PUMP_RUN_BIT) ? 1U : 0U;
    cmd->valve1_close = (ctrl & UCAN_VALVE1_CLOSE_BIT) ? 1U : 0U;
    cmd->valve2_close = (ctrl & UCAN_VALVE2_CLOSE_BIT) ? 1U : 0U;
}

/* =========================
 * 初始化
 * ========================= */
void UserCAN_Init(void)
{
    CAN_FilterTypeDef canFilter;

    memset(&g_userCanRxCmd, 0, sizeof(g_userCanRxCmd));
    g_send_request_flag = 0;

    /* 联调阶段先全放行 */
    canFilter.FilterBank = 0;
    canFilter.FilterMode = CAN_FILTERMODE_IDMASK;
    canFilter.FilterScale = CAN_FILTERSCALE_32BIT;
    canFilter.FilterIdHigh = 0x0000;
    canFilter.FilterIdLow = 0x0000;
    canFilter.FilterMaskIdHigh = 0x0000;
    canFilter.FilterMaskIdLow = 0x0000;
    canFilter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    canFilter.FilterActivation = ENABLE;

    if (HAL_CAN_ConfigFilter(&hcan, &canFilter) != HAL_OK)
    {
        printf("HAL_CAN_ConfigFilter fail\r\n");
    }
    else
    {
        printf("HAL_CAN_ConfigFilter ok\r\n");
    }
}

HAL_StatusTypeDef UserCAN_Start(void)
{
    if (HAL_CAN_Start(&hcan) != HAL_OK)
    {
        printf("HAL_CAN_Start fail\r\n");
        return HAL_ERROR;
    }

    printf("HAL_CAN_Start ok\r\n");
    return HAL_OK;
}

/* =========================
 * 发送传感器数据
 * 大端发送:
 * [P_H][P_L][T_H][T_L]
 * ========================= */
HAL_StatusTypeDef UserCAN_SendSensorData(float pressure_kpa, float temp_c)
{
    CAN_TxHeaderTypeDef txHeader;
    uint32_t txMailbox;
    uint8_t txData[4];
    int16_t pressure_x100;
    int16_t temp_x100;

    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) == 0U)
    {
        return HAL_BUSY;
    }

    pressure_x100 = UserCAN_FloatToX100(pressure_kpa);
    temp_x100     = UserCAN_FloatToX100(temp_c);

    txData[0] = (uint8_t)((pressure_x100 >> 8) & 0xFF);
    txData[1] = (uint8_t)(pressure_x100 & 0xFF);
    txData[2] = (uint8_t)((temp_x100 >> 8) & 0xFF);
    txData[3] = (uint8_t)(temp_x100 & 0xFF);

    txHeader.StdId = UCAN_ID_SLAVE_TO_MASTER_DATA;
    txHeader.ExtId = 0x00;
    txHeader.IDE   = CAN_ID_STD;
    txHeader.RTR   = CAN_RTR_DATA;
    txHeader.DLC   = 4;
    txHeader.TransmitGlobalTime = DISABLE;

    if (HAL_CAN_AddTxMessage(&hcan, &txHeader, txData, &txMailbox) != HAL_OK)
    {
        printf("UserCAN_SendSensorData fail, err=0x%08lX\r\n", HAL_CAN_GetError(&hcan));
        return HAL_ERROR;
    }

    return HAL_OK;
}

/* =========================
 * 轮询接收
 * ========================= */
void UserCAN_PollRx(void)
{
    CAN_RxHeaderTypeDef rxHeader;
    uint8_t rxData[8];

    while (HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0) > 0U)
    {
        if (HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &rxHeader, rxData) != HAL_OK)
        {
            printf("HAL_CAN_GetRxMessage fail, err=0x%08lX\r\n", HAL_CAN_GetError(&hcan));
            return;
        }

        printf("SLAVE CAN RX: StdId=0x%03lX DLC=%d Data=",
               (unsigned long)rxHeader.StdId,
               rxHeader.DLC);
        for (uint8_t i = 0; i < rxHeader.DLC; i++)
        {
            printf("%02X ", rxData[i]);
        }
        printf("\r\n");

        if (rxHeader.IDE != CAN_ID_STD)
        {
            printf("ignore: not std frame\r\n");
            continue;
        }

        if (rxHeader.StdId != UCAN_ID_MASTER_TO_SLAVE_CTRL)
        {
            printf("ignore: id not match\r\n");
            continue;
        }

        if (rxHeader.DLC < 1U)
        {
            printf("ignore: dlc < 1\r\n");
            continue;
        }

        UserCAN_ParseCtrlByte(rxData[0], &g_userCanRxCmd);

        printf("cmd parse: req=%d pump=%d v1_close=%d v2_close=%d\r\n",
               g_userCanRxCmd.request_send,
               g_userCanRxCmd.pump_run,
               g_userCanRxCmd.valve1_close,
               g_userCanRxCmd.valve2_close);

        /* 先执行输出 */
        UserCAN_ApplyOutputs(&g_userCanRxCmd);

        printf("outputs applied\r\n");

        /* 再处理发送请求 */
        if (g_userCanRxCmd.request_send)
        {
            g_send_request_flag = 1;
            printf("send request set\r\n");
        }
    }
}

const UserCAN_RxCmd_t* UserCAN_GetLastCmd(void)
{
    return &g_userCanRxCmd;
}

uint8_t UserCAN_ConsumeSendRequest(void)
{
    uint8_t flag = g_send_request_flag;
    g_send_request_flag = 0;
    return flag;
}

/* =========================
 * 输出控制
 * 泵：
 * PB3 和 PA15 同电平 -> 停
 * PB3 和 PA15 不同电平 -> 转
 *
 * 这里固定成：
 * 运行：PB3=1, PA15=0
 * 停止：PB3=0, PA15=0
 *
 * 阀：
 * PA5 = 阀1，高电平闭合
 * PA6 = 阀2，高电平闭合
 * ========================= */
void UserCAN_ApplyOutputs(const UserCAN_RxCmd_t *cmd)
{
    if (cmd == NULL)
        return;

    UserCAN_SetPump(cmd->pump_run);
    UserCAN_SetValve1(cmd->valve1_close);
    UserCAN_SetValve2(cmd->valve2_close);
}

void UserCAN_SetPump(uint8_t run)
{
    if (run)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);
        printf("SetPump: RUN  -> PB3=1 PA15=0\r\n");
    }
    else
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);
        printf("SetPump: STOP -> PB3=0 PA15=0\r\n");
    }
}

void UserCAN_SetValve1(uint8_t close)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, close ? GPIO_PIN_SET : GPIO_PIN_RESET);
    printf("SetValve1: %s\r\n", close ? "CLOSE" : "OPEN");
}

void UserCAN_SetValve2(uint8_t close)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, close ? GPIO_PIN_SET : GPIO_PIN_RESET);
    printf("SetValve2: %s\r\n", close ? "CLOSE" : "OPEN");
}