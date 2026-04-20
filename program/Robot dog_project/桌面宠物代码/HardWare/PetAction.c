/**
  ******************************************************************************
  * @文件    PetAction.c
  * @说明    桌面宠物动作控制 v2 - 全面优化版
  *
  *  优化内容：
  *  1. 所有长循环内喂狗(IWDG_ReloadCounter)，防止看门狗复位
  *  2. 所有动作结束时自动设置 Action_Mode=2（站立）
  *  3. 跳舞重写：动作编排更丰富，时长合理，可随时中断
  *  4. 伸手/放下：改为带循环+退出机制
  *  5. 伸懒腰/拉伸：缩短等待时间，加快节奏
  *  6. 俯卧撑：增加喂狗，减少次数
  *  7. 左右摇摆：增加自动退出
  ******************************************************************************
  */
#include "stm32f10x.h"                  // Device header
#include "Servo.h"
#include "Delay.h"
#include "BlueTooth.h"
#include "Serial.h"

/* ==================== MP3 指令定义（保持不变）==================== */
#define MP3_SHUFFLE_PLAY       0
#define MP3_ONE_CYCLE_STOP     1
#define MP3_ONE_CYCLE_START    2
#define MP3_TYPE_CYCLE_START   3
#define MP3_TYPE_CYCLE_STOP    4
#define MP3_NEXT_SONG          5
#define MP3_LAST_SONG          6
#define MP3_ADD_VOLUME         7
#define MP3_SUB_VOLUME         8
#define MP3_STOP_PLAY          9
#define MP3_START_PLAY         10
#define MP3_PAUSE_PLAY         11
#define MP3_ALL_CYCLE_START    12
#define MP3_ALL_CYCLE_STOP     13
#define MP3_PLAY_MUSIC         54
#define MP3_PLAY_ART           55
#define MP3_PLAY_CHILDREN      56
#define MP3_VOLUME_SET_80      57
#define MP3_VOLUME_SET_60      58
#define MP3_VOLUME_SET_40      59
#define MP3_VOLUME_SET_20      60

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

/* ==================== 配置宏 ==================== */
#define Chongfunumber    2   // 动作重复次数（前进后退左转右转）
#define SwingRepeatnumber 3  // 摇摆重复次数
#define HelloRepeatnumber 4  // 打招呼重复次数
#define DanceRepeatnumber 2  // 跳舞重复次数 ★新增

uint16_t PAnumbers = Chongfunumber;
uint16_t TiaoTurn  = 0;
uint16_t TiaoTurn2 = 0;
#define fa_ci 1

/* ============================================================
 *  喂狗辅助宏 — 每个长循环内调用，防止看门狗超时复位
 * ============================================================ */
#define FEED_DOG()  IWDG_ReloadCounter()

/* ============================================================
 *  1. 放松趴下（Action_Mode=0）★优化：加快过渡
 * ============================================================ */
void Action_relaxed_getdowm(void)
{
	Servo_Angle1(20);
	Servo_Angle2(20);
	Delay_ms(60);
	Servo_Angle3(160);
	Servo_Angle4(160);
	Action_Mode = 2;  // ★修复：完成后自动回站立
}

/* ============================================================
 *  2. 坐下（Action_Mode=1）
 * ============================================================ */
void Action_sit(void)
{
	Servo_Angle1(90);
	Servo_Angle2(90);
	Delay_ms(60);
	Servo_Angle3(20);
	Servo_Angle4(20);
	if(WeiBa == 1) Action_Mode = 9;
	else Action_Mode = 2;  // ★添加退出
}

/* ============================================================
 *  3. 站立（Action_Mode=2）
 * ============================================================ */
void Action_upright(void)
{
	Servo_Angle1(90);
	Servo_Angle2(90);
	Delay_ms(60);
	Servo_Angle3(90);
	Servo_Angle4(90);
	if(WeiBa == 1) Action_Mode = 9;
}

void Action_upright2(void)
{
	Servo_Angle3(90);
	Servo_Angle4(90);
	Delay_ms(60);
	Servo_Angle1(90);
	Servo_Angle2(90);
	if(WeiBa == 1) Action_Mode = 9;
}

/* ============================================================
 *  4. 趴下（Action_Mode=3）
 * ============================================================ */
void Action_getdowm(void)
{
	Servo_Angle1(20);
	Servo_Angle2(20);
	Delay_ms(60);
	Servo_Angle3(20);
	Servo_Angle4(20);
	if(WeiBa == 1) Action_Mode = 9;
	else Action_Mode = 2;  // ★添加退出
}

