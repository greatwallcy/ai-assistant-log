/**
 * ============================================================================
 * 文件名称 : main.c
 * 产品名称 : 一芯一管+主动均衡 (Onedie_EQ_V1.0)
 * MCU型号  : STM32F103VET6 (LQFP100)
 * 开发环境 : STM32CubeIDE + FreeRTOS
 * 编制日期 : 2026-04-14
 * ============================================================================
 */

#include "main.h"
#include "app_freertos.h"
#include "bsp_i2c.h"
#include "bsp_spi.h"
#include "bsp_can.h"
#include "bsp_balance.h"

/* ======================== 全局句柄 ======================== */
I2C_HandleTypeDef hi2c1;        /* EEPROM 硬件I2C */
SPI_HandleTypeDef hspi1;        /* 菊花链通讯1 */
SPI_HandleTypeDef hspi2;        /* 菊花链通讯2 */
CAN_HandleTypeDef hcan1;        /* CAN通讯 */

/* ======================== 私有函数声明 ======================== */
static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI2_Init(void);
static void MX_CAN1_Init(void);

/**
 * @brief  主函数入口
 */
int main(void)
{
    /* HAL 初始化 */
    HAL_Init();

    /* 系统时钟配置: 72MHz */
    SystemClock_Config();

    /* 外设初始化 */
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_SPI1_Init();
    MX_SPI2_Init();
    MX_CAN1_Init();

    /* BSP层初始化 */
    BSP_I2C_Init();
    BSP_SPI_Init();
    BSP_CAN_Init();
    BSP_Balance_Init();

    /* 启动 FreeRTOS 调度器 */
    app_freertos_start();

    /* 不应执行到这里 */
    while (1)
    {
    }
}

/**
 * @brief  系统时钟配置
 *         HSE 8MHz -> PLL -> SYSCLK 72MHz
 *         APB1: 36MHz, APB2: 72MHz
 */
