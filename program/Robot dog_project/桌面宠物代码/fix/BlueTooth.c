#include "stm32f10x.h"                  // Device header
#include "PWM.h"
#include "PetAction.h"
#include "Face_Config.h"
#include "Serial.h"

uint16_t AllLed=1;  //开启灯光
uint16_t BreatheLed=0;//开启呼吸灯
uint16_t Sustainedmove=0;//持续运动


uint16_t Action_Mode=0;
uint16_t SpeedDelay=200;
uint16_t SwingDelay=6;
uint16_t Face_Mode=0;
uint8_t WeiBa=0;
uint8_t USART1_RxFlag = 0;  // 接收完成标志
uint8_t USART1_RxData;

// 新增：OLED更新标志（中断置位，主循环执行）
volatile uint8_t OLED_NeedUpdate = 0;
volatile uint8_t Pending_Face_Mode = 0;
volatile uint16_t Pending_Action_Mode = 0;

void BlueTooth_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//开启GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);//开启串口时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//开启GPIOB时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);//开启串口时钟
	//语音
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;//复用推挽输出模式
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_9;//默认PA9是USART1_TX的复用
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;//复用上拉输入模式
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_10;//默认PA9是USART1_RX的复用
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	//蓝牙
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;//复用推挽输出模式
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_10;//默认PB10是USART3_TX的复用
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;//复用上拉输入模式
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_11;//默认PB11是USART3_RX的复用
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	USART_InitTypeDef UASRT_InitStructure;//USART初始化
	UASRT_InitStructure.USART_BaudRate=9600;//波特率9600
	UASRT_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;//不需要硬件流控制
	UASRT_InitStructure.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;//接受与发送均打开
	UASRT_InitStructure.USART_Parity=USART_Parity_No;//不需要奇偶校验
	UASRT_InitStructure.USART_StopBits=USART_StopBits_1;//停止位为1
	UASRT_InitStructure.USART_WordLength=USART_WordLength_8b;//字长8位
	USART_Init(USART1,&UASRT_InitStructure);
	USART_Init(USART3,&UASRT_InitStructure);
	
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);//语音接收中断配置，也就是如果接送到消息就直接中断
	USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);//蓝牙接收中断配置，也就是如果接送到消息就直接中断
	
	//中断
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//分组2
	//语音中断配置
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel=USART1_IRQn;//特定的通道
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;//通道使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;//抢占式优先级为1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;//响应优先级为1
	NVIC_Init(&NVIC_InitStructure);
	//蓝牙中断配置
	NVIC_InitStructure.NVIC_IRQChannel=USART3_IRQn;//特定的通道
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;//通道使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;//抢占式优先级为2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;//响应优先级为1
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(USART1,ENABLE);//USART1使能打开
	USART_Cmd(USART3,ENABLE);//USART3使能打开
}

/**
 * @brief 统一的指令处理函数（中断和主循环共用）
 * @param cmd: 接收到的指令字节
 */
