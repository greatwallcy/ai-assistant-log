#include "stm32f10x.h"                  // Device header
#include "Servo.h"
#include "Delay.h"
#include "BlueTooth.h"
#include "Serial.h"
// 1. 新增：MP3指令索引宏定义（与MP3_Send数组一一对应，避免错误）
#define MP3_SHUFFLE_PLAY       0   // 随机播放
#define MP3_ONE_CYCLE_STOP     1   // 单曲循环结束
#define MP3_ONE_CYCLE_START    2   // 单曲循环开始
#define MP3_TYPE_CYCLE_START   3   // 类型循环开始
#define MP3_TYPE_CYCLE_STOP    4   // 类型循环结束
#define MP3_NEXT_SONG          5   // 下一曲
#define MP3_LAST_SONG          6   // 上一曲
#define MP3_ADD_VOLUME         7   // 音量加
#define MP3_SUB_VOLUME         8   // 音量减
#define MP3_STOP_PLAY          9   // 停止播放
#define MP3_START_PLAY         10  // 播放
#define MP3_PAUSE_PLAY         11  // 暂停
#define MP3_ALL_CYCLE_START    12  // 全部循环开始
#define MP3_ALL_CYCLE_STOP     13  // 全部循环结束
#define MP3_PLAY_MUSIC         54  // 普通音乐播放
#define MP3_PLAY_ART           55  // 戏曲播放
#define MP3_PLAY_CHILDREN      56  // 儿歌播放
#define MP3_VOLUME_SET_80      57   // 音量设置为80
#define MP3_VOLUME_SET_60      58   // 音量设置为60
#define MP3_VOLUME_SET_40      59   // 音量设置为40
#define MP3_VOLUME_SET_20      60   // 音量设置为20

// 通用函数：设置MP3音量为40%，并入队发送
// 设置MP3音量为12(30级中的40%%)，规格书规定最大30级
void MP3_Volume40(void) {
    Serial_SetMP3Cmd(MP3_VOLUME_SET_40);
    Serial_SendMP3CmdToQueue();
}

