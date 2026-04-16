/**
 * @file    balance_test.c
 * @brief   主控板均衡功能测试程序（第一版）
 * @target  STM32F103VET6 + VPS2606 + 74HC154 + INA226 + TMP112
 * @purpose 验证每个均衡模块的开启/关闭、方向控制、电流采集、电压采集、温度采集
 */

#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>

/*===========================================================================
 *  硬件引脚定义（对照接口定义表）
 *===========================================================================*/

/* --- 均衡模块 1（采集板 A）--- */
#define EQ1_EN_PORT     GPIOA
#define EQ1_EN_PIN      GPIO_Pin_0      /* PA0  VPS2606 ENa */
#define EQ1_CD_PORT     GPIOA
#define EQ1_CD_PIN      GPIO_Pin_1      /* PA1  VPS2606 CDa */
#define EQ1_CTR1_PORT   GPIOC
#define EQ1_CTR1_PIN    GPIO_Pin_1      /* PC1  CTR_a1 */
#define EQ1_CTR2_PORT   GPIOC
#define EQ1_CTR2_PIN    GPIO_Pin_0      /* PC0  CTR_a2 */
#define EQ1_CTR3_PORT   GPIOC
#define EQ1_CTR3_PIN    GPIO_Pin_2      /* PC2  CTR_a3 */
#define EQ1_CTR4_PORT   GPIOC
#define EQ1_CTR4_PIN    GPIO_Pin_3      /* PC3  CTR_a4 */
#define EQ1_SDA_PORT    GPIOA
#define EQ1_SDA_PIN     GPIO_Pin_2      /* PA2  INA226/TMP112 SDA */
#define EQ1_SCL_PORT    GPIOA
#define EQ1_SCL_PIN     GPIO_Pin_3      /* PA3  INA226/TMP112 SCL */

/* --- 均衡模块 2（采集板 B）--- */
#define EQ2_EN_PORT     GPIOE
#define EQ2_EN_PIN      GPIO_Pin_6      /* PE6  VPS2606 ENb */
#define EQ2_CD_PORT     GPIOE
#define EQ2_CD_PIN      GPIO_Pin_4      /* PE4  VPS2606 CDb */
#define EQ2_CTR1_PORT   GPIOA
#define EQ2_CTR1_PIN    GPIO_Pin_5      /* PA5  CTR_b1 */
#define EQ2_CTR2_PORT   GPIOA
#define EQ2_CTR2_PIN    GPIO_Pin_4      /* PA4  CTR_b2 */
#define EQ2_CTR3_PORT   GPIOA
#define EQ2_CTR3_PIN    GPIO_Pin_6      /* PA6  CTR_b3 */
#define EQ2_CTR4_PORT   GPIOE
#define EQ2_CTR4_PIN    GPIO_Pin_5      /* PE5  CTR_b4 */
#define EQ2_SDA_PORT    GPIOE
#define EQ2_SDA_PIN     GPIO_Pin_2      /* PE2  SDA_b */
#define EQ2_SCL_PORT    GPIOE
#define EQ2_SCL_PIN     GPIO_Pin_3      /* PE3  SCL_b */

/* --- 均衡模块 3（采集板 C）--- */
#define EQ3_EN_PORT     GPIOE
#define EQ3_EN_PIN      GPIO_Pin_13     /* PE13 VPS2606 ENc */
#define EQ3_CD_PORT     GPIOE
#define EQ3_CD_PIN      GPIO_Pin_12     /* PE12 VPS2606 CDc */
#define EQ3_CTR1_PORT   GPIOB
#define EQ3_CTR1_PIN    GPIO_Pin_10     /* PB10 CTR_c1 */
#define EQ3_CTR2_PORT   GPIOB
#define EQ3_CTR2_PIN    GPIO_Pin_11     /* PB11 CTR_c2 */
#define EQ3_CTR3_PORT   GPIOE
#define EQ3_CTR3_PIN    GPIO_Pin_15     /* PE15 CTR_c3 */
#define EQ3_CTR4_PORT   GPIOE
#define EQ3_CTR4_PIN    GPIO_Pin_14     /* PE14 CTR_c4 */
#define EQ3_SDA_PORT    GPIOE
#define EQ3_SDA_PIN     GPIO_Pin_10     /* PE10 SDA_c */
#define EQ3_SCL_PORT    GPIOE
#define EQ3_SCL_PIN     GPIO_Pin_11     /* PE11 SCL_c */

