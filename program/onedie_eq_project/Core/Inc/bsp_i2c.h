/**
 * ============================================================================
 * 文件名称 : bsp_i2c.h
 * 功能描述 : I2C BSP驱动 - 模拟I2C + 硬件I2C
 * ============================================================================
 */

#ifndef __BSP_I2C_H
#define __BSP_I2C_H

#include "stm32f1xx_hal.h"
#include "main.h"

/* ======================== 硬件I2C (EEPROM) ======================== */
void BSP_I2C_Init(void);
HAL_StatusTypeDef BSP_EEPROM_Write(uint16_t memAddr, uint8_t *pData, uint16_t len);
HAL_StatusTypeDef BSP_EEPROM_Read(uint16_t memAddr, uint8_t *pData, uint16_t len);

/* ======================== 模拟I2C (均衡模块传感器) ======================== */
typedef struct {
    GPIO_TypeDef *sdaPort;
    uint16_t sdaPin;
    GPIO_TypeDef *sclPort;
    uint16_t sclPin;
} SoftI2C_t;

void BSP_SoftI2C_Init(SoftI2C_t *i2c);
HAL_StatusTypeDef BSP_SoftI2C_Write(SoftI2C_t *i2c, uint8_t addr, uint8_t *pData, uint16_t len);
HAL_StatusTypeDef BSP_SoftI2C_Read(SoftI2C_t *i2c, uint8_t addr, uint8_t *pData, uint16_t len);

/* 4个均衡模块 + 1路铜排温度检测的模拟I2C实例 */
extern SoftI2C_t softI2C_Module1;
extern SoftI2C_t softI2C_Module2;
extern SoftI2C_t softI2C_Module3;
extern SoftI2C_t softI2C_Module4;
extern SoftI2C_t softI2C_CbTemp;     /* 铜排温度检测 (ADS1115) */

#endif /* __BSP_I2C_H */
