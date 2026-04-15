/**
 * ============================================================================
 * 文件名称 : bsp_balance.c
 * 功能描述 : 均衡模块BSP驱动实现
 * ============================================================================
 */

#include "bsp_balance.h"
#include "bsp_i2c.h"

/* INA226 地址 (取决于A0/A1引脚, 假设各模块不同地址) */
#define INA226_ADDR_MODULE1     0x40
#define INA226_ADDR_MODULE2     0x41
#define INA226_ADDR_MODULE3     0x42
#define INA226_ADDR_MODULE4     0x43

/* TMP112 地址 */
#define TMP112_ADDR_MODULE1     0x48
#define TMP112_ADDR_MODULE2     0x49
#define TMP112_ADDR_MODULE3     0x4A
#define TMP112_ADDR_MODULE4     0x4B

/* INA226 寄存器 */
#define INA226_REG_BUS_VOLT     0x02
#define INA226_REG_SHUNT_VOLT   0x01
#define INA226_REG_CURRENT      0x04

/* TMP112 寄存器 */
#define TMP112_REG_TEMP         0x00

/* 模块引脚映射表 */
typedef struct {
    GPIO_TypeDef *enPort;  uint16_t enPin;     /* VPS2606 EN */
    GPIO_TypeDef *cdPort;  uint16_t cdPin;     /* VPS2606 CD (方向) */
    GPIO_TypeDef *ctrPort[4]; uint16_t ctrPin[4]; /* 74HC154 CTR */
    SoftI2C_t *i2c;
    uint8_t ina226Addr;
    uint8_t tmp112Addr;
} BalanceModuleCfg_t;

static BalanceModuleCfg_t modules[4];

/**
 * @brief  均衡模块初始化
 */
void BSP_Balance_Init(void)
{
    /* 模块1 */
    modules[0].enPort = BAL1_EN_PORT; modules[0].enPin = BAL1_EN_PIN;
    modules[0].cdPort = BAL1_CD_PORT; modules[0].cdPin = BAL1_CD_PIN;
    modules[0].ctrPort[0] = BAL1_CTR1_PORT; modules[0].ctrPin[0] = BAL1_CTR1_PIN;
    modules[0].ctrPort[1] = BAL1_CTR2_PORT; modules[0].ctrPin[1] = BAL1_CTR2_PIN;
    modules[0].ctrPort[2] = BAL1_CTR3_PORT; modules[0].ctrPin[2] = BAL1_CTR3_PIN;
    modules[0].ctrPort[3] = BAL1_CTR4_PORT; modules[0].ctrPin[3] = BAL1_CTR4_PIN;
    modules[0].i2c = &softI2C_Module1;
    modules[0].ina226Addr = INA226_ADDR_MODULE1;
    modules[0].tmp112Addr = TMP112_ADDR_MODULE1;

    /* 模块2 */
    modules[1].enPort = BAL2_EN_PORT; modules[1].enPin = BAL2_EN_PIN;
    modules[1].cdPort = BAL2_CD_PORT; modules[1].cdPin = BAL2_CD_PIN;
    modules[1].ctrPort[0] = (GPIO_TypeDef*)BAL2_CTR1_PORT; modules[1].ctrPin[0] = BAL2_CTR1_PIN;
    modules[1].ctrPort[1] = (GPIO_TypeDef*)BAL2_CTR2_PORT; modules[1].ctrPin[1] = BAL2_CTR2_PIN;
    modules[1].ctrPort[2] = (GPIO_TypeDef*)BAL2_CTR3_PORT; modules[1].ctrPin[2] = BAL2_CTR3_PIN;
    modules[1].ctrPort[3] = (GPIO_TypeDef*)BAL2_CTR4_PORT; modules[1].ctrPin[3] = BAL2_CTR4_PIN;
    modules[1].i2c = &softI2C_Module2;
    modules[1].ina226Addr = INA226_ADDR_MODULE2;
    modules[1].tmp112Addr = TMP112_ADDR_MODULE2;

    /* 模块3 */
    modules[2].enPort = BAL3_EN_PORT; modules[2].enPin = BAL3_EN_PIN;
    modules[2].cdPort = BAL3_CD_PORT; modules[2].cdPin = BAL3_CD_PIN;
    modules[2].ctrPort[0] = (GPIO_TypeDef*)BAL3_CTR1_PORT; modules[2].ctrPin[0] = BAL3_CTR1_PIN;
    modules[2].ctrPort[1] = (GPIO_TypeDef*)BAL3_CTR2_PORT; modules[2].ctrPin[1] = BAL3_CTR2_PIN;
    modules[2].ctrPort[2] = (GPIO_TypeDef*)BAL3_CTR3_PORT; modules[2].ctrPin[2] = BAL3_CTR3_PIN;
    modules[2].ctrPort[3] = (GPIO_TypeDef*)BAL3_CTR4_PORT; modules[2].ctrPin[3] = BAL3_CTR4_PIN;
    modules[2].i2c = &softI2C_Module3;
    modules[2].ina226Addr = INA226_ADDR_MODULE3;
    modules[2].tmp112Addr = TMP112_ADDR_MODULE3;

    /* 模块4 */
    modules[3].enPort = BAL4_EN_PORT; modules[3].enPin = BAL4_EN_PIN;
    modules[3].cdPort = BAL4_CD_PORT; modules[3].cdPin = BAL4_CD_PIN;
    modules[3].ctrPort[0] = (GPIO_TypeDef*)BAL4_CTR1_PORT; modules[3].ctrPin[0] = BAL4_CTR1_PIN;
    modules[3].ctrPort[1] = (GPIO_TypeDef*)BAL4_CTR2_PORT; modules[3].ctrPin[1] = BAL4_CTR2_PIN;
    modules[3].ctrPort[2] = (GPIO_TypeDef*)BAL4_CTR3_PORT; modules[3].ctrPin[2] = BAL4_CTR3_PIN;
    modules[3].ctrPort[3] = (GPIO_TypeDef*)BAL4_CTR4_PORT; modules[3].ctrPin[3] = BAL4_CTR4_PIN;
    modules[3].i2c = &softI2C_Module4;
    modules[3].ina226Addr = INA226_ADDR_MODULE4;
    modules[3].tmp112Addr = TMP112_ADDR_MODULE4;

    /* 初始化所有模拟I2C */
    for (int i = 0; i < 4; i++)
    {
        BSP_SoftI2C_Init(modules[i].i2c);
    }

    /* 确保所有均衡关闭 */
    BSP_Balance_AllStop();
}