/* ============================================================
 *  5. 前进（Action_Mode=4）★优化：增加喂狗
 * ============================================================ */
void Action_advance(void)
{
	while(Action_Mode == 4)
	{
		PAnumbers = Chongfunumber;
		while((PAnumbers || Sustainedmove) && Action_Mode == 4)
		{
			Servo_Angle2(45);	Servo_Angle3(45);
			Delay_ms(SpeedDelay); FEED_DOG();
			if(Action_Mode != 4) break;
			Servo_Angle1(135);	Servo_Angle4(135);
			Delay_ms(SpeedDelay); FEED_DOG();
			if(Action_Mode != 4) break;
			Servo_Angle2(90);	Servo_Angle3(90);
			Delay_ms(SpeedDelay); FEED_DOG();
			if(Action_Mode != 4) break;
			Servo_Angle1(90);	Servo_Angle4(90);
			Delay_ms(SpeedDelay); FEED_DOG();
			if(Action_Mode != 4) break;

			Servo_Angle1(45);	Servo_Angle4(45);
			Delay_ms(SpeedDelay); FEED_DOG();
			if(Action_Mode != 4) break;
			Servo_Angle2(135);	Servo_Angle3(135);
			Delay_ms(SpeedDelay); FEED_DOG();
			if(Action_Mode != 4) break;
			Servo_Angle1(90);	Servo_Angle4(90);
			Delay_ms(SpeedDelay); FEED_DOG();
			if(Action_Mode != 4) break;
			Servo_Angle2(90);	Servo_Angle3(90);
			Delay_ms(SpeedDelay); FEED_DOG();
			if(Action_Mode != 4) break;

			PAnumbers--;
		}
		if(Sustainedmove != 1 && Action_Mode == 4)
			Action_Mode = 2;
	}
}

/* ============================================================
 *  6. 后退（Action_Mode=5）★优化：增加喂狗
 * ============================================================ */
void Action_back(void)
{
	while(Action_Mode == 5)
	{
		PAnumbers = Chongfunumber;
		while((PAnumbers || Sustainedmove) && Action_Mode == 5)
		{
			Servo_Angle2(135);	Servo_Angle3(135);
			Delay_ms(SpeedDelay); FEED_DOG();
			if(Action_Mode != 5) break;
			Servo_Angle1(45);	Servo_Angle4(45);
			Delay_ms(SpeedDelay); FEED_DOG();
			if(Action_Mode != 5) break;
			Servo_Angle2(90);	Servo_Angle3(90);
			Delay_ms(SpeedDelay); FEED_DOG();
			if(Action_Mode != 5) break;
			Servo_Angle1(90);	Servo_Angle4(90);
			Delay_ms(SpeedDelay); FEED_DOG();
			if(Action_Mode != 5) break;

			Servo_Angle1(135);	Servo_Angle4(135);
			Delay_ms(SpeedDelay); FEED_DOG();
			if(Action_Mode != 5) break;
			Servo_Angle2(45);	Servo_Angle3(45);
			Delay_ms(SpeedDelay); FEED_DOG();
			if(Action_Mode != 5) break;
			Servo_Angle1(90);	Servo_Angle4(90);
			Delay_ms(SpeedDelay); FEED_DOG();
			if(Action_Mode != 5) break;
			Servo_Angle2(90);	Servo_Angle3(90);
			Delay_ms(SpeedDelay); FEED_DOG();
			if(Action_Mode != 5) break;

			PAnumbers--;
		}
		if(Sustainedmove != 1 && Action_Mode == 5)
			Action_Mode = 2;
	}
}

/* ============================================================
 *  7. 左转（Action_Mode=6）★优化：增加喂狗
 * ============================================================ */
void Action_Lrotation(void)
{
	while(Action_Mode == 6)
	{
		PAnumbers = Chongfunumber + Chongfunumber;
		while((PAnumbers || Sustainedmove) && Action_Mode == 6)
		{
			Servo_Angle2(45);  Servo_Angle3(135);
			Delay_ms(SpeedDelay); FEED_DOG();
			if(Action_Mode != 6) break;
			Servo_Angle1(45);  Servo_Angle4(135);
			Delay_ms(SpeedDelay); FEED_DOG();
			if(Action_Mode != 6) break;
			Servo_Angle2(90);  Servo_Angle3(90);
			Delay_ms(SpeedDelay); FEED_DOG();
			if(Action_Mode != 6) break;
			Servo_Angle1(90);  Servo_Angle4(90);
			Delay_ms(SpeedDelay); FEED_DOG();
			if(Action_Mode != 6) break;

			PAnumbers--;
		}
		if(Sustainedmove != 1 && Action_Mode == 6)
			Action_Mode = 2;
	}
}

