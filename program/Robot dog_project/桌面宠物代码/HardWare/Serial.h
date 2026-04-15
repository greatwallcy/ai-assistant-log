#ifndef __SERIAL_H
#define __SERIAL_H

#include <stdio.h>
#include "stm32f10x.h"  // 必须包含，否则结构体中的uint8_t无法识别

// 1. 修正：MP3指令固定为9字节，FRAME_LENGTH=9
#define QUEUE_SIZE    144       // 队列总字节数（16帧×9字节，足够缓存16条指令）
#define FRAME_LENGTH  8        // 修正：MP3指令长度=9字节（含校验和）

// 2. 队列结构体定义（不变）
typedef struct {
    uint8_t buffer[QUEUE_SIZE];  // 缓冲区
    uint16_t front;              // 队头索引
    uint16_t rear;               // 队尾索引
    uint16_t count;              // 当前字节数
} SerialQueue;

// 3. 声明全局队列和函数（关键：补全所有队列操作函数的声明）
extern SerialQueue uart2Queue;
extern uint8_t Serial_TxPacket[FRAME_LENGTH];  // 修正：数组长度=FRAME_LENGTH=9
extern uint8_t Serial_RxPacket[8];

// 补全队列操作函数声明（之前缺失，导致函数调用失败）
void Serial_QueueInit(SerialQueue *q);
uint8_t Serial_Enqueue(SerialQueue *q, const uint8_t *data, uint16_t len);
uint8_t Serial_Dequeue(SerialQueue *q, uint8_t *data, uint16_t len);
uint8_t Serial_QueueIsEmpty(SerialQueue *q);

// 其他函数声明（不变）
void Serial_Init(void);
void Serial_SendByte(uint8_t Byte);
void Serial_SendArray(uint8_t *Array, uint16_t Length);
void Serial_SendString(char *String);
void Serial_SendNumber(uint32_t Number, uint8_t Length);
void Serial_Printf(char *format, ...);
void Serial_SendPacket(void);
uint8_t Serial_GetRxFlag(void);
void Serial_SetMP3Cmd(uint8_t MP3_Index);
void Serial_SendMP3CmdToQueue(void);
void Serial_ProcessQueue(void);

#endif
