/**
 * ============================================================================
 * 文件名称 : task_fan.c
 * 功能描述 : 风扇控制任务
 * ============================================================================
 */

#include "app_freertos.h"
#include "main.h"

/* 风扇控制参数 */
#define FAN_TEMP_ON_C           40.0f   /* 启动温度 */
#define FAN_TEMP_OFF_C          35.0f   /* 关闭温度 */

/**
 * @brief  风扇控制任务
 *         根据温度自动控制风扇开关
 */
void Task_FanControl(void *argument)
{
    uint8_t fanState = 0;

    for (;;)
    {
        /* TODO: 读取最高温度 */
        float maxTemp = 0; /* 从g_CellData获取 */

        if (!fanState && maxTemp > FAN_TEMP_ON_C)
        {
            HAL_GPIO_WritePin(FAN_PWR_PORT, FAN_PWR_PIN, GPIO_PIN_SET);
            fanState = 1;
        }
        else if (fanState && maxTemp < FAN_TEMP_OFF_C)
        {
            HAL_GPIO_WritePin(FAN_PWR_PORT, FAN_PWR_PIN, GPIO_PIN_RESET);
            fanState = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