/* ============================================================
 *  8. 右转（Action_Mode=7）★优化：增加喂狗
 * ============================================================ */
void Action_Rrotation(void)
{
	while(Action_Mode == 7)
	{
		PAnumbers = Chongfunumber + Chongfunumber;
		while((PAnumbers || Sustainedmove) && Action_Mode == 7)
		{
			Servo_Angle1(45);  Servo_Angle4(135);
			Delay_ms(SpeedDelay); FEED_DOG();
			if(Action_Mode != 7) break;
			Servo_Angle2(45);  Servo_Angle3(135);
			Delay_ms(SpeedDelay); FEED_DOG();
			if(Action_Mode != 7) break;
			Servo_Angle1(90);  Servo_Angle4(90);
			Delay_ms(SpeedDelay); FEED_DOG();
			if(Action_Mode != 7) break;
			Servo_Angle2(90);  Servo_Angle3(90);
			Delay_ms(SpeedDelay); FEED_DOG();
			if(Action_Mode != 7) break;

			PAnumbers--;
		}
		if(Sustainedmove != 1 && Action_Mode == 7)
			Action_Mode = 2;
	}
}

/* ============================================================
 *  9. 摇摆（Action_Mode=8）★优化：喂狗+加速回位
 * ============================================================ */
void Action_Swing(void)
{
	uint16_t SwingNumber = SwingRepeatnumber;
	while(SwingNumber && Action_Mode == 8)
	{
		for(uint8_t i = 30; i < 150; i++)
		{
			Servo_Angle1(i); Servo_Angle2(i);
			Servo_Angle3(i); Servo_Angle4(i);
			Delay_ms(SwingDelay);
			FEED_DOG();
			if(Action_Mode != 8) break;
		}
		if(Action_Mode != 8) break;
		for(uint8_t i = 150; i > 30; i--)
		{
			Servo_Angle1(i); Servo_Angle2(i);
			Servo_Angle3(i); Servo_Angle4(i);
			Delay_ms(SwingDelay);
			FEED_DOG();
			if(Action_Mode != 8) break;
		}
		if(Action_Mode != 8) break;
		SwingNumber--;
	}
	// 回位
	for(uint8_t i = 30; i <= 90; i++)
	{
		Servo_Angle1(i); Servo_Angle2(i);
		Servo_Angle3(i); Servo_Angle4(i);
		Delay_ms(SwingDelay);
	}
	if(Action_Mode == 8)
		Action_Mode = 2;
}

/* ============================================================
 *  10. 摇尾巴（Action_Mode=9）★优化：喂狗+自动退出
 * ============================================================ */
void Action_SwingTail(void)
{
	uint16_t SwingTailNumber = 3;
	Delay_ms(40);
	while(SwingTailNumber && Action_Mode == 9)
	{
		for(uint8_t i = 30; i < 150; i++)
		{
			WServo_Angle(i);
			Delay_ms(SwingDelay);
			FEED_DOG();
			if(Action_Mode != 9) break;
		}
		if(Action_Mode != 9) break;
		for(uint8_t i = 150; i > 30; i--)
		{
			WServo_Angle(i);
			Delay_ms(SwingDelay);
			FEED_DOG();
			if(Action_Mode != 9) break;
		}
		if(Action_Mode != 9) break;
		SwingTailNumber--;
	}
	WServo_Angle(90);  // 尾巴回中
	if(Action_Mode == 9)
		Action_Mode = 2;  // ★修复：自动退出
}

/* ============================================================
 *  11. 向前跳（Action_Mode=10）
 * ============================================================ */
void Action_JumpU(void)
{
	if(TiaoTurn == 0)
	{
		Servo_Angle1(140); Servo_Angle4(35);
		Delay_ms(SpeedDelay); FEED_DOG();
		Servo_Angle2(140); Servo_Angle3(35);
		Delay_ms(SpeedDelay + 80); FEED_DOG();
		Action_Mode = 2;
		TiaoTurn = 1;
	}
	else
	{
		Servo_Angle2(140); Servo_Angle3(35);
		Delay_ms(SpeedDelay); FEED_DOG();
		Servo_Angle1(140); Servo_Angle4(35);
		Delay_ms(SpeedDelay + 80); FEED_DOG();
		Action_Mode = 2;
		TiaoTurn = 0;
	}
}