const uint8_t MP3_Send[][8] =
{
	0x7E,0xFF,0x06,0x18,0x00,0x00,0x00,0xEF,//随机播放--30
	0x7E,0xFF,0x06,0x19,0x00,0x00,0x01,0xEF,//单曲循环结束--31
	0x7E,0xFF,0x06,0x19,0x00,0x00,0x00,0xEF,//单曲循环开始--32
	0x7E,0xFF,0x06,0x11,0x00,0x00,0x01,0xEF,//循环播放开始--33
	0x7E,0xFF,0x06,0x11,0x00,0x00,0x00,0xEF,//循环播放结束--34
	0x7E,0xFF,0x06,0x01,0x00,0x00,0x00,0xEF,//下一曲--35
	0x7E,0xFF,0x06,0x02,0x00,0x00,0x00,0xEF,//上一曲--36
	0x7E,0xFF,0x06,0x04,0x00,0x00,0x00,0xEF,//音量加--37
	0x7E,0xFF,0x06,0x05,0x00,0x00,0x00,0xEF,//音量减--38
	0x7E,0xFF,0x06,0x16,0x00,0x00,0x00,0xEF,//停止播放--39
	0x7E,0xFF,0x06,0x0D,0x00,0x00,0x00,0xEF,//播放--40
	0x7E,0xFF,0x06,0x0E,0x00,0x00,0x00,0xEF,//暂停--41
	0x7E,0xFF,0x06,0x11,0x00,0x00,0x01,0xEF,//循环所有目录开始--42
	0x7E,0xFF,0x06,0x11,0x00,0x00,0x00,0xEF,//循环所有目录结束--43---13
	0x7E,0xFF,0x06,0x0F,0x00,0x01,0x01,0xEF,//指定播放音乐--可能--50
	0x7E,0xFF,0x06,0x0F,0x00,0x01,0x02,0xEF,//指定播放音乐--宁夏--51
	0x7E,0xFF,0x06,0x0F,0x00,0x01,0x03,0xEF,//指定播放音乐--成都--52
	0x7E,0xFF,0x06,0x0F,0x00,0x01,0x04,0xEF,//指定播放音乐--沙漠骆驼--53
	0x7E,0xFF,0x06,0x0F,0x00,0x01,0x05,0xEF,//指定播放音乐--平凡之路--54
	0x7E,0xFF,0x06,0x0F,0x00,0x01,0x06,0xEF,//指定播放音乐--这世界那么多人--55
	0x7E,0xFF,0x06,0x0F,0x00,0x01,0x07,0xEF,//指定播放音乐--知否知否--56
	0x7E,0xFF,0x06,0x0F,0x00,0x01,0x08,0xEF,//指定播放音乐--我和我的祖国--57
	0x7E,0xFF,0x06,0x0F,0x00,0x01,0x09,0xEF,//指定播放音乐--悟空--58
	0x7E,0xFF,0x06,0x0F,0x00,0x01,0x10,0xEF,//指定播放音乐--富士山下--59
	0x7E,0xFF,0x06,0x0F,0x00,0x02,0x01,0xEF,//指定播放戏曲--贵妃醉酒--60
	0x7E,0xFF,0x06,0x0F,0x00,0x02,0x02,0xEF,//指定播放戏曲--武家坡--61
	0x7E,0xFF,0x06,0x0F,0x00,0x02,0x03,0xEF,//指定播放戏曲--卜算子.咏梅--62
	0x7E,0xFF,0x06,0x0F,0x00,0x02,0x04,0xEF,//指定播放戏曲--四郎探母--63
	0x7E,0xFF,0x06,0x0F,0x00,0x02,0x05,0xEF,//指定播放戏曲--秦琼观阵--64
	0x7E,0xFF,0x06,0x0F,0x00,0x02,0x06,0xEF,//指定播放戏曲--太平歌词白蛇传--65
	0x7E,0xFF,0x06,0x0F,0x00,0x02,0x07,0xEF,//指定播放戏曲--相思赋予谁--66
	0x7E,0xFF,0x06,0x0F,0x00,0x02,0x08,0xEF,//指定播放戏曲--梨花颂--67
	0x7E,0xFF,0x06,0x0F,0x00,0x02,0x09,0xEF,//指定播放戏曲--穆桂英挂帅--68
	0x7E,0xFF,0x06,0x0F,0x00,0x02,0x10,0xEF,//指定播放戏曲--探清水河--69
	0x7E,0xFF,0x06,0x0F,0x00,0x03,0x01,0xEF,//指定播放动物--牛--70
	0x7E,0xFF,0x06,0x0F,0x00,0x03,0x02,0xEF,//指定播放动物--狗--71
	0x7E,0xFF,0x06,0x0F,0x00,0x03,0x03,0xEF,//指定播放动物--驴--72
	0x7E,0xFF,0x06,0x0F,0x00,0x03,0x04,0xEF,//指定播放动物--鸟--73
	0x7E,0xFF,0x06,0x0F,0x00,0x03,0x05,0xEF,//指定播放动物--公鸡--74
	0x7E,0xFF,0x06,0x0F,0x00,0x03,0x06,0xEF,//指定播放动物--马--75
	0x7E,0xFF,0x06,0x0F,0x00,0x03,0x07,0xEF,//指定播放动物--蛙--76
	0x7E,0xFF,0x06,0x0F,0x00,0x03,0x08,0xEF,//指定播放动物--鸭--77
	0x7E,0xFF,0x06,0x0F,0x00,0x03,0x09,0xEF,//指定播放动物--猪--78
	0x7E,0xFF,0x06,0x0F,0x00,0x03,0x10,0xEF,//指定播放动物--大象--79
	0x7E,0xFF,0x06,0x0F,0x00,0x04,0x01,0xEF,//指定播放儿歌--两只老虎--80
	0x7E,0xFF,0x06,0x0F,0x00,0x04,0x02,0xEF,//指定播放儿歌--数鸭子--81
	0x7E,0xFF,0x06,0x0F,0x00,0x04,0x03,0xEF,//指定播放儿歌--蜗牛与黄莺鸟--82
	0x7E,0xFF,0x06,0x0F,0x00,0x04,0x04,0xEF,//指定播放儿歌--春天在哪里--83
	0x7E,0xFF,0x06,0x0F,0x00,0x04,0x05,0xEF,//指定播放儿歌--小燕子穿花衣--84
	0x7E,0xFF,0x06,0x0F,0x00,0x04,0x06,0xEF,//指定播放儿歌--世上只有妈妈好--85
	0x7E,0xFF,0x06,0x0F,0x00,0x04,0x07,0xEF,//指定播放儿歌--找朋友--86
	0x7E,0xFF,0x06,0x0F,0x00,0x04,0x08,0xEF,//指定播放儿歌--小老鼠上灯台--87
	0x7E,0xFF,0x06,0x0F,0x00,0x04,0x09,0xEF,//指定播放儿歌--小毛驴--88
	0x7E,0xFF,0x06,0x0F,0x00,0x04,0x10,0xEF,//指定播放儿歌--葫芦娃--89
	0x7E,0xFF,0x06,0x0F,0x00,0x01,0x01,0xEF,//普通音乐播放
	0x7E,0xFF,0x06,0x0F,0x00,0x02,0x01,0xEF,//戏曲播放
	0x7E,0xFF,0x06,0x0F,0x00,0x03,0x01,0xEF,//儿歌播放
	0x7E,0xFF,0x06,0x06,0x00,0x00,0x19,0xEF,//音量设置为25(最大30级)
	0x7E,0xFF,0x06,0x06,0x00,0x00,0x06,0xEF,//音量设置为6
	0x7E,0xFF,0x06,0x06,0x00,0x00,0x0C,0xEF,//音量设置为12(30级中的40%%)
	0x7E,0xFF,0x06,0x06,0x00,0x00,0x06,0xEF,//音量设置为6
	
};
	/*舵机位置
	    1        2
	
	

	
	    3        4
	*/
	
	//舵机充当腿，括号里的值为0时表示的是腿向前进的方向甩，90度是站立

