#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include <stdarg.h>
#include "PetAction.h"  
#include "Serial.h"
uint8_t Serial_TxPacket[FRAME_LENGTH];				//定义发送数据包数组，数据包格式：FF 01 02 03 04 FE
uint8_t Serial_RxPacket[8];				//定义接收数据包数组
uint8_t Serial_RxFlag;					//定义接收数据包标志位

//增加队列函数
// 定义全局队列
SerialQueue uart2Queue;

// 1. 队列初始化
void Serial_QueueInit(SerialQueue *q) {
    q->front = 0;
    q->rear = 0;
    q->count = 0;
}

// 2. 入队操作（中断安全，返回1成功，0失败）
uint8_t Serial_Enqueue(SerialQueue *q, const uint8_t *data, uint16_t len) {
    if (q == NULL || data == NULL || len == 0) return 0;
    
    // 检查队列是否有足够空间
    if (q->count + len > QUEUE_SIZE) {
        return 0;  // 队列满，入队失败（可根据需求改为覆盖旧数据）
    }
    
    // 关闭中断，确保入队过程不被打断
    __set_PRIMASK(1);
    
    // 拷贝数据到队列
    for (uint16_t i = 0; i < len; i++) {
        q->buffer[q->rear] = data[i];
        q->rear = (q->rear + 1) % QUEUE_SIZE;  // 环形递增
    }
    q->count += len;
    
    // 恢复中断
    __set_PRIMASK(0);
    return 1;
}

// 3. 出队操作（返回1成功，0失败）
uint8_t Serial_Dequeue(SerialQueue *q, uint8_t *data, uint16_t len) {
    if (q == NULL || data == NULL || len == 0) return 0;
    
    // 检查队列是否有足够数据
    if (q->count < len) {
        return 0;  // 数据不足，出队失败
    }
    
    // 关闭中断，确保出队过程不被打断
    __set_PRIMASK(1);
    
    // 从队列拷贝数据
    for (uint16_t i = 0; i < len; i++) {
        data[i] = q->buffer[q->front];
        q->front = (q->front + 1) % QUEUE_SIZE;  // 环形递增
    }
    q->count -= len;
    
    // 恢复中断
    __set_PRIMASK(0);
    return 1;
}

// 4. 判断队列是否为空
uint8_t Serial_QueueIsEmpty(SerialQueue *q) {
    return (q->count == 0) ? 1 : 0;
}
//队列函数到此结束


/**
  * 函    数：串口初始化,串口改到UATR2，PA2，PA3
  * 参    数：无
  * 返 回 值：无
  */
void Serial_Init(void)
{
	// 1. USART2硬件初始化（完整代码，不能省略）
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);  // 开启USART2时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);   // 开启GPIOA时钟

    // GPIO配置：PA2(TX)复用推挽，PA3(RX)上拉输入
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // USART2参数配置（9600bps，8N1）
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx;  // 仅发送
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART2, &USART_InitStructure);

    // 关键：使能USART2（不使能则串口不工作）
    USART_Cmd(USART2, ENABLE);

    // 2. 队列初始化（必须在main调用Serial_Init时执行）
    Serial_QueueInit(&uart2Queue);
}

/**
  * 函    数：串口发送一个字节
  * 参    数：Byte 要发送的一个字节
  * 返 回 值：无
  */
void Serial_SendByte(uint8_t Byte)
{
	USART_SendData(USART2, Byte);		//将字节数据写入数据寄存器，写入后USART自动生成时序波形
	while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);	//等待发送完成
	/*下次写入数据寄存器会自动清除发送完成标志位，故此循环后，无需清除标志位*/
}

/**
  * 函    数：串口发送一个数组
  * 参    数：Array 要发送数组的首地址
  * 参    数：Length 要发送数组的长度
  * 返 回 值：无
  */
void Serial_SendArray(uint8_t *Array, uint16_t Length)
{
	uint16_t i;
	for (i = 0; i < Length; i ++)		//遍历数组
	{
		Serial_SendByte(Array[i]);		//依次调用Serial_SendByte发送每个字节数据
	}
}
/**
  * @brief  拷贝MP3指令到发送缓存（接口函数，避免直接操作缓存）
  * @param  MP3_Index：MP3_Send数组的索引（如MP3_SHUFFLE_PLAY）
  */
// 4. 修正Serial_SetMP3Cmd：循环9次（FRAME_LENGTH=9）
void Serial_SetMP3Cmd(uint8_t MP3_Index) {
    __set_PRIMASK(1);  // 关闭中断保护
    // 循环FRAME_LENGTH次（9次），拷贝完整9字节指令
    for (uint8_t i = 0; i < FRAME_LENGTH; i++) {
        // 确保MP3_Send是9字节数组，且索引MP3_Index有效
        Serial_TxPacket[i] = MP3_Send[MP3_Index][i];
    }
    __set_PRIMASK(0);
}
/**
  * @brief  发送MP3指令（核心：中断保护+阻塞发送9字节）
  */
