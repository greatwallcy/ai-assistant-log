/**
 * ============================================================================
 * 文件名称 : task_balance.c (修改后)
 * 功能描述 : 均衡控制任务 - 一芯一管主动均衡核心逻辑
 *             新增: 主动均衡启动自检任务 (Task_ActiveBalanceStartup)
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

/* ======================== 主动均衡启动参数 ======================== */
#define AB_INA226_ADDR          0x40    /* INA226AIDGSR 7-bit地址 (1000000) */
#define AB_TMP112_ADDR          0x48    /* TMP112AIDRLR 7-bit地址 (1001000) */
#define AB_DURATION_SEC         60      /* 均衡持续时间 (秒) */
#define AB_SAMPLE_INTERVAL_MS   1000    /* 采集间隔 (毫秒) */
#define AB_SAMPLE_COUNT         (AB_DURATION_SEC * 1000 / AB_SAMPLE_INTERVAL_MS)  /* 60次 */

/* INA226 寄存器 */
#define INA226_REG_BUS_VOLT     0x02
#define INA226_REG_CURRENT      0x04

/* TMP112 寄存器 */
#define TMP112_REG_TEMP         0x00

/* ======================== 电芯数据结构 ======================== */
typedef struct {
    float voltage_mv;           /* 电压 (mV) */
    float temperature_c;        /* 温度 (°C) */
    uint16_t current_ma;        /* 均衡电流 (mA) */
    uint8_t  is_balancing;      /* 是否正在均衡 */
} CellData_t;

/* ======================== 主动均衡采样记录 ======================== */
typedef struct {
    float voltage_mv;
    float current_ma;
    float temperature_c;
} AB_SampleRecord_t;

/* 全局电芯数据 */
static CellData_t g_CellData[TOTAL_CELL_NUM];

/* 主动均衡采样数据存储 */
static AB_SampleRecord_t g_AB_Samples[AB_SAMPLE_COUNT];
static uint8_t g_AB_SampleCount = 0;

/* ======================== 主动均衡传感器读取 (直接I2C) ======================== */

/**
 * @brief  通过INA226读取总线电压
 * @param  i2c: 模拟I2C实例
 * @param  addr: INA226 7-bit地址
 * @return 电压值 (mV)
 */
static float AB_ReadVoltage(SoftI2C_t *i2c, uint8_t addr)
{
    uint8_t buf[2];
    uint8_t reg = INA226_REG_BUS_VOLT;

    if (BSP_SoftI2C_Write(i2c, addr, &reg, 1) != HAL_OK) return 0.0f;
    if (BSP_SoftI2C_Read(i2c, addr, buf, 2) != HAL_OK) return 0.0f;

    uint16_t raw = ((uint16_t)buf[0] << 8) | buf[1];
    return raw * 1.25f;  /* INA226 总线电压 LSB = 1.25mV */
}

/**
 * @brief  通过INA226读取电流
 * @param  i2c: 模拟I2C实例
 * @param  addr: INA226 7-bit地址
 * @return 电流值 (mA)
 */
static float AB_ReadCurrent(SoftI2C_t *i2c, uint8_t addr)
{
    uint8_t buf[2];
    uint8_t reg = INA226_REG_CURRENT;

    if (BSP_SoftI2C_Write(i2c, addr, &reg, 1) != HAL_OK) return 0.0f;
    if (BSP_SoftI2C_Read(i2c, addr, buf, 2) != HAL_OK) return 0.0f;

    int16_t raw = ((int16_t)buf[0] << 8) | buf[1];
    return raw * 0.1f;  /* 需根据分流电阻值校准 */
}

/**
 * @brief  通过TMP112读取温度
 * @param  i2c: 模拟I2C实例
 * @param  addr: TMP112 7-bit地址
 * @return 温度值 (°C)
 */
static float AB_ReadTemperature(SoftI2C_t *i2c, uint8_t addr)
{
    uint8_t buf[2];
    uint8_t reg = TMP112_REG_TEMP;

    if (BSP_SoftI2C_Write(i2c, addr, &reg, 1) != HAL_OK) return 0.0f;
    if (BSP_SoftI2C_Read(i2c, addr, buf, 2) != HAL_OK) return 0.0f;

    int16_t raw = ((int16_t)buf[0] << 8) | buf[1];
    raw >>= 4;  /* TMP112: 12位精度, 高12位有效 */
    return raw * 0.0625f;  /* LSB = 0.0625°C */
}

/* ======================== 主动均衡启动自检任务 ======================== */

/**
 * @brief  主动均衡启动自检任务
 *         执行流程:
 *         1. 关闭所有模块均衡
 *         2. 延时1S
 *         3. 打开模块1均衡 (EN=1, CD=0 充电均衡)
 *         4. 均衡1分钟, 每1S采集电流/电压/温度
 *         5. 关闭模块1均衡
 */
void Task_ActiveBalanceStartup(void *argument)
{
    /* ========== 步骤1: 关闭所有模块的主动均衡 ========== */
    BSP_Balance_AllStop();

    /* ========== 步骤2: 延时1S ========== */
    vTaskDelay(pdMS_TO_TICKS(1000));

    /* ========== 步骤3: 打开均衡模块1, 充电均衡 (EN=1, CD=0) ========== */
    /* CD=0: 充电均衡方向 (低电平) */
    HAL_GPIO_WritePin(BAL1_CD_PORT, BAL1_CD_PIN, GPIO_PIN_RESET);
    /* EN=1: 使能均衡 (高电平) */
    HAL_GPIO_WritePin(BAL1_EN_PORT, BAL1_EN_PIN, GPIO_PIN_SET);

    /* ========== 步骤4: 均衡1分钟, 每1S采集数据 ========== */
    g_AB_SampleCount = 0;

    for (uint16_t i = 0; i < AB_SAMPLE_COUNT; i++)
    {
        /* 读取电压 (INA226, 地址0x40) */
        float voltage = AB_ReadVoltage(&softI2C_Module1, AB_INA226_ADDR);

        /* 读取电流 (INA226, 地址0x40) */
        float current = AB_ReadCurrent(&softI2C_Module1, AB_INA226_ADDR);

        /* 读取温度 (TMP112, 地址0x48) */
        float temperature = AB_ReadTemperature(&softI2C_Module1, AB_TMP112_ADDR);

        /* 存储采样数据 */
        if (g_AB_SampleCount < AB_SAMPLE_COUNT)
        {
            g_AB_Samples[g_AB_SampleCount].voltage_mv = voltage;
            g_AB_Samples[g_AB_SampleCount].current_ma = current;
            g_AB_Samples[g_AB_SampleCount].temperature_c = temperature;
            g_AB_SampleCount++;
        }

        /* 等待1S采集间隔 */
        vTaskDelay(pdMS_TO_TICKS(AB_SAMPLE_INTERVAL_MS));
    }

    /* ========== 步骤5: 关闭模块1的均衡 ========== */
    HAL_GPIO_WritePin(BAL1_EN_PORT, BAL1_EN_PIN, GPIO_PIN_RESET);

    /* 主动均衡自检完成, 任务挂起 */
    vTaskSuspend(NULL);
}

/* ======================== 原有均衡控制任务 ======================== */

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
