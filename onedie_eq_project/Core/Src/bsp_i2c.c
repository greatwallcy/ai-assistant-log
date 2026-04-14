/**
 * ============================================================================
 * 文件名称 : bsp_i2c.c
 * 功能描述 : I2C BSP驱动实现 - 模拟I2C + EEPROM硬件I2C
 * ============================================================================
 */

#include "bsp_i2c.h"

/* I2C延迟 (72MHz下约5us) */
#define I2C_DELAY()  { volatile uint16_t i = 36; while(i--); }

/* ======================== 模拟I2C实例 ======================== */
SoftI2C_t softI2C_Module1 = {
    .sdaPort = BAL1_SDA_PORT, .sdaPin = BAL1_SDA_PIN,
    .sclPort = BAL1_SCL_PORT, .sclPin = BAL1_SCL_PIN
};
SoftI2C_t softI2C_Module2 = {
    .sdaPort = BAL2_SDA_PORT, .sdaPin = BAL2_SDA_PIN,
    .sclPort = BAL2_SCL_PORT, .sclPin = BAL2_SCL_PIN
};
SoftI2C_t softI2C_Module3 = {
    .sdaPort = BAL3_SDA_PORT, .sdaPin = BAL3_SDA_PIN,
    .sclPort = BAL3_SCL_PORT, .sclPin = BAL3_SCL_PIN
};
SoftI2C_t softI2C_Module4 = {
    .sdaPort = BAL4_SDA_PORT, .sdaPin = BAL4_SDA_PIN,
    .sclPort = BAL4_SCL_PORT, .sclPin = BAL4_SCL_PIN
};

/* ======================== 硬件I2C ======================== */
void BSP_I2C_Init(void)
{
    /* 由MX_I2C1_Init()完成, 此处做额外配置 */
}

HAL_StatusTypeDef BSP_EEPROM_Write(uint16_t memAddr, uint8_t *pData, uint16_t len)
{
    /* AT24C16 页写入 (16字节/页) */
    return HAL_I2C_Mem_Write(&EEPROM_I2C, EEPROM_I2C_ADDR, memAddr,
                             I2C_MEMADD_SIZE_16BIT, pData, len, 100);
}

HAL_StatusTypeDef BSP_EEPROM_Read(uint16_t memAddr, uint8_t *pData, uint16_t len)
{
    return HAL_I2C_Mem_Read(&EEPROM_I2C, EEPROM_I2C_ADDR, memAddr,
                            I2C_MEMADD_SIZE_16BIT, pData, len, 100);
}

/* ======================== 模拟I2C ======================== */

static void SoftI2C_SDA_High(SoftI2C_t *i2c)  { HAL_GPIO_WritePin(i2c->sdaPort, i2c->sdaPin, GPIO_PIN_SET); }
static void SoftI2C_SDA_Low(SoftI2C_t *i2c)   { HAL_GPIO_WritePin(i2c->sdaPort, i2c->sdaPin, GPIO_PIN_RESET); }
static void SoftI2C_SCL_High(SoftI2C_t *i2c)  { HAL_GPIO_WritePin(i2c->sclPort, i2c->sclPin, GPIO_PIN_SET); }
static void SoftI2C_SCL_Low(SoftI2C_t *i2c)   { HAL_GPIO_WritePin(i2c->sclPort, i2c->sclPin, GPIO_PIN_RESET); }
static uint8_t SoftI2C_SDA_Read(SoftI2C_t *i2c) { return HAL_GPIO_ReadPin(i2c->sdaPort, i2c->sdaPin); }

void BSP_SoftI2C_Init(SoftI2C_t *i2c)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* SDA & SCL: 推挽输出, 默认高 */
    HAL_GPIO_WritePin(i2c->sdaPort, i2c->sdaPin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(i2c->sclPort, i2c->sclPin, GPIO_PIN_SET);

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;  /* 开漏输出 */
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    GPIO_InitStruct.Pin = i2c->sdaPin;
    HAL_GPIO_Init(i2c->sdaPort, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = i2c->sclPin;
    HAL_GPIO_Init(i2c->sclPort, &GPIO_InitStruct);
}

static void SoftI2C_Start(SoftI2C_t *i2c)
{
    SoftI2C_SDA_High(i2c);
    SoftI2C_SCL_High(i2c);
    I2C_DELAY();
    SoftI2C_SDA_Low(i2c);
    I2C_DELAY();
    SoftI2C_SCL_Low(i2c);
}

static void SoftI2C_Stop(SoftI2C_t *i2c)
{
    SoftI2C_SDA_Low(i2c);
    SoftI2C_SCL_High(i2c);
    I2C_DELAY();
    SoftI2C_SDA_High(i2c);
    I2C_DELAY();
}

static uint8_t SoftI2C_WriteByte(SoftI2C_t *i2c, uint8_t data)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        SoftI2C_SCL_Low(i2c);
        if (data & 0x80) SoftI2C_SDA_High(i2c);
        else SoftI2C_SDA_Low(i2c);
        I2C_DELAY();
        SoftI2C_SCL_High(i2c);
        I2C_DELAY();
        data <<= 1;
    }
    /* 读ACK */
    SoftI2C_SCL_Low(i2c);
    SoftI2C_SDA_High(i2c);
    I2C_DELAY();
    SoftI2C_SCL_High(i2c);
    uint8_t ack = !SoftI2C_SDA_Read(i2c);
    SoftI2C_SCL_Low(i2c);
    return ack;
}

static uint8_t SoftI2C_ReadByte(SoftI2C_t *i2c, uint8_t ack)
{
    uint8_t data = 0;
    SoftI2C_SDA_High(i2c);
    for (uint8_t i = 0; i < 8; i++)
    {
        data <<= 1;
        SoftI2C_SCL_Low(i2c);
        I2C_DELAY();
        SoftI2C_SCL_High(i2c);
        if (SoftI2C_SDA_Read(i2c)) data |= 0x01;
        I2C_DELAY();
    }
    /* 发ACK/NACK */
    SoftI2C_SCL_Low(i2c);
    if (ack) SoftI2C_SDA_Low(i2c);
    else SoftI2C_SDA_High(i2c);
    I2C_DELAY();
    SoftI2C_SCL_High(i2c);
    I2C_DELAY();
    SoftI2C_SCL_Low(i2c);
    return data;
}

HAL_StatusTypeDef BSP_SoftI2C_Write(SoftI2C_t *i2c, uint8_t addr, uint8_t *pData, uint16_t len)
{
    SoftI2C_Start(i2c);
    if (!SoftI2C_WriteByte(i2c, addr & 0xFE))  /* 写地址 */
    {
        SoftI2C_Stop(i2c);
        return HAL_ERROR;
    }
    for (uint16_t i = 0; i < len; i++)
    {
        if (!SoftI2C_WriteByte(i2c, pData[i]))
        {
            SoftI2C_Stop(i2c);
            return HAL_ERROR;
        }
    }
    SoftI2C_Stop(i2c);
    return HAL_OK;
}

HAL_StatusTypeDef BSP_SoftI2C_Read(SoftI2C_t *i2c, uint8_t addr, uint8_t *pData, uint16_t len)
{
    SoftI2C_Start(i2c);
    if (!SoftI2C_WriteByte(i2c, addr | 0x01))  /* 读地址 */
    {
        SoftI2C_Stop(i2c);
        return HAL_ERROR;
    }
    for (uint16_t i = 0; i < len; i++)
    {
        pData[i] = SoftI2C_ReadByte(i2c, (i < len - 1) ? 1 : 0);
    }
    SoftI2C_Stop(i2c);
    return HAL_OK;
}
