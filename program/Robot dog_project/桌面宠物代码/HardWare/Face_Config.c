/**
  ******************************************************************************
  * @文件    Face_Config.c
  * @说明    桌面宠物 动态表情 v4 - 8种卡通风表情
  *
  *  128x64 OLED 坐标系：(0,0)=左上角, X向右0~127, Y向下0~63
  *
  *  Face_Mode:
  *    0 惊讶  1 微笑  2 难过  3 吃瓜
  *    4 鼓掌  5 俏皮  6 得意  7 睡觉
  *
  *  设计原则：
  *  - 眼睛用实心圆/弧线，不使用 ClearArea
  *  - 左右对称，间距适中
  *  - 动画节拍统一管理
  ******************************************************************************
  */
#include "stm32f10x.h"
#include "OLED.h"
#include "BlueTooth.h"
#include "Face_Config.h"

/* ==================== 布局常量 ==================== */
#define LX      38          // 左眼中心X
#define RX      90          // 右眼中心X
#define EY      22          // 眼睛Y（正常状态）
#define CX      64          // 脸中心X
#define MX      64          // 嘴巴中心X
#define MY      48          // 嘴巴Y
#define BLUSH_Y 38          // 腮红Y

/* ==================== 动画状态 ==================== */
static uint8_t tick       = 0;     // 全局节拍
static uint8_t blink_on   = 0;     // 1=正在闭眼
static uint8_t blink_wait = 25;    // 眨眼等待计数
static uint8_t bounce     = 0;     // 弹跳相位 0~3
static uint8_t shake      = 0;     // 抖动相位 0~3
static uint8_t wave       = 0;     // 挥手相位 0~3
static uint8_t squint_sw  = 0;     // 眯眼开关 0/1
static uint8_t breath     = 0;     // 呼吸 0/1
static uint8_t chew       = 0;     // 咀嚼相位 0~3 (吃瓜用)
static uint8_t tear_drop  = 0;     // 泪滴位置 (难过用)

/* ============================================================
 *  眼睛绘制组件
 * ============================================================ */

/* 实心大圆眼 —— 惊讶/通用 */
static void EyeBigRound(int16_t cx, int16_t cy)
{
	OLED_DrawCircle(cx, cy, 10, OLED_FILLED);
}

/* 实心小圆眼 —— 难过 */
static void EyeSmallRound(int16_t cx, int16_t cy)
{
	OLED_DrawCircle(cx, cy, 7, OLED_FILLED);
}

/* 快乐弯弯眼 ^_^ —— 微笑/鼓掌 */
static void EyeHappyArc(int16_t cx, int16_t cy)
{
	OLED_DrawArc(cx, cy + 5, 10, 190, 350, OLED_UNFILLED);
}

/* 超开心眯眼（月牙）—— 鼓掌 */
static void EyeSquintArc(int16_t cx, int16_t cy)
{
	OLED_DrawArc(cx, cy + 3, 7, 210, 330, OLED_UNFILLED);
}

/* 闭眼横线 —— 眨眼/俏皮wink */
static void EyeClosedLine(int16_t cx, int16_t cy)
{
	OLED_DrawLine(cx - 10, cy, cx + 10, cy);
}

/* 闭眼弧线 —— 睡觉 */
static void EyeSleepArc(int16_t cx, int16_t cy)
{
	OLED_DrawArc(cx, cy + 4, 10, 210, 330, OLED_UNFILLED);
}

/* 半闭眼 —— 得意 */
static void EyeHalfClosed(int16_t cx, int16_t cy)
{
	OLED_DrawLine(cx - 10, cy, cx + 10, cy);
	OLED_DrawArc(cx, cy + 5, 10, 180, 270, OLED_UNFILLED);
	OLED_DrawArc(cx, cy + 5, 10, 270, 360, OLED_UNFILLED);
	// 下半圆实心遮住，用横线+上弧模拟半闭
}

/* 斜眼（瞳孔偏移）—— 吃瓜 */
static void EyeLookSide(int16_t cx, int16_t cy, int16_t dx)
{
	OLED_DrawCircle(cx, cy, 9, OLED_UNFILLED);  // 眼眶
	OLED_DrawCircle(cx + dx, cy, 5, OLED_FILLED);  // 瞳孔
}

/* ============================================================
 *  眉毛绘制组件
 * ============================================================ */

/* 平眉 */
static void BrowFlat(int16_t cx, int16_t cy)
{
	OLED_DrawLine(cx - 8, cy, cx + 8, cy);
}