/* --- 均衡模块 4（采集板 D）--- */
#define EQ4_EN_PORT     GPIOB
#define EQ4_EN_PIN      GPIO_Pin_0      /* PB0  VPS2606 ENd */
#define EQ4_CD_PORT     GPIOC
#define EQ4_CD_PIN      GPIO_Pin_5      /* PC5  VPS2606 CDd */
#define EQ4_CTR1_PORT   GPIOE
#define EQ4_CTR1_PIN    GPIO_Pin_8      /* PE8  CTR_d1 */
#define EQ4_CTR2_PORT   GPIOE
#define EQ4_CTR2_PIN    GPIO_Pin_9      /* PE9  CTR_d2 */
#define EQ4_CTR3_PORT   GPIOE
#define EQ4_CTR3_PIN    GPIO_Pin_7      /* PE7  CTR_d3 */
#define EQ4_CTR4_PORT   GPIOB
#define EQ4_CTR4_PIN    GPIO_Pin_1      /* PB1  CTR_d4 */
#define EQ4_SDA_PORT    GPIOA
#define EQ4_SDA_PIN     GPIO_Pin_7      /* PA7  SDA_d */
#define EQ4_SCL_PORT    GPIOC
#define EQ4_SCL_PIN     GPIO_Pin_4      /* PC4  SCL_d */

/*===========================================================================
 *  INA226 寄存器定义
 *===========================================================================*/
#define INA226_ADDR             0x40    /* 默认地址（A0=A1=GND） */
#define INA226_REG_CONFIG       0x00
#define INA226_REG_SHUNT_V      0x01
#define INA226_REG_BUS_V        0x02
#define INA226_REG_POWER        0x03
#define INA226_REG_CURRENT      0x04
#define INA226_REG_CALIB        0x05

/* INA226 配置：连续转换，8次平均，1.1ms转换时间 */
#define INA226_CONFIG_VALUE     0x4527
/* 校准值：Rshunt=10mΩ, Imax=10A → CAL = 0.00512 / (10mΩ * 10A/32768) ≈ 1677 */
#define INA226_CALIB_VALUE      1677
/* LSB: 电流 = raw * 0.000610A, 电压 = raw * 1.25mV, 分流电压 = raw * 2.5uV */

/*===========================================================================
 *  TMP112 寄存器定义
 *===========================================================================*/
#define TMP112_ADDR             0x48    /* 默认地址 */
#define TMP112_REG_TEMP         0x00
#define TMP112_REG_CONFIG       0x01
#define TMP112_CONFIG_VALUE     0x60A0  /* 12bit, 连续转换 */

/*===========================================================================
 *  均衡模块数据结构
 *===========================================================================*/
typedef struct {
    uint8_t     index;          /* 模块编号 1-4 */
    GPIO_TypeDef *en_port;
    uint16_t    en_pin;
    GPIO_TypeDef *cd_port;
    uint16_t    cd_pin;
    GPIO_TypeDef *ctr_port[4];  /* 74HC154 地址线 */
    uint16_t    ctr_pin[4];
    GPIO_TypeDef *sda_port;
    uint16_t    sda_pin;
    GPIO_TypeDef *scl_port;
    uint16_t    scl_pin;
    /* 测量结果 */
    float       current_a;      /* 均衡电流 (A) */
    float       voltage_v;      /* 采集板侧电压 (V) */
    float       temp_c;         /* 温度 (°C) */
} BalanceModule_t;

