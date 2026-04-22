#ifndef _PRESSURETEST_H
#define _PRESSURETEST_H
#include "stm32f1xx_hal.h"

/* =========================
 * XGZP6859D I2C 基本参数
 * ========================= */
#define XGZP6859_I2C_ADDR_7BIT        (0x6D)
#define XGZP6859_I2C_ADDR             (XGZP6859_I2C_ADDR_7BIT << 1)   /* HAL使用8位地址 */

#define PRESSURETEST_I2C_TIMEOUT      100

/* =========================
 * 寄存器地址
 * ========================= */
#define XGZP6859_REG_DATA_MSB         0x06
#define XGZP6859_REG_DATA_CSB         0x07
#define XGZP6859_REG_DATA_LSB         0x08

#define XGZP6859_REG_TEMP_MSB         0x09
#define XGZP6859_REG_TEMP_LSB         0x0A

#define XGZP6859_REG_CMD              0x30
#define XGZP6859_REG_SYS_CONFIG       0xA5
#define XGZP6859_REG_P_CONFIG         0xA6

/* =========================
 * CMD寄存器 bit定义
 * [7:4] sleep_time
 * [3]   sco
 * [2:0] measurement_control
 * ========================= */
#define XGZP6859_CMD_SCO_BIT          (1U << 3)

/* measurement_control<2:0> */
typedef enum
{
    XGZP6859_MODE_SINGLE_TEMP        = 0x00,   /* 000b 单次温度采集 */
    XGZP6859_MODE_SINGLE_PRESSURE    = 0x01,   /* 001b 单次压力采集 */
    XGZP6859_MODE_COMBINED_ONCE      = 0x02,   /* 010b 组合采集一次 */
    XGZP6859_MODE_SLEEP_PERIODIC     = 0x03    /* 011b 休眠周期采集 */
} XGZP6859_Mode_t;

/* sleep_time<7:4>，仅休眠工作模式有效 */
typedef enum
{
    XGZP6859_SLEEP_INVALID   = 0x00,
    XGZP6859_SLEEP_62P5MS    = 0x01,
    XGZP6859_SLEEP_125MS     = 0x02,
    XGZP6859_SLEEP_187P5MS   = 0x03,
    XGZP6859_SLEEP_250MS     = 0x04,
    XGZP6859_SLEEP_312P5MS   = 0x05,
    XGZP6859_SLEEP_375MS     = 0x06,
    XGZP6859_SLEEP_437P5MS   = 0x07,
    XGZP6859_SLEEP_500MS     = 0x08,
    XGZP6859_SLEEP_562P5MS   = 0x09,
    XGZP6859_SLEEP_625MS     = 0x0A,
    XGZP6859_SLEEP_687P5MS   = 0x0B,
    XGZP6859_SLEEP_750MS     = 0x0C,
    XGZP6859_SLEEP_812P5MS   = 0x0D,
    XGZP6859_SLEEP_875MS     = 0x0E,
    XGZP6859_SLEEP_1000MS    = 0x0F
} XGZP6859_SleepTime_t;

/* =========================
 * P_CONFIG寄存器 bit定义
 * ========================= */
/* Gain_P<5:3> */
typedef enum
{
    XGZP6859_GAIN_1X    = 0x00,
    XGZP6859_GAIN_2X    = 0x01,
    XGZP6859_GAIN_4X    = 0x02,
    XGZP6859_GAIN_8X    = 0x03,
    XGZP6859_GAIN_16X   = 0x04,
    XGZP6859_GAIN_32X   = 0x05,
    XGZP6859_GAIN_64X   = 0x06,
    XGZP6859_GAIN_128X  = 0x07
} XGZP6859_Gain_t;

/* OSR_P<2:0> */
typedef enum
{
    XGZP6859_OSR_1024   = 0x00,
    XGZP6859_OSR_2048   = 0x01,
    XGZP6859_OSR_4096   = 0x02,
    XGZP6859_OSR_8192   = 0x03,
    XGZP6859_OSR_256    = 0x04,
    XGZP6859_OSR_512    = 0x05,
    XGZP6859_OSR_16384  = 0x06,
    XGZP6859_OSR_32768  = 0x07
} XGZP6859_Osr_t;

/* =========================
 * 数据结构体
 * ========================= */
typedef struct
{
    I2C_HandleTypeDef *hi2c;

    /* 量程信息，用于压力换算 */
    float span_kpa;          /* 量程跨度，例如 0~20kPa 填20，-20~0kPa填20，-40~40kPa填80 */
    uint16_t k_value;        /* 按手册查表得到的k值 */

    /* 输出格式标记 */
    uint8_t unipolar;        /* 0: 有符号输出  1: 无符号输出（仅当Data_out_control=1时有效） */
} PressureTest_HandleTypeDef;

