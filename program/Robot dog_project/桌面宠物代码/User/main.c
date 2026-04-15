#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "BlueTooth.h"
#include "Servo.h"
#include "PetAction.h"
#include "Face_Config.h"
#include "PWM.h"
#include "Serial.h"



/***************************************************************************************
  * 本程序由博主我Sngels_wyh创建并免费开源共享
  * 你可以任意查看、使用和修改，并应用到自己的项目之中
  * 
  * 程序名称：				基于STM32F103C8T6单片机的桌面宠物小项目
  * 此程序更新时间：			2025.2.16
  * 
  * 主要在立创开源平台、CSDN网站、哔哩哔哩、抖音分享自己的作品。
	*	相关的资料可以去立创开源硬件平台网站上搜索,作品附件放置更新网盘，如有更新可方便快速查看
	* 地址:https://oshwhub.com/sngelswyh/stm32-smart-desktop-pet
  ***************************************************************************************/


uint16_t Time;
uint16_t HuXi;
uint16_t PanDuan=1;
uint16_t Wait=0;

int main(void)
{
	Servo_Init();
	OLED_Init();//OLED初始化
	Serial_Init();		
	BlueTooth_Init();//蓝牙初始化
	OLED_ShowImage(0,0,128,64,Face_sleep);
	OLED_Update();
	Action_upright();
	Serial_SendByte(0xAA);

	while(1)
	{	

		if(Action_Mode==0){Action_relaxed_getdowm();WServo_Angle(90);}//放松趴下
		else if(Action_Mode==1){Action_sit();}//坐下
		else if(Action_Mode==2){Action_upright();}//站立
		else if(Action_Mode==3){Action_getdowm();}//趴下
		else if(Action_Mode==4){Action_advance();}//前进
		else if(Action_Mode==5){Action_back();}//后退
		else if(Action_Mode==6){Action_Lrotation();}//左转
		else if(Action_Mode==7){Action_Rrotation();}//右转
		else if(Action_Mode==8){Action_Swing();}//前后摇摆-----修改
		else if(Action_Mode==9){Action_SwingTail();}//摇尾巴
		else if(Action_Mode==10){Action_JumpU();}//前跳
		else if(Action_Mode==11){Action_JumpD();}//后跳
		else if(Action_Mode==12){Action_upright2();}//站立方式2
		else if(Action_Mode==13){Action_Hello();}//打招呼
		else if(Action_Mode==14){Action_stretch();}//伸懒腰
		else if(Action_Mode==15){Action_Lstretch();}//后腿拉伸
		else if(Action_Mode==16){Action_dance();}//跳舞
		else if(Action_Mode==17){Action_left_right_Lstretch();}//左右摇摆
		else if(Action_Mode==18){Action_push_up();}//俯卧撑
		else if(Action_Mode==19){Action_stretch_hand();}//伸手
		else if(Action_Mode==20){Action_lay_down();}//放下
		else if(Action_Mode==21){music_shuffle_play();}//随机播放
		else if(Action_Mode==23){music_one_cycle_start();}//单曲循环---开始
		else if(Action_Mode==22){music_one_cycle_stop();}//单曲循环---结束
		else if(Action_Mode==24){music_type_cycle_start();}//类型循环---开始
		else if(Action_Mode==25){music_type_cycle_stop();}//类型循环---结束
		else if(Action_Mode==26){music_next_song();}//下一曲
		else if(Action_Mode==27){music_last_song();}//上一曲
		else if(Action_Mode==28){music_volume_set_80();}//音量加
		else if(Action_Mode==29){music_volume_set_60();}//音量减
		else if(Action_Mode==30){music_stop_play();}//停止播放
		else if(Action_Mode==31){music_start_play();}//播放
		else if(Action_Mode==32){music_pause_play();}//暂停
		else if(Action_Mode==33){music_all_cycle_start();}//全部循环---开始
		else if(Action_Mode==34){music_all_cycle_stop();}//全部循环---结束
		else if(Action_Mode==35){music_play_music();}//普通音乐播放
		else if(Action_Mode==36){music_play_art();}//戏曲播放
		else if(Action_Mode==37){music_play_childern();}//儿歌播放
		else if(Action_Mode==38){music_volume_set_40();}//儿歌播放
		else if(Action_Mode==39){music_volume_set_20();}//儿歌播放


				 // 2. 关键：触发出队和发送（必须调用，否则数据在队列中不发送）

   
		
//左右摇摆

//前后摇摆
//	uint16_t SwingDelay=10;
//	uint16_t SwingNumber=SwingRepeatnumber;

//		for(uint8_t i=30;i<150;i++)
//		{
//			Servo_Angle1(i);
//			Servo_Angle2(i);
//			Servo_Angle3(i);
//			Servo_Angle4(i);
//			Delay_ms(SwingDelay);
//		}
//		for(uint8_t i=150;i>30;i--)
//		{
//			Servo_Angle1(i);
//			Servo_Angle2(i);
//			Servo_Angle3(i);
//			Servo_Angle4(i);
//			Delay_ms(SwingDelay);
//		}





//俯卧撑程序



//打招呼程序
//		for(uint16_t i=10;i<50;i+=2)
//		{
//			Servo_Angle2(i);
//			Delay_ms (50);
//		}
//		for(uint16_t j=50;j>10;j-=2)
//		{
//			Servo_Angle2(j);
//			Delay_ms (50);
//		}
		
//		
////伸手


////放下




		
	}
}


void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET)
	{	
//		if(AllLed==1 && BreatheLed==0)//如果灯光开启且不开启呼吸灯,亮度拉满
//		{
//			PWM_LED1(20000);
//			PWM_LED2(20000);
//		}
//		else if(AllLed==1 && BreatheLed==1)//如果灯光开启且开启呼吸灯
//		{
//			if(PanDuan==1)
//			{
//				HuXi+=100;
//				PWM_LED1(HuXi);
//				PWM_LED2(HuXi);
//				if(HuXi==20000)
//					PanDuan=2;
//			}
//			else if(PanDuan==2)
//			{
//				HuXi-=100;
//				PWM_LED1(HuXi);
//				PWM_LED2(HuXi);
//				if(HuXi==0)
//				{
//					PanDuan=3;
//				}
//			}
//			else if(PanDuan==3)
//			{
//				Wait+=1000;
//				if(Wait==20000)
//				{
//					PanDuan=1;
//					Wait=0;
//				}
//			}
//		}
//		else if(AllLed==0)
//		{
//			PWM_LED1(0);
//			PWM_LED2(0);
//		}
		
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
	}
}
