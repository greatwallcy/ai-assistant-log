#include "stm32f10x.h"                  // Device header
#include "PWM.h"

void Servo_Init()
{
	PWM_Init();	
}

// 角度限制宏：防止超出舵机安全范围导致堵转烧毁
#define ANGLE_MIN  0
#define ANGLE_MAX  180
#define CLAMP_ANGLE(a) ((a) < ANGLE_MIN ? ANGLE_MIN : ((a) > ANGLE_MAX ? ANGLE_MAX : (a)))

void Servo_Angle1(float Angle)//左上
{
	Angle = CLAMP_ANGLE(Angle);
	PWM_SetCompare1(Angle / 180 * 2000 + 500);			
}

void Servo_Angle2(float Angle)//右上
{
	Angle = CLAMP_ANGLE(Angle);
	PWM_SetCompare2((180-Angle) / 180 * 2000 + 500);		
}

void Servo_Angle3(float Angle)//左下
{
	Angle = CLAMP_ANGLE(Angle);
	PWM_SetCompare3(Angle / 180 * 2000 + 500);			
}

void Servo_Angle4(float Angle)//右下
{
	Angle = CLAMP_ANGLE(Angle);
	PWM_SetCompare4((180-Angle) / 180 * 2000 + 500);			
}

void WServo_Angle(float Angle)//尾巴
{
	Angle = CLAMP_ANGLE(Angle);
	PWM_WSetCompare(Angle / 180 * 2000 + 500);			
}