/**
 * @brief  选择74HC154译码器通道
 */
void BSP_Balance_SelectChannel(BalanceModule_t module, uint8_t channel)
{
    if (module >= 4 || channel >= 4) return;

    /* 将channel (0-3) 写入4位控制线 */
    for (uint8_t i = 0; i < 4; i++)
    {
        GPIO_PinState state = (channel >> i) & 0x01 ? GPIO_PIN_SET : GPIO_PIN_RESET;
        HAL_GPIO_WritePin(modules[module].ctrPort[i], modules[module].ctrPin[i], state);
    }
}

/**
 * @brief  启动指定通道均衡
 */
void BSP_Balance_Start(BalanceModule_t module, uint8_t channel)
{
    if (module >= 4) return;
    BSP_Balance_SelectChannel(module, channel);
    HAL_GPIO_WritePin(modules[module].enPort, modules[module].enPin, GPIO_PIN_SET);
}

/**
 * @brief  停止指定通道均衡
 */
void BSP_Balance_Stop(BalanceModule_t module, uint8_t channel)
{
    if (module >= 4) return;
    HAL_GPIO_WritePin(modules[module].enPort, modules[module].enPin, GPIO_PIN_RESET);
}

/**
 * @brief  停止所有均衡
 */
void BSP_Balance_AllStop(void)
{
    for (int i = 0; i < 4; i++)
    {
        HAL_GPIO_WritePin(modules[i].enPort, modules[i].enPin, GPIO_PIN_RESET);
    }
}

/**
 * @brief  设置均衡方向
 */
void BSP_Balance_SetDirection(BalanceModule_t module, BalanceDirection_t dir)
{
    if (module >= 4) return;
    HAL_GPIO_WritePin(modules[module].cdPort, modules[module].cdPin,
                      dir == BAL_DIR_CHARGE ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/**
 * @brief  读取电芯电压 (通过INA226)
 */
float BSP_Balance_ReadVoltage(BalanceModule_t module, uint8_t channel)
{
    if (module >= 4) return 0.0f;

    uint8_t buf[2];
    /* 通过模拟I2C读取INA226总线电压寄存器 */
    uint8_t reg = INA226_REG_BUS_VOLT;
    BSP_SoftI2C_Write(modules[module].i2c, modules[module].ina226Addr, &reg, 1);
    BSP_SoftI2C_Read(modules[module].i2c, modules[module].ina226Addr, buf, 2);

    uint16_t raw = ((uint16_t)buf[0] << 8) | buf[1];
    return raw * 1.25f;  /* INA226 LSB = 1.25mV */
}

/**
 * @brief  读取温度 (通过TMP112)
 */
float BSP_Balance_ReadTemperature(BalanceModule_t module, uint8_t channel)
{
    if (module >= 4) return 0.0f;

    uint8_t buf[2];
    uint8_t reg = TMP112_REG_TEMP;
    BSP_SoftI2C_Write(modules[module].i2c, modules[module].tmp112Addr, &reg, 1);
    BSP_SoftI2C_Read(modules[module].i2c, modules[module].tmp112Addr, buf, 2);

    int16_t raw = ((int16_t)buf[0] << 8) | buf[1];
    raw >>= 4;  /* TMP112: 12位精度, 高12位有效 */
    return raw * 0.0625f;  /* LSB = 0.0625°C */
}

/**
 * @brief  读取均衡电流 (通过INA226)
 */
float BSP_Balance_ReadCurrent(BalanceModule_t module, uint8_t channel)
{
    if (module >= 4) return 0.0f;

    uint8_t buf[2];
    uint8_t reg = INA226_REG_CURRENT;
    BSP_SoftI2C_Write(modules[module].i2c, modules[module].ina226Addr, &reg, 1);
    BSP_SoftI2C_Read(modules[module].i2c, modules[module].ina226Addr, buf, 2);

    int16_t raw = ((int16_t)buf[0] << 8) | buf[1];
    return raw * 0.1f;  /* 需根据分流电阻值校准 */
}