// 将MP3指令入队（替代原Serial_SendMP3Cmd的直接发送）
void Serial_SendMP3CmdToQueue(void) {
    // 将9字节指令入队
    Serial_Enqueue(&uart2Queue, Serial_TxPacket, FRAME_LENGTH);
}
// 处理队列：主循环中调用，按顺序发送队列中的数据
void Serial_ProcessQueue(void) {
    uint8_t txByte;
    
    // 若队列非空，且当前串口空闲（无发送中数据），则发送1字节
    while (!Serial_QueueIsEmpty(&uart2Queue)) {
        // 出队1字节
        if (Serial_Dequeue(&uart2Queue, &txByte, 1)) {
            // 发送前关闭低优先级中断（确保单字节发送不被打断）
            NVIC_DisableIRQ(USART3_IRQn);
            // 阻塞发送1字节（复用原Serial_SendByte逻辑）
            USART_SendData(USART2, txByte);
            while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
            // 恢复中断
            NVIC_EnableIRQ(USART3_IRQn);
        } else {
            break;  // 出队失败，退出循环
        }
    }
}


/**
  * 函    数：串口发送一个字符串
  * 参    数：String 要发送字符串的首地址
  * 返 回 值：无
  */
void Serial_SendString(char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i ++)//遍历字符数组（字符串），遇到字符串结束标志位后停止
	{
		Serial_SendByte(String[i]);		//依次调用Serial_SendByte发送每个字节数据
	}
}

/**
  * 函    数：次方函数（内部使用）
  * 返 回 值：返回值等于X的Y次方
  */
uint32_t Serial_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;	//设置结果初值为1
	while (Y --)			//执行Y次
	{
		Result *= X;		//将X累乘到结果
	}
	return Result;
}

/**
  * 函    数：串口发送数字
  * 参    数：Number 要发送的数字，范围：0~4294967295
  * 参    数：Length 要发送数字的长度，范围：0~10
  * 返 回 值：无
  */
void Serial_SendNumber(uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i ++)		//根据数字长度遍历数字的每一位
	{
		Serial_SendByte(Number / Serial_Pow(10, Length - i - 1) % 10 + '0');	//依次调用Serial_SendByte发送每位数字
	}
}

/**
  * 函    数：使用printf需要重定向的底层函数
  * 参    数：保持原始格式即可，无需变动
  * 返 回 值：保持原始格式即可，无需变动
  */
int fputc(int ch, FILE *f)
{
	Serial_SendByte(ch);			//将printf的底层重定向到自己的发送字节函数
	return ch;
}

/**
  * 函    数：自己封装的prinf函数
  * 参    数：format 格式化字符串
  * 参    数：... 可变的参数列表
  * 返 回 值：无
  */
void Serial_Printf(char *format, ...)
{
	char String[100];				//定义字符数组
	va_list arg;					//定义可变参数列表数据类型的变量arg
	va_start(arg, format);			//从format开始，接收参数列表到arg变量
	vsprintf(String, format, arg);	//使用vsprintf打印格式化字符串和参数列表到字符数组中
	va_end(arg);					//结束变量arg
	Serial_SendString(String);		//串口发送字符数组（字符串）
}

/**
  * 函    数：串口发送数据包
  * 参    数：无
  * 返 回 值：无
  * 说    明：调用此函数后，Serial_TxPacket数组的内容将加上包头（FF）包尾（FE）后，作为数据包发送出去
  */
void Serial_SendPacket(void)
{
//	Serial_SendByte(0xFF);
	Serial_SendArray(Serial_TxPacket, 8);
//	Serial_SendByte(0xFE);
}

/**
  * 函    数：获取串口接收数据包标志位
  * 参    数：无
  * 返 回 值：串口接收数据包标志位，范围：0~1，接收到数据包后，标志位置1，读取后标志位自动清零
  */
uint8_t Serial_GetRxFlag(void)
{
	if (Serial_RxFlag == 1)			//如果标志位为1
	{
		Serial_RxFlag = 0;
		return 1;					//则返回1，并自动清零标志位
	}
	return 0;						//如果标志位为0，则返回0
}

/**
  * 函    数：USART2中断函数
  * 参    数：无
  * 返 回 值：无
  * 注意事项：此函数为中断函数，无需调用，中断触发后自动执行
  *           函数名为预留的指定名称，可以从启动文件复制
  *           请确保函数名正确，不能有任何差异，否则中断函数将不能进入
  */
void USART2_IRQHandler(void)
{
	static uint8_t RxState = 0;		//定义表示当前状态机状态的静态变量
	static uint8_t pRxPacket = 0;	//定义表示当前接收数据位置的静态变量
	if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET)		//判断是否是USART2的接收事件触发的中断
	{
		uint8_t RxData = USART_ReceiveData(USART2);				//读取数据寄存器，存放在接收的数据变量
		
		/*使用状态机的思路，依次处理数据包的不同部分*/
		
		/*当前状态为0，接收数据包包头*/
		if (RxState == 0)
		{
			if (RxData == 0xFF)			//如果数据确实是包头
			{
				RxState = 1;			//置下一个状态
				pRxPacket = 0;			//数据包的位置归零
			}
		}
		/*当前状态为1，接收数据包数据*/
		else if (RxState == 1)
		{
			Serial_RxPacket[pRxPacket] = RxData;	//将数据存入数据包数组的指定位置
			pRxPacket ++;				//数据包的位置自增
			if (pRxPacket >= 4)			//如果收够4个数据
			{
				RxState = 2;			//置下一个状态
			}
		}
		/*当前状态为2，接收数据包包尾*/
		else if (RxState == 2)
		{
			if (RxData == 0xFE)			//如果数据确实是包尾部
			{
				RxState = 0;			//状态归0
				Serial_RxFlag = 1;		//接收数据包标志位置1，成功接收一个数据包
			}
		}
		
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);		//清除标志位
	}
}