/* 惊讶上扬眉 /\ */
static void BrowSurprise(int16_t cx, int16_t cy)
{
	OLED_DrawLine(cx - 8, cy + 3, cx, cy - 2);
	OLED_DrawLine(cx, cy - 2, cx + 8, cy + 3);
}

/* 开心弯眉 */
static void BrowHappy(int16_t cx, int16_t cy)
{
	OLED_DrawArc(cx, cy + 6, 9, 190, 350, OLED_UNFILLED);
}

/* 难过下垂眉 (内高外低) */
static void BrowSad(int16_t cx, int16_t cy)
{
	OLED_DrawLine(cx - 8, cy + 3, cx + 8, cy - 2);
}

/* 俏皮一高一低 */
static void BrowUp(int16_t cx, int16_t cy)
{
	OLED_DrawLine(cx - 7, cy - 1, cx + 7, cy + 2);
}

static void BrowNormal(int16_t cx, int16_t cy)
{
	OLED_DrawLine(cx - 7, cy, cx + 7, cy);
}

/* 得意自信眉 */
static void BrowProud(int16_t cx, int16_t cy)
{
	OLED_DrawArc(cx, cy + 4, 8, 200, 340, OLED_UNFILLED);
}

/* ============================================================
 *  嘴巴绘制组件
 * ============================================================ */

/* 惊讶 O 型嘴 */
static void MouthO(int16_t cx, int16_t cy)
{
	OLED_DrawEllipse(cx, cy, 5, 6, OLED_UNFILLED);
}

/* 微笑弧线 */
static void MouthSmile(int16_t cx, int16_t cy)
{
	OLED_DrawArc(cx, cy - 8, 14, 220, 320, OLED_UNFILLED);
}

/* 大笑弧线 —— 鼓掌 */
static void MouthBigSmile(int16_t cx, int16_t cy)
{
	OLED_DrawArc(cx, cy - 6, 16, 210, 330, OLED_UNFILLED);
}

/* 张嘴大笑 + 舌头 —— 鼓掌 */
static void MouthLaugh(int16_t cx, int16_t cy)
{
	OLED_DrawArc(cx, cy - 6, 16, 200, 340, OLED_UNFILLED);
	OLED_DrawArc(cx, cy + 4, 6, 0, 180, OLED_UNFILLED);  // 舌头
}

/* 难过下弯弧线 */
static void MouthFrown(int16_t cx, int16_t cy)
{
	OLED_DrawArc(cx, cy + 6, 10, 10, 170, OLED_UNFILLED);
}

/* 嘟嘴（难过小嘴） */
static void MouthPout(int16_t cx, int16_t cy)
{
	OLED_DrawEllipse(cx, cy, 4, 5, OLED_UNFILLED);
}

/* 平嘴 */
static void MouthFlat(int16_t cx, int16_t cy)
{
	OLED_DrawLine(cx - 8, cy, cx + 8, cy);
}

/* 微张小嘴（睡觉/呼吸用） */
static void MouthSmallO(int16_t cx, int16_t cy)
{
	OLED_DrawCircle(cx, cy, 3, OLED_UNFILLED);
}

/* 歪嘴笑（俏皮用）—— 右边上翘 */
static void MouthSmirk(int16_t cx, int16_t cy)
{
	OLED_DrawLine(cx - 8, cy, cx, cy - 3);
	OLED_DrawLine(cx, cy - 3, cx + 8, cy - 5);
}

/* 得意咧嘴笑 */
static void MouthGrin(int16_t cx, int16_t cy)
{
	OLED_DrawArc(cx, cy - 8, 14, 210, 330, OLED_UNFILLED);
	// 小牙齿
	OLED_DrawLine(cx - 4, cy - 1, cx - 2, cy + 1);
	OLED_DrawLine(cx + 2, cy + 1, cx + 4, cy - 1);
}

/* ============================================================
 *  特效组件
 * ============================================================ */

/* 泪滴（难过用） */
static void DrawTear(int16_t cx, int16_t cy, uint8_t phase)
{
	int16_t y = cy + (phase & 7);
	if(y < 56)
	{
		OLED_DrawLine(cx, cy, cx, y);
		OLED_DrawCircle(cx, y + 1, 1, OLED_FILLED);
	}
}

/* 手臂+挥手（打招呼用） */
static void DrawHand(int16_t cx, int16_t cy, uint8_t phase)
{
	int16_t off = 0;
	switch(phase & 3)
	{ case 0: off=0; break; case 1: off=-5; break; case 2: off=0; break; case 3: off=5; break; }
	OLED_DrawLine(cx, cy, cx, cy + 10 + off);
	OLED_DrawCircle(cx, cy + 13 + off, 3, OLED_UNFILLED);
	OLED_DrawLine(cx - 2, cy + 15 + off, cx - 2, cy + 18 + off);
	OLED_DrawLine(cx + 2, cy + 15 + off, cx + 2, cy + 18 + off);
}

