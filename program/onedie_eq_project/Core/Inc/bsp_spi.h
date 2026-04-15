/**
 * ============================================================================
 * 文件名称 : bsp_spi.h
 * 功能描述 : SPI BSP驱动 - 菊花链通讯
 * ============================================================================
 */

#ifndef __BSP_SPI_H
#define __BSP_SPI_H

#include "stm32f1xx_hal.h"

void BSP_SPI_Init(void);
HAL_StatusTypeDef BSP_SPI1_TransmitReceive(uint8_t *pTx, uint8_t *pRx, uint16_t len);
HAL_StatusTypeDef BSP_SPI2_TransmitReceive(uint8_t *pTx, uint8_t *pRx, uint16_t len);

#endif /* __BSP_SPI_H */