static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* 使能PWR和BKP时钟 */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_RCC_BKP_CLK_ENABLE();

    /* 配置LSE (32.768KHz RTC晶振) */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;  /* 8MHz * 9 = 72MHz */
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /* 时钟树配置 */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;    /* HCLK = 72MHz */
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;     /* APB1 = 36MHz */
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;     /* APB2 = 72MHz */

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief  GPIO 初始化
 */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 使能所有GPIO时钟 */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();

    /* 禁用JTAG, 保留SWD (释放 PB3, PB4, PA15) */
    __HAL_AFIO_REMAP_SWJ_NOJTAG();

    /* ---- EEPROM 写保护 PE0 ---- */
    HAL_GPIO_WritePin(EEPROM_WP_PORT, EEPROM_WP_PIN, GPIO_PIN_SET);  /* 默认写保护 */
    GPIO_InitStruct.Pin = EEPROM_WP_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(EEPROM_WP_PORT, &GPIO_InitStruct);

    /* ---- 均衡模块1 GPIO ---- */
    /* ENa (PA0) - 输出, 默认关闭 */
    HAL_GPIO_WritePin(BAL1_EN_PORT, BAL1_EN_PIN, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = BAL1_EN_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(BAL1_EN_PORT, &GPIO_InitStruct);

    /* CDa (PA1) - 输出 */
    HAL_GPIO_WritePin(BAL1_CD_PORT, BAL1_CD_PIN, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = BAL1_CD_PIN;
    HAL_GPIO_Init(BAL1_CD_PORT, &GPIO_InitStruct);

    /* CTR_a (PC0-PC3) - 译码器控制输出 */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* ---- 均衡模块2 GPIO ---- */
    HAL_GPIO_WritePin(BAL2_EN_PORT, BAL2_EN_PIN, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = BAL2_EN_PIN | BAL2_CD_PIN | BAL2_CTR4_PIN;
    HAL_GPIO_Init(BAL2_EN_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = BAL2_CTR1_PIN | BAL2_CTR2_PIN | BAL2_CTR3_PIN;
    HAL_GPIO_Init(BAL2_CTR1_PORT, &GPIO_InitStruct);

    /* ---- 均衡模块3 GPIO ---- */
    HAL_GPIO_WritePin(BAL3_EN_PORT, BAL3_EN_PIN, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = BAL3_EN_PIN | BAL3_CD_PIN | BAL3_CTR3_PIN | BAL3_CTR4_PIN;
    HAL_GPIO_Init(BAL3_EN_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = BAL3_CTR1_PIN | BAL3_CTR2_PIN;
    HAL_GPIO_Init(BAL3_CTR1_PORT, &GPIO_InitStruct);

    /* ---- 均衡模块4 GPIO ---- */
    HAL_GPIO_WritePin(BAL4_EN_PORT, BAL4_EN_PIN, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = BAL4_EN_PIN;
    HAL_GPIO_Init(BAL4_EN_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = BAL4_CD_PIN | BAL4_SCL_PIN;
    HAL_GPIO_Init(BAL4_SCL_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = BAL4_CTR1_PIN | BAL4_CTR2_PIN | BAL4_CTR3_PIN;
    HAL_GPIO_Init(BAL4_CTR1_PORT, &GPIO_InitStruct);

    /* ---- 均衡模块4 CTR_d4 (PB1) ---- */
    GPIO_InitStruct.Pin = BAL4_CTR4_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(BAL4_CTR4_PORT, &GPIO_InitStruct);

    /* ---- 菊花链1 EN (PB12) ---- */
    HAL_GPIO_WritePin(DC1_EN_PORT, DC1_EN_PIN, GPIO_PIN_SET);  /* 默认不选中 */
    GPIO_InitStruct.Pin = DC1_EN_PIN;
    HAL_GPIO_Init(DC1_EN_PORT, &GPIO_InitStruct);

    /* ---- 菊花链2 EN (PD8) ---- */
    HAL_GPIO_WritePin(DC2_EN_PORT, DC2_EN_PIN, GPIO_PIN_SET);  /* 默认不选中 */
    GPIO_InitStruct.Pin = DC2_EN_PIN;
    HAL_GPIO_Init(DC2_EN_PORT, &GPIO_InitStruct);

    /* ---- 地址输入 (PD5, PD6) - 上拉输入 ---- */
    GPIO_InitStruct.Pin = ADDR_UPPER_PIN | ADDR_LOWER_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(ADDR_UPPER_PORT, &GPIO_InitStruct);

    /* ---- 风扇控制 (PD7) - 输出 ---- */
    HAL_GPIO_WritePin(FAN_PWR_PORT, FAN_PWR_PIN, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = FAN_PWR_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(FAN_PWR_PORT, &GPIO_InitStruct);

    /* ---- 预留输入 (PD2, PD3, PD4) - 上拉输入 ---- */
    GPIO_InitStruct.Pin = FIRE_DETECT_PIN | MSD_FEEDBACK_PIN | FAN_FEEDBACK_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(FIRE_DETECT_PORT, &GPIO_InitStruct);

    /* ---- CAN 收发器 Standby 控制（如有需要可在此添加）---- */
}

/**
 * @brief  I2C1 初始化 (EEPROM - AT24C16M/TR)
 *         PB8 = SCL, PB9 = SDA
 *         标准模式 100KHz
 */
static void MX_I2C1_Init(void)
{
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief  SPI1 初始化 (菊花链通讯1 - DNB1101B)
 *         PB3 = SCK, PB4 = MISO, PB5 = MOSI
 *         模式: 主机, CPOL=LOW, CPHA=1Edge
 */
static void MX_SPI1_Init(void)
{
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;  /* 72/8 = 9MHz */
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;

    if (HAL_SPI_Init(&hspi1) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief  SPI2 初始化 (菊花链通讯2 - DNB1101B)
 *         PB13 = SCK, PB14 = MISO, PB15 = MOSI
 */
static void MX_SPI2_Init(void)
{
    hspi2.Instance = SPI2;
    hspi2.Init.Mode = SPI_MODE_MASTER;
    hspi2.Init.Direction = SPI_DIRECTION_2LINES;
    hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi2.Init.NSS = SPI_NSS_SOFT;
    hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;  /* 36/8 = 4.5MHz */
    hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;

    if (HAL_SPI_Init(&hspi2) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief  CAN1 初始化
 *         PD0 = RX, PD1 = TX
 *         波特率: 500Kbps
 */
static void MX_CAN1_Init(void)
{
    hcan1.Instance = CAN1;
    hcan1.Init.Prescaler = 9;           /* APB1=36MHz / 9 / (1+6+1) = 500K */
    hcan1.Init.Mode = CAN_MODE_NORMAL;
    hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
    hcan1.Init.TimeSeg1 = CAN_BS1_6TQ;
    hcan1.Init.TimeSeg2 = CAN_BS2_1TQ;
    hcan1.Init.TimeTriggeredMode = DISABLE;
    hcan1.Init.AutoBusOff = ENABLE;
    hcan1.Init.AutoWakeUp = DISABLE;
    hcan1.Init.AutoRetransmission = ENABLE;
    hcan1.Init.ReceiveFifoLocked = DISABLE;
    hcan1.Init.TransmitFifoPriority = DISABLE;

    if (HAL_CAN_Init(&hcan1) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief  错误处理
 */
void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}
