#ifndef __USER_CAN_H
#define __USER_CAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "can.h"
#include <stdint.h>

/* =========================
 * CAN ID
 * ========================= */
#define UCAN_NODE_ID   0x01U

#define UCAN_ID_MASTER_TO_SLAVE_CTRL   (0x100U + UCAN_NODE_ID)
#define UCAN_ID_SLAVE_TO_MASTER_DATA   (0x180U + UCAN_NODE_ID)

/* =========================
 * 主机->从机 控制位定义
 * Byte0
 * ========================= */
#define UCAN_REQ_SEND_BIT              (1U << 0)
#define UCAN_PUMP_RUN_BIT              (1U << 1)
#define UCAN_VALVE1_CLOSE_BIT          (1U << 2)
#define UCAN_VALVE2_CLOSE_BIT          (1U << 3)

/* =========================
 * 接收命令结构体
 * ========================= */
typedef struct
{
    uint8_t request_send;
    uint8_t pump_run;
    uint8_t valve1_close;
    uint8_t valve2_close;
} UserCAN_RxCmd_t;

/* =========================
 * 发送数据结构体
 * 两位小数，发送 x100 后的整数
 * ========================= */
typedef struct
{
    int16_t pressure_x100;   // kPa * 100
    int16_t temp_x100;       // ℃ * 100
} UserCAN_TxData_t;

/* =========================
 * 接口
 * ========================= */
void UserCAN_Init(void);
HAL_StatusTypeDef UserCAN_Start(void);
void UserCAN_PollRx(void);

HAL_StatusTypeDef UserCAN_SendSensorData(float pressure_kpa, float temp_c);

const UserCAN_RxCmd_t* UserCAN_GetLastCmd(void);
uint8_t UserCAN_ConsumeSendRequest(void);

/* 输出控制 */
void UserCAN_ApplyOutputs(const UserCAN_RxCmd_t *cmd);
void UserCAN_SetPump(uint8_t run);
void UserCAN_SetValve1(uint8_t close);
void UserCAN_SetValve2(uint8_t close);

#ifdef __cplusplus
}
#endif

#endif