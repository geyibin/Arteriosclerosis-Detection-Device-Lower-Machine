#include "pressureTest.h"
#include <string.h>

/* =========================
 * 底层读写：严格按官方 demo 风格
 * 写：0xDA + [reg, data]
 * 读：先 0xDA 发 reg，再 0xDB 读数据
 * ========================= */

static HAL_StatusTypeDef PT_WriteBytes(PressureTest_HandleTypeDef *dev, uint8_t *buf, uint16_t len)
{
    if ((dev == NULL) || (dev->hi2c == NULL) || (buf == NULL) || (len == 0))
    {
        return HAL_ERROR;
    }

    return HAL_I2C_Master_Transmit(dev->hi2c,
                                   XGZP6859_I2C_ADDR,          // 0xDA
                                   buf,
                                   len,
                                   1000);
}

static HAL_StatusTypeDef PT_ReadBytes(PressureTest_HandleTypeDef *dev, uint8_t reg, uint8_t *buf, uint16_t len)
{
    HAL_StatusTypeDef ret;

    if ((dev == NULL) || (dev->hi2c == NULL) || (buf == NULL) || (len == 0))
    {
        return HAL_ERROR;
    }

    ret = HAL_I2C_Master_Transmit(dev->hi2c,
                                  XGZP6859_I2C_ADDR,          // 0xDA
                                  &reg,
                                  1,
                                  1000);
    if (ret != HAL_OK)
    {
        return ret;
    }

    return HAL_I2C_Master_Receive(dev->hi2c,
                                  (XGZP6859_I2C_ADDR | 0x01), // 0xDB
                                  buf,
                                  len,
                                  1000);
}

/* =========================
 * 基础接口
 * ========================= */

HAL_StatusTypeDef PressureTest_WriteReg(PressureTest_HandleTypeDef *dev, uint8_t reg, uint8_t data)
{
    uint8_t send[2];

    send[0] = reg;
    send[1] = data;

    return PT_WriteBytes(dev, send, 2);
}

HAL_StatusTypeDef PressureTest_ReadReg(PressureTest_HandleTypeDef *dev, uint8_t reg, uint8_t *data)
{
    return PT_ReadBytes(dev, reg, data, 1);
}

HAL_StatusTypeDef PressureTest_ReadRegs(PressureTest_HandleTypeDef *dev, uint8_t reg, uint8_t *buf, uint16_t len)
{
    return PT_ReadBytes(dev, reg, buf, len);
}

/* =========================
 * 初始化
 * 注意：这里不再用 HAL_I2C_IsDeviceReady
 * 官方 demo 也没有这么干
 * ========================= */
HAL_StatusTypeDef PressureTest_Init(PressureTest_HandleTypeDef *dev,
                                    I2C_HandleTypeDef *hi2c,
                                    float span_kpa,
                                    uint16_t k_value)
{
    if ((dev == NULL) || (hi2c == NULL))
    {
        return HAL_ERROR;
    }

    memset(dev, 0, sizeof(PressureTest_HandleTypeDef));
    dev->hi2c = hi2c;
    dev->span_kpa = span_kpa;
    dev->k_value = (k_value == 0) ? PressureTest_GetKValue(span_kpa) : k_value;
    dev->unipolar = 0;

    HAL_Delay(100);
    return HAL_OK;
}

/* 这个接口保留，但你现在主要用 StartCombinedOnce */
HAL_StatusTypeDef PressureTest_SetMode(PressureTest_HandleTypeDef *dev,
                                       XGZP6859_Mode_t mode,
                                       XGZP6859_SleepTime_t sleep_time)
{
    uint8_t cmd = PRESSURETEST_BUILD_CMD(sleep_time, 1, mode);
    return PressureTest_WriteReg(dev, XGZP6859_REG_CMD, cmd);
}

HAL_StatusTypeDef PressureTest_SetSysConfig(PressureTest_HandleTypeDef *dev, uint8_t sys_cfg)
{
    return PressureTest_WriteReg(dev, XGZP6859_REG_SYS_CONFIG, sys_cfg);
}

HAL_StatusTypeDef PressureTest_SetPConfig(PressureTest_HandleTypeDef *dev, uint8_t p_cfg)
{
    return PressureTest_WriteReg(dev, XGZP6859_REG_P_CONFIG, p_cfg);
}

/* =========================
 * 按官方 demo：
 * 1. 读 A5
 * 2. 清 bit1
 * 3. 写回 A5
 * 4. 写 30 = 0x0A
 * ========================= */