static BalanceModule_t g_modules[4];

/*===========================================================================
 *  延时函数
 *===========================================================================*/
static void delay_us(uint32_t us)
{
    us *= (SystemCoreClock / 1000000);
    while (us--) __NOP();
}

static void delay_ms(uint32_t ms)
{
    while (ms--) delay_us(1000);
}

/*===========================================================================
 *  GPIO 输出控制
 *===========================================================================*/
static void gpio_set(GPIO_TypeDef *port, uint16_t pin, uint8_t state)
{
    if (state)
        GPIO_SetBits(port, pin);
    else
        GPIO_ResetBits(port, pin);
}

/*===========================================================================
 *  软件 I2C（模拟 I2C，每个模块独立总线）
 *===========================================================================*/
static void i2c_sda_high(BalanceModule_t *m)  { gpio_set(m->sda_port, m->sda_pin, 1); }
static void i2c_sda_low(BalanceModule_t *m)   { gpio_set(m->sda_port, m->sda_pin, 0); }
static void i2c_scl_high(BalanceModule_t *m)  { gpio_set(m->scl_port, m->scl_pin, 1); }
static void i2c_scl_low(BalanceModule_t *m)   { gpio_set(m->scl_port, m->scl_pin, 0); }

static void i2c_delay(void) { delay_us(5); }

static void i2c_start(BalanceModule_t *m)
{
    i2c_sda_high(m); i2c_scl_high(m); i2c_delay();
    i2c_sda_low(m);  i2c_delay();
    i2c_scl_low(m);  i2c_delay();
}

static void i2c_stop(BalanceModule_t *m)
{
    i2c_sda_low(m);  i2c_delay();
    i2c_scl_high(m); i2c_delay();
    i2c_sda_high(m); i2c_delay();
}

static void i2c_send_ack(BalanceModule_t *m)
{
    i2c_sda_low(m);  i2c_delay();
    i2c_scl_high(m); i2c_delay();
    i2c_scl_low(m);  i2c_delay();
    i2c_sda_high(m);
}

static uint8_t i2c_wait_ack(BalanceModule_t *m)
{
    uint8_t ack;
    i2c_sda_high(m); i2c_delay();
    i2c_scl_high(m); i2c_delay();
    /* 配置为输入读取 ACK */
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = m->sda_pin;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(m->sda_port, &gpio);
    ack = GPIO_ReadInputDataBit(m->sda_port, m->sda_pin);
    /* 恢复为输出 */
    gpio.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(m->sda_port, &gpio);
    i2c_scl_low(m); i2c_delay();
    return (ack == 0) ? 0 : 1;  /* 0 = ACK */
}

static void i2c_send_byte(BalanceModule_t *m, uint8_t data)
{
    for (int i = 0; i < 8; i++) {
        if (data & 0x80) i2c_sda_high(m);
        else             i2c_sda_low(m);
        data <<= 1;
        i2c_delay();
        i2c_scl_high(m); i2c_delay();
        i2c_scl_low(m);  i2c_delay();
    }
}

static uint8_t i2c_read_byte(BalanceModule_t *m, uint8_t ack)
{
    uint8_t data = 0;
    i2c_sda_high(m);
    /* 配置为输入 */
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = m->sda_pin;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(m->sda_port, &gpio);
    for (int i = 0; i < 8; i++) {
        data <<= 1;
        i2c_scl_high(m); i2c_delay();
        if (GPIO_ReadInputDataBit(m->sda_port, m->sda_pin))
            data |= 0x01;
        i2c_scl_low(m); i2c_delay();
    }
    /* 恢复为输出 */
    gpio.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(m->sda_port, &gpio);
    if (ack) i2c_send_ack(m);
    else     { i2c_sda_high(m); i2c_delay(); i2c_scl_high(m); i2c_delay(); i2c_scl_low(m); }
    return data;
}

