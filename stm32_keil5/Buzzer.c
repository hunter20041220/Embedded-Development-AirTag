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
// ========== 音乐播放功能 ==========
// 有源蜂鸣器无法播放旋律，使用简单的beep代替
void Buzzer_PlaySeeYouAgain(void)
{
    // 用节奏模拟旋律
    Buzzer_BeepTimes(3, 200);  // 三声短响
    Delay_ms(300);
    Buzzer_BeepTimes(2, 400);  // 两声长响
    Delay_ms(300);
    Buzzer_BeepTimes(3, 200);  // 三声短响
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

// ========== 音乐播放功能（通用） ==========

#if BUZZER_TYPE == BUZZER_TYPE_PASSIVE
// 音符频率定义 (Hz)
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784

// 设置蜂鸣器频率
static void Buzzer_SetFrequency(uint16_t freq)
{
    uint16_t period;
    if (freq == 0) {
        TIM_Cmd(TIM1, DISABLE);
        return;
    }
    
    // 计算ARR值: 72MHz / (预分频+1) / (ARR+1) = 目标频率
    // 使用预分频72，则计数频率为1MHz
    period = 1000000 / freq - 1;
    
    TIM_SetAutoreload(TIM1, period);
    TIM_SetCompare1(TIM1, period / 2);  // 50%占空比
    TIM_Cmd(TIM1, ENABLE);
}

// 播放See You Again旋律片段
void Buzzer_PlaySeeYouAgain(void)
{
    // See You Again主旋律片段 (简化版)
    // 节奏: 1 = 四分音符(500ms), 0.5 = 八分音符(250ms), 2 = 二分音符(1000ms)
    
    typedef struct {
        uint16_t freq;      // 音符频率
        uint16_t duration;  // 持续时间(ms)
    } Note;
    
    Note melody[] = {
        {NOTE_G4, 500},  // It's been a
        {NOTE_G4, 250},  // long
        {NOTE_A4, 250},  // day
        {NOTE_G4, 500},  // with-
        {NOTE_E4, 500},  // out
        {NOTE_D4, 500},  // you
        {NOTE_C4, 1000}, // friend
        {0, 200},        // 休止
        
        {NOTE_G4, 500},  // And I'll
        {NOTE_G4, 250},  // tell
        {NOTE_A4, 250},  // you
        {NOTE_G4, 500},  // all
        {NOTE_E4, 500},  // a-
        {NOTE_D4, 500},  // bout
        {NOTE_E4, 1000}, // it
        {0, 200},        // 休止
        
        {NOTE_G4, 500},  // When I
        {NOTE_A4, 500},  // see
        {NOTE_G4, 500},  // you
        {NOTE_E4, 500},  // a-
        {NOTE_D4, 1500}, // gain
        {0, 0}           // 结束
    };
    
    uint8_t i = 0;
    while (melody[i].freq != 0 || melody[i].duration != 0)
    {
        if (melody[i].freq == 0) {
            // 休止符
            TIM_Cmd(TIM1, DISABLE);
  
    // 用节奏模拟旋律
    Buzzer_BeepTimes(3, 200);  // 三声短响
    Delay_ms(300);
    Buzzer_BeepTimes(2, 400);  // 两声长响
    Delay_ms(300);
    Buzzer_BeepTimes(3, 200);  // 三声短响
}
#endif

#endif
