/**
 * ============================================================================
 * 文件名称 : main.h
 * 产品名称 : 一芯一管+主动均衡 (Onedie_EQ_V1.0)
 * MCU型号  : STM32F103VET6 (LQFP100)
 * 开发环境 : STM32CubeIDE + FreeRTOS
 * 编制日期 : 2026-04-14
 * ============================================================================
 */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* ======================== 系统时钟配置 ======================== */
#define HSE_VALUE          8000000U    /* 外部晶振 8MHz */
#define HSE_RTC_VALUE      32768U      /* RTC晶振 32.768KHz */

/* ======================== EEPROM (AT24C16M/TR) ======================== */
#define EEPROM_I2C_ADDR    0xA0        /* 7-bit 地址 */
#define EEPROM_I2C         hi2c1       /* 使用 I2C1 (PB8/PB9) */
#define EEPROM_WP_PORT     GPIOE
#define EEPROM_WP_PIN      GPIO_PIN_0  /* PE0 - 写保护 */

/* ======================== 均衡模块数量 ======================== */
#define BALANCE_MODULE_NUM 4

/* ======================== 均衡模块1 ======================== */
/* 模拟I2C */
#define BAL1_SDA_PORT      GPIOA
#define BAL1_SDA_PIN       GPIO_PIN_2  /* PA2 */
#define BAL1_SCL_PORT      GPIOA
#define BAL1_SCL_PIN       GPIO_PIN_3  /* PA3 */
/* VPS2606 控制 */
#define BAL1_EN_PORT       GPIOA
#define BAL1_EN_PIN        GPIO_PIN_0  /* PA0 - 均衡启停 */
#define BAL1_CD_PORT       GPIOA
#define BAL1_CD_PIN        GPIO_PIN_1  /* PA1 - 均衡方向 */
/* 74HC154 译码器 (4线控制) */
#define BAL1_CTR1_PORT     GPIOC
#define BAL1_CTR1_PIN      GPIO_PIN_1  /* PC1 */
#define BAL1_CTR2_PORT     GPIOC
#define BAL1_CTR2_PIN      GPIO_PIN_0  /* PC0 */
#define BAL1_CTR3_PORT     GPIOC
#define BAL1_CTR3_PIN      GPIO_PIN_2  /* PC2 */
#define BAL1_CTR4_PORT     GPIOC
#define BAL1_CTR4_PIN      GPIO_PIN_3  /* PC3 */

/* ======================== 均衡模块2 ======================== */
#define BAL2_SDA_PORT      GPIOE
#define BAL2_SDA_PIN       GPIO_PIN_2  /* PE2 */
#define BAL2_SCL_PORT      GPIOE
#define BAL2_SCL_PIN       GPIO_PIN_3  /* PE3 */
#define BAL2_EN_PORT       GPIOE
#define BAL2_EN_PIN        GPIO_PIN_6  /* PE6 */
#define BAL2_CD_PORT       GPIOE
#define BAL2_CD_PIN        GPIO_PIN_4  /* PE4 */
#define BAL2_CTR1_PORT     GPIOA
#define BAL2_CTR1_PIN      GPIO_PIN_5  /* PA5 */
#define BAL2_CTR2_PORT     GPIOA
#define BAL2_CTR2_PIN      GPIO_PIN_4  /* PA4 */
#define BAL2_CTR3_PORT     GPIOA
#define BAL2_CTR3_PIN      GPIO_PIN_6  /* PA6 */
#define BAL2_CTR4_PORT     GPIOE
#define BAL2_CTR4_PIN      GPIO_PIN_5  /* PE5 */

/* ======================== 均衡模块3 ======================== */
#define BAL3_SDA_PORT      GPIOE
#define BAL3_SDA_PIN       GPIO_PIN_10 /* PE10 */
#define BAL3_SCL_PORT      GPIOE
#define BAL3_SCL_PIN       GPIO_PIN_11 /* PE11 */
#define BAL3_EN_PORT       GPIOE
#define BAL3_EN_PIN        GPIO_PIN_13 /* PE13 */
#define BAL3_CD_PORT       GPIOE
#define BAL3_CD_PIN        GPIO_PIN_12 /* PE12 */
#define BAL3_CTR1_PORT     GPIOB
#define BAL3_CTR1_PIN      GPIO_PIN_10 /* PB10 */
#define BAL3_CTR2_PORT     GPIOB
#define BAL3_CTR2_PIN      GPIO_PIN_11 /* PB11 */
#define BAL3_CTR3_PORT     GPIOE
#define BAL3_CTR3_PIN      GPIO_PIN_15 /* PE15 */
#define BAL3_CTR4_PORT     GPIOE
#define BAL3_CTR4_PIN      GPIO_PIN_14 /* PE14 */

