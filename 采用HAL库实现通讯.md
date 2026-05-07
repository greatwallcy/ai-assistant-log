---
title: 采用HAL库实现通讯——STM32 HAL库 SPI/I2C/UART 完全指南
date: 2026-05-07
tags: [STM32, HAL, SPI, I2C, UART, 嵌入式]
categories: 嵌入式开发
description: 系统介绍 STM32 HAL 库下 SPI、I2C、UART 三种通讯协议的实现方法，涵盖协议原理、CubeMX 配置、编程接口详解、实战案例及常见问题排查。
---

# 采用HAL库实现通讯

> **—— STM32 HAL库 SPI / I2C / UART 通讯完全指南**

本文档系统介绍 STM32 HAL 库下三种常用通讯协议的实现方法：**SPI**、**I2C** 和 **UART（串口）**。每种协议均涵盖基础原理、CubeMX 配置、编程接口详解、实战案例及常见问题排查，适合从入门到进阶的完整学习路径。

---

## 三种协议速查对比

| 特性 | SPI | I2C | UART |
|------|-----|-----|------|
| 线数 | 4线（MOSI/MISO/SCK/NSS） | 2线（SDA/SCL） | 2线（TX/RX） |
| 速率 | 几MHz ~ 几十MHz | 100K ~ 3.4MHz | 115200bps 常见 |
| 通信方式 | 全双工 | 半双工 | 全双工 |
| 主从关系 | 一主多从 | 多主多从 | 点对点 |
| 典型应用 | FLASH、屏幕、传感器 | OLED、EEPROM、传感器 | 调试、模块通信 |
| 是否需要上拉 | 否 | 是（4.7KΩ） | 否 |

---

## 第一章 SPI 通讯

### 1.1 SPI 协议基础

SPI（Serial Peripheral Interface）是高速同步串行总线，适用于短距离高速双向传输。常见外设包括 NOR FLASH、TFT 屏幕、摄像头模块等。

#### 1.1.1 线束说明

| 信号线 | 全称 | 方向 | 说明 |
|--------|------|------|------|
| MOSI | Master Out Slave In | 主→从 | 主机发送数据线 |
| MISO | Master In Slave Out | 从→主 | 主机接收数据线 |
| SCK | Serial Clock | 主→从 | 时钟信号，由主机产生 |
| NSS/CS | Negative Slave Select | 主→从 | 片选信号，低电平有效 |

接线原则：一一对应连接，MOSI 接 MOSI，MISO 接 MISO，SCK 接 SCK。

#### 1.1.2 SPI 五大参数

| 参数 | 选项 | 推荐/说明 |
|------|------|-----------|
| 波特率 | 无固定范围，通常几MHz~几十MHz | 在设备和PCB承受范围内选最大值 |
| 位序 | MSB First / LSB First | 通常选 MSB First（高位先传） |
| 数据位宽 | 8bit / 16bit | 一般选 8bit |
| 时钟极性(CPOL) | Low / High | Low=空闲低电平；High=空闲高电平 |
| 时钟相位(CPHA) | 1 Edge / 2 Edge | CPOL+CPHA 组合决定采样时刻（4种模式） |

#### 1.1.3 SPI 四种工作模式

| 模式 | CPOL | CPHA | 说明 | 常见设备 |
|------|------|------|------|----------|
| Mode 0 | 0 | 0 | 空闲低电平，第一个边沿采样 | FLASH（W25Qxx）、大多数传感器 |
| Mode 1 | 0 | 1 | 空闲低电平，第二个边沿采样 | 部分SD卡 |
| Mode 2 | 1 | 0 | 空闲高电平，第一个边沿采样 | 较少使用 |
| Mode 3 | 1 | 1 | 空闲高电平，第二个边沿采样 | 部分TFT屏幕 |

> 💡 实践中最常见的是 **Mode 0**（CPOL=0, CPHA=0），FLASH 芯片几乎都用这个模式。

### 1.2 CubeMX 配置要点

| 配置项 | 推荐值 | 说明 |
|--------|--------|------|
| CPOL | Low | 时钟空闲态为低电平 |
| CPHA | 1 Edge | 第一个时钟边沿采样（Mode 0） |
| First Bit | MSB First | 高位先传输 |
| Prescaler | 根据需求 | 杜邦线接法建议低速（≤1MHz），PCB可提高 |
| NSS | Software | 软件控制CS，灵活性更高 |