/* ============================================================
 *  12. 向后跳（Action_Mode=11）
 * ============================================================ */
void Action_JumpD(void)
{
	if(TiaoTurn2 == 0)
	{
		Servo_Angle4(35); Servo_Angle1(140);
		Delay_ms(SpeedDelay); FEED_DOG();
		Servo_Angle3(35); Servo_Angle2(140);
		Delay_ms(SpeedDelay); FEED_DOG();
		Action_Mode = 12;
		TiaoTurn2 = 1;
	}
	else
	{
		Servo_Angle3(35); Servo_Angle2(140);
		Delay_ms(SpeedDelay); FEED_DOG();
		Servo_Angle4(35); Servo_Angle1(140);
		Delay_ms(SpeedDelay); FEED_DOG();
		Action_Mode = 12;
		TiaoTurn2 = 0;
	}
}

/* ============================================================
 *  13. 打招呼（Action_Mode=13）★优化：增加喂狗
 * ============================================================ */
void Action_Hello(void)
{
	uint16_t HelloNumber = HelloRepeatnumber;
	Servo_Angle3(20);
	Servo_Angle4(45);
	Delay_ms(60);
	Servo_Angle1(90);
	while(HelloNumber && Action_Mode == 13)
	{
		for(int i = 0; i <= 45; i++)
		{
			if(Action_Mode != 13) break;
			Servo_Angle2(i);
			Delay_ms(SwingDelay);
			FEED_DOG();
		}
		if(Action_Mode != 13) break;
		for(int i = 45; i >= 0; i--)
		{
			if(Action_Mode != 13) break;
			Servo_Angle2(i);
			Delay_ms(SwingDelay);
			FEED_DOG();
		}
		if(Action_Mode != 13) break;
		HelloNumber--;
	}
	if(Action_Mode == 13)
		Action_Mode = 2;
}

/* ============================================================
 *  14. 伸懒腰（Action_Mode=14）★优化：缩短延迟+喂狗
 * ============================================================ */
void Action_stretch(void)
{
	Servo_Angle3(90); Servo_Angle4(90);
	Delay_ms(50);

	// 前腿下压
	for(int i = 90; i > 15; i--)
	{
		Servo_Angle1(i); Servo_Angle2(i);
		if(Action_Mode != 14) { Action_Mode = 2; return; }
		Delay_ms(10); FEED_DOG();
	}
	// ★喂狗
	FEED_DOG();

	// 前腿回位
	for(int i = 15; i < 90; i++)
	{
		Servo_Angle1(i); Servo_Angle2(i);
		if(Action_Mode != 14) { Action_Mode = 2; return; }
		Delay_ms(10); FEED_DOG();
	}
	FEED_DOG();

	// 后腿下压
	for(int i = 90; i < 165; i++)
	{
		Servo_Angle3(i); Servo_Angle4(i);
		if(Action_Mode != 14) { Action_Mode = 2; return; }
		Delay_ms(10); FEED_DOG();
	}
	FEED_DOG();

	// 后腿回位
	for(int i = 165; i > 90; i--)
	{
		Servo_Angle3(i); Servo_Angle4(i);
		if(Action_Mode != 14) { Action_Mode = 2; return; }
		Delay_ms(10); FEED_DOG();
	}

	Action_Mode = 15;  // 转到拉伸腿
}

/* ============================================================
 *  15. 后腿拉伸（Action_Mode=15）★优化：缩短延迟+喂狗+自动退出
 * ============================================================ */
