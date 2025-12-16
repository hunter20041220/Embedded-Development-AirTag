#include "MPU6050.h"
#include "Delay.h"

// 函数前向声明
static uint8_t I2C_ReadByteFromReg(uint8_t addr, uint8_t reg);

// 软件I2C引脚定义
#define I2C_PORT        GPIOB
#define I2C_SCL_PIN     GPIO_Pin_8   // PB8
#define I2C_SDA_PIN     GPIO_Pin_9   // PB9
#define I2C_RCC         RCC_APB2Periph_GPIOB

// I2C引脚操作宏
#define SCL_H()     GPIO_SetBits(I2C_PORT, I2C_SCL_PIN)
#define SCL_L()     GPIO_ResetBits(I2C_PORT, I2C_SCL_PIN)
#define SDA_H()     GPIO_SetBits(I2C_PORT, I2C_SDA_PIN)
#define SDA_L()     GPIO_ResetBits(I2C_PORT, I2C_SDA_PIN)
#define SDA_READ()  GPIO_ReadInputDataBit(I2C_PORT, I2C_SDA_PIN)

// 软件I2C延时
static void I2C_Delay(void)
{
    uint8_t i = 10;
    while(i--);
}

// SDA设置为输出
static void SDA_OUT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = I2C_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(I2C_PORT, &GPIO_InitStructure);
}

// SDA设置为输入
static void SDA_IN(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = I2C_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(I2C_PORT, &GPIO_InitStructure);
}

// I2C起始信号
static void I2C_Start(void)
{
    SDA_OUT();
    SDA_H();
    SCL_H();
    I2C_Delay();
    SDA_L();
    I2C_Delay();
    SCL_L();
}

// I2C停止信号
static void I2C_Stop(void)
{
    SDA_OUT();
    SCL_L();
    SDA_L();
    I2C_Delay();
    SCL_H();
    I2C_Delay();
    SDA_H();
    I2C_Delay();
}

// I2C等待应答
static uint8_t I2C_WaitAck(void)
{
    uint8_t timeout = 0;
    SDA_IN();
    SDA_H();
    I2C_Delay();
    SCL_H();
    I2C_Delay();
    while(SDA_READ())
    {
        timeout++;
        if(timeout > 250)
        {
            I2C_Stop();
            return 1;
        }
    }
    SCL_L();
    return 0;
}

// I2C发送应答
static void I2C_Ack(void)
{
    SCL_L();
    SDA_OUT();
    SDA_L();
    I2C_Delay();
    SCL_H();
    I2C_Delay();
    SCL_L();
}

// I2C不发送应答
static void I2C_NAck(void)
{
    SCL_L();
    SDA_OUT();
    SDA_H();
    I2C_Delay();
    SCL_H();
    I2C_Delay();
    SCL_L();
}

// I2C发送一个字节
static void I2C_SendByte(uint8_t data)
{
    uint8_t i;
    SDA_OUT();
    SCL_L();
    for(i = 0; i < 8; i++)
    {
        if(data & 0x80)
            SDA_H();
        else
            SDA_L();
        data <<= 1;
        I2C_Delay();
        SCL_H();
        I2C_Delay();
        SCL_L();
        I2C_Delay();
    }
}

// I2C接收一个字节
static uint8_t I2C_ReadByte(uint8_t ack)
{
    uint8_t i, data = 0;
    SDA_IN();
    for(i = 0; i < 8; i++)
    {
        SCL_L();
        I2C_Delay();
        SCL_H();
        data <<= 1;
        if(SDA_READ())
            data |= 1;
        I2C_Delay();
    }
    if(ack)
        I2C_Ack();
    else
        I2C_NAck();
    return data;
}

// MPU6050写寄存器
static void I2C_WriteByte(uint8_t addr, uint8_t reg, uint8_t data)
{
    I2C_Start();
    I2C_SendByte(addr);
    I2C_WaitAck();
    I2C_SendByte(reg);
    I2C_WaitAck();
    I2C_SendByte(data);
    I2C_WaitAck();
    I2C_Stop();
}

