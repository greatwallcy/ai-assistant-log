/**
 * ============================================================================
 * 文件名称 : task_idle.c
 * 功能描述 : 空闲/看门狗任务
 * ============================================================================
 */

#include "app_freertos.h"
#include "main.h"

/**
 * @brief  空闲任务 - 喂狗 + 系统监控
 */
void Task_IdleWatchdog(void *argument)
{
    for (;;)
    {
        /* TODO: 喂独立看门狗 IWDG */
        /* HAL_IWDG_Refresh(&hiwdg); */

        /* TODO: 监控各任务堆栈使用情况 */
        /* TODO: 检测硬件故障 */

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
