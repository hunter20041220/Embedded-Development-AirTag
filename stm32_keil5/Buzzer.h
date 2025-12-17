#ifndef __BUZZER_H
#define __BUZZER_H

#include "stm32f10x.h"

// 蜂鸣器类型选择
#define BUZZER_TYPE_ACTIVE   1  // 有源蜂鸣器（高电平响）
#define BUZZER_TYPE_PASSIVE  2  // 无源蜂鸣器（需要PWM）

#define BUZZER_TYPE  BUZZER_TYPE_ACTIVE  // 默认有源蜂鸣器

// 基础控制函数
void Buzzer_Init(void);
void Buzzer_On(void);       // 有源蜂鸣器：直接响；无源蜂鸣器：启动PWM
void Buzzer_Off(void);      // 关闭蜂鸣器
void Buzzer_Toggle(void);   // 翻转状态

// 高级功能（适用于有源蜂鸣器）
void Buzzer_Beep(uint16_t ms);           // 响ms毫秒后自动停止
void Buzzer_BeepTimes(uint8_t times, uint16_t duration); // 响n次，每次duration毫秒

// 音乐播放（适用于无源蜂鸣器）
void Buzzer_PlaySeeYouAgain(void);  // 播放See You Again旋律片段

#endif