HAL_StatusTypeDef PressureTest_StartCombinedOnce(PressureTest_HandleTypeDef *dev)
{
    HAL_StatusTypeDef ret;
    uint8_t tmp = 0;

    ret = PressureTest_ReadReg(dev, 0xA5, &tmp);
    if (ret != HAL_OK)
    {
        return ret;
    }

    tmp &= 0xFD;   // 清 bit1，严格照官方 demo
    ret = PressureTest_WriteReg(dev, 0xA5, tmp);
    if (ret != HAL_OK)
    {
        return ret;
    }

    return PressureTest_WriteReg(dev, 0x30, 0x0A);
}

HAL_StatusTypeDef PressureTest_StartSleepPeriodic(PressureTest_HandleTypeDef *dev, XGZP6859_SleepTime_t sleep_time)
{
    return PressureTest_SetMode(dev, XGZP6859_MODE_SLEEP_PERIODIC, sleep_time);
}

/* 官方 demo 是：读 0x30，直到 bit3 = 1 才继续
 * 所以这里定义成：
 * bit3=1 -> busy=0（完成）
 * bit3=0 -> busy=1（还没完成）
 */
HAL_StatusTypeDef PressureTest_IsBusy(PressureTest_HandleTypeDef *dev, uint8_t *busy)
{
    HAL_StatusTypeDef ret;
    uint8_t val = 0;

    if ((dev == NULL) || (busy == NULL))
    {
        return HAL_ERROR;
    }

    ret = PressureTest_ReadReg(dev, 0x30, &val);
    if (ret != HAL_OK)
    {
        return ret;
    }

    *busy = ((val & 0x08) != 0) ? 0 : 1;
    return HAL_OK;
}

/* =========================
 * 原始数据读取
 * 为了稳，直接逐字节读，和官方 demo 一致
 * ========================= */
HAL_StatusTypeDef PressureTest_ReadRawPressure(PressureTest_HandleTypeDef *dev, uint32_t *pressure_adc)
{
    HAL_StatusTypeDef ret;
    uint8_t b0, b1, b2;

    if ((dev == NULL) || (pressure_adc == NULL))
    {
        return HAL_ERROR;
    }

    ret = PressureTest_ReadReg(dev, 0x06, &b0);
    if (ret != HAL_OK) return ret;
    ret = PressureTest_ReadReg(dev, 0x07, &b1);
    if (ret != HAL_OK) return ret;
    ret = PressureTest_ReadReg(dev, 0x08, &b2);
    if (ret != HAL_OK) return ret;

    *pressure_adc = ((uint32_t)b0 << 16) |
                    ((uint32_t)b1 << 8)  |
                    ((uint32_t)b2);

    return HAL_OK;
}

HAL_StatusTypeDef PressureTest_ReadRawTemperature(PressureTest_HandleTypeDef *dev, uint16_t *temp_adc)
{
    HAL_StatusTypeDef ret;
    uint8_t b0, b1;

    if ((dev == NULL) || (temp_adc == NULL))
    {
        return HAL_ERROR;
    }

    ret = PressureTest_ReadReg(dev, 0x09, &b0);
    if (ret != HAL_OK) return ret;
    ret = PressureTest_ReadReg(dev, 0x0A, &b1);
    if (ret != HAL_OK) return ret;

    *temp_adc = ((uint16_t)b0 << 8) | b1;
    return HAL_OK;
}

HAL_StatusTypeDef PressureTest_ReadRawData(PressureTest_HandleTypeDef *dev, uint32_t *pressure_adc, uint16_t *temp_adc)
{
    HAL_StatusTypeDef ret;

    ret = PressureTest_ReadRawPressure(dev, pressure_adc);
    if (ret != HAL_OK) return ret;

    ret = PressureTest_ReadRawTemperature(dev, temp_adc);
    if (ret != HAL_OK) return ret;

    return HAL_OK;
}

/* =========================
 * 一次完整读取：启动 -> 等完成 -> 读值 -> 换算
 * ========================= */
HAL_StatusTypeDef PressureTest_ReadData(PressureTest_HandleTypeDef *dev, PressureTest_DataTypeDef *data)
{
    HAL_StatusTypeDef ret;
    uint8_t busy = 1;
    uint32_t tickstart;

    if ((dev == NULL) || (data == NULL))
    {
        return HAL_ERROR;
    }

    ret = PressureTest_StartCombinedOnce(dev);
    if (ret != HAL_OK)
    {
        return ret;
    }

    tickstart = HAL_GetTick();
    while (busy)
    {
        ret = PressureTest_IsBusy(dev, &busy);
        if (ret != HAL_OK)
        {
            return ret;
        }

        if ((HAL_GetTick() - tickstart) > 100)
        {
            return HAL_TIMEOUT;
        }

        HAL_Delay(1);
    }

    ret = PressureTest_ReadRawData(dev, &data->pressure_adc, &data->temp_adc);
    if (ret != HAL_OK)
    {
        return ret;
    }

    data->pressure_kpa  = PressureTest_ConvertPressure(data->pressure_adc, dev->k_value);
    data->temperature_c = PressureTest_ConvertTemperature(data->temp_adc);

    return HAL_OK;
}

