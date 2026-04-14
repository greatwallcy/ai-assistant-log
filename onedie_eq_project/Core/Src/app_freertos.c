/**
 * ============================================================================
 * 文件名称 : app_freertos.c
 * 产品名称 : 一芯一管+主动均衡 (Onedie_EQ_V1.0)
 * 功能描述 : FreeRTOS 任务创建与调度入口
 * ============================================================================
 */

#include "app_freertos.h"

/* ======================== 全局句柄 ======================== */
TaskHandle_t hTaskCanComm = NULL;
TaskHandle_t hTaskSpiDaisy = NULL;
TaskHandle_t hTaskBalance = NULL;
TaskHandle_t hTaskMonitor = NULL;
TaskHandle_t hTaskEeprom = NULL;
TaskHandle_t hTaskFan = NULL;
TaskHandle_t hTaskIdle = NULL;

/* ======================== 队列/信号量 ======================== */
QueueHandle_t xCanTxQueue = NULL;
QueueHandle_t xCanRxQueue = NULL;
QueueHandle_t xSpiMsgQueue = NULL;
SemaphoreHandle_t xI2cMutex = NULL;

/**
 * @brief  创建所有任务和资源，启动调度器
 */
void app_freertos_start(void)
{
    /* 创建互斥量 */
    xI2cMutex = xSemaphoreCreateMutex();

    /* 创建消息队列 */
    xCanTxQueue = xQueueCreate(16, sizeof(uint8_t[8]));    /* CAN TX: 16条 × 8字节 */
    xCanRxQueue = xQueueCreate(16, sizeof(uint8_t[8]));    /* CAN RX: 16条 × 8字节 */
    xSpiMsgQueue = xQueueCreate(8, sizeof(uint8_t[16]));   /* SPI: 8条 × 16字节 */

    /* 创建任务 */
    xTaskCreate(Task_CanComm,       "CAN",       STACK_CAN_COMM,   NULL, TASK_PRIO_CAN_COMM,    &hTaskCanComm);
    xTaskCreate(Task_SpiDaisyChain, "SPI_DC",    STACK_SPI_DAISY,  NULL, TASK_PRIO_SPI_DAISY,   &hTaskSpiDaisy);
    xTaskCreate(Task_BalanceControl,"BALANCE",   STACK_BALANCE,    NULL, TASK_PRIO_BALANCE,     &hTaskBalance);
    xTaskCreate(Task_VoltTempMonitor,"MONITOR",  STACK_MONITOR,    NULL, TASK_PRIO_MONITOR,     &hTaskMonitor);
    xTaskCreate(Task_EepromManager, "EEPROM",    STACK_EEPROM,     NULL, TASK_PRIO_EEPROM,      &hTaskEeprom);
    xTaskCreate(Task_FanControl,    "FAN",       STACK_FAN,        NULL, TASK_PRIO_FAN,         &hTaskFan);
    xTaskCreate(Task_IdleWatchdog,  "IDLE",      STACK_IDLE,       NULL, TASK_PRIO_IDLE,        &hTaskIdle);

    /* 启动调度器 (不应返回) */
    vTaskStartScheduler();
}
