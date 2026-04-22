#ifndef __MASTER_CAN_H
#define __MASTER_CAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "can.h"
#include <stdint.h>

#define MCAN_ID_MASTER_TO_SLAVE_CTRL   0x101U
#define MCAN_ID_SLAVE_TO_MASTER_DATA   0x181U

typedef struct
{
    float pressure_kpa;
    float temp_c;
    int16_t pressure_x100;
    int16_t temp_x100;
} MasterCAN_SensorData_t;

void MasterCAN_Init(void);
HAL_StatusTypeDef MasterCAN_Start(void);
void MasterCAN_PollRx(void);
HAL_StatusTypeDef MasterCAN_SendCtrlByte(uint8_t node_id, uint8_t ctrl);

const MasterCAN_SensorData_t* MasterCAN_GetLastSensorData(void);

#ifdef __cplusplus
}
#endif

#endif