/* 星星装饰（得意/鼓掌用） */
static void DrawStars(int16_t x, int16_t y, uint8_t phase)
{
	if((phase & 7) < 4)
	{
		OLED_ShowChar(x, y, '*', OLED_6X8);
	}
}

/* 音符装饰（鼓掌/吃瓜用） */
static void DrawNote(int16_t x, int16_t y, uint8_t phase)
{
	if((phase & 7) < 4)
	{
		OLED_ShowChar(x, y, '~', OLED_6X8);
	}
}

/* ============================================================
 *   8 种表情帧绘制
 * ============================================================ */

/* --- 0 惊讶 😮 --- */
/* 大圆眼 + 上扬眉 + O嘴 + 偶尔眨眼 */
static void DrawSurprise(void)
{
	if(blink_on)
	{
		EyeClosedLine(LX, EY);
		EyeClosedLine(RX, EY);
		MouthFlat(MX, MY);
	}
	else
	{
		BrowSurprise(LX, 8);
		BrowSurprise(RX, 8);

		EyeBigRound(LX, EY);
		EyeBigRound(RX, EY);

		MouthO(MX, MY);
	}
}

/* --- 1 微笑 😊 --- */
/* 弯笑眼 + 微笑弧 + 腮红 + 弹跳 */
static void DrawSmile(void)
{
	int16_t y = (bounce & 3) - 1;  // -1, 0, 1, 0

	BrowHappy(LX, 10 + y);
	BrowHappy(RX, 10 + y);

	EyeHappyArc(LX, EY + y);
	EyeHappyArc(RX, EY + y);

	MouthSmile(MX, MY + y);

	OLED_DrawCircle(20, BLUSH_Y + y, 4, OLED_UNFILLED);   // 左腮红
	OLED_DrawCircle(108, BLUSH_Y + y, 4, OLED_UNFILLED);  // 右腮红
}

/* --- 2 难过 😢 --- */
/* 小圆眼下垂眉 + 嘟嘴 + 泪滴 */
static void DrawSad(void)
{
	int16_t y = breath ? 0 : 1;  // 轻微呼吸起伏

	BrowSad(LX, 10 + y);
	BrowSad(RX, 10 + y);

	EyeSmallRound(LX, EY + 2 + y);  // 眼睛位置略低
	EyeSmallRound(RX, EY + 2 + y);

	MouthPout(MX, MY + y);

	// 泪滴（右眼）
	if(tick % 4 == 0) tear_drop++;
	DrawTear(RX + 6, EY + 8, tear_drop);
}

/* --- 3 吃瓜 🍉 --- */
/* 斜眼看 + 咀嚼嘴 + 西瓜装饰 */
static void DrawMelon(void)
{
	BrowFlat(LX, 10);
	BrowFlat(RX, 10);

	// 眼睛向右看
	EyeLookSide(LX, EY, 4);
	EyeLookSide(RX, EY, 4);

	// 咀嚼动画：嘴巴开合
	if(chew & 1)
	{
		MouthO(MX, MY);  // 张嘴
	}
	else
	{
		MouthSmile(MX, MY);  // 闭嘴微笑
	}

	// 西瓜装饰（右上角）
	OLED_DrawArc(120, 6, 6, 180, 360, OLED_FILLED);  // 半圆西瓜
	OLED_DrawLine(114, 6, 126, 6);                    // 切面线
	// 西瓜籽
	OLED_DrawPoint(117, 8);
	OLED_DrawPoint(120, 9);
	OLED_DrawPoint(123, 8);
}

/* --- 4 鼓掌 👏 --- */
/* 眯笑眼 + 大笑 + 侧边闪光 */
static void DrawClap(void)
{
	int16_t y = (bounce & 1) ? -2 : 0;

	BrowHappy(LX, 10 + y);
	BrowHappy(RX, 10 + y);

	if(squint_sw)
	{
		EyeSquintArc(LX, EY + y);
		EyeSquintArc(RX, EY + y);
	}
	else
	{
		EyeHappyArc(LX, EY + y);
		EyeHappyArc(RX, EY + y);
	}

	MouthBigSmile(MX, MY + y);

	OLED_DrawCircle(18, BLUSH_Y + y, 4, OLED_UNFILLED);   // 左腮红
	OLED_DrawCircle(110, BLUSH_Y + y, 4, OLED_UNFILLED);  // 右腮红

	// 侧边闪光/星星
	DrawStars(4, 4, tick);
	DrawStars(118, 4, tick + 4);
	DrawNote(8, 16, tick + 2);
	DrawNote(114, 16, tick + 6);
}