#define Chongfunumber 2  //动作重复次数、前进后退左转右转
#define SwingRepeatnumber 3  //摇摆重复次数
#define HelloRepeatnumber 4  //打招呼重复次数

uint16_t PAnumbers=Chongfunumber;//动作重复次数

uint16_t TiaoTurn=0;
uint16_t TiaoTurn2=0;
#define fa_ci 1 
//uint16_t ci_fuzhi=fa_ci;

void Action_relaxed_getdowm(void)
{
	Servo_Angle1(20);
	Servo_Angle2(20);
	Delay_ms(80);
	Servo_Angle3(160);
	Servo_Angle4(160);
}

void Action_upright(void)//站立
{
	Servo_Angle1(90);
	Servo_Angle2(90);
	Delay_ms(80);
	Servo_Angle3(90);
	Servo_Angle4(90);
	
	if(WeiBa==1)
	{
		Action_Mode=9;
	}
}

void Action_upright2(void)//站立
{
	Servo_Angle3(90);
	Servo_Angle4(90);
	Delay_ms(80);
	Servo_Angle1(90);
	Servo_Angle2(90);
	
	if(WeiBa==1)
	{
		Action_Mode=9;
	}
}

void Action_getdowm(void)//趴下
{
	Servo_Angle1(20);
	Servo_Angle2(20);
	Delay_ms(80);
	Servo_Angle3(20);
	Servo_Angle4(20);
	
	if(WeiBa==1)
	{
		Action_Mode=9;
	}
}

void Action_sit(void)//坐下
{
	Servo_Angle1(90);
	Servo_Angle2(90);
	Delay_ms(80);
	Servo_Angle3(20);
	Servo_Angle4(20);
	
	if(WeiBa==1)
	{
		Action_Mode=9;
	}
}



void Action_advance(void)//前进
{

	while(Action_Mode==4)
	{
		PAnumbers=Chongfunumber;
			while((PAnumbers || Sustainedmove)&& Action_Mode==4)
			{
				Servo_Angle2(45);	
				Servo_Angle3(45);
				Delay_ms(SpeedDelay);
				if(Action_Mode!=4)break;
				Servo_Angle1(135);	
				Servo_Angle4(135);
				Delay_ms(SpeedDelay);
				if(Action_Mode!=4)break;
				Servo_Angle2(90);	
				Servo_Angle3(90);
				Delay_ms(SpeedDelay);
				if(Action_Mode!=4)break;
				Servo_Angle1(90);	
				Servo_Angle4(90);
				Delay_ms(SpeedDelay);
				if(Action_Mode!=4)break;
		
				Servo_Angle1(45);	
				Servo_Angle4(45);
				Delay_ms(SpeedDelay);
				if(Action_Mode!=4)break;
				Servo_Angle2(135);	
				Servo_Angle3(135);
				Delay_ms(SpeedDelay);
				if(Action_Mode!=4)break;
				Servo_Angle1(90);	
				Servo_Angle4(90);
				Delay_ms(SpeedDelay);
				if(Action_Mode!=4)break;
				Servo_Angle2(90);	
				Servo_Angle3(90);
				Delay_ms(SpeedDelay);
				if(Action_Mode!=4)break;
				
				PAnumbers--;
			}
			if(Sustainedmove!=1 && Action_Mode==4)
				Action_Mode=2;
	}
}

void Action_back(void)//后退
{
	while(Action_Mode==5)
	{
		PAnumbers=Chongfunumber;
		while((PAnumbers || Sustainedmove) && Action_Mode==5 )
		{
			Servo_Angle2(135);	
			Servo_Angle3(135);
			Delay_ms(SpeedDelay);
			if(Action_Mode!=5)break;
			Servo_Angle1(45);	
			Servo_Angle4(45);
			Delay_ms(SpeedDelay);
			if(Action_Mode!=5)break;
			Servo_Angle2(90);	
			Servo_Angle3(90);
			Delay_ms(SpeedDelay);
			if(Action_Mode!=5)break;
			Servo_Angle1(90);	
			Servo_Angle4(90);
			Delay_ms(SpeedDelay);
			if(Action_Mode!=5)break;
			
			Servo_Angle1(135);	
			Servo_Angle4(135);
			Delay_ms(SpeedDelay);
			if(Action_Mode!=5)break;
			Servo_Angle2(45);	
			Servo_Angle3(45);
			Delay_ms(SpeedDelay);
			if(Action_Mode!=5)break;
			Servo_Angle1(90);	
			Servo_Angle4(90);
			Delay_ms(SpeedDelay);
			if(Action_Mode!=5)break;
			Servo_Angle2(90);	
			Servo_Angle3(90);
			Delay_ms(SpeedDelay);
			if(Action_Mode!=5)break;
			
			PAnumbers--;
		}
		if(Sustainedmove!=1 && Action_Mode==5)
		Action_Mode=2;

		
	}
}

