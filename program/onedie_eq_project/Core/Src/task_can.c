/**
 * ============================================================================
 * 文件名称 : task_can.c
 * 功能描述 : CAN 通讯任务 - 收发管理
 * ============================================================================
 */

#include "app_freertos.h"
#include "bsp_can.h"
#include "main.h"

/**
 * @brief  CAN通讯任务
 *         处理与上级/下级BMU的CAN数据收发
 */
void Task_CanComm(void *argument)
{
    uint8_t txData[8];
    uint8_t rxData[8];
    CAN_RxHeaderTypeDef rxHeader;

    /* 配置CAN滤波器 (接收所有报文) */
    BSP_CAN_ConfigFilter();

    /* 启动CAN */
    HAL_CAN_Start(&hcan1);
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);

    for (;;)
    {
        /* 发送队列处理 */
        if (xQueueReceive(xCanTxQueue, txData, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            BSP_CAN_Transmit(txData, 8);
        }

        /* 接收处理 (由中断回调填充 xCanRxQueue) */
        if (xQueueReceive(xCanRxQueue, rxData, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            /* TODO: 解析CAN报文, 根据协议处理 */
            /* 例: 上级下发均衡指令, 上报电压温度数据等 */
        }

        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