/*===========================================================================
 *  INA226 读写
 *===========================================================================*/
static uint8_t ina226_write_reg(BalanceModule_t *m, uint8_t reg, uint16_t value)
{
    i2c_start(m);
    i2c_send_byte(m, INA226_ADDR << 1);     /* 写地址 */
    if (i2c_wait_ack(m)) { i2c_stop(m); return 1; }
    i2c_send_byte(m, reg);
    i2c_wait_ack(m);
    i2c_send_byte(m, (value >> 8) & 0xFF);
    i2c_wait_ack(m);
    i2c_send_byte(m, value & 0xFF);
    i2c_wait_ack(m);
    i2c_stop(m);
    return 0;
}

static uint16_t ina226_read_reg(BalanceModule_t *m, uint8_t reg)
{
    uint16_t value;
    i2c_start(m);
    i2c_send_byte(m, INA226_ADDR << 1);
    if (i2c_wait_ack(m)) { i2c_stop(m); return 0xFFFF; }
    i2c_send_byte(m, reg);
    i2c_wait_ack(m);
    i2c_start(m);  /* 重复起始 */
    i2c_send_byte(m, (INA226_ADDR << 1) | 1);  /* 读地址 */
    if (i2c_wait_ack(m)) { i2c_stop(m); return 0xFFFF; }
    value = i2c_read_byte(m, 1) << 8;
    value |= i2c_read_byte(m, 0);
    i2c_stop(m);
    return value;
}

static uint8_t ina226_init(BalanceModule_t *m)
{
    if (ina226_write_reg(m, INA226_REG_CONFIG, INA226_CONFIG_VALUE)) return 1;
    if (ina226_write_reg(m, INA226_REG_CALIB, INA226_CALIB_VALUE))  return 1;
    return 0;
}

static float ina226_read_current(BalanceModule_t *m)
{
    int16_t raw = (int16_t)ina226_read_reg(m, INA226_REG_CURRENT);
    return raw * 0.000610f;  /* 单位：A */
}

static float ina226_read_bus_voltage(BalanceModule_t *m)
{
    uint16_t raw = ina226_read_reg(m, INA226_REG_BUS_V);
    return raw * 0.00125f;  /* 单位：V */
}

static float ina226_read_shunt_voltage(BalanceModule_t *m)
{
    int16_t raw = (int16_t)ina226_read_reg(m, INA226_REG_SHUNT_V);
    return raw * 0.0000025f;  /* 单位：V */
}

/*===========================================================================
 *  TMP112 读取温度
 *===========================================================================*/
static float tmp112_read_temp(BalanceModule_t *m)
{
    int16_t raw;
    i2c_start(m);
    i2c_send_byte(m, TMP112_ADDR << 1);
    if (i2c_wait_ack(m)) { i2c_stop(m); return -999.0f; }
    i2c_send_byte(m, TMP112_REG_TEMP);
    i2c_wait_ack(m);
    i2c_start(m);
    i2c_send_byte(m, (TMP112_ADDR << 1) | 1);
    if (i2c_wait_ack(m)) { i2c_stop(m); return -999.0f; }
    raw = i2c_read_byte(m, 1) << 8;
    raw |= i2c_read_byte(m, 0);
    i2c_stop(m);
    /* TMP112: 12bit模式，高12位有效，0.0625°C/LSB */
    raw >>= 4;
    if (raw & 0x800) raw |= 0xF000;  /* 符号扩展 */
    return raw * 0.0625f;
}

/*===========================================================================
 *  均衡控制函数
 *===========================================================================*/

/**
 * @brief  设置 VPS2606 使能和方向
 * @param  m: 均衡模块
 * @param  enable: 1=开启, 0=关闭
 * @param  direction: 0=正向充电, 1=反向放电
 *
 *  VPS2606 使能逻辑：
 *    EN=0, CD=0 → 模块不工作（待机 <5uA）
 *    EN=1, CD=0 → 正向传输（充电）
 *    EN=1, CD=1 → 反向传输（放电）
 */
