#ifndef __LED_H
#define __LED_H

#include "stm32f10x.h"

// LED控制函数
void LED_Init(void);
void LED_On(void);
void LED_Off(void);
void LED_Toggle(void);
void LED_Set(uint8_t state); // 1=亮, 0=灭

#endif