> ⚠️ 注意：杜邦线连接时 SPI 速率建议 ≤1MHz，实测 1MHz 以上杜邦线传输极不稳定。PCB 走线可适当提高到 10MHz+。

**CS（片选）GPIO 配置：**

GPIO 输出速度需根据实际通讯速率选择。若 SPI 速率较高，CS 引脚建议选择 High Speed 或 Very High Speed，以减少信号边沿延迟。

### 1.3 数据发送 — HAL_SPI_Transmit

**发送逻辑：** ① 准备发送数据 → ② 拉低CS选中从机 → ③ 发送数据 → ④ 拉高CS释放从机

**编程接口：**

```c
HAL_StatusTypeDef HAL_SPI_Transmit(
    SPI_HandleTypeDef *hspi,   // SPI句柄，如 &hspi1
    uint8_t *pData,            // 发送数据缓冲区指针
    uint16_t Size,             // 发送字节数
    uint32_t Timeout           // 超时时间(ms)，HAL_MAX_DELAY=无限等待
);
```

**完整示例：**

```c
uint8_t txData[] = {0xAA, 0xBB};
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);  // CS拉低
HAL_SPI_Transmit(&hspi1, txData, 2, HAL_MAX_DELAY);
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);    // CS拉高
```

### 1.4 数据接收 — HAL_SPI_Receive

**接收逻辑：** ① 定义接收缓冲区 → ② 拉低CS → ③ 接收数据 → ④ 拉高CS

```c
HAL_StatusTypeDef HAL_SPI_Receive(
    SPI_HandleTypeDef *hspi,   // SPI句柄
    uint8_t *pData,            // 接收缓冲区指针
    uint16_t Size,             // 接收字节数
    uint32_t Timeout           // 超时时间(ms)
);
```

### 1.5 同时收发 — HAL_SPI_TransmitReceive

全双工模式下，SPI 可以同时发送和接收数据。发送的同时，总线上从机返回的数据会被写入接收缓冲区。

```c
HAL_StatusTypeDef HAL_SPI_TransmitReceive(
    SPI_HandleTypeDef *hspi,   // SPI句柄
    uint8_t *pTxData,          // 发送缓冲区
    uint8_t *pRxData,          // 接收缓冲区
    uint16_t Size,             // 数据长度(字节)
    uint32_t Timeout           // 超时时间(ms)
);
```

### 1.6 实战：W25Q128 FLASH 读写

#### 1.6.1 FLASH 常用指令集

| 指令 | 操作码 | 说明 |
|------|--------|------|
| Write Enable | 0x06 | 写使能，写入前必须发送 |
| Write Disable | 0x04 | 写禁止 |
| Read Status Reg | 0x05 | 读状态寄存器SR0 |
| Read Status Reg-1 | 0x35 | 读状态寄存器SR1 |
| Read Data | 0x03 | 读数据（+24位地址） |
| Page Program | 0x02 | 页编程，写入数据（+24位地址） |
| Sector Erase | 0x20 | 扇区擦除（+24位地址），擦除单位4KB |
| Chip Erase | 0xC7 | 整片擦除 |
| Read Device ID | 0x90 | 读取厂商ID和设备ID |
| JEDEC ID | 0x9F | 读取JEDEC标准ID（3字节） |

#### 1.6.2 读取 FLASH ID

验证 SPI 通讯是否正常的第一步：读取芯片 ID。W25Q128 应返回 Manufacturer ID = 0xEF, Device ID = 0x17。

```c
void CheckFlashID(void) {
    uint8_t idCmd[] = {0x90, 0x00, 0x00, 0x00};  // 读ID指令+3字节地址
    uint8_t id[2] = {0};
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, idCmd, 4, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi1, id, 2, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    // 预期返回: id[0]=0xEF(Winbond), id[1]=0x17(W25Q128)
    char buf[32];
    sprintf(buf, "Flash ID: 0x%02X, 0x%02X\r\n", id[0], id[1]);
    HAL_UART_Transmit(&huart1, (uint8_t*)buf, strlen(buf), HAL_MAX_DELAY);
}
```

#### 1.6.3 读取状态寄存器

状态寄存器用于检查芯片当前状态：是否忙碌（BUSY）、写使能是否生效（WEL）、写保护位等。