static void balance_set(BalanceModule_t *m, uint8_t enable, uint8_t direction)
{
    gpio_set(m->en_port, m->en_pin, enable);
    gpio_set(m->cd_port, m->cd_pin, direction);
}

/**
 * @brief  设置 74HC154 译码器地址（选择 13 串中的哪一串）
 * @param  m: 均衡模块
 * @param  channel: 0~12（对应第1~13串）
 */
static void balance_select_cell(BalanceModule_t *m, uint8_t channel)
{
    if (channel > 12) channel = 12;
    /* 74HC154: 4位二进制地址，输出 Y0-Y12 有效 */
    gpio_set(m->ctr_port[0], m->ctr_pin[0], (channel >> 0) & 1);
    gpio_set(m->ctr_port[1], m->ctr_pin[1], (channel >> 1) & 1);
    gpio_set(m->ctr_port[2], m->ctr_pin[2], (channel >> 2) & 1);
    gpio_set(m->ctr_port[3], m->ctr_pin[3], (channel >> 3) & 1);
}

/**
 * @brief  关闭均衡（安全状态）
 */
static void balance_stop(BalanceModule_t *m)
{
    balance_set(m, 0, 0);       /* VPS2606 关闭 */
    balance_select_cell(m, 0);  /* 74HC154 选 Y0 */
}

/*===========================================================================
 *  串口打印（调试）
 *===========================================================================*/
static void uart_init(void)
{
    /* USART1: PA9=TX, PA10=RX, 115200bps */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
    
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = GPIO_Pin_9;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);
    gpio.GPIO_Pin = GPIO_Pin_10;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);
    
    USART_InitTypeDef usart;
    usart.USART_BaudRate = 115200;
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_Parity = USART_Parity_No;
    usart.USART_Mode = USART_Mode_Tx;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART1, &usart);
    USART_Cmd(USART1, ENABLE);
}

static void uart_send_char(char c)
{
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, c);
}

static void uart_print(const char *str)
{
    while (*str) uart_send_char(*str++);
}

static void uart_printf_float(const char *prefix, float val, const char *unit)
{
    char buf[64];
    int int_part = (int)val;
    int dec_part = (int)((val - int_part) * 1000);
    if (dec_part < 0) dec_part = -dec_part;
    sprintf(buf, "%s%d.%03d %s\r\n", prefix, int_part, dec_part, unit);
    uart_print(buf);
}

/*===========================================================================
 *  GPIO 初始化
 *===========================================================================*/
static void gpio_init_output(GPIO_TypeDef *port, uint16_t pin)
{
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = pin;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(port, &gpio);
    GPIO_ResetBits(port, pin);
}

static void gpio_init_od(GPIO_TypeDef *port, uint16_t pin)
{
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = pin;
    gpio.GPIO_Mode = GPIO_Mode_Out_OD;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(port, &gpio);
    GPIO_SetBits(port, pin);  /* 开漏默认高 */
}

static void system_gpio_init(void)
{
    /* 使能所有 GPIO 时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
                           RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD |
                           RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE);
    
    /* 禁用 JTAG，保留 SWD（释放 PB3, PB4, PA15 用于 GPIO） */
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    
    /* 为每个均衡模块初始化 GPIO */
    for (int i = 0; i < 4; i++) {
        BalanceModule_t *m = &g_modules[i];
        /* EN, CD: 推挽输出 */
        gpio_init_output(m->en_port, m->en_pin);
        gpio_init_output(m->cd_port, m->cd_pin);
        /* CTR1~CTR4: 推挽输出 */
        for (int j = 0; j < 4; j++) {
            gpio_init_output(m->ctr_port[j], m->ctr_pin[j]);
        }
        /* SDA, SCL: 开漏输出（模拟 I2C） */
        gpio_init_od(m->sda_port, m->sda_pin);
        gpio_init_od(m->scl_port, m->scl_pin);
    }
}