/* =========================
 * 数据换算
 * ========================= */
float PressureTest_ConvertPressure(uint32_t pressure_adc, uint16_t k_value)
{
    int32_t signed_adc;

    if (k_value == 0)
    {
        return 0.0f;
    }

    if (pressure_adc & 0x800000UL)
    {
        signed_adc = (int32_t)pressure_adc - 16777216;
        return ((float)signed_adc / (float)k_value);
    }
    else
    {
        return ((float)pressure_adc / (float)k_value);
    }
}

float PressureTest_ConvertTemperature(uint16_t temp_adc)
{
    int32_t signed_adc;

    if (temp_adc & 0x8000U)
    {
        signed_adc = (int32_t)temp_adc - 65536;
        return ((float)signed_adc / 256.0f);
    }
    else
    {
        return ((float)temp_adc / 256.0f);
    }
}

uint16_t PressureTest_GetKValue(float span_kpa)
{
    if (span_kpa < 1.0f)         return 8192;
    else if (span_kpa < 2.0f)    return 4096;
    else if (span_kpa < 4.0f)    return 2048;
    else if (span_kpa <= 8.0f)   return 1024;
    else if (span_kpa <= 16.0f)  return 512;
    else if (span_kpa <= 32.0f)  return 256;
    else if (span_kpa <= 65.0f)  return 128;
    else if (span_kpa <= 131.0f) return 64;
    else if (span_kpa <= 260.0f) return 32;
    else if (span_kpa <= 500.0f) return 16;
    else                         return 0;
}

HAL_StatusTypeDef PressureTest_ReadOnce(PressureTest_HandleTypeDef *dev, PressureTest_DataTypeDef *data)
{
    HAL_StatusTypeDef ret;
    uint8_t tmp = 0;
    uint8_t p_msb, p_csb, p_lsb;
    uint8_t t_msb, t_lsb;
    float pressure_pa;

    if ((dev == NULL) || (data == NULL))
    {
        return HAL_ERROR;
    }

    /* 1. 读 A5，清 bit1，按官方 demo 习惯走 */
    ret = PressureTest_ReadReg(dev, 0xA5, &tmp);
    if (ret != HAL_OK) return ret;

    tmp &= 0xFD;
    ret = PressureTest_WriteReg(dev, 0xA5, tmp);
    if (ret != HAL_OK) return ret;

    /* 2. 触发一次组合采样 */
    ret = PressureTest_WriteReg(dev, 0x30, 0x0A);
    if (ret != HAL_OK) return ret;

    /* 3. 给足一点时间，别再用太激进的短延时 */
    HAL_Delay(50);

    /* 4. 读压力原始值 */
    ret = PressureTest_ReadReg(dev, 0x06, &p_msb);
    if (ret != HAL_OK) return ret;

    ret = PressureTest_ReadReg(dev, 0x07, &p_csb);
    if (ret != HAL_OK) return ret;

    ret = PressureTest_ReadReg(dev, 0x08, &p_lsb);
    if (ret != HAL_OK) return ret;

    /* 5. 读温度原始值 */
    ret = PressureTest_ReadReg(dev, 0x09, &t_msb);
    if (ret != HAL_OK) return ret;

    ret = PressureTest_ReadReg(dev, 0x0A, &t_lsb);
    if (ret != HAL_OK) return ret;

    data->pressure_adc = ((uint32_t)p_msb << 16) |
                         ((uint32_t)p_csb << 8)  |
                         ((uint32_t)p_lsb);

    data->temp_adc = ((uint16_t)t_msb << 8) |
                     ((uint16_t)t_lsb);

    /* 手册默认这里算出来是 Pa，不是 kPa */
    pressure_pa = PressureTest_ConvertPressure(data->pressure_adc, dev->k_value);

    /* 统一转成 kPa 给外部用 */
    data->pressure_kpa  = pressure_pa / 1000.0f;
    data->temperature_c = PressureTest_ConvertTemperature(data->temp_adc);

    return HAL_OK;
}

void PressureTest_PrintData(PressureTest_HandleTypeDef *dev)
{
    PressureTest_DataTypeDef data;
    HAL_StatusTypeDef ret;

    ret = PressureTest_ReadOnce(dev, &data);
    if (ret == HAL_OK)
    {
        printf("P_RAW=%lu, T_RAW=%u, Pressure=%.3f kPa, Temp=%.3f C\r\n",
               data.pressure_adc,
               data.temp_adc,
               data.pressure_kpa,
               data.temperature_c);
    }
    else
    {
        printf("Pressure read fail, ret=%d, i2c_err=0x%08lX\r\n",
               ret,
               HAL_I2C_GetError(dev->hi2c));
    }
}