void Action_Lrotation(void)//向左旋转
{
	while(Action_Mode==6)
	{
		PAnumbers=Chongfunumber;
		PAnumbers=PAnumbers+Chongfunumber;
		while((PAnumbers || Sustainedmove) && Action_Mode==6)
		{
			Servo_Angle2(45);
			Servo_Angle3(135);
			Delay_ms(SpeedDelay);
			if(Action_Mode!=6)break;
			Servo_Angle1(45);
			Servo_Angle4(135);
			Delay_ms(SpeedDelay);
			if(Action_Mode!=6)break;
			Servo_Angle2(90);
			Servo_Angle3(90);	
			Delay_ms(SpeedDelay);
			if(Action_Mode!=6)break;
			Servo_Angle1(90);
			Servo_Angle4(90);	
			Delay_ms(SpeedDelay);
			if(Action_Mode!=6)break;
			
			PAnumbers--;
		}
		if(Sustainedmove!=1 && Action_Mode==6)
		Action_Mode=2;
	}

}


void Action_Rrotation(void)//向右旋转
{
	while(Action_Mode==7)
	{
		PAnumbers=Chongfunumber;
		PAnumbers=PAnumbers+Chongfunumber;
		while((PAnumbers || Sustainedmove)  && Action_Mode==7)
		{
			Servo_Angle1(45);
			Servo_Angle4(135);
			Delay_ms(SpeedDelay);
			if(Action_Mode!=7)break;
			Servo_Angle2(45);
			Servo_Angle3(135);
			Delay_ms(SpeedDelay);
			if(Action_Mode!=7)break;
			Servo_Angle1(90);
			Servo_Angle4(90);	
			Delay_ms(SpeedDelay);
			if(Action_Mode!=7)break;
			Servo_Angle2(90);
			Servo_Angle3(90);	
			Delay_ms(SpeedDelay);
			if(Action_Mode!=7)break;
			
			PAnumbers--;
		}
		if(Sustainedmove!=1 && Action_Mode==7)
		Action_Mode=2;
	}

}

void Action_Swing(void)//摇摆
{
	uint16_t SwingNumber=SwingRepeatnumber;
	while(SwingNumber && Action_Mode==8)
	{
		for(uint8_t i=30;i<150;i++)
		{
			Servo_Angle1(i);
			Servo_Angle2(i);
			Servo_Angle3(i);
			Servo_Angle4(i);
			Delay_ms(SwingDelay);
			if(Action_Mode!=8)break;
		}
		if(Action_Mode!=8)break;
		for(uint8_t i=150;i>30;i--)
		{
			Servo_Angle1(i);
			Servo_Angle2(i);
			Servo_Angle3(i);
			Servo_Angle4(i);
			Delay_ms(SwingDelay);
			if(Action_Mode!=8)break;
		}
		if(Action_Mode!=8)break;
		
		SwingNumber--;
	}
		for(uint8_t i=30;i<90;i++)
		{
			Servo_Angle1(i);
			Servo_Angle2(i);
			Servo_Angle3(i);
			Servo_Angle4(i);
			Delay_ms(SwingDelay);
			if(Action_Mode!=8)break;
		}
		if(Action_Mode==8)
		Action_Mode=2;
}


void Action_SwingTail(void)//摇尾巴
{
	uint16_t SwingTailNumber=3;
	Delay_ms(60);
	while(SwingTailNumber && Action_Mode==9)
	{
		for(uint8_t i=30;i<150;i++)
		{
			WServo_Angle(i);
			Delay_ms(SwingDelay);
			if(Action_Mode!=9)break;
		}
		if(Action_Mode!=9)break;
		for(uint8_t i=150;i>30;i--)
		{
			WServo_Angle(i);
			Delay_ms(SwingDelay);
			if(Action_Mode!=9)break;
		}
		if(Action_Mode!=9)break;
		SwingTailNumber--;
	}
	Delay_ms(60);
}

void Action_JumpU(void)//向前跳
{
	if(TiaoTurn==0)
	{
		Servo_Angle1(140);
		Servo_Angle4(35);
		Delay_ms(SpeedDelay);
		
		Servo_Angle2(140);
		Servo_Angle3(35);
		Delay_ms(SpeedDelay+80);
		
		Action_Mode=2;
		TiaoTurn=1;
	}
	else
	{
		Servo_Angle2(140);
		Servo_Angle3(35);
		Delay_ms(SpeedDelay);
		
		Servo_Angle1(140);
		Servo_Angle4(35);
		Delay_ms(SpeedDelay+80);
		
		Action_Mode=2;
		TiaoTurn=0;
	}
}