/*===========================================================================
 *  模块初始化
 *===========================================================================*/
static void modules_init(void)
{
    /* 模块 1（采集板 A）*/
    g_modules[0].index = 1;
    g_modules[0].en_port = EQ1_EN_PORT;  g_modules[0].en_pin = EQ1_EN_PIN;
    g_modules[0].cd_port = EQ1_CD_PORT;  g_modules[0].cd_pin = EQ1_CD_PIN;
    g_modules[0].ctr_port[0] = EQ1_CTR1_PORT; g_modules[0].ctr_pin[0] = EQ1_CTR1_PIN;
    g_modules[0].ctr_port[1] = EQ1_CTR2_PORT; g_modules[0].ctr_pin[1] = EQ1_CTR2_PIN;
    g_modules[0].ctr_port[2] = EQ1_CTR3_PORT; g_modules[0].ctr_pin[2] = EQ1_CTR3_PIN;
    g_modules[0].ctr_port[3] = EQ1_CTR4_PORT; g_modules[0].ctr_pin[3] = EQ1_CTR4_PIN;
    g_modules[0].sda_port = EQ1_SDA_PORT; g_modules[0].sda_pin = EQ1_SDA_PIN;
    g_modules[0].scl_port = EQ1_SCL_PORT; g_modules[0].scl_pin = EQ1_SCL_PIN;
    
    /* 模块 2（采集板 B）*/
    g_modules[1].index = 2;
    g_modules[1].en_port = EQ2_EN_PORT;  g_modules[1].en_pin = EQ2_EN_PIN;
    g_modules[1].cd_port = EQ2_CD_PORT;  g_modules[1].cd_pin = EQ2_CD_PIN;
    g_modules[1].ctr_port[0] = EQ2_CTR1_PORT; g_modules[1].ctr_pin[0] = EQ2_CTR1_PIN;
    g_modules[1].ctr_port[1] = EQ2_CTR2_PORT; g_modules[1].ctr_pin[1] = EQ2_CTR2_PIN;
    g_modules[1].ctr_port[2] = EQ2_CTR3_PORT; g_modules[1].ctr_pin[2] = EQ2_CTR3_PIN;
    g_modules[1].ctr_port[3] = EQ2_CTR4_PORT; g_modules[1].ctr_pin[3] = EQ2_CTR4_PIN;
    g_modules[1].sda_port = EQ2_SDA_PORT; g_modules[1].sda_pin = EQ2_SDA_PIN;
    g_modules[1].scl_port = EQ2_SCL_PORT; g_modules[1].scl_pin = EQ2_SCL_PIN;
    
    /* 模块 3（采集板 C）*/
    g_modules[2].index = 3;
    g_modules[2].en_port = EQ3_EN_PORT;  g_modules[2].en_pin = EQ3_EN_PIN;
    g_modules[2].cd_port = EQ3_CD_PORT;  g_modules[2].cd_pin = EQ3_CD_PIN;
    g_modules[2].ctr_port[0] = EQ3_CTR1_PORT; g_modules[2].ctr_pin[0] = EQ3_CTR1_PIN;
    g_modules[2].ctr_port[1] = EQ3_CTR2_PORT; g_modules[2].ctr_pin[1] = EQ3_CTR2_PIN;
    g_modules[2].ctr_port[2] = EQ3_CTR3_PORT; g_modules[2].ctr_pin[2] = EQ3_CTR3_PIN;
    g_modules[2].ctr_port[3] = EQ3_CTR4_PORT; g_modules[2].ctr_pin[3] = EQ3_CTR4_PIN;
    g_modules[2].sda_port = EQ3_SDA_PORT; g_modules[2].sda_pin = EQ3_SDA_PIN;
    g_modules[2].scl_port = EQ3_SCL_PORT; g_modules[2].scl_pin = EQ3_SCL_PIN;
    
    /* 模块 4（采集板 D）*/
    g_modules[3].index = 4;
    g_modules[3].en_port = EQ4_EN_PORT;  g_modules[3].en_pin = EQ4_EN_PIN;
    g_modules[3].cd_port = EQ4_CD_PORT;  g_modules[3].cd_pin = EQ4_CD_PIN;
    g_modules[3].ctr_port[0] = EQ4_CTR1_PORT; g_modules[3].ctr_pin[0] = EQ4_CTR1_PIN;
    g_modules[3].ctr_port[1] = EQ4_CTR2_PORT; g_modules[3].ctr_pin[1] = EQ4_CTR2_PIN;
    g_modules[3].ctr_port[2] = EQ4_CTR3_PORT; g_modules[3].ctr_pin[2] = EQ4_CTR3_PIN;
    g_modules[3].ctr_port[3] = EQ4_CTR4_PORT; g_modules[3].ctr_pin[3] = EQ4_CTR4_PIN;
    g_modules[3].sda_port = EQ4_SDA_PORT; g_modules[3].sda_pin = EQ4_SDA_PIN;
    g_modules[3].scl_port = EQ4_SCL_PORT; g_modules[3].scl_pin = EQ4_SCL_PIN;
}

