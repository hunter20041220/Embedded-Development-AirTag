#include "HCSR501.h"
#include "stm32f10x.h"

// HCSR501 PIR sensor connected to PB11 (OUT pin)
#define PIR_PORT    GPIOB
#define PIR_PIN     GPIO_Pin_11
#define PIR_RCC     RCC_APB2Periph_GPIOB

void HCSR501_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // Enable GPIOB clock
    RCC_APB2PeriphClockCmd(PIR_RCC, ENABLE);
    
    // Configure PB11 as floating input (or pull-down if module needs it)
    GPIO_InitStructure.GPIO_Pin = PIR_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; // Input pull-down
    GPIO_Init(PIR_PORT, &GPIO_InitStructure);
}

uint8_t HCSR501_ReadMotion(void)
{
    // Read PB11: HIGH (1) = motion detected, LOW (0) = no motion
    return (GPIO_ReadInputDataBit(PIR_PORT, PIR_PIN) == Bit_SET) ? 1 : 0;
}
