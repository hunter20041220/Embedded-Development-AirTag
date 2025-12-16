#include "LED.h"

/* LED接线: PA15 -> 330Ω -> LED正极 -> LED负极 -> GND */

void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // ⚠️ 重要：禁用JTAG以释放PA15引脚 (保留SWD调试功能)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); // 禁用JTAG，保留SWD
    
    // 开启GPIOA时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    // PA15配置为推挽输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; // LED不需要高速
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 初始状态：熄灭
    LED_Off();
}

void LED_On(void)
{
    GPIO_SetBits(GPIOA, GPIO_Pin_15);   // 高电平点亮 (标准接法：PA15 -> 电阻 -> LED+ -> LED- -> GND)
}

void LED_Off(void)
{
    GPIO_ResetBits(GPIOA, GPIO_Pin_15); // 低电平熄灭
}

void LED_Toggle(void)
{
    if (GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_15))
    {
        LED_On();  // 当前是高电平(灭)，点亮
    }
    else
    {
        LED_Off(); // 当前是低电平(亮)，熄灭
    }
}

void LED_Set(uint8_t state)
{
    if (state)
        LED_On();
    else
        LED_Off();
}