void Action_JumpD(void)//向后跳
{
	if(TiaoTurn2==0){
		Servo_Angle4(35);
		Servo_Angle1(140);
		Delay_ms(SpeedDelay);
	
		Servo_Angle3(35);
		Servo_Angle2(140);
		Delay_ms(SpeedDelay);
	
		Action_Mode=12;
		TiaoTurn2=1;
	}
	else
	{
		Servo_Angle3(35);
		Servo_Angle2(140);
		Delay_ms(SpeedDelay);
	
		Servo_Angle4(35);
		Servo_Angle1(140);
		Delay_ms(SpeedDelay);
	
		Action_Mode=12;
		TiaoTurn2=0;
	}
}

void Action_Hello(void)
{
	uint16_t HelloNumber=HelloRepeatnumber;
	
	Servo_Angle3(20);
	Servo_Angle4(45);
	Delay_ms(80);
	Servo_Angle1(90);
	while(HelloNumber && Action_Mode==13)
	{
		if(Action_Mode!=13)break;
		for(int i=0;i<=45;i++)
		{
			if(Action_Mode!=13)break;
			Servo_Angle2(i);
			Delay_ms(SwingDelay);
		}
		for(int i=45;i>0;i--)
		{
			if(Action_Mode!=13)break;
			Servo_Angle2(i);
			Delay_ms(SwingDelay);
		}
		if(Action_Mode!=13)break;
		
		HelloNumber--;
	}
	if(Action_Mode==13)
	Action_Mode=2;
}

void Action_stretch(void)//伸懒腰
{
	Servo_Angle3(90);
	Servo_Angle4(90);
	Delay_ms(80);
	for(int i=90;i>10;i--)
	{
		Servo_Angle1(i);
		Servo_Angle2(i);
		if(Action_Mode!=14)break;
		Delay_ms(15);
	}
	for(int i=10;i<90;i++)
	{
		Servo_Angle1(i);
		Servo_Angle2(i);
		if(Action_Mode!=14)break;
		Delay_ms(15);
	}
	for(int i=90;i<170;i++)
	{
		Servo_Angle3(i);
		Servo_Angle4(i);
		if(Action_Mode!=14)break;
		Delay_ms(15);
	}
	
	for(int i=170;i>90;i--)
	{
		Servo_Angle3(i);
		Servo_Angle4(i);
		if(Action_Mode!=14)break;
		Delay_ms(15);
	}
	if(Action_Mode==14)
	Action_Mode=15;
}

void Action_Lstretch(void)//后腿拉伸
{
	int breakvalue=1;
	int temp=3;
	while(breakvalue)
	{
		Servo_Angle1(90);
		Servo_Angle2(20);
		Delay_ms(60);
		Servo_Angle4(110);
		for(int i=90;i<180;i++)
		{
			if(Action_Mode!=15)break;
			Servo_Angle3(i);
			Delay_ms(6);
		}
		while(temp && Action_Mode==15)
		{
			for(int i=180;i>150;i--)
			{
				if(Action_Mode!=15)break;
				Servo_Angle3(i);
				Delay_ms(15);
			}
			temp--;
		}
		if(Action_Mode!=15)break;
		Delay_ms(100);
		Servo_Angle1(90);
		Servo_Angle2(90);
		if(Action_Mode!=15)break;
		Delay_ms(80);
		Servo_Angle3(90);
		Servo_Angle4(90);
		Delay_ms(100);
		if(Action_Mode!=15)break;
		
		temp=3;
		
		Servo_Angle2(90);
		Servo_Angle1(20);
		if(Action_Mode!=15)break;
		Delay_ms(60);
		Servo_Angle3(110);
		for(int i=90;i<180;i++)
		{
			if(Action_Mode!=15)break;
			Servo_Angle4(i);
			Delay_ms(6);
		}
		while(temp && Action_Mode==15)
		{
			for(int i=180;i>150;i--)
			{
				if(Action_Mode!=15)break;
				Servo_Angle4(i);
				Delay_ms(15);
			}
			temp--;
		}
		if(Action_Mode==15)
		Action_Mode=2;
		
		
		breakvalue=0;
	}
}