```c
void ReadStatusReg(void) {
    uint8_t cmd, sr;
    // 读SR0（含BUSY、WEL、BP0~BP3等位）
    cmd = 0x05;
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);
    HAL_SPI_Receive(&hspi1, &sr, 1, 100);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    char buf[32];
    sprintf(buf, "SR0: 0x%02X\r\n", sr);
    HAL_UART_Transmit(&huart1, (uint8_t*)buf, strlen(buf), 100);
}
```

> 💡 实用技巧：写入前先检查 BUSY 位（SR0 bit0），为 1 时表示芯片正在擦写，需等待。

#### 1.6.4 FLASH 读取数据

读取流程：发送读指令(0x03) + 24位地址 → 持续读取数据直到拉高CS

```c
static uint8_t LoadLEDState(void) {
    uint8_t readDataCmd[] = {0x03, 0x00, 0x00, 0x00}; // 读指令+地址0x000000
    uint8_t ledstate = 0;
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, readDataCmd, 4, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi1, &ledstate, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    return ledstate;
}
```

#### 1.6.5 FLASH 写入数据（完整流程）

FLASH 写入必须遵循严格流程：**写使能 → 扇区擦除 → 等待完成 → 写使能 → 页编程 → 等待完成**。

```
┌─────────────┐
│  写使能(0x06) │
└──────┬──────┘
       ▼
┌─────────────────┐
│ 扇区擦除(0x20)+地址│ ← 必须先擦除再写入！
└──────┬──────────┘
       ▼
┌──────────────┐
│ 等待BUSY位清除  │ ← 读SR0, bit0=0表示完成
└──────┬───────┘
       ▼
┌─────────────┐
│  写使能(0x06) │
└──────┬──────┘
       ▼
┌───────────────────┐
│ 页编程(0x02)+地址+数据│ ← 单次最多写256字节
└──────┬────────────┘
       ▼
┌──────────────┐
│ 等待BUSY位清除  │
└──────────────┘
```

```c
static void SaveLEDstate(uint8_t ledstate) {
    uint8_t cmd, status;
    char buf[64];

    // ===== Step 1: 写使能 =====
    cmd = 0x06;
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

    // ===== Step 2: 扇区擦除 =====
    uint8_t sectorEraseCmd[] = {0x20, 0x00, 0x00, 0x00};
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, sectorEraseCmd, 4, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

    // ===== Step 3: 等待擦除完成 =====
    do {
        cmd = 0x05;
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
        HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
        HAL_SPI_Receive(&hspi1, &status, 1, HAL_MAX_DELAY);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    } while (status & 0x01);  // BUSY位为1则继续等待

    // ===== Step 4: 写使能（再次） =====
    cmd = 0x06;
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

    // ===== Step 5: 页编程 =====
    uint8_t pageProgCmd[] = {0x02, 0x00, 0x00, 0x00, ledstate};
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, pageProgCmd, 5, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

    // ===== Step 6: 等待写入完成 =====
    do {
        cmd = 0x05;
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
        HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
        HAL_SPI_Receive(&hspi1, &status, 1, HAL_MAX_DELAY);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    } while (status & 0x01);

    // ===== 验证写入 =====
    uint8_t verify = LoadLEDState();
    sprintf(buf, "写入%d, 读回%d %s\r\n", ledstate, verify,
            (verify == ledstate) ? "OK" : "FAIL");
    HAL_UART_Transmit(&huart1, (uint8_t*)buf, strlen(buf), HAL_MAX_DELAY);
}
```

> ⚠️ **关键注意事项：**
> 1. FLASH 必须**先擦除再写入**，不能直接覆盖。擦除的最小单位是扇区(4KB)。
> 2. 页编程单次最多写入 **256 字节**，且不能跨页。跨页需要分多次写入。
> 3. 擦除和写入都是**异步操作**，必须轮询 BUSY 位确认完成后才能进行下一步。
> 4. 建议用 `do-while` 而非 `HAL_Delay` 来等待，更高效且不浪费CPU时间。

---

## 第二章 I2C 通讯

### 2.1 I2C 协议基础

I2C（Inter-Integrated Circuit）是两线制串行总线，支持多主多从架构，广泛用于低速外设通信。

#### 2.1.1 硬件要求