void Action_Lstretch(void)
{
	int temp = 3;

	// 左后腿拉伸
	Servo_Angle1(90); Servo_Angle2(20);
	Delay_ms(40);
	Servo_Angle4(110);
	for(int i = 90; i < 175; i++)
	{
		if(Action_Mode != 15) { Action_Mode = 2; return; }
		Servo_Angle3(i);
		Delay_ms(5); FEED_DOG();
	}
	FEED_DOG();

	// 抖动
	while(temp && Action_Mode == 15)
	{
		for(int i = 175; i > 155; i--)
		{
			if(Action_Mode != 15) break;
			Servo_Angle3(i);
			Delay_ms(12); FEED_DOG();
		}
		temp--;
	}
	if(Action_Mode != 15) { Action_Mode = 2; return; }

	// 回位
	Delay_ms(60);
	Servo_Angle1(90); Servo_Angle2(90);
	Delay_ms(40); FEED_DOG();
	Servo_Angle3(90); Servo_Angle4(90);
	Delay_ms(60); FEED_DOG();

	// 右后腿拉伸
	temp = 3;
	Servo_Angle2(90); Servo_Angle1(20);
	Delay_ms(40);
	Servo_Angle3(110);
	for(int i = 90; i < 175; i++)
	{
		if(Action_Mode != 15) { Action_Mode = 2; return; }
		Servo_Angle4(i);
		Delay_ms(5); FEED_DOG();
	}
	FEED_DOG();

	// 抖动
	while(temp && Action_Mode == 15)
	{
		for(int i = 175; i > 155; i--)
		{
			if(Action_Mode != 15) break;
			Servo_Angle4(i);
			Delay_ms(12); FEED_DOG();
		}
		temp--;
	}

	// 回位并退出
	Servo_Angle1(90); Servo_Angle2(90);
	Delay_ms(40); FEED_DOG();
	Servo_Angle3(90); Servo_Angle4(90);

	Action_Mode = 2;  // ★修复：自动退出
}

/* ============================================================
 *  16. 跳舞（Action_Mode=16）★★★ 完全重写 ★★★
 *
 *  原版问题：
 *    - 缺少 Action_Mode=2 自动退出，卡住后出不来
 *    - 动作单一（所有舵机同时同角度移动）
 *    - 外层循环只跑1次，节奏不明显
 *
 *  优化版：
 *    - 编排4个阶段的动作（摇摆→前后晃→扭身→节奏摆）
 *    - 每阶段有节奏感的停顿
 *    - 重复2轮后自动退出
 *    - 每步检查 Action_Mode 可随时中断
 *    - 全程喂狗
 * ============================================================ */
void Action_dance(void)
{
	uint16_t round;
	uint16_t step_delay = 3;  // ★不再用局部 SwingDelay，用独立变量

	for(round = 0; round < DanceRepeatnumber; round++)
	{
		if(Action_Mode != 16) break;

		/* === 第1段：左右对称摇摆（60°→120°→90°）=== */
		for(uint8_t i = 60; i < 120; i++)
		{
			Servo_Angle1(i);    Servo_Angle2(150 - i);
			Servo_Angle3(i);    Servo_Angle4(150 - i);
			Delay_ms(step_delay); FEED_DOG();
			if(Action_Mode != 16) goto dance_end;
		}
		// ★节奏停顿
		Delay_ms(80); FEED_DOG();

		for(uint8_t i = 120; i > 60; i--)
		{
			Servo_Angle1(i);    Servo_Angle2(150 - i);
			Servo_Angle3(i);    Servo_Angle4(150 - i);
			Delay_ms(step_delay); FEED_DOG();
			if(Action_Mode != 16) goto dance_end;
		}
		Delay_ms(80); FEED_DOG();

		/* === 第2段：前后一起晃（90°→40°→90°）=== */
		for(uint8_t i = 90; i > 40; i--)
		{
			Servo_Angle1(i); Servo_Angle2(i);
			Servo_Angle3(i); Servo_Angle4(i);
			Delay_ms(step_delay); FEED_DOG();
			if(Action_Mode != 16) goto dance_end;
		}
		Delay_ms(60); FEED_DOG();

		for(uint8_t i = 40; i < 90; i++)
		{
			Servo_Angle1(i); Servo_Angle2(i);
			Servo_Angle3(i); Servo_Angle4(i);
			Delay_ms(step_delay); FEED_DOG();
			if(Action_Mode != 16) goto dance_end;
		}
		Delay_ms(80); FEED_DOG();

		/* === 第3段：扭身（前腿反向+后腿反向）=== */
		for(uint8_t i = 90; i < 130; i++)
		{
			Servo_Angle1(i);        Servo_Angle2(180 - i);
			Servo_Angle3(180 - i);  Servo_Angle4(i);
			Delay_ms(step_delay); FEED_DOG();
			if(Action_Mode != 16) goto dance_end;
		}
		Delay_ms(60); FEED_DOG();

		for(uint8_t i = 130; i > 50; i--)
		{
			Servo_Angle1(i);        Servo_Angle2(180 - i);
			Servo_Angle3(180 - i);  Servo_Angle4(i);
			Delay_ms(step_delay); FEED_DOG();
			if(Action_Mode != 16) goto dance_end;
		}
		Delay_ms(60); FEED_DOG();

		for(uint8_t i = 50; i <= 90; i++)
		{
			Servo_Angle1(i);        Servo_Angle2(180 - i);
			Servo_Angle3(180 - i);  Servo_Angle4(i);
			Delay_ms(step_delay); FEED_DOG();
			if(Action_Mode != 16) goto dance_end;
		}
		Delay_ms(100); FEED_DOG();

		/* === 第4段：快速节奏摆（小幅高频）=== */
		for(uint8_t rep = 0; rep < 3; rep++)
		{
			for(uint8_t i = 80; i < 100; i++)
			{
				Servo_Angle1(i); Servo_Angle2(i);
				Servo_Angle3(i); Servo_Angle4(i);
				Delay_ms(2); FEED_DOG();
				if(Action_Mode != 16) goto dance_end;
			}
			for(uint8_t i = 100; i > 80; i--)
			{
				Servo_Angle1(i); Servo_Angle2(i);
				Servo_Angle3(i); Servo_Angle4(i);
				Delay_ms(2); FEED_DOG();
				if(Action_Mode != 16) goto dance_end;
			}
		}
		Delay_ms(100); FEED_DOG();
	}

dance_end:
	// 回到站立
	Servo_Angle1(90); Servo_Angle2(90);
	Servo_Angle3(90); Servo_Angle4(90);
	Action_Mode = 2;  // ★★★ 关键修复：自动退出 ★★★
}

