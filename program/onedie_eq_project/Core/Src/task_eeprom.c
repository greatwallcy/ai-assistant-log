/**
 * ============================================================================
 * 文件名称 : task_eeprom.c
 * 功能描述 : EEPROM 管理任务 - AT24C16M/TR 参数存储
 * ============================================================================
 */

#include "app_freertos.h"
#include "bsp_i2c.h"
#include "main.h"

/* EEPROM 内存布局 */
#define EEPROM_ADDR_BASE        0x0000
#define EEPROM_ADDR_PARAM       0x0000   /* 系统参数区 (256B) */
#define EEPROM_ADDR_HISTORY     0x0100   /* 历史记录区 */
#define EEPROM_SIZE             2048     /* AT24C16 = 2KB */

/**
 * @brief  EEPROM管理任务
 *         周期性保存关键参数, 支持掉电恢复
 */
void Task_EepromManager(void *argument)
{
    for (;;)
    {
        /* TODO: 检查参数变更标志 */
        /* TODO: 写入均衡参数、校准值等到EEPROM */

        /* 解除写保护 -> 写入 -> 写保护 */
        HAL_GPIO_WritePin(EEPROM_WP_PORT, EEPROM_WP_PIN, GPIO_PIN_RESET);
        /* BSP_EEPROM_Write(...); */
        HAL_GPIO_WritePin(EEPROM_WP_PORT, EEPROM_WP_PIN, GPIO_PIN_SET);

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
