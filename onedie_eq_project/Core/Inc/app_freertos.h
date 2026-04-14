/**
 * ============================================================================
 * 文件名称 : app_freertos.h
 * 产品名称 : 一芯一管+主动均衡 (Onedie_EQ_V1.0)
 * 功能描述 : FreeRTOS 应用层任务声明
 * ============================================================================
 */

#ifndef __APP_FREERTOS_H
#define __APP_FREERTOS_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* ======================== 任务优先级 ======================== */
/* 数值越大优先级越高 */
#define TASK_PRIO_CAN_COMM      osPriorityAboveNormal   /* CAN通讯 */
#define TASK_PRIO_SPI_DAISY     osPriorityAboveNormal   /* 菊花链SPI通讯 */
#define TASK_PRIO_BALANCE       osPriorityNormal        /* 均衡控制 */
#define TASK_PRIO_MONITOR       osPriorityNormal        /* 电压温度采集监控 */
#define TASK_PRIO_EEPROM        osPriorityBelowNormal   /* EEPROM读写 */
#define TASK_PRIO_FAN           osPriorityLow           /* 风扇控制 */
#define TASK_PRIO_IDLE          osPriorityIdle          /* 空闲/看门狗 */

/* ======================== 任务栈大小 ======================== */
#define STACK_CAN_COMM          256
#define STACK_SPI_DAISY         256
#define STACK_BALANCE           512
#define STACK_MONITOR           512
#define STACK_EEPROM            128
#define STACK_FAN               128
#define STACK_IDLE              64

/* ======================== 队列/信号量声明 ======================== */
extern QueueHandle_t xCanTxQueue;
extern QueueHandle_t xCanRxQueue;
extern QueueHandle_t xSpiMsgQueue;
extern SemaphoreHandle_t xI2cMutex;
extern SemaphoreHandle_t xUartMutex;

/* ======================== 函数声明 ======================== */
void app_freertos_start(void);

/* 任务函数 */
void Task_CanComm(void *argument);
void Task_SpiDaisyChain(void *argument);
void Task_BalanceControl(void *argument);
void Task_VoltTempMonitor(void *argument);
void Task_EepromManager(void *argument);
void Task_FanControl(void *argument);
void Task_IdleWatchdog(void *argument);

#endif /* __APP_FREERTOS_H */
