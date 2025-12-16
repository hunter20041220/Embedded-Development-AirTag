#include "Delay.h"

/* 延时函数 (基于72MHz系统时钟) */

void Delay_us(uint32_t us)
{
    uint32_t i;
    // 72MHz系统时钟
    // 实测调整系数为10（考虑编译优化和循环开销）
    for (i = 0; i < us * 10; i++)
    {
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
    }
}

void Delay_ms(uint32_t ms)
{
    uint32_t i, j;
    for (i = 0; i < ms; i++)
    {
        for (j = 0; j < 7200; j++) // 72MHz / 10000 ≈ 7200
        {
            __NOP();
        }
    }
}

void Delay_s(uint32_t s)
{
    uint32_t i;
    for (i = 0; i < s; i++)
    {
        Delay_ms(1000);
    }
}