void Action_dance(void)//跳舞
{
uint16_t i=1;
	while(i)
	{
		i--;
	uint16_t SwingDelay=2;
		//左右摆
		for(uint8_t i=90;i>30;i--)
		{
			uint8_t j;
			j=180-i;
			Servo_Angle1(i);//起始j=90，起始i=90
			Servo_Angle2(j);//结束j=150,i=30
			Servo_Angle3(i);
			Servo_Angle4(j);
			if(Action_Mode!=16)break;
			Delay_ms(SwingDelay);
		}
		for(uint8_t i=30;i<90;i++)
		{
			uint8_t j;
			j=180-i;
			Servo_Angle1(i);//起始j=150,i=30
			Servo_Angle2(j);//结束j=90,i=90
			Servo_Angle3(i);
			Servo_Angle4(j);
			if(Action_Mode!=16)break;
			Delay_ms(SwingDelay);

		}
	//前后摆
		for(uint8_t i=90;i<150;i++)
		{
			Servo_Angle1(i);//起始j=90，起始i=90
			Servo_Angle2(i);//结束j=150,i=150
			Servo_Angle3(i);
			Servo_Angle4(i);
			if(Action_Mode!=16)break;
			Delay_ms(SwingDelay);
		}
			for(uint8_t i=150;i>90;i--)
		{
			Servo_Angle1(i);//起始j=150，起始i=150
			Servo_Angle2(i);//结束j=90,i=90
			Servo_Angle3(i);
			Servo_Angle4(i);
			if(Action_Mode!=16)break;
			Delay_ms(SwingDelay);

		}
		//左右摆
			for(uint8_t i=90;i<150;i++)
		{
			uint8_t j;
			j=180-i;
			Servo_Angle1(i);//起始j=90,i=90
			Servo_Angle2(j);//结束j=30,i=150
			Servo_Angle3(i);
			Servo_Angle4(j);
			if(Action_Mode!=16)break;
			Delay_ms(SwingDelay);

		}
			for(uint8_t i=150;i>90;i--)
		{
			uint8_t j;
			j=180-i;
			Servo_Angle1(i);//起始j=30,i=150
			Servo_Angle2(j);//结束j=90,i=90
			Servo_Angle3(i);
			Servo_Angle4(j);
			if(Action_Mode!=16)break;
			Delay_ms(SwingDelay);

		}
		//前后摆
			for(uint8_t i=90;i>30;i--)
		{
			Servo_Angle1(i);//起始j=90，起始i=90
			Servo_Angle2(i);//结束j=30,i=30
			Servo_Angle3(i);
			Servo_Angle4(i);
			if(Action_Mode!=16)break;
			Delay_ms(SwingDelay);
		}
			for(uint8_t i=30;i<90;i++)
		{
			Servo_Angle1(i);//起始j=30，起始i=30
			Servo_Angle2(i);//结束j=90,i=90
			Servo_Angle3(i);
			Servo_Angle4(i);
			if(Action_Mode!=16)break;
			Delay_ms(SwingDelay);

		}
	}
}

void Action_left_right_Lstretch(void)//左右摇摆
{
	uint16_t i=3;
	while(i>0)
	{
		i--;
	uint16_t SwingDelay=10;


		for(uint8_t i=30;i<150;i++)
		{
			uint8_t j;
			j=150-i;
			Servo_Angle1(j);
			Servo_Angle2(i);
			Servo_Angle3(j);
			Servo_Angle4(i);
			if(Action_Mode!=17)break;
			Delay_ms(SwingDelay);
		}
		for(uint8_t i=150;i>30;i--)
		{
			uint8_t j;
			j=150-i;
			Servo_Angle1(j);
			Servo_Angle2(i);
			Servo_Angle3(j);
			Servo_Angle4(i);
			if(Action_Mode!=17)break;
			Delay_ms(SwingDelay);

		}
	}
}

void Action_push_up(void)//做俯卧撑
{
	uint16_t i=5;
	while(i>0)
	{
		i--;
	Servo_Angle1(90);
	Servo_Angle2(90);
	Servo_Angle3(90);
	Servo_Angle4(90);
		if(Action_Mode!=18)break;
	Delay_ms(80);
	for(int i=90;i>10;i--)
	{
		int j;
		j=180-i;
		Servo_Angle1(i);
		Servo_Angle2(i);
		Servo_Angle3(j);
		Servo_Angle4(j);
		if(Action_Mode!=18)break;
		Delay_ms(15);
	}
	for(int i=10;i<90;i++)
	{
		int j;
		j=180-i;
		Servo_Angle1(i);
		Servo_Angle2(i);
		Servo_Angle3(j);
		Servo_Angle4(j);
		if(Action_Mode!=18)break;
		Delay_ms(15);
	}
		}
}

void Action_stretch_hand(void)//伸手
{
				Servo_Angle2(10);
}
void Action_lay_down(void)//放下
{
  			Servo_Angle2(90);
}

