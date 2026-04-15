/**
 * ============================================================================
 * 文件名称 : bsp_balance.h
 * 功能描述 : 均衡模块BSP驱动 - VPS2606 + 74HC154 + INA226 + TMP112
 * ============================================================================
 */

#ifndef __BSP_BALANCE_H
#define __BSP_BALANCE_H

#include "stm32f1xx_hal.h"

/* 均衡方向 */
typedef enum {
    BAL_DIR_DISCHARGE = 0,  /* 放电均衡 */
    BAL_DIR_CHARGE = 1      /* 充电均衡 */
} BalanceDirection_t;

/* 均衡模块索引 */
typedef enum {
    BAL_MODULE_1 = 0,
    BAL_MODULE_2,
    BAL_MODULE_3,
    BAL_MODULE_4
} BalanceModule_t;

/* ======================== 函数声明 ======================== */
void BSP_Balance_Init(void);

/* 通道选择 (74HC154译码器) */
void BSP_Balance_SelectChannel(BalanceModule_t module, uint8_t channel);

/* 均衡启停控制 (VPS2606) */
void BSP_Balance_Start(BalanceModule_t module, uint8_t channel);
void BSP_Balance_Stop(BalanceModule_t module, uint8_t channel);
void BSP_Balance_AllStop(void);

/* 均衡方向控制 */
void BSP_Balance_SetDirection(BalanceModule_t module, BalanceDirection_t dir);

/* 传感器读取 */
float BSP_Balance_ReadVoltage(BalanceModule_t module, uint8_t channel);
float BSP_Balance_ReadTemperature(BalanceModule_t module, uint8_t channel);
float BSP_Balance_ReadCurrent(BalanceModule_t module, uint8_t channel);

#endif /* __BSP_BALANCE_H */