/* --- 5 俏皮 😜 --- */
/* wink一眼 + 歪嘴笑 + 吐舌 */
static void DrawPlayful(void)
{
	int16_t y = (wave & 1) ? -1 : 1;

	// 左眼wink，右眼正常
	if(squint_sw)
	{
		EyeClosedLine(LX, EY + y);   // wink
		EyeBigRound(RX, EY + y);     // 右眼圆睁
	}
	else
	{
		EyeBigRound(LX, EY + y);
		EyeBigRound(RX, EY + y);
	}

	// 左眉上挑，右眉正常
	BrowUp(LX, 8 + y);
	BrowNormal(RX, 8 + y);

	// 歪嘴笑
	MouthSmirk(MX, MY + y);

	// 吐舌（小三角）
	OLED_DrawLine(MX + 2, MY + 1 + y, MX + 5, MY + 5 + y);
	OLED_DrawLine(MX + 5, MY + 5 + y, MX + 8, MY + 1 + y);

	// 星星装饰
	DrawStars(118, 4, tick + 2);
}

/* --- 6 得意 😏 --- */
/* 半闭眼 + 自信眉 + 咧嘴笑 + 星星 */
static void DrawProud(void)
{
	int16_t y = breath ? -1 : 0;  // 轻微呼吸

	BrowProud(LX, 10 + y);
	BrowProud(RX, 10 + y);

	// 半闭眼 + 微微上抬
	EyeHalfClosed(LX, EY - 1 + y);
	EyeHalfClosed(RX, EY - 1 + y);

	MouthGrin(MX, MY + y);

	// 星星装饰
	DrawStars(6, 2, tick);
	DrawStars(118, 2, tick + 3);
	DrawStars(10, 14, tick + 5);
	DrawStars(114, 14, tick + 7);
}

/* --- 7 睡觉 😴 --- */
/* 闭眼下弧线 + 微张嘴 + 呼吸起伏 + Zzz */
static void DrawSleep(void)
{
	int16_t y = breath ? -1 : 1;

	BrowFlat(LX, 12 + y);
	BrowFlat(RX, 12 + y);

	EyeSleepArc(LX, EY + y);
	EyeSleepArc(RX, EY + y);

	MouthSmallO(MX, MY + y);

	// Zzz 动画
	OLED_ShowChar(102, 8, 'Z', OLED_8X16);
	if(breath == 0)
		OLED_ShowChar(114, 2, 'z', OLED_6X8);
}

/* ============================================================
 *  主入口
 * ============================================================ */
void Face_Config(void)
{
	/* 1. 节拍 */
	tick++;

	/* 2. 眨眼 */
	if(blink_wait > 0)
	{
		blink_wait--;
	}
	else
	{
		if(blink_on == 0)
		{
			blink_on = 1;
			blink_wait = 3;                          // 闭眼持续3拍
		}
		else
		{
			blink_on = 0;
			blink_wait = 20 + (tick & 0x1F);        // 20~51拍后眨眼
		}
	}

	/* 3. 弹跳（每3拍） */
	if(tick % 3 == 0) bounce++;

	/* 4. 抖动（每2拍） */
	if((tick & 1) == 0) shake++;

	/* 5. 挥手（每5拍） */
	if(tick % 5 == 0) wave++;

	/* 6. 眯眼切换（每6拍） */
	if(tick % 6 == 0) squint_sw ^= 1;

	/* 7. 呼吸（每15拍） */
	if(tick % 15 == 0) breath ^= 1;

	/* 8. 咀嚼（每8拍，吃瓜用） */
	if(tick % 8 == 0) chew++;

	/* 9. 绘制 */
	OLED_Clear();

	switch(Face_Mode)
	{
		case 0: DrawSurprise();  break;   // 惊讶
		case 1: DrawSmile();     break;   // 微笑
		case 2: DrawSad();       break;   // 难过
		case 3: DrawMelon();     break;   // 吃瓜
		case 4: DrawClap();      break;   // 鼓掌
		case 5: DrawPlayful();   break;   // 俏皮
		case 6: DrawProud();     break;   // 得意
		case 7: DrawSleep();     break;   // 睡觉
		default: break;
	}

	OLED_Update();
}