/*===========================================================================
 *  测试函数
 *===========================================================================*/

/**
 * @brief  测试单个均衡模块
 * @param  m: 均衡模块指针
 *
 *  测试流程：
 *  1. 初始化 INA226 和 TMP112
 *  2. 读取初始电流/电压/温度（模块关闭状态）
 *  3. 开启正向充电，扫描 13 个通道
 *  4. 开启反向放电，扫描 13 个通道
 *  5. 关闭模块，确认回到待机状态
 */
static void test_module(BalanceModule_t *m)
{
    char buf[128];
    sprintf(buf, "\r\n===== 测试均衡模块 %d =====\r\n", m->index);
    uart_print(buf);
    
    /* 1. 初始化 I2C 外设 */
    uart_print("[1] 初始化 INA226...");
    if (ina226_init(m)) {
        uart_print(" FAIL (I2C NACK)\r\n");
        return;
    }
    uart_print(" OK\r\n");
    
    /* 2. 关闭状态测量 */
    balance_stop(m);
    delay_ms(100);
    
    uart_print("[2] 关闭状态测量:\r\n");
    m->current_a = ina226_read_current(m);
    m->voltage_v = ina226_read_bus_voltage(m);
    m->temp_c = tmp112_read_temp(m);
    uart_printf_float("  电流: ", m->current_a, "A");
    uart_printf_float("  电压: ", m->voltage_v, "V");
    uart_printf_float("  温度: ", m->temp_c, "°C");
    
    /* 3. 正向充电测试（EN=1, CD=0） */
    uart_print("[3] 正向充电测试 (EN=1, CD=0):\r\n");
    for (int ch = 0; ch < 13; ch++) {
        balance_select_cell(m, ch);
        balance_set(m, 1, 0);  /* 开启正向 */
        delay_ms(200);         /* 等待 VPS2606 启动 */
        
        m->current_a = ina226_read_current(m);
        m->voltage_v = ina226_read_bus_voltage(m);
        
        sprintf(buf, "  CH%02d: I=", ch + 1);
        uart_print(buf);
        uart_printf_float("", m->current_a, "A  V=");
        sprintf(buf, "  CH%02d: ", ch + 1);
        char tmp[32];
        int i1 = (int)m->current_a;
        int i2 = (int)((m->current_a - i1) * 1000);
        if (i2 < 0) i2 = -i2;
        int v1 = (int)m->voltage_v;
        int v2 = (int)((m->voltage_v - v1) * 1000);
        sprintf(buf, "  CH%02d: I=%d.%03dA  V=%d.%03dV\r\n", ch+1, i1, i2, v1, v2);
        uart_print(buf);
        
        balance_set(m, 0, 0);  /* 关闭 */
        delay_ms(50);
    }
    
    /* 4. 反向放电测试（EN=1, CD=1） */
    uart_print("[4] 反向放电测试 (EN=1, CD=1):\r\n");
    for (int ch = 0; ch < 13; ch++) {
        balance_select_cell(m, ch);
        balance_set(m, 1, 1);  /* 开启反向 */
        delay_ms(200);
        
        m->current_a = ina226_read_current(m);
        m->voltage_v = ina226_read_bus_voltage(m);
        
        int i1 = (int)m->current_a;
        int i2 = (int)((m->current_a - i1) * 1000);
        if (i2 < 0) i2 = -i2;
        int v1 = (int)m->voltage_v;
        int v2 = (int)((m->voltage_v - v1) * 1000);
        sprintf(buf, "  CH%02d: I=%d.%03dA  V=%d.%03dV\r\n", ch+1, i1, i2, v1, v2);
        uart_print(buf);
        
        balance_set(m, 0, 0);
        delay_ms(50);
    }
    
    /* 5. 温度读取 */
    uart_print("[5] 温度读取:\r\n");
    m->temp_c = tmp112_read_temp(m);
    int t1 = (int)m->temp_c;
    int t2 = (int)((m->temp_c - t1) * 100);
    if (t2 < 0) t2 = -t2;
    sprintf(buf, "  TMP112: %d.%02d °C\r\n", t1, t2);
    uart_print(buf);
    
    /* 6. 安全关闭 */
    balance_stop(m);
    uart_print("[6] 模块已安全关闭\r\n");
    uart_print("===== 测试完成 =====\r\n");
}

