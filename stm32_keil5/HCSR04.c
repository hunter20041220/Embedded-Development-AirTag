#include "HCSR04.h"
#include "Delay.h"

/* HC-SR04超声波测距模块
 * 接线:
 *   VCC  -> 5V
 *   Trig -> PB0 (触发引脚)
 *   Echo -> PB1 (回响引脚)
 *   GND  -> GND
 * 
 * 测距原理:
 *   1. Trig发送10us以上高电平触发
 *   2. 模块自动发送8个40kHz超声波
 *   3. Echo输出高电平脉宽 = 超声波往返时间
 *   4. 距离(cm) = 脉宽(us) * 0.017 (声速340m/s)
 */

#define TRIG_PIN  GPIO_Pin_0
#define ECHO_PIN  GPIO_Pin_1
#define TRIG_PORT GPIOB
#define ECHO_PORT GPIOB

#define TRIG_HIGH()  GPIO_SetBits(TRIG_PORT, TRIG_PIN)
#define TRIG_LOW()   GPIO_ResetBits(TRIG_PORT, TRIG_PIN)
#define ECHO_READ()  GPIO_ReadInputDataBit(ECHO_PORT, ECHO_PIN)

void HCSR04_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 开启GPIOB时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    // PB0 (Trig) 配置为推挽输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = TRIG_PIN;
    GPIO_Init(TRIG_PORT, &GPIO_InitStructure);
    
    // PB1 (Echo) 配置为浮空输入
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = ECHO_PIN;
    GPIO_Init(ECHO_PORT, &GPIO_InitStructure);
    
    // Trig初始状态为低电平
    TRIG_LOW();
}

float HCSR04_GetDistance(void)
{
    uint32_t timeout;
    uint32_t pulse_width = 0;
    float distance;
    
    // 1. 发送10us以上的高电平触发信号
    TRIG_LOW();
    Delay_us(2);
    TRIG_HIGH();
    Delay_us(15);  // 保持15us高电平
    TRIG_LOW();
    
    // 2. 等待Echo变为高电平 (超时10ms)
    timeout = 10000;
    while (ECHO_READ() == 0 && timeout > 0)
    {
        timeout--;
    }
    if (timeout == 0)
        return -1.0f;  // 超时，测距失败
    
    // 3. 测量Echo高电平持续时间（软件计数）
    //    每次循环约0.5us（72MHz，约36个时钟周期）
    pulse_width = 0;
    timeout = 50000;  // 约25ms超时
    while (ECHO_READ() == 1 && timeout > 0)
    {
        pulse_width++;
        timeout--;
    }
    if (timeout == 0)
        return -1.0f;  // 超时
    
    // 4. 计算距离: 脉宽(实际us) = pulse_width * 0.5us
    //    距离(cm) = 脉宽(us) * 0.034 / 2 = pulse_width * 0.5 * 0.017
    //    距离 = pulse_width * 0.0085
    distance = pulse_width * 0.0085f;
    
    // 5. 有效范围检查 (只检查大于0)
    if (distance <= 0)
        return -1.0f;
    
    return distance;
}
