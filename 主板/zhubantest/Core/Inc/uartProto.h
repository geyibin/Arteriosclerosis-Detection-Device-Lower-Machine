#ifndef __UART_PROTO_H
#define __UART_PROTO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "usart.h"
#include <stdint.h>

void UartProto_Init(void);
void UartProto_RxCpltCallback(UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif

#endif