1. IO口必须配置为**开漏输出**（Open-Drain），以满足"线与"逻辑——总线上所有设备输出高阻态时，由外部上拉电阻拉高；任一设备拉低时，总线即为低。
2. SDA 和 SCL 线各需外接 **4.7KΩ 上拉电阻**到 VCC。
3. 上拉电阻值可根据总线速率和负载电容调整：速率越高，电阻值应越小（但不宜低于1KΩ）。

#### 2.1.2 通讯时序

| 阶段 | 信号特征 | 说明 |
|------|----------|------|
| 起始位(S) | SCL高电平时，SDA产生下降沿 | 主机发起通讯 |
| 寻址 | 7位/10位从机地址 + R/W位 | R/W: 0=写, 1=读 |
| 数据传输 | SDA在SCL低电平时变化，高电平时采样 | 每字节后跟1位ACK/NACK |
| 停止位(P) | SCL高电平时，SDA产生上升沿 | 主机结束通讯 |

#### 2.1.3 传输速率

| 模式 | 速率 | STM32F103支持 |
|------|------|---------------|
| 标准模式(Standard) | 100 Kbps | ✅ 支持 |
| 快速模式(Fast) | 400 Kbps | ✅ 支持 |
| 快速增强模式(Fast-Plus) | 1 Mbps | ❌ 不支持 |
| 高速模式(High-Speed) | 3.4 Mbps | ❌ 不支持 |

快速模式下可设置占空比：2/1（低电平时间:高电平时间 = 2:1）或 16/9。

### 2.2 实战：0.96寸 OLED 通讯

以 0.96 寸 SSD1306 OLED 屏幕为例，从机地址 0x78（写）/ 0x79（读）。

#### 2.2.1 驱动初始化指令

| 指令 | 说明 |
|------|------|
| 0x00 | 命令模式标识（后续字节为命令） |
| 0x8D, 0x14 | 使能电荷泵（Charge Pump） |
| 0xAF | 打开屏幕显示 |
| 0xA5 | 全屏点亮（调试用，忽略RAM内容） |
| 0xA4 | 正常显示（按RAM内容显示） |

#### 2.2.2 发送驱动指令

```c
// 在I2C初始化之后调用
uint8_t commands[] = {0x00, 0x8D, 0x14, 0xAF, 0xA5};
HAL_I2C_Master_Transmit(&hi2c1, 0x78, commands, 5, HAL_MAX_DELAY);
```

#### 2.2.3 读取从机数据

```c
uint8_t dataRcvd;
HAL_I2C_Master_Receive(&hi2c1, 0x78, &dataRcvd, 1, HAL_MAX_DELAY);

// 判断第6位状态（示例：检测OLED是否忙碌）
if ((dataRcvd & (0x01 << 6)) == 0) {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);  // LED亮
} else {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);    // LED灭
}
```

#### 2.2.4 编程接口详解

```c
// 主机发送
HAL_StatusTypeDef HAL_I2C_Master_Transmit(
    I2C_HandleTypeDef *hi2c,   // I2C句柄
    uint16_t DevAddress,       // 从设备地址（7位地址左移1位）
    uint8_t *pData,            // 发送数据缓冲区
    uint16_t Size,             // 数据字节数
    uint32_t Timeout           // 超时时间(ms)
);

// 主机接收
HAL_StatusTypeDef HAL_I2C_Master_Receive(
    I2C_HandleTypeDef *hi2c,   // I2C句柄
    uint16_t DevAddress,       // 从设备地址
    uint8_t *pData,            // 接收缓冲区
    uint16_t Size,             // 接收字节数
    uint32_t Timeout           // 超时时间(ms)
);
```

> ⚠️ **I2C 重要注意事项：**
> 1. **运算符优先级**：C 语言中 `==` 的优先级高于 `&`。`if(dataRcvd & (0x01<<6) == 0)` 会先算 `==` 再算 `&`，导致逻辑错误。正确写法：`if((dataRcvd & (0x01 << 6)) == 0)`
> 2. `HAL_I2C_Master_Transmit/Receive` 必须在 I2C 初始化函数之后调用。
> 3. **地址格式**：HAL库使用 7 位地址左移 1 位的格式。如 SSD1306 的 7 位地址是 0x3C，传入时应为 0x78。