static void Process_Voice_Command(uint8_t cmd)
{
	Sustainedmove=0;//持续运动关闭
	
	// 先设置Face_Mode和Action_Mode的待定值
	uint8_t need_update = 1;
	
	switch(cmd)
	{
		case 0x29: Pending_Face_Mode=0; Pending_Action_Mode=0; break;  //放松的趴下
		case 0x30: Pending_Face_Mode=1; Pending_Action_Mode=1; break;  //蹲下
		case 0x31: Pending_Face_Mode=5; Pending_Action_Mode=2; break;  //直立
		case 0x32: Pending_Face_Mode=1; Pending_Action_Mode=3; break;  //趴下
		case 0x33: Pending_Face_Mode=2; Pending_Action_Mode=4; break;  //前进
		case 0x34: Pending_Face_Mode=2; Pending_Action_Mode=5; break;  //后退
		case 0x35: Pending_Face_Mode=2; Pending_Action_Mode=6; break;  //左转
		case 0x36: Pending_Face_Mode=2; Pending_Action_Mode=7; break;  //右转
		case 0x37: Pending_Face_Mode=4; Pending_Action_Mode=8; break;  //摇摆
		case 0x38: //减少移动延迟，增加移动速度
		{
			if(SpeedDelay==120){Pending_Face_Mode=3;}
			if(SpeedDelay>100) SpeedDelay-=20;
			else { Pending_Face_Mode=2; SpeedDelay=200; }
			need_update = 0;
			break;
		}
		case 0x39: //减少摇摆延迟，增加摇摆速度
		{
			if(SwingDelay==4){Pending_Face_Mode=3;}
			if(SwingDelay>3) SwingDelay--;
			else { Pending_Face_Mode=4; SwingDelay=9; }
			need_update = 0;
			break;
		}
		case 0x40: //摇尾巴
		{
			(WeiBa==0)?(WeiBa=1):(WeiBa=0);
			Pending_Face_Mode=1; Pending_Action_Mode=9;
			break;
		}
		case 0x41: Pending_Face_Mode=2; Pending_Action_Mode=10; break; //向前跳
		case 0x42: Pending_Face_Mode=2; Pending_Action_Mode=11; break; //向后跳
		case 0x43: Pending_Face_Mode=6; Pending_Action_Mode=13; break; //打招呼
		case 0x44: AllLed=1; need_update=0; break;  //开启灯光
		case 0x45: AllLed=0; need_update=0; break;  //关闭灯光
		case 0x46: BreatheLed=1; need_update=0; break; //开启呼吸灯
		case 0x47: BreatheLed=0; need_update=0; break; //关闭呼吸灯
		case 0x48: Pending_Face_Mode=6; Pending_Action_Mode=14; break; //伸懒腰
		case 0x49: Pending_Face_Mode=6; Pending_Action_Mode=15; break; //拉伸腿
		case 0x50: Pending_Face_Mode=6; Pending_Action_Mode=16; break; //跳舞
		case 0x51: Pending_Face_Mode=6; Pending_Action_Mode=17; break; //左右摇摆
		case 0x52: Pending_Face_Mode=6; Pending_Action_Mode=18; break; //俯卧撑
		case 0x53: Pending_Face_Mode=6; Pending_Action_Mode=19; break; //伸手
		case 0x54: Pending_Face_Mode=6; Pending_Action_Mode=20; break; //放下
		case 0x60: Pending_Face_Mode=6; Pending_Action_Mode=21; break; //随机播放
		case 0x61: Pending_Face_Mode=6; Pending_Action_Mode=22; break; //单曲循环结束
		case 0x62: Pending_Face_Mode=6; Pending_Action_Mode=23; break; //单曲循环开始
		case 0x63: Pending_Face_Mode=6; Pending_Action_Mode=24; break; //循环播放开始
		case 0x64: Pending_Face_Mode=6; Pending_Action_Mode=25; break; //循环播放结束
		case 0x65: Pending_Face_Mode=6; Pending_Action_Mode=26; break; //下一曲
		case 0x66: Pending_Face_Mode=6; Pending_Action_Mode=27; break; //上一曲
		case 0x67: Pending_Face_Mode=6; Pending_Action_Mode=28; break; //音量加
		case 0x68: Pending_Face_Mode=6; Pending_Action_Mode=29; break; //音量减
		case 0x69: Pending_Face_Mode=6; Pending_Action_Mode=30; break; //停止播放
		case 0x70: Pending_Face_Mode=6; Pending_Action_Mode=31; break; //播放
		case 0x71: Pending_Face_Mode=6; Pending_Action_Mode=32; break; //暂停
		case 0x72: Pending_Face_Mode=6; Pending_Action_Mode=33; break; //循环所有目录开始
		case 0x73: Pending_Face_Mode=6; Pending_Action_Mode=34; break; //循环所有目录结束
		case 0x74: Pending_Face_Mode=6; Pending_Action_Mode=35; break; //普通音乐播放
		case 0x75: Pending_Face_Mode=6; Pending_Action_Mode=36; break; //戏曲播放
		case 0x76: Pending_Face_Mode=6; Pending_Action_Mode=37; break; //儿歌播放
		case 0x77: Pending_Face_Mode=6; Pending_Action_Mode=38; break; //音量设置为40
		case 0x78: Pending_Face_Mode=6; Pending_Action_Mode=39; break; //音量设置为20
		default: need_update=0; break; //未知指令，忽略
	}
	
	if(need_update)
	{
		OLED_NeedUpdate = 1;  //通知主循环更新OLED
	}
}

/**
 * @brief 主循环中调用，处理中断中积攒的指令
 */
