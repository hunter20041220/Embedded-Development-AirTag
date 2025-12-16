#ifndef __DHT22_H
#define __DHT22_H

#include "stm32f10x.h"

typedef struct {
    float temperature;  // 温度 (°C)
    float humidity;     // 湿度 (%RH)
    uint8_t valid;      // 数据有效标志 (1=有效, 0=无效)
} DHT22_Data;

void DHT22_Init(void);
uint8_t DHT22_Read(DHT22_Data *data);

#endif