### 2.3 进阶：I2C DMA 传输

对于大量数据传输（如刷新整个OLED屏幕），使用 DMA 可释放 CPU 资源。

```c
// DMA方式发送（非阻塞）
HAL_I2C_Master_Transmit_DMA(&hi2c1, 0x78, buffer, size);

// 回调函数：发送完成时自动调用
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c) {
    // 发送完成处理
}
```

---

## 第三章 UART 串口通讯

### 3.1 串口协议基础

UART（Universal Asynchronous Receiver/Transmitter）是异步串行通讯接口，只需 TX/RX 两根线即可实现全双工通信。

| 参数 | 常见值 | 说明 |
|------|--------|------|
| 波特率 | 9600 / 115200 | 收发双方必须一致 |
| 数据位 | 8 bit | 最常用 |
| 停止位 | 1 bit | 可选1/1.5/2 |
| 校验位 | None | 可选None/Even/Odd |

### 3.2 数据发送 — HAL_UART_Transmit

发送四种类型数据的完整示例：

```c
// 定义四种数据类型
uint8_t byte_Number = 0x5A;              // 单字节十六进制
uint8_t byte_Array[] = {1, 2, 3, 4, 5};  // 字节数组
char ch = 'a';                            // 单个字符
char *str = "HELLO WORLD";                // 字符串

// 发送（参数统一为 uint8_t* 类型）
HAL_UART_Transmit(&huart1, &byte_Number, 1, HAL_MAX_DELAY);
HAL_UART_Transmit(&huart1, byte_Array, 5, HAL_MAX_DELAY);
HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
```

**参数类型转换说明：**

| 数据类型 | 取地址方式 | 是否需要转换 | 原因 |
|----------|-----------|-------------|------|
| uint8_t 单字节 | `&byte_Number` | 否 | 本身就是 uint8_t* |
| uint8_t 数组 | `byte_Array` | 否 | 数组名即为首地址 |
| char 字符 | `(uint8_t *)&ch` | 是 | char* 需强转为 uint8_t* |
| char* 字符串 | `(uint8_t *)str` | 是 | 同上 |

### 3.3 数据接收 — HAL_UART_Receive

```c
uint8_t dataRcvd;
HAL_UART_Receive(&huart1, &dataRcvd, 1, HAL_MAX_DELAY);

// 方式一：ASCII码模式（串口助手设置"文本"发送）
if (dataRcvd == '0') {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
} else if (dataRcvd == '1') {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}

// 方式二：HEX模式（串口助手设置"HEX"发送）
if (dataRcvd == 0x00) {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
} else if (dataRcvd == 0x01) {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}
```

> 💡 串口助手设置：发送 ASCII 码时选"文本"模式；发送十六进制时选"HEX"模式。两者接收端代码不同。

### 3.4 进阶：中断接收与 DMA 接收

阻塞式接收会卡死 CPU，实际项目中推荐使用中断或 DMA 方式。

#### 3.4.1 中断接收（推荐）

```c
// 启动中断接收（收到1字节后触发回调）
HAL_UART_Receive_IT(&huart1, &rxByte, 1);

// 接收完成回调函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        // 处理接收到的数据 rxByte
        processCommand(rxByte);
        // 重新启动下一次接收（重要！）
        HAL_UART_Receive_IT(&huart1, &rxByte, 1);
    }
}
```

#### 3.4.2 DMA 接收（大数据量）

```c
// 启动DMA接收（接收N字节后触发回调）
HAL_UART_Receive_DMA(&huart1, rxBuf, BUF_SIZE);

// DMA接收完成回调
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    // 处理 rxBuf 中的数据
}
```

#### 3.4.3 printf 重定向到串口

将 printf 重定向到串口，方便调试输出。在代码中添加以下函数：

```c
#include <stdio.h>

int fputc(int ch, FILE *f) {
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

// 使用：直接 printf("Hello %d\n", value);
```

> ⚠️ 使用 printf 重定向需要在 Keil 中勾选 **"Use MicroLIB"** 选项。

---

## 第四章 常见问题与调试技巧

### 4.1 SPI 常见问题