// MPU6050读寄存器
static uint8_t I2C_ReadByteFromReg(uint8_t addr, uint8_t reg)
{
    uint8_t data;
    I2C_Start();
    I2C_SendByte(addr);
    I2C_WaitAck();
    I2C_SendByte(reg);
    I2C_WaitAck();
    I2C_Start();
    I2C_SendByte(addr | 0x01);
    I2C_WaitAck();
    data = I2C_ReadByte(0);
    I2C_Stop();
    return data;
}

static void I2C_ReadBytes(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t i;
    I2C_Start();
    I2C_SendByte(addr);
    I2C_WaitAck();
    I2C_SendByte(reg);
    I2C_WaitAck();
    I2C_Start();
    I2C_SendByte(addr | 0x01);
    I2C_WaitAck();
    for(i = 0; i < len; i++)
    {
        if(i == len - 1)
            buf[i] = I2C_ReadByte(0);
        else
            buf[i] = I2C_ReadByte(1);
    }
    I2C_Stop();
}

void MPU6050_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 使能GPIO时钟
    RCC_APB2PeriphClockCmd(I2C_RCC, ENABLE);
    
    // 配置GPIO为开漏输出
    GPIO_InitStructure.GPIO_Pin = I2C_SCL_PIN | I2C_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(I2C_PORT, &GPIO_InitStructure);
    
    SCL_H();
    SDA_H();
    
    Delay_ms(100);  // 等待MPU6050稳定
    
    // 解除睡眠模式
    I2C_WriteByte(MPU6050_ADDR, MPU6050_PWR_MGMT_1, 0x00);
    Delay_ms(10);
    
    // 设置采样率分频器
    I2C_WriteByte(MPU6050_ADDR, MPU6050_SMPLRT_DIV, 0x07);
    
    // 配置低通滤波器
    I2C_WriteByte(MPU6050_ADDR, MPU6050_CONFIG, 0x06);
    
    // 配置陀螺仪量程: ±2000°/s
    I2C_WriteByte(MPU6050_ADDR, MPU6050_GYRO_CONFIG, 0x18);
    
    // 配置加速度计量程: ±16g
    I2C_WriteByte(MPU6050_ADDR, MPU6050_ACCEL_CONFIG, 0x18);
}

uint8_t MPU6050_CheckID(void)
{
    return I2C_ReadByteFromReg(MPU6050_ADDR, MPU6050_WHO_AM_I);
}

// 扫描两个可能的I2C地址，返回WHO_AM_I寄存器的原始值
void MPU6050_ScanIDs(uint8_t *id68, uint8_t *id69)
{
    if (id68) *id68 = I2C_ReadByteFromReg(0xD0, MPU6050_WHO_AM_I); // 0x68<<1
    if (id69) *id69 = I2C_ReadByteFromReg(0xD2, MPU6050_WHO_AM_I); // 0x69<<1
}

uint8_t MPU6050_Read(MPU6050_Data *data)
{
    uint8_t buf[14];
    
    // 读取14字节数据 (加速度+温度+陀螺仪)
    I2C_ReadBytes(MPU6050_ADDR, MPU6050_ACCEL_XOUT_H, buf, 14);
    
    // 解析数据
    data->accel_x = (buf[0] << 8) | buf[1];
    data->accel_y = (buf[2] << 8) | buf[3];
    data->accel_z = (buf[4] << 8) | buf[5];
    data->temp = (buf[6] << 8) | buf[7];
    data->gyro_x = (buf[8] << 8) | buf[9];
    data->gyro_y = (buf[10] << 8) | buf[11];
    data->gyro_z = (buf[12] << 8) | buf[13];
    
    return 1;
}

float MPU6050_GetTemp(int16_t temp_raw)
{
    // 温度计算公式: Temp = (TEMP_OUT / 340) + 36.53
    return (temp_raw / 340.0f) + 36.53f;
}