/* ============================================================
 *  17. 左右摇摆（Action_Mode=17）★修复：增加自动退出+喂狗
 * ============================================================ */
void Action_left_right_Lstretch(void)
{
	uint16_t rounds = 3;

	while(rounds > 0 && Action_Mode == 17)
	{
		for(uint8_t i = 30; i < 150; i++)
		{
			uint8_t j = 150 - i;
			Servo_Angle1(j); Servo_Angle2(i);
			Servo_Angle3(j); Servo_Angle4(i);
			Delay_ms(10);
			FEED_DOG();
			if(Action_Mode != 17) goto lrs_end;
		}
		for(uint8_t i = 150; i > 30; i--)
		{
			uint8_t j = 150 - i;
			Servo_Angle1(j); Servo_Angle2(i);
			Servo_Angle3(j); Servo_Angle4(i);
			Delay_ms(10);
			FEED_DOG();
			if(Action_Mode != 17) goto lrs_end;
		}
		rounds--;
	}

lrs_end:
	Servo_Angle1(90); Servo_Angle2(90);
	Servo_Angle3(90); Servo_Angle4(90);
	Action_Mode = 2;  // ★修复：自动退出
}

/* ============================================================
 *  18. 俯卧撑（Action_Mode=18）★优化：减少次数+喂狗+自动退出
 * ============================================================ */
void Action_push_up(void)
{
	uint16_t i = 3;  // ★从5次减少到3次

	while(i > 0 && Action_Mode == 18)
	{
		i--;
		Servo_Angle1(90); Servo_Angle2(90);
		Servo_Angle3(90); Servo_Angle4(90);
		Delay_ms(60); FEED_DOG();

		// 下压
		for(int j = 90; j > 15; j--)
		{
			Servo_Angle1(j); Servo_Angle2(j);
			Servo_Angle3(180 - j); Servo_Angle4(180 - j);
			Delay_ms(12); FEED_DOG();
			if(Action_Mode != 18) goto pu_end;
		}
		// 起身
		for(int j = 15; j < 90; j++)
		{
			Servo_Angle1(j); Servo_Angle2(j);
			Servo_Angle3(180 - j); Servo_Angle4(180 - j);
			Delay_ms(12); FEED_DOG();
			if(Action_Mode != 18) goto pu_end;
		}
		FEED_DOG();
	}

pu_end:
	Servo_Angle1(90); Servo_Angle2(90);
	Servo_Angle3(90); Servo_Angle4(90);
	Action_Mode = 2;  // ★修复：自动退出
}

/* ============================================================
 *  19. 伸手（Action_Mode=19）★★修复：加入退出机制
 * ============================================================ */