| 问题 | 可能原因 | 解决方案 |
|------|----------|----------|
| 读取ID全为0xFF | SPI未初始化/接线错误 | 检查MOSI/MISO是否交叉，CS是否正确控制 |
| 读取ID全为0x00 | 从机未上电/CS未拉低 | 检查供电，确认CS时序 |
| 数据不稳定 | 速率过高/杜邦线过长 | 降低SPI速率，缩短连线 |
| 写入后数据丢失 | 未等待BUSY位清除 | 用do-while轮询SR0的bit0 |
| 只能写不能读 | MISO线断开 | 检查MISO接线 |

### 4.2 I2C 常见问题

| 问题 | 可能原因 | 解决方案 |
|------|----------|----------|
| 无ACK应答 | 地址错误/设备未接 | 用逻辑分析仪抓地址，检查上拉电阻 |
| 总线锁死(SDA一直低) | 通讯异常中断 | 发送9个时钟脉冲释放总线 |
| 数据错乱 | 时序不满足 | 降低I2C速率，检查上拉电阻值 |
| 只能发不能收 | 地址格式错误 | 确认使用7位地址左移1位格式 |

### 4.3 UART 常见问题

| 问题 | 可能原因 | 解决方案 |
|------|----------|----------|
| 收到乱码 | 波特率不匹配 | 确认收发双方波特率一致 |
| 收不到数据 | TX/RX未交叉 | TX接对方RX，RX接对方TX |
| 数据丢失 | 接收溢出 | 使用中断/DMA替代阻塞接收 |
| 偶发错误 | 线路干扰 | 缩短连线，加屏蔽，降低波特率 |

### 4.4 通用调试建议

1. **善用逻辑分析仪**：查看实际波形，确认时序是否正确。
2. **先低速后高速**：调试时先用最低速率确认通讯正常，再逐步提高。
3. **检查 HAL 返回值**：所有 HAL 函数都有返回值，务必检查是否为 `HAL_OK`。
4. **使用串口打印调试**：关键步骤添加 printf 输出，快速定位问题。
5. **注意堆栈空间**：使用 sprintf 等函数时确保缓冲区足够大，避免栈溢出。

---

## 附录 HAL 库通讯函数速查表

### SPI 函数

| 函数 | 功能 | 阻塞/非阻塞 |
|------|------|-------------|
| `HAL_SPI_Transmit` | 发送数据 | 阻塞 |
| `HAL_SPI_Receive` | 接收数据 | 阻塞 |
| `HAL_SPI_TransmitReceive` | 同时收发 | 阻塞 |
| `HAL_SPI_Transmit_IT` | 中断发送 | 非阻塞 |
| `HAL_SPI_Receive_IT` | 中断接收 | 非阻塞 |
| `HAL_SPI_Transmit_DMA` | DMA发送 | 非阻塞 |
| `HAL_SPI_Receive_DMA` | DMA接收 | 非阻塞 |

### I2C 函数

| 函数 | 功能 | 阻塞/非阻塞 |
|------|------|-------------|
| `HAL_I2C_Master_Transmit` | 主机发送 | 阻塞 |
| `HAL_I2C_Master_Receive` | 主机接收 | 阻塞 |
| `HAL_I2C_Master_Transmit_IT` | 中断发送 | 非阻塞 |
| `HAL_I2C_Master_Receive_IT` | 中断接收 | 非阻塞 |
| `HAL_I2C_Master_Transmit_DMA` | DMA发送 | 非阻塞 |
| `HAL_I2C_Master_Receive_DMA` | DMA接收 | 非阻塞 |
| `HAL_I2C_Mem_Write` | 写EEPROM等存储器 | 阻塞 |
| `HAL_I2C_Mem_Read` | 读EEPROM等存储器 | 阻塞 |

### UART 函数

| 函数 | 功能 | 阻塞/非阻塞 |
|------|------|-------------|
| `HAL_UART_Transmit` | 发送数据 | 阻塞 |
| `HAL_UART_Receive` | 接收数据 | 阻塞 |
| `HAL_UART_Transmit_IT` | 中断发送 | 非阻塞 |
| `HAL_UART_Receive_IT` | 中断接收（推荐） | 非阻塞 |
| `HAL_UART_Transmit_DMA` | DMA发送 | 非阻塞 |
| `HAL_UART_Receive_DMA` | DMA接收 | 非阻塞 |

---

> 📝 **版权声明**：本文为原创技术分享，遵循 CC 4.0 BY-SA 版权协议，转载请注明出处。