void music_shuffle_play(void)//随机播放（Action_Mode=21）
{
    static uint16_t ci_fuzhi = fa_ci; 
    // 重置标志：新指令到来时，允许再次发送
    if(USART1_RxFlag == 1)
    {
        ci_fuzhi = fa_ci;
        USART1_RxFlag = 0;
    }
    // 仅当Action_Mode匹配且允许发送时，执行一次发送
    if(Action_Mode == 21 && ci_fuzhi == 1)
    {
        MP3_Volume40();                        // 先设置音量40%
				Serial_SetMP3Cmd(MP3_SHUFFLE_PLAY);
        Serial_SendMP3CmdToQueue();           // ★修复：入队发送  // 拷贝下一曲指令
        ci_fuzhi = 0;         // 禁止重复发送
        Action_Mode = 2;      // 发送完成后重置Action_Mode
    }
}
void music_one_cycle_stop(void)//单曲循环--结束
{
	static uint16_t ci_fuzhi = fa_ci; 
    if(USART1_RxFlag == 1)
    {
        ci_fuzhi = fa_ci;
        USART1_RxFlag = 0;
    }
    if(Action_Mode == 22 && ci_fuzhi == 1)
    {
				Serial_SetMP3Cmd(MP3_ONE_CYCLE_STOP);
        Serial_SendMP3CmdToQueue();           // ★修复：入队发送  
   //     Serial_SendMP3Cmd();              // 中断保护发送
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}
void music_one_cycle_start(void)//单曲循环--开始
{
	static uint16_t ci_fuzhi = fa_ci; 
    if(USART1_RxFlag == 1)
    {
        ci_fuzhi = fa_ci;
        USART1_RxFlag = 0;
    }
    if(Action_Mode == 23 && ci_fuzhi == 1)
    {
        Serial_SetMP3Cmd(MP3_ONE_CYCLE_START);
        Serial_SendMP3CmdToQueue();           // ★修复：入队发送  
  //      Serial_SendMP3Cmd();  
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}

void music_type_cycle_start(void)//类型循环开始（Action_Mode=24）
{
    static uint16_t ci_fuzhi = fa_ci; 
    if(USART1_RxFlag == 1)
    {
        ci_fuzhi = fa_ci;
        USART1_RxFlag = 0;
    }
    if(Action_Mode == 24 && ci_fuzhi == 1)
    {
				Serial_SetMP3Cmd(MP3_TYPE_CYCLE_START);
        Serial_SendMP3CmdToQueue();           // ★修复：入队发送  
  //      Serial_SendMP3Cmd();  
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}

void music_type_cycle_stop(void)//类型循环结束（Action_Mode=25）
{
    static uint16_t ci_fuzhi = fa_ci; 
    if(USART1_RxFlag == 1)
    {
        ci_fuzhi = fa_ci;
        USART1_RxFlag = 0;
    }
    if(Action_Mode == 25 && ci_fuzhi == 1)
    {
        Serial_SetMP3Cmd(MP3_TYPE_CYCLE_STOP);
        Serial_SendMP3CmdToQueue();           // ★修复：入队发送  
  //      Serial_SendMP3Cmd();  
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}
void music_next_song(void)//下一曲
{
	 static uint16_t ci_fuzhi = fa_ci; 
    if(USART1_RxFlag == 1)
    {
        ci_fuzhi = fa_ci;
        USART1_RxFlag = 0;
    }
    if(Action_Mode == 26 && ci_fuzhi == 1)
    {
				Serial_SetMP3Cmd(MP3_NEXT_SONG);
        Serial_SendMP3CmdToQueue();           // ★修复：入队发送  
  //      Serial_SendMP3Cmd();  
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}
void music_last_song(void)//上一曲
{
	static uint16_t ci_fuzhi = fa_ci; 
    if(USART1_RxFlag == 1)
    {
        ci_fuzhi = fa_ci;
        USART1_RxFlag = 0;
    }
    if(Action_Mode == 27 && ci_fuzhi == 1)
    {
				Serial_SetMP3Cmd(MP3_LAST_SONG);
        Serial_SendMP3CmdToQueue();           // ★修复：入队发送  
  //      Serial_SendMP3Cmd();
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}
void music_volume_set_80(void)//音量设置为80
{
	static uint16_t ci_fuzhi = fa_ci; 
    if(USART1_RxFlag == 1)
    {
        ci_fuzhi = fa_ci;
        USART1_RxFlag = 0;
    }
    if(Action_Mode == 28 && ci_fuzhi == 1)
    {
				Serial_SetMP3Cmd(MP3_VOLUME_SET_80);
        Serial_SendMP3CmdToQueue();           // ★修复：入队发送  
 //       Serial_SendMP3Cmd();
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}
void music_volume_set_60(void)//音量设置为60
{
	static uint16_t ci_fuzhi = fa_ci; 
    if(USART1_RxFlag == 1)
    {
        ci_fuzhi = fa_ci;
        USART1_RxFlag = 0;
    }
    if(Action_Mode == 29 && ci_fuzhi == 1)
    {
				Serial_SetMP3Cmd(MP3_VOLUME_SET_60);  
        Serial_SendMP3CmdToQueue();           // ★修复：入队发送
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}
void music_stop_play(void)//停止播放
{
	static uint16_t ci_fuzhi = fa_ci; 
    if(USART1_RxFlag == 1)
    {
        ci_fuzhi = fa_ci;
        USART1_RxFlag = 0;
    }
    if(Action_Mode == 30 && ci_fuzhi == 1)
    {
        Serial_SetMP3Cmd(MP3_STOP_PLAY);
        Serial_SendMP3CmdToQueue();           // ★修复：入队发送  
  //      Serial_SendMP3Cmd();
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}
void music_start_play(void)//播放
{
	static uint16_t ci_fuzhi = fa_ci; 
    if(USART1_RxFlag == 1)
    {
        ci_fuzhi = fa_ci;
        USART1_RxFlag = 0;
    }
    if(Action_Mode == 31 && ci_fuzhi == 1)
    {
        MP3_Volume40();                        // 先设置音量40%
        Serial_SetMP3Cmd(MP3_START_PLAY);
        Serial_SendMP3CmdToQueue();           // ★修复：入队发送  // 拷贝播放指令
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}
void music_pause_play(void)//暂停
{
	static uint16_t ci_fuzhi = fa_ci; 
    if(USART1_RxFlag == 1)
    {
        ci_fuzhi = fa_ci;
        USART1_RxFlag = 0;
    }
    if(Action_Mode == 32 && ci_fuzhi == 1)
    {
				Serial_SetMP3Cmd(MP3_PAUSE_PLAY);
        Serial_SendMP3CmdToQueue();           // ★修复：入队发送  // 拷贝暂停指令
  //      Serial_SendMP3Cmd();              // 中断保护发送
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}
void music_all_cycle_start(void)//全部循环--开始
{
	static uint16_t ci_fuzhi = fa_ci; 
    if(USART1_RxFlag == 1)
    {
        ci_fuzhi = fa_ci;
        USART1_RxFlag = 0;
    }
    if(Action_Mode == 33 && ci_fuzhi == 1)
    {
        Serial_SetMP3Cmd(MP3_ALL_CYCLE_START);
        Serial_SendMP3CmdToQueue();           // ★修复：入队发送  
  //     Serial_SendMP3Cmd();
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}
void music_all_cycle_stop(void)//全部循环--结束
{
	static uint16_t ci_fuzhi = fa_ci; 
    if(USART1_RxFlag == 1)
    {
        ci_fuzhi = fa_ci;
        USART1_RxFlag = 0;
    }
    if(Action_Mode == 34 && ci_fuzhi == 1)
    {
        Serial_SetMP3Cmd(MP3_ALL_CYCLE_STOP);
        Serial_SendMP3CmdToQueue();           // ★修复：入队发送  
  //      Serial_SendMP3Cmd();
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}
void music_play_music(void)//普通音乐播放
{
	static uint16_t ci_fuzhi = fa_ci; 
    if(USART1_RxFlag == 1)
    {
        ci_fuzhi = fa_ci;
        USART1_RxFlag = 0;
    }
    if(Action_Mode == 35 && ci_fuzhi == 1)
    {
        MP3_Volume40();                        // 先设置音量40%
        Serial_SetMP3Cmd(MP3_PLAY_MUSIC);
        Serial_SendMP3CmdToQueue();           // ★修复：入队发送  
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}
void music_play_art(void)//戏曲播放
{
	static uint16_t ci_fuzhi = fa_ci; 
    if(USART1_RxFlag == 1)
    {
        ci_fuzhi = fa_ci;
        USART1_RxFlag = 0;
    }
    if(Action_Mode == 36 && ci_fuzhi == 1)
    {
        MP3_Volume40();                        // 先设置音量40%
        Serial_SetMP3Cmd(MP3_PLAY_ART);
        Serial_SendMP3CmdToQueue();           // ★修复：入队发送  
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}
// 儿歌播放函数（其他音乐函数同理修改）
void music_play_childern(void) {
    static uint16_t ci_fuzhi = fa_ci; 
    if (USART1_RxFlag == 1) {
        ci_fuzhi = fa_ci;
        USART1_RxFlag = 0;
    }
    if (Action_Mode == 37 && ci_fuzhi == 1) {
        MP3_Volume40();                        // 先设置音量40%
        Serial_SetMP3Cmd(MP3_PLAY_CHILDREN);  // 拷贝指令到缓存
        Serial_SendMP3CmdToQueue();           // 指令入队（核心修改）
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}
void music_volume_set_40(void)//音量设置为40
{
	static uint16_t ci_fuzhi = fa_ci; 
    if(USART1_RxFlag == 1)
    {
        ci_fuzhi = fa_ci;
        USART1_RxFlag = 0;
    }
    if(Action_Mode == 38 && ci_fuzhi == 1)
    {
				Serial_SetMP3Cmd(MP3_VOLUME_SET_40);
        Serial_SendMP3CmdToQueue();           // ★修复：入队发送  
  //      Serial_SendMP3Cmd();
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}
void music_volume_set_20(void)//音量设置为20
{
	static uint16_t ci_fuzhi = fa_ci; 
    if(USART1_RxFlag == 1)
    {
        ci_fuzhi = fa_ci;
        USART1_RxFlag = 0;
    }
    if(Action_Mode == 39 && ci_fuzhi == 1)
    {
				Serial_SetMP3Cmd(MP3_VOLUME_SET_20);  
        Serial_SendMP3CmdToQueue();           // ★修复：入队发送
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}
