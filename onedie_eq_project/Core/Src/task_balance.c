/**
 * ============================================================================
 * 文件名称 : task_balance.c
 * 功能描述 : 均衡控制任务 - 一芯一管主动均衡核心逻辑
 *             每个均衡模块控制4个电芯，共4个模块 = 16串电芯
 * ============================================================================
 */

#include "app_freertos.h"
#include "bsp_balance.h"
#include "bsp_i2c.h"
#include "main.h"

/* ======================== 均衡参数 ======================== */
#define CELL_NUM_PER_MODULE     4
#define TOTAL_CELL_NUM          16      /* 4模块 × 4电芯 */
#define BALANCE_THRESHOLD_MV    10      /* 均衡触发阈值 (mV) */
#define BALANCE_CURRENT_MA      500     /* 均衡目标电流 (mA) */
#define MONITOR_PERIOD_MS       100     /* 采集周期 */

/* ======================== 电芯数据结构 ======================== */
typedef struct {
    float voltage_mv;           /* 电压 (mV) */
    float temperature_c;        /* 温度 (°C) */
    uint16_t current_ma;        /* 均衡电流 (mA) */
    uint8_t  is_balancing;      /* 是否正在均衡 */
} CellData_t;

/* 全局电芯数据 */
static CellData_t g_CellData[TOTAL_CELL_NUM];

/**
 * @brief  均衡控制任务
 *         周期性采集电压温度, 判断是否需要均衡, 执行均衡动作
 */
void Task_BalanceControl(void *argument)
{
    /* 初始化均衡硬件 (全部关闭) */
    BSP_Balance_AllStop();

    for (;;)
    {
        /* 第一步: 采集所有电芯电压和温度 (通过INA226 + TMP112) */
        for (uint8_t mod = 0; mod < BALANCE_MODULE_NUM; mod++)
        {
            for (uint8_t ch = 0; ch < CELL_NUM_PER_MODULE; ch++)
            {
                uint8_t idx = mod * CELL_NUM_PER_MODULE + ch;
                /* 选择通道 (74HC154译码器) */
                BSP_Balance_SelectChannel(mod, ch);

                /* 读取电压 (INA226) */
                g_CellData[idx].voltage_mv = BSP_Balance_ReadVoltage(mod, ch);

                /* 读取温度 (TMP112) */
                g_CellData[idx].temperature_c = BSP_Balance_ReadTemperature(mod, ch);
            }
        }

        /* 第二步: 计算平均电压, 找出需要均衡的电芯 */
        float sum = 0;
        for (uint8_t i = 0; i < TOTAL_CELL_NUM; i++)
        {
            sum += g_CellData[i].voltage_mv;
        }
        float avg_mv = sum / TOTAL_CELL_NUM;

        /* 第三步: 执行均衡 */
        for (uint8_t mod = 0; mod < BALANCE_MODULE_NUM; mod++)
        {
            for (uint8_t ch = 0; ch < CELL_NUM_PER_MODULE; ch++)
            {
                uint8_t idx = mod * CELL_NUM_PER_MODULE + ch;
                float diff = g_CellData[idx].voltage_mv - avg_mv;

                if (diff > BALANCE_THRESHOLD_MV)
                {
                    /* 电压偏高 -> 放电均衡 */
                    BSP_Balance_SetDirection(mod, BAL_DIR_DISCHARGE);
                    BSP_Balance_Start(mod, ch);
                    g_CellData[idx].is_balancing = 1;
                }
                else if (diff < -BALANCE_THRESHOLD_MV)
                {
                    /* 电压偏低 -> 充电均衡 */
                    BSP_Balance_SetDirection(mod, BAL_DIR_CHARGE);
                    BSP_Balance_Start(mod, ch);
                    g_CellData[idx].is_balancing = 1;
                }
                else
                {
                    /* 在阈值范围内 -> 停止均衡 */
                    BSP_Balance_Stop(mod, ch);
                    g_CellData[idx].is_balancing = 0;
                }
            }
        }

        /* 等待下一个采集周期 */
        vTaskDelay(pdMS_TO_TICKS(MONITOR_PERIOD_MS));
    }
}

/**
 * @brief  电压温度监控任务
 *         定时上报数据, 处理过压/过温保护
 */
void Task_VoltTempMonitor(void *argument)
{
    for (;;)
    {
        /* TODO: 检查过压保护 (>4.25V) */
        /* TODO: 检查过温保护 (>60°C) */
        /* TODO: 通过CAN上报数据给上级BMU */

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
