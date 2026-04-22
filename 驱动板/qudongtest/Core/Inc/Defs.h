#ifndef __DEFS_H
#define __DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include "stdlib.h"
#include "string.h"

//void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

void Error_Handler(void);

/* Private defines -----------------------------------------------------------*/
#define Motor1_cs_Pin GPIO_PIN_0
#define Motor1_cs_GPIO_Port GPIOE
#define Motor2_cs_Pin GPIO_PIN_1
#define Motor2_cs_GPIO_Port GPIOE
#define Motor3_cs_Pin GPIO_PIN_2
#define Motor3_cs_GPIO_Port GPIOE
#define Motor4_cs_Pin GPIO_PIN_3
#define Motor4_cs_GPIO_Port GPIOE
#define Motor5_cs_Pin GPIO_PIN_4
#define Motor5_cs_GPIO_Port GPIOE
#define Motor6_cs_Pin GPIO_PIN_5
#define Motor6_cs_GPIO_Port GPIOE
#define Motor7_cs_Pin GPIO_PIN_6
#define Motor7_cs_GPIO_Port GPIOE
#define Motor8_cs_Pin GPIO_PIN_7
#define Motor8_cs_GPIO_Port GPIOE
#define Motor9_cs_Pin GPIO_PIN_8
#define Motor9_cs_GPIO_Port GPIOE
#define Motor10_cs_Pin GPIO_PIN_9
#define Motor10_cs_GPIO_Port GPIOE
#define Motor11_cs_Pin GPIO_PIN_10
#define Motor11_cs_GPIO_Port GPIOE
#define Motor12_cs_Pin GPIO_PIN_11
#define Motor12_cs_GPIO_Port GPIOE
#define Motor13_cs_Pin GPIO_PIN_12
#define Motor13_cs_GPIO_Port GPIOE
#define Motor14_cs_Pin GPIO_PIN_13
#define Motor14_cs_GPIO_Port GPIOE
#define Motor15_cs_Pin GPIO_PIN_14
#define Motor15_cs_GPIO_Port GPIOE
#define Motor16_cs_Pin GPIO_PIN_15
#define Motor16_cs_GPIO_Port GPIOE

#define Exti_Gpio1_Pin GPIO_PIN_0
#define Exti_Gpio1_GPIO_Port GPIOF
#define Exti_Gpio2_Pin GPIO_PIN_1
#define Exti_Gpio2_GPIO_Port GPIOF
#define Exti_Gpio3_Pin GPIO_PIN_2
#define Exti_Gpio3_GPIO_Port GPIOF
#define Exti_Gpio4_Pin GPIO_PIN_3
#define Exti_Gpio4_GPIO_Port GPIOF
#define Exti_Gpio5_Pin GPIO_PIN_4
#define Exti_Gpio5_GPIO_Port GPIOF
#define Exti_Gpio6_Pin GPIO_PIN_5
#define Exti_Gpio6_GPIO_Port GPIOF
#define Exti_Gpio7_Pin GPIO_PIN_6
#define Exti_Gpio7_GPIO_Port GPIOF
#define Exti_Gpio8_Pin GPIO_PIN_7
#define Exti_Gpio8_GPIO_Port GPIOF

#define Exti_Gpio1_VCC_Pin GPIO_PIN_8
#define Exti_Gpio1_VCC_GPIO_Port GPIOF
#define Exti_Gpio2_VCC_Pin GPIO_PIN_9
#define Exti_Gpio2_VCC_GPIO_Port GPIOF
#define Exti_Gpio3_VCC_Pin GPIO_PIN_10
#define Exti_Gpio3_VCC_GPIO_Port GPIOF
#define Exti_Gpio4_VCC_Pin GPIO_PIN_11
#define Exti_Gpio4_VCC_GPIO_Port GPIOF
#define Exti_Gpio5_VCC_Pin GPIO_PIN_12
#define Exti_Gpio5_VCC_GPIO_Port GPIOF
#define Exti_Gpio6_VCC_Pin GPIO_PIN_13
#define Exti_Gpio6_VCC_GPIO_Port GPIOF
#define Exti_Gpio7_VCC_Pin GPIO_PIN_14
#define Exti_Gpio7_VCC_GPIO_Port GPIOF
#define Exti_Gpio8_VCC_Pin GPIO_PIN_15
#define Exti_Gpio8_VCC_GPIO_Port GPIOF

#define Relay1_Pin GPIO_PIN_0
#define Relay1_GPIO_Port GPIOG
#define Relay2_Pin GPIO_PIN_1
#define Relay2_GPIO_Port GPIOG
#define Relay3_Pin GPIO_PIN_2
#define Relay3_GPIO_Port GPIOG
#define Relay4_Pin GPIO_PIN_3
#define Relay4_GPIO_Port GPIOG

#define Signal_in_Pin GPIO_PIN_0
#define Signal_in_GPIO_Port GPIOB
#define Signal_out_Pin GPIO_PIN_1
#define Signal_out_GPIO_Port GPIOB

#define DCmotor1__Pin GPIO_PIN_6
#define DCmotor1__GPIO_Port GPIOC
#define DCmotor1_C7_Pin GPIO_PIN_7
#define DCmotor1_C7_GPIO_Port GPIOC
#define DCmotor2__Pin GPIO_PIN_8
#define DCmotor2__GPIO_Port GPIOC
#define DCmotor2_C9_Pin GPIO_PIN_9
#define DCmotor2_C9_GPIO_Port GPIOC
#define DCmotor4__Pin GPIO_PIN_8
#define DCmotor4__GPIO_Port GPIOB
#define DCmotor4_B9_Pin GPIO_PIN_9
#define DCmotor4_B9_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */




#ifdef __cplusplus
}
#endif

#endif /* __DEFS_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
