/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "can.h"
#include "i2c.h"
#include "iwdg.h"
#include "usart.h"
#include "gpio.h"
#include "pressureTest.h"
#include <stdio.h>
#include "userCan.h"
#pragma import(__use_no_semihosting)
PressureTest_HandleTypeDef g_pressure_dev;
PressureTest_DataTypeDef   g_pressure_data;
extern PressureTest_HandleTypeDef g_pressure_dev;
PressureTest_DataTypeDef data;
uint8_t pressure_sensor_ok = 0;
struct __FILE
{
  int handle;
};

FILE __stdout;
FILE __stdin;

void _sys_exit(int x)
{
  (void)x;
}
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
static int16_t Debug_FloatToX100(float value)
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

static void Debug_PrintSensorAndCanFrame(float pressure_kpa, float temp_c)
{
    int16_t pressure_x100;
    int16_t temp_x100;
    uint8_t txData[4];

    pressure_x100 = Debug_FloatToX100(pressure_kpa);
    temp_x100     = Debug_FloatToX100(temp_c);

    txData[0] = (uint8_t)((pressure_x100 >> 8) & 0xFF);
    txData[1] = (uint8_t)(pressure_x100 & 0xFF);
    txData[2] = (uint8_t)((temp_x100 >> 8) & 0xFF);
    txData[3] = (uint8_t)(temp_x100 & 0xFF);

    printf("sensor: P=%.2f kPa, T=%.2f C\r\n", pressure_kpa, temp_c);
    printf("x100  : P=%d, T=%d\r\n", pressure_x100, temp_x100);
    printf("CAN TX: %02X %02X %02X %02X\r\n",
           txData[0], txData[1], txData[2], txData[3]);
}
int main(void)
{
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_CAN_Init();
  MX_I2C1_Init();
  // MX_IWDG_Init();	
  MX_USART1_UART_Init();
UserCAN_Init();
printf("stm32 init done\r\n");

PressureTest_Init(&g_pressure_dev, &hi2c1, 20.0f, 0);
printf("pressure init ok, k=%d\r\n", g_pressure_dev.k_value);

UserCAN_Init();
if (UserCAN_Start() == HAL_OK)
{
    printf("CAN start ok\r\n");
}
else
{
    printf("CAN start fail\r\n");
}
while (1)
{
    UserCAN_PollRx();

    if (UserCAN_ConsumeSendRequest())
    {
        if (PressureTest_ReadOnce(&g_pressure_dev, &g_pressure_data) == HAL_OK)
        {
            if (UserCAN_SendSensorData(g_pressure_data.pressure_kpa,
                                       g_pressure_data.temperature_c) == HAL_OK)
            {
                printf("CAN send: P=%.4f kPa, T=%.2f C\r\n",
                       g_pressure_data.pressure_kpa,
                       g_pressure_data.temperature_c);
            }
            else
            {
                printf("CAN send fail\r\n");
            }
        }
        else
        {
            printf("pressure read fail\r\n");
        }
    }
}

}
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