void Action_stretch_hand(void)
{
	Servo_Angle2(10);
	Delay_ms(300);  // ★固定延迟300ms后自动退出
	FEED_DOG();
	Action_Mode = 2;  // ★修复：自动退出，不再死锁
}

/* ============================================================
 *  20. 放下（Action_Mode=20）★★修复：加入退出机制
 * ============================================================ */
void Action_lay_down(void)
{
	Servo_Angle2(90);
	Delay_ms(300);  // ★固定延迟300ms后自动退出
	FEED_DOG();
	Action_Mode = 2;  // ★修复：自动退出，不再死锁
}

/* ============================================================
 *  MP3 音乐函数（保持不变）
 * ============================================================ */

void music_shuffle_play(void)
{
    static uint16_t ci_fuzhi = fa_ci;
    if(USART1_RxFlag == 1) { ci_fuzhi = fa_ci; USART1_RxFlag = 0; }
    if(Action_Mode == 21 && ci_fuzhi == 1)
    {
        MP3_Volume40();
        Serial_SetMP3Cmd(MP3_SHUFFLE_PLAY);
        Serial_SendMP3CmdToQueue();
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}

void music_one_cycle_stop(void)
{
    static uint16_t ci_fuzhi = fa_ci;
    if(USART1_RxFlag == 1) { ci_fuzhi = fa_ci; USART1_RxFlag = 0; }
    if(Action_Mode == 22 && ci_fuzhi == 1)
    {
        Serial_SetMP3Cmd(MP3_ONE_CYCLE_STOP);
        Serial_SendMP3CmdToQueue();
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}

void music_one_cycle_start(void)
{
    static uint16_t ci_fuzhi = fa_ci;
    if(USART1_RxFlag == 1) { ci_fuzhi = fa_ci; USART1_RxFlag = 0; }
    if(Action_Mode == 23 && ci_fuzhi == 1)
    {
        Serial_SetMP3Cmd(MP3_ONE_CYCLE_START);
        Serial_SendMP3CmdToQueue();
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}

void music_type_cycle_start(void)
{
    static uint16_t ci_fuzhi = fa_ci;
    if(USART1_RxFlag == 1) { ci_fuzhi = fa_ci; USART1_RxFlag = 0; }
    if(Action_Mode == 24 && ci_fuzhi == 1)
    {
        Serial_SetMP3Cmd(MP3_TYPE_CYCLE_START);
        Serial_SendMP3CmdToQueue();
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}

void music_type_cycle_stop(void)
{
    static uint16_t ci_fuzhi = fa_ci;
    if(USART1_RxFlag == 1) { ci_fuzhi = fa_ci; USART1_RxFlag = 0; }
    if(Action_Mode == 25 && ci_fuzhi == 1)
    {
        Serial_SetMP3Cmd(MP3_TYPE_CYCLE_STOP);
        Serial_SendMP3CmdToQueue();
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}

void music_next_song(void)
{
    static uint16_t ci_fuzhi = fa_ci;
    if(USART1_RxFlag == 1) { ci_fuzhi = fa_ci; USART1_RxFlag = 0; }
    if(Action_Mode == 26 && ci_fuzhi == 1)
    {
        Serial_SetMP3Cmd(MP3_NEXT_SONG);
        Serial_SendMP3CmdToQueue();
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}

void music_last_song(void)
{
    static uint16_t ci_fuzhi = fa_ci;
    if(USART1_RxFlag == 1) { ci_fuzhi = fa_ci; USART1_RxFlag = 0; }
    if(Action_Mode == 27 && ci_fuzhi == 1)
    {
        Serial_SetMP3Cmd(MP3_LAST_SONG);
        Serial_SendMP3CmdToQueue();
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}

void music_volume_set_80(void)
{
    static uint16_t ci_fuzhi = fa_ci;
    if(USART1_RxFlag == 1) { ci_fuzhi = fa_ci; USART1_RxFlag = 0; }
    if(Action_Mode == 28 && ci_fuzhi == 1)
    {
        Serial_SetMP3Cmd(MP3_VOLUME_SET_80);
        Serial_SendMP3CmdToQueue();
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}

void music_volume_set_60(void)
{
    static uint16_t ci_fuzhi = fa_ci;
    if(USART1_RxFlag == 1) { ci_fuzhi = fa_ci; USART1_RxFlag = 0; }
    if(Action_Mode == 29 && ci_fuzhi == 1)
    {
        Serial_SetMP3Cmd(MP3_VOLUME_SET_60);
        Serial_SendMP3CmdToQueue();
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}

void music_stop_play(void)
{
    static uint16_t ci_fuzhi = fa_ci;
    if(USART1_RxFlag == 1) { ci_fuzhi = fa_ci; USART1_RxFlag = 0; }
    if(Action_Mode == 30 && ci_fuzhi == 1)
    {
        Serial_SetMP3Cmd(MP3_STOP_PLAY);
        Serial_SendMP3CmdToQueue();
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}

void music_start_play(void)
{
    static uint16_t ci_fuzhi = fa_ci;
    if(USART1_RxFlag == 1) { ci_fuzhi = fa_ci; USART1_RxFlag = 0; }
    if(Action_Mode == 31 && ci_fuzhi == 1)
    {
        MP3_Volume40();
        Serial_SetMP3Cmd(MP3_START_PLAY);
        Serial_SendMP3CmdToQueue();
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}

void music_pause_play(void)
{
    static uint16_t ci_fuzhi = fa_ci;
    if(USART1_RxFlag == 1) { ci_fuzhi = fa_ci; USART1_RxFlag = 0; }
    if(Action_Mode == 32 && ci_fuzhi == 1)
    {
        Serial_SetMP3Cmd(MP3_PAUSE_PLAY);
        Serial_SendMP3CmdToQueue();
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}

void music_all_cycle_start(void)
{
    static uint16_t ci_fuzhi = fa_ci;
    if(USART1_RxFlag == 1) { ci_fuzhi = fa_ci; USART1_RxFlag = 0; }
    if(Action_Mode == 33 && ci_fuzhi == 1)
    {
        Serial_SetMP3Cmd(MP3_ALL_CYCLE_START);
        Serial_SendMP3CmdToQueue();
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}

void music_all_cycle_stop(void)
{
    static uint16_t ci_fuzhi = fa_ci;
    if(USART1_RxFlag == 1) { ci_fuzhi = fa_ci; USART1_RxFlag = 0; }
    if(Action_Mode == 34 && ci_fuzhi == 1)
    {
        Serial_SetMP3Cmd(MP3_ALL_CYCLE_STOP);
        Serial_SendMP3CmdToQueue();
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}

void music_play_music(void)
{
    static uint16_t ci_fuzhi = fa_ci;
    if(USART1_RxFlag == 1) { ci_fuzhi = fa_ci; USART1_RxFlag = 0; }
    if(Action_Mode == 35 && ci_fuzhi == 1)
    {
        MP3_Volume40();
        Serial_SetMP3Cmd(MP3_PLAY_MUSIC);
        Serial_SendMP3CmdToQueue();
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}

void music_play_art(void)
{
    static uint16_t ci_fuzhi = fa_ci;
    if(USART1_RxFlag == 1) { ci_fuzhi = fa_ci; USART1_RxFlag = 0; }
    if(Action_Mode == 36 && ci_fuzhi == 1)
    {
        MP3_Volume40();
        Serial_SetMP3Cmd(MP3_PLAY_ART);
        Serial_SendMP3CmdToQueue();
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}

void music_play_childern(void)
{
    static uint16_t ci_fuzhi = fa_ci;
    if(USART1_RxFlag == 1) { ci_fuzhi = fa_ci; USART1_RxFlag = 0; }
    if(Action_Mode == 37 && ci_fuzhi == 1)
    {
        MP3_Volume40();
        Serial_SetMP3Cmd(MP3_PLAY_CHILDREN);
        Serial_SendMP3CmdToQueue();
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}

void music_volume_set_40(void)
{
    static uint16_t ci_fuzhi = fa_ci;
    if(USART1_RxFlag == 1) { ci_fuzhi = fa_ci; USART1_RxFlag = 0; }
    if(Action_Mode == 38 && ci_fuzhi == 1)
    {
        Serial_SetMP3Cmd(MP3_VOLUME_SET_40);
        Serial_SendMP3CmdToQueue();
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}

void music_volume_set_20(void)
{
    static uint16_t ci_fuzhi = fa_ci;
    if(USART1_RxFlag == 1) { ci_fuzhi = fa_ci; USART1_RxFlag = 0; }
    if(Action_Mode == 39 && ci_fuzhi == 1)
    {
        Serial_SetMP3Cmd(MP3_VOLUME_SET_20);
        Serial_SendMP3CmdToQueue();
        ci_fuzhi = 0;
        Action_Mode = 2;
    }
}
