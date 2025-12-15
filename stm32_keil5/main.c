#include "stm32f10x.h"
#include "OLED.h" // 引用之前的头文件

void Delay_Test(uint32_t ms) {
    uint32_t i, j;
    for(i=0; i<ms; i++) for(j=0; j<7000; j++); 
}

// 手动写满全屏的函数 (直接操作像素)
void OLED_Fill(uint8_t data) {
    uint8_t i, n;
    for(i=0; i<8; i++) {
        OLED_WriteCommand(0xB0 + i); // 设置页地址
        OLED_WriteCommand(0x00);     // 设置列低地址
        OLED_WriteCommand(0x10);     // 设置列高地址
        for(n=0; n<128; n++) {
            OLED_WriteData(data);    // 写入数据
        }
    }
}

int main(void) {
    OLED_Init();
    OLED_Clear();
    
    // 在第1行第1列显示 "hello"
    OLED_ShowString(1, 1, "hello");
    
    while(1) {
        // 显示完成后保持
    }
}