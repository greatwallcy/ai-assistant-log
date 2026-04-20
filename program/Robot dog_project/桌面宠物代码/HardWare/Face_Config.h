/**
  ******************************************************************************
  * @文件    Face_Config.h
  * @说明    动态表情系统头文件 v4 - 8种表情
  *          Face_Mode值:
  *          0=惊讶  1=微笑  2=难过  3=吃瓜
  *          4=鼓掌  5=调皮  6=得意  7=睡
  ******************************************************************************
  */
#ifndef __FACE_CONFIG_H
#define __FACE_CONFIG_H

#include <stdint.h>

/* 表情模式枚举 */
#define FACE_SURPRISE   0   // 惊讶 - 大圆眼 + O嘴 + 上扬眉
#define FACE_SMILE      1   // 微笑 - 弯笑眼 + 微笑弧 + 腮红
#define FACE_SAD        2   // 难过 - 下垂眉 + 嘟嘴 + 泪滴
#define FACE_MELON      3   // 吃瓜 - 斜眼 + 咀嚼 + 西瓜装饰
#define FACE_CLAP       4   // 鼓掌 - 眯笑眼 + 大笑 + 闪光特效
#define FACE_PLAYFUL    5   // 俏皮 - 眨眼 + 歪嘴 + 吐舌
#define FACE_PROUD      6   // 得意 - 半闭眼 + 嘲笑 + 星星
#define FACE_SLEEP      7   // 睡觉 - 闭眼弧线 + 呼吸 + Zzz

/* 主函数 */
void Face_Config(void);

#endif /* __FACE_CONFIG_H */
