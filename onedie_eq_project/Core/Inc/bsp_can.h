/**
 * ============================================================================
 * 文件名称 : bsp_can.h
 * 功能描述 : CAN BSP驱动
 * ============================================================================
 */

#ifndef __BSP_CAN_H
#define __BSP_CAN_H

#include "stm32f1xx_hal.h"

void BSP_CAN_Init(void);
void BSP_CAN_ConfigFilter(void);
HAL_StatusTypeDef BSP_CAN_Transmit(uint8_t *pData, uint8_t len);
HAL_StatusTypeDef BSP_CAN_Receive(uint8_t *pData, uint8_t *pLen);

#endif /* __BSP_CAN_H */