typedef struct
{
    uint32_t pressure_adc;   /* 24位原始压力ADC */
    uint16_t temp_adc;       /* 16位原始温度ADC */

    float pressure_kpa;      /* 换算后的压力 */
    float temperature_c;     /* 换算后的温度 */
} PressureTest_DataTypeDef;

/* =========================
 * 基础读写接口
 * ========================= */
HAL_StatusTypeDef PressureTest_WriteReg(PressureTest_HandleTypeDef *dev, uint8_t reg, uint8_t data);
HAL_StatusTypeDef PressureTest_ReadReg(PressureTest_HandleTypeDef *dev, uint8_t reg, uint8_t *data);
HAL_StatusTypeDef PressureTest_ReadRegs(PressureTest_HandleTypeDef *dev, uint8_t reg, uint8_t *buf, uint16_t len);

/* =========================
 * 初始化与配置
 * ========================= */
HAL_StatusTypeDef PressureTest_Init(PressureTest_HandleTypeDef *dev,
                                    I2C_HandleTypeDef *hi2c,
                                    float span_kpa,
                                    uint16_t k_value);

HAL_StatusTypeDef PressureTest_SetMode(PressureTest_HandleTypeDef *dev,
                                       XGZP6859_Mode_t mode,
                                       XGZP6859_SleepTime_t sleep_time);

HAL_StatusTypeDef PressureTest_SetSysConfig(PressureTest_HandleTypeDef *dev, uint8_t sys_cfg);
HAL_StatusTypeDef PressureTest_SetPConfig(PressureTest_HandleTypeDef *dev, uint8_t p_cfg);

/* =========================
 * 常用功能接口
 * ========================= */
HAL_StatusTypeDef PressureTest_StartCombinedOnce(PressureTest_HandleTypeDef *dev);
HAL_StatusTypeDef PressureTest_StartSleepPeriodic(PressureTest_HandleTypeDef *dev, XGZP6859_SleepTime_t sleep_time);
HAL_StatusTypeDef PressureTest_IsBusy(PressureTest_HandleTypeDef *dev, uint8_t *busy);

HAL_StatusTypeDef PressureTest_ReadRawPressure(PressureTest_HandleTypeDef *dev, uint32_t *pressure_adc);
HAL_StatusTypeDef PressureTest_ReadRawTemperature(PressureTest_HandleTypeDef *dev, uint16_t *temp_adc);
HAL_StatusTypeDef PressureTest_ReadRawData(PressureTest_HandleTypeDef *dev, uint32_t *pressure_adc, uint16_t *temp_adc);

HAL_StatusTypeDef PressureTest_ReadData(PressureTest_HandleTypeDef *dev, PressureTest_DataTypeDef *data);

/* =========================
 * 数据换算接口
 * 手册公式：
 * 正压/正温：Pressure = Pressure_ADC / k
 *            Temperature = Temperature_ADC / 256
 *
 * 负压/负温：Pressure = (Pressure_ADC - 16777216) / k
 *            Temperature = (Temperature_ADC - 65536) / 256
 * ========================= */
float PressureTest_ConvertPressure(uint32_t pressure_adc, uint16_t k_value);
float PressureTest_ConvertTemperature(uint16_t temp_adc);
HAL_StatusTypeDef PressureTest_ReadOnce(PressureTest_HandleTypeDef *dev, PressureTest_DataTypeDef *data);
void PressureTest_PrintData(PressureTest_HandleTypeDef *dev);
/* 按量程跨度选择k值
 * 例如：
 * 0~20kPa      -> span = 20
 * -20~0kPa     -> span = 20
 * -40~40kPa    -> span = 80
 */
uint16_t PressureTest_GetKValue(float span_kpa);

/* =========================
 * 一些常用宏
 * ========================= */
#define PRESSURETEST_BUILD_CMD(sleep, sco, mode)   (uint8_t)((((sleep) & 0x0F) << 4) | (((sco) & 0x01) << 3) | ((mode) & 0x07))

/* 组合模式单次采集命令：sco=1, mode=010b */
#define PRESSURETEST_CMD_COMBINED_ONCE             PRESSURETEST_BUILD_CMD(0x00, 1, XGZP6859_MODE_COMBINED_ONCE)

/* 单次压力采集命令：sco=1, mode=001b */
#define PRESSURETEST_CMD_SINGLE_PRESSURE           PRESSURETEST_BUILD_CMD(0x00, 1, XGZP6859_MODE_SINGLE_PRESSURE)

/* 单次温度采集命令：sco=1, mode=000b */
#define PRESSURETEST_CMD_SINGLE_TEMP               PRESSURETEST_BUILD_CMD(0x00, 1, XGZP6859_MODE_SINGLE_TEMP)

#ifdef __cplusplus
}
#endif


#endif 