void BlueTooth_ProcessPending(void)
{
	if(OLED_NeedUpdate)
	{
		OLED_NeedUpdate = 0;
		Face_Mode = Pending_Face_Mode;
		Action_Mode = Pending_Action_Mode;
		Face_Config();  //在主循环中安全地更新OLED
	}
}

/**
 * @brief 蓝牙处理待定指令（USART3中断用，只设Action，不操作OLED）
 */
static void Process_BT_Command(uint8_t cmd)
{
	Sustainedmove=1;//持续运动开启
	
	switch(cmd)
	{
		case 0x29: Pending_Face_Mode=0; Pending_Action_Mode=0; OLED_NeedUpdate=1; break;
		case 0x30: Pending_Face_Mode=1; Pending_Action_Mode=1; OLED_NeedUpdate=1; break;
		case 0x31: Pending_Face_Mode=5; Pending_Action_Mode=2; OLED_NeedUpdate=1; break;
		case 0x32: Pending_Face_Mode=1; Pending_Action_Mode=3; OLED_NeedUpdate=1; break;
		case 0x33: Pending_Face_Mode=2; Pending_Action_Mode=4; OLED_NeedUpdate=1; break;
		case 0x34: Pending_Face_Mode=2; Pending_Action_Mode=5; OLED_NeedUpdate=1; break;
		case 0x35: Pending_Face_Mode=2; Pending_Action_Mode=6; OLED_NeedUpdate=1; break;
		case 0x36: Pending_Face_Mode=2; Pending_Action_Mode=7; OLED_NeedUpdate=1; break;
		case 0x37: Pending_Face_Mode=4; Pending_Action_Mode=8; OLED_NeedUpdate=1; break;
		case 0x38:
		{
			if(SpeedDelay==120){Pending_Face_Mode=3; OLED_NeedUpdate=1;}
			if(SpeedDelay>100) SpeedDelay-=20;
			else { Pending_Face_Mode=2; SpeedDelay=200; OLED_NeedUpdate=1; }
			break;
		}
		case 0x39:
		{
			if(SwingDelay==4){Pending_Face_Mode=3; OLED_NeedUpdate=1;}
			if(SwingDelay>3) SwingDelay--;
			else { Pending_Face_Mode=4; SwingDelay=9; OLED_NeedUpdate=1; }
			break;
		}
		case 0x40:
		{
			(WeiBa==0)?(WeiBa=1):(WeiBa=0);
			Pending_Face_Mode=1; Pending_Action_Mode=9; OLED_NeedUpdate=1;
			break;
		}
		case 0x41: Pending_Face_Mode=2; Pending_Action_Mode=10; OLED_NeedUpdate=1; break;
		case 0x42: Pending_Face_Mode=2; Pending_Action_Mode=11; OLED_NeedUpdate=1; break;
		case 0x43: Pending_Face_Mode=6; Pending_Action_Mode=13; OLED_NeedUpdate=1; break;
		case 0x44: AllLed=1; break;
		case 0x45: AllLed=0; break;
		case 0x46: BreatheLed=1; break;
		case 0x47: BreatheLed=0; break;
		case 0x48: Pending_Face_Mode=6; Pending_Action_Mode=14; OLED_NeedUpdate=1; break;
		case 0x49: Pending_Face_Mode=6; Pending_Action_Mode=15; OLED_NeedUpdate=1; break;
		default: break;
	}
}

/**
 * ★★★ 修复1：USART1中断 — 数据只读一次，不在中断中操作OLED ★★★
 */
void USART1_IRQHandler(void)
{
	if(USART_GetITStatus(USART1,USART_IT_RXNE)==SET)
	{
		uint8_t rxData = USART_ReceiveData(USART1);  // ★只读一次，存到局部变量
		USART1_RxData = rxData;
		USART1_RxFlag = 1;
		
		Process_Voice_Command(rxData);  // ★用局部变量处理，不再重复读DR
		
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);
	}
}

/**
 * ★★★ 修复2：USART3中断（蓝牙）— 同样修复 ★★★
 */
void USART3_IRQHandler(void)
{
	if(USART_GetITStatus(USART3,USART_IT_RXNE)==SET)
	{
		uint8_t rxData = USART_ReceiveData(USART3);  // ★只读一次
		
		Process_BT_Command(rxData);
		
		USART_ClearITPendingBit(USART3,USART_IT_RXNE);
	}
}
