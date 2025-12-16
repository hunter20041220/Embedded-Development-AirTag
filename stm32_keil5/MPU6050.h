#ifndef __MPU6050_H
#define __MPU6050_H

#include "stm32f10x.h"

// MPU6050寄存器地址
#define MPU6050_ADDR        0xD0  // AD0接GND时地址 (0x68<<1)
#define MPU6050_WHO_AM_I    0x75
#define MPU6050_PWR_MGMT_1  0x6B
#define MPU6050_SMPLRT_DIV  0x19
#define MPU6050_CONFIG      0x1A
#define MPU6050_GYRO_CONFIG 0x1B
#define MPU6050_ACCEL_CONFIG 0x1C
#define MPU6050_ACCEL_XOUT_H 0x3B

typedef struct {
    int16_t accel_x;    // 加速度X轴
    int16_t accel_y;    // 加速度Y轴
    int16_t accel_z;    // 加速度Z轴
    int16_t temp;       // 温度
    int16_t gyro_x;     // 陀螺仪 X轴
    int16_t gyro_y;     // 陀螺仪 Y轴
    int16_t gyro_z;     // 陀螺仪 Z轴
} MPU6050_Data;

void MPU6050_Init(void);
uint8_t MPU6050_CheckID(void);
void MPU6050_ScanIDs(uint8_t *id68, uint8_t *id69);
uint8_t MPU6050_Read(MPU6050_Data *data);
float MPU6050_GetTemp(int16_t temp_raw);

#endif