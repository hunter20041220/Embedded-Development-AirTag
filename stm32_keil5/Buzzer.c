#include "Buzzer.h"
#include "Delay.h"

/* 蜂鸣器接线: PA8 (TIM1_CH1)
 * VCC -> 5V 或 3.3V (根据蜂鸣器规格)
 * I/O -> PA8
 * GND -> GND
 */

#if BUZZER_TYPE == BUZZER_TYPE_ACTIVE
// ========== 有源蜂鸣器模式 (GPIO控制) ==========

void Buzzer_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 开启GPIOA时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    // PA8配置为推挽输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 初始状态：关闭
    Buzzer_Off();
}

void Buzzer_On(void)
{
    GPIO_SetBits(GPIOA, GPIO_Pin_8);  // 高电平，有源蜂鸣器响
}

void Buzzer_Off(void)
{
    GPIO_ResetBits(GPIOA, GPIO_Pin_8); // 低电平，关闭
}

void Buzzer_Toggle(void)
{
    if (GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_8))
        Buzzer_Off();
    else
        Buzzer_On();
}

void Buzzer_Beep(uint16_t ms)
{
    Buzzer_On();
    Delay_ms(ms);
    Buzzer_Off();
}

void Buzzer_BeepTimes(uint8_t times, uint16_t duration)
{
    uint8_t i;
    for (i = 0; i < times; i++)
    {
        Buzzer_On();
        Delay_ms(duration);
        Buzzer_Off();
        if (i < times - 1) // 最后一次不延时
            Delay_ms(duration);
    }
}

#elif BUZZER_TYPE == BUZZER_TYPE_PASSIVE
// ========== 无源蜂鸣器模式 (PWM控制) ==========

void Buzzer_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    
    // 开启TIM1和GPIOA时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 | RCC_APB2Periph_GPIOA, ENABLE);
    
    // PA8配置为TIM1_CH1复用推挽输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // TIM1时基配置 (产生2.7kHz PWM)
    // 72MHz / (26+1) / (1000+1) = 2666Hz ≈ 2.7kHz
    TIM_TimeBaseStructure.TIM_Period = 999;            // ARR自动重装载值
    TIM_TimeBaseStructure.TIM_Prescaler = 26;          // 预分频器
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;   // 高级定时器特有
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
    
    // TIM1 PWM模式配置
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_Pulse = 500;  // 占空比50% (CCR值)
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
    
    // 使能TIM1主输出（高级定时器特有）
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
    
    // 初始状态：关闭（不启动定时器）
    Buzzer_Off();
}

void Buzzer_On(void)
{
    TIM_Cmd(TIM1, ENABLE);  // 启动PWM输出
}

void Buzzer_Off(void)
{
    TIM_Cmd(TIM1, DISABLE); // 停止PWM输出
}

void Buzzer_Toggle(void)
{
    if (TIM1->CR1 & TIM_CR1_CEN) // 检查TIM1是否使能
        Buzzer_Off();
    else
        Buzzer_On();
}

void Buzzer_Beep(uint16_t ms)
{
    Buzzer_On();
    Delay_ms(ms);
    Buzzer_Off();
}

void Buzzer_BeepTimes(uint8_t times, uint16_t duration)
{
    uint8_t i;
    for (i = 0; i < times; i++)
    {
        Buzzer_On();
        Delay_ms(duration);
        Buzzer_Off();
        if (i < times - 1)
            Delay_ms(duration);
    }
}

#endif