/*===========================================================================
 *  主函数
 *===========================================================================*/
int main(void)
{
    /* 系统时钟初始化（默认 72MHz） */
    SystemInit();
    
    /* 初始化模块数据 */
    modules_init();
    
    /* 初始化 GPIO */
    system_gpio_init();
    
    /* 初始化串口 */
    uart_init();
    
    uart_print("\r\n");
    uart_print("========================================\r\n");
    uart_print("  Onedie EQ 主动均衡功能测试 v1.0\r\n");
    uart_print("  MCU: STM32F103VET6\r\n");
    uart_print("  日期: 2026-04-16\r\n");
    uart_print("========================================\r\n");
    
    /* 测试所有 4 个均衡模块 */
    for (int i = 0; i < 4; i++) {
        test_module(&g_modules[i]);
        delay_ms(500);
    }
    
    uart_print("\r\n所有模块测试完成！\r\n");
    
    /* 主循环：持续读取所有模块的电流/电压/温度 */
    while (1) {
        uart_print("\r\n--- 实时监控 ---\r\n");
        for (int i = 0; i < 4; i++) {
            BalanceModule_t *m = &g_modules[i];
            m->current_a = ina226_read_current(m);
            m->voltage_v = ina226_read_bus_voltage(m);
            m->temp_c = tmp112_read_temp(m);
            
            char buf[128];
            int i1 = (int)m->current_a;
            int i2 = (int)((m->current_a - i1) * 1000); if (i2<0) i2=-i2;
            int v1 = (int)m->voltage_v;
            int v2 = (int)((m->voltage_v - v1) * 1000);
            int t1 = (int)m->temp_c;
            int t2 = (int)((m->temp_c - t1) * 100); if (t2<0) t2=-t2;
            sprintf(buf, "  Module%d: I=%d.%03dA V=%d.%03dV T=%d.%02dC\r\n",
                    m->index, i1, i2, v1, v2, t1, t2);
            uart_print(buf);
        }
        delay_ms(2000);
    }
}