/* ======================== 均衡模块4 ======================== */
#define BAL4_SDA_PORT      GPIOA
#define BAL4_SDA_PIN       GPIO_PIN_7  /* PA7 */
#define BAL4_SCL_PORT      GPIOC
#define BAL4_SCL_PIN       GPIO_PIN_4  /* PC4 */
#define BAL4_EN_PORT       GPIOB
#define BAL4_EN_PIN        GPIO_PIN_0  /* PB0 */
#define BAL4_CD_PORT       GPIOC
#define BAL4_CD_PIN        GPIO_PIN_5  /* PC5 */
#define BAL4_CTR1_PORT     GPIOE
#define BAL4_CTR1_PIN      GPIO_PIN_8  /* PE8 */
#define BAL4_CTR2_PORT     GPIOE
#define BAL4_CTR2_PIN      GPIO_PIN_9  /* PE9 */
#define BAL4_CTR3_PORT     GPIOE
#define BAL4_CTR3_PIN      GPIO_PIN_7  /* PE7 */
#define BAL4_CTR4_PORT     GPIOB
#define BAL4_CTR4_PIN      GPIO_PIN_1  /* PB1 */

/* ======================== 菊花链通讯1 (SPI1) ======================== */
#define DC1_SPI            hspi1
#define DC1_MOSI_PORT      GPIOB
#define DC1_MOSI_PIN       GPIO_PIN_5  /* PB5 */
#define DC1_MISO_PORT      GPIOB
#define DC1_MISO_PIN       GPIO_PIN_4  /* PB4 */
#define DC1_SCK_PORT       GPIOB
#define DC1_SCK_PIN        GPIO_PIN_3  /* PB3 */
#define DC1_EN_PORT        GPIOB
#define DC1_EN_PIN         GPIO_PIN_12 /* PB12 */

/* ======================== 菊花链通讯2 (SPI2) ======================== */
#define DC2_SPI            hspi2
#define DC2_MOSI_PORT      GPIOB
#define DC2_MOSI_PIN       GPIO_PIN_15 /* PB15 */
#define DC2_MISO_PORT      GPIOB
#define DC2_MISO_PIN       GPIO_PIN_14 /* PB14 */
#define DC2_SCK_PORT       GPIOB
#define DC2_SCK_PIN        GPIO_PIN_13 /* PB13 */
#define DC2_EN_PORT        GPIOD
#define DC2_EN_PIN         GPIO_PIN_8  /* PD8 */

/* ======================== CAN 通讯 (CA-IS3062VW) ======================== */
#define CAN_TX_PORT        GPIOD
#define CAN_TX_PIN         GPIO_PIN_1  /* PD1 */
#define CAN_RX_PORT        GPIOD
#define CAN_RX_PIN         GPIO_PIN_0  /* PD0 */

/* ======================== 地址分配 ======================== */
#define ADDR_UPPER_PORT    GPIOD
#define ADDR_UPPER_PIN     GPIO_PIN_5  /* PD5 - 上级BMU */
#define ADDR_LOWER_PORT    GPIOD
#define ADDR_LOWER_PIN     GPIO_PIN_6  /* PD6 - 下级BMU */

/* ======================== 风冷控制 ======================== */
#define FAN_PWR_PORT       GPIOD
#define FAN_PWR_PIN        GPIO_PIN_7  /* PD7 - 风扇供电 */
#define FAN_FEEDBACK_PORT  GPIOD
#define FAN_FEEDBACK_PIN   GPIO_PIN_4  /* PD4 - 风扇反馈 */

/* ======================== 预留输入 ======================== */
#define MSD_FEEDBACK_PORT  GPIOD
#define MSD_FEEDBACK_PIN   GPIO_PIN_3  /* PD3 - MSD反馈 */
#define FIRE_DETECT_PORT   GPIOD
#define FIRE_DETECT_PIN    GPIO_PIN_2  /* PD2 - 消防检测 */

/* ======================== 均衡模块5 (新增模拟I2C) ======================== */
#define BAL5_SDA_PORT      GPIOB
#define BAL5_SDA_PIN       GPIO_PIN_7  /* PB7 */
#define BAL5_SCL_PORT      GPIOB
#define BAL5_SCL_PIN       GPIO_PIN_6  /* PB6 */

/* ======================== SWD 烧录口 ======================== */
/* PA13 = SWDIO, PA14 = SWCLK, Pin14 = NRST */

/* ======================== 函数声明 ======================== */
void SystemClock_Config(void);
void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
