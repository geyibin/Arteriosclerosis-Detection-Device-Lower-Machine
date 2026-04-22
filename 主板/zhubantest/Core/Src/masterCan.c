#include "masterCan.h"
#include <stdio.h>
#include <string.h>

extern CAN_HandleTypeDef hcan;

static MasterCAN_SensorData_t g_mcan_sensor_data;
static uint8_t g_mcan_last_node_id = 0;

#define MCAN_MIN_NODE_ID    0x01U
#define MCAN_MAX_NODE_ID    0x04U

/* 从机数据帧ID基址：0x181 = 0x180 + 1 */
#define MCAN_ID_SLAVE_DATA_BASE   0x180U

void MasterCAN_Init(void)
{
    CAN_FilterTypeDef canFilter;

    memset(&g_mcan_sensor_data, 0, sizeof(g_mcan_sensor_data));
    g_mcan_last_node_id = 0;

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
        printf("MasterCAN filter config fail\r\n");
    }
    else
    {
        printf("MasterCAN filter config ok\r\n");
    }
}

HAL_StatusTypeDef MasterCAN_Start(void)
{
    if (HAL_CAN_Start(&hcan) != HAL_OK)
    {
        printf("MasterCAN start fail\r\n");
        return HAL_ERROR;
    }

    printf("MasterCAN start ok\r\n");
    return HAL_OK;
}

HAL_StatusTypeDef MasterCAN_SendCtrlByte(uint8_t node_id, uint8_t ctrl)
{
    CAN_TxHeaderTypeDef txHeader;
    uint32_t txMailbox;
    uint8_t txData[1];

    if ((node_id < MCAN_MIN_NODE_ID) || (node_id > MCAN_MAX_NODE_ID))
    {
        printf("MasterCAN invalid node id: %d\r\n", node_id);
        return HAL_ERROR;
    }

    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) == 0U)
    {
        printf("MasterCAN tx mailbox full\r\n");
        return HAL_BUSY;
    }

    txData[0] = ctrl;

    txHeader.StdId = 0x100U + node_id;   /* 从机1=0x101，从机2=0x102 */
    txHeader.ExtId = 0x00;
    txHeader.IDE   = CAN_ID_STD;
    txHeader.RTR   = CAN_RTR_DATA;
    txHeader.DLC   = 1;
    txHeader.TransmitGlobalTime = DISABLE;

    if (HAL_CAN_AddTxMessage(&hcan, &txHeader, txData, &txMailbox) != HAL_OK)
    {
        printf("MasterCAN send fail, err=0x%08lX\r\n", HAL_CAN_GetError(&hcan));
        return HAL_ERROR;
    }

    printf("CAN TX -> slave %d: ID=0x%03lX DATA=%02X\r\n",
           node_id,
           (unsigned long)txHeader.StdId,
           ctrl);

    return HAL_OK;
}

void MasterCAN_PollRx(void)
{
    CAN_RxHeaderTypeDef rxHeader;
    uint8_t rxData[8];
    uint8_t node_id;

    while (HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0) > 0U)
    {
        if (HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &rxHeader, rxData) != HAL_OK)
        {
            printf("MasterCAN rx fail, err=0x%08lX\r\n", HAL_CAN_GetError(&hcan));
            return;
        }

        if (rxHeader.IDE != CAN_ID_STD)
        {
            continue;
        }

        /* 接收从机1~4的数据帧：0x181 ~ 0x184 */
        if ((rxHeader.StdId < (MCAN_ID_SLAVE_DATA_BASE + MCAN_MIN_NODE_ID)) ||
            (rxHeader.StdId > (MCAN_ID_SLAVE_DATA_BASE + MCAN_MAX_NODE_ID)))
        {
            continue;
        }

        if (rxHeader.DLC < 4U)
        {
            continue;
        }

        node_id = (uint8_t)(rxHeader.StdId - MCAN_ID_SLAVE_DATA_BASE);
        g_mcan_last_node_id = node_id;

        g_mcan_sensor_data.pressure_x100 = (int16_t)(((uint16_t)rxData[0] << 8) | rxData[1]);
        g_mcan_sensor_data.temp_x100     = (int16_t)(((uint16_t)rxData[2] << 8) | rxData[3]);

        g_mcan_sensor_data.pressure_kpa = g_mcan_sensor_data.pressure_x100 / 100.0f;
        g_mcan_sensor_data.temp_c       = g_mcan_sensor_data.temp_x100 / 100.0f;

        printf("CAN RX <- slave %d: P=%.4f kPa, T=%.2f C\r\n",
               node_id,
               g_mcan_sensor_data.pressure_kpa,
               g_mcan_sensor_data.temp_c);
    }
}

const MasterCAN_SensorData_t* MasterCAN_GetLastSensorData(void)
{
    return &g_mcan_sensor_data;
}