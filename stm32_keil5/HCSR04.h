#ifndef __HCSR04_H
#define __HCSR04_H

#include "stm32f10x.h"

// HC-SR04超声波测距模块
void HCSR04_Init(void);
float HCSR04_GetDistance(void);  // 返回距离(cm)，失败返回-1

#endif
