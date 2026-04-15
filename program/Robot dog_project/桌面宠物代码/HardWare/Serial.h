#ifndef __SERIAL_H
#define __SERIAL_H

#include <stdio.h>
#include "stm32f10x.h"

// MP3指令固定为8字节，FRAME_LENGTH=8
#define QUEUE_SIZE    144       // 队列缓冲区字节数(18帧x8字节,足够缓存18条指令)
#define FRAME_LENGTH  8         // 每条MP3指令长度=8字节(含校验)

// 队列结构体定义（环形）
typedef struct {
    uint8_t buffer[QUEUE_SIZE];  // 队列缓冲区
    uint16_t front;              // 队头指针
    uint16_t rear;               // 队尾指针
    uint16_t count;              // 当前字节数
} SerialQueue;

// 全局队列和函数声明
extern SerialQueue uart2Queue;
extern uint8_t Serial_TxPacket[FRAME_LENGTH];  // 发送缓存数组
extern uint8_t Serial_RxPacket[8];

// 队列操作函数
void Serial_QueueInit(SerialQueue *q);
uint8_t Serial_Enqueue(SerialQueue *q, const uint8_t *data, uint16_t len);
uint8_t Serial_Dequeue(SerialQueue *q, uint8_t *data, uint16_t len);
uint8_t Serial_QueueIsEmpty(SerialQueue *q);

// 串口函数
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
