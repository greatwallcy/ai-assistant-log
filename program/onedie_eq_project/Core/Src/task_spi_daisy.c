/**
 * ============================================================================
 * 文件名称 : task_spi_daisy.c
 * 功能描述 : 菊花链SPI通讯任务 - DNB1101B 电芯监控芯片通讯
 * ============================================================================
 */

#include "app_freertos.h"
#include "bsp_spi.h"
#include "main.h"

/**
 * @brief  菊花链通讯任务
 *         通过SPI1/SPI2与DNB1101B芯片通讯, 读取电芯数据
 */
void Task_SpiDaisyChain(void *argument)
{
    uint8_t txBuf[16];
    uint8_t rxBuf[16];

    for (;;)
    {
        /* 菊花链1 通讯 */
        HAL_GPIO_WritePin(DC1_EN_PORT, DC1_EN_PIN, GPIO_PIN_RESET);  /* 选中 */
        /* TODO: 发送读取命令, 解析响应 */
        /* BSP_SPI1_TransmitReceive(txBuf, rxBuf, len); */
        HAL_GPIO_WritePin(DC1_EN_PORT, DC1_EN_PIN, GPIO_PIN_SET);    /* 释放 */

        vTaskDelay(pdMS_TO_TICKS(10));

        /* 菊花链2 通讯 */
        HAL_GPIO_WritePin(DC2_EN_PORT, DC2_EN_PIN, GPIO_PIN_RESET);
        /* TODO: 发送读取命令, 解析响应 */
        HAL_GPIO_WritePin(DC2_EN_PORT, DC2_EN_PIN, GPIO_PIN_SET);

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
