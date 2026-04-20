/**
 * ============================================================================
 * 文件名称 : FreeRTOSConfig.h
 * 功能描述 : FreeRTOS 配置 - STM32F103VET6 @ 72MHz
 * ============================================================================
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* STM32F1xx specific */
#include "stm32f1xx.h"

/* ======================== 调度器配置 ======================== */
#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configUSE_TICKLESS_IDLE                 0
#define configCPU_CLOCK_HZ                     (72000000UL)
#define configTICK_RATE_HZ                     ((TickType_t)1000)  /* 1ms tick */
#define configMAX_PRIORITIES                   (7)
#define configMINIMAL_STACK_SIZE               ((uint16_t)128)
#define configMAX_TASK_NAME_LEN                (16)
#define configUSE_16_BIT_TICKS                 0
#define configIDLE_SHOULD_YIELD                1

/* ======================== 内存配置 ======================== */
/* STM32F103VET6: 48KB RAM */
#define configTOTAL_HEAP_SIZE                  ((size_t)(32 * 1024))  /* 32KB */
#define configSUPPORT_STATIC_ALLOCATION        0
#define configSUPPORT_DYNAMIC_ALLOCATION       1

/* ======================== 任务功能 ======================== */
#define configUSE_MUTEXES                      1
#define configUSE_RECURSIVE_MUTEXES            0
#define configUSE_COUNTING_SEMAPHORES          1
#define configUSE_QUEUE_SETS                   0
#define configUSE_TASK_NOTIFICATIONS           1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES   1

/* ======================== 内存分配 ======================== */
#define configCHECK_FOR_STACK_OVERFLOW         2  /* 堆栈溢出检测 */
#define configUSE_MALLOC_FAILED_HOOK           1
#define configUSE_TRACE_FACILITY               0
#define configUSE_STATS_FORMATTING_FUNCTIONS   0
#define configUSE_IDLE_HOOK                    0
#define configUSE_TICK_HOOK                    0

/* ======================== 软件定时器 ======================== */
#define configUSE_TIMERS                       1
#define configTIMER_TASK_PRIORITY              (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH               10
#define configTIMER_TASK_STACK_DEPTH           (configMINIMAL_STACK_SIZE * 2)

/* ======================== 协程 (不使用) ======================== */
#define configUSE_CO_ROUTINES                  0
#define configMAX_CO_ROUTINE_PRIORITIES        (2)

/* ======================== API 包含 ======================== */
#define INCLUDE_vTaskPrioritySet               1
#define INCLUDE_uxTaskPriorityGet              1
#define INCLUDE_vTaskDelete                    1
#define INCLUDE_vTaskSuspend                   1
#define INCLUDE_xResumeFromISR                 1
#define INCLUDE_vTaskDelayUntil                1
#define INCLUDE_vTaskDelay                     1
#define INCLUDE_xTaskGetSchedulerState         1
#define INCLUDE_xTaskGetCurrentTaskHandle      1
#define INCLUDE_uxTaskGetStackHighWaterMark    1
#define INCLUDE_xTaskGetIdleTaskHandle         0
#define INCLUDE_eTaskGetState                  0

/* ======================== Cortex-M3 中断优先级 ======================== */
/* STM32F1xx 使用4位优先级 */
#ifdef __NVIC_PRIO_BITS
  #define configPRIO_BITS __NVIC_PRIO_BITS
#else
  #define configPRIO_BITS 4
#endif

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5

#define configKERNEL_INTERRUPT_PRIORITY \
    (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/* ======================== 中断处理函数映射 ======================== */
#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
