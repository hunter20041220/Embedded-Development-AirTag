#include "stm32f10x.h"
#include "OLED.h"
#include "RC522.h"
#include "Delay.h"
#include <stdio.h>
#include <string.h>

// Global debug variable used by DHT22 driver for step debugging
uint8_t debug_step = 0;

int main(void)
{
    char str[32];
    uint8_t uid[5];
    uint8_t status;

    // 初始化 OLED
    OLED_Init();
    OLED_Clear();
    OLED_ShowString(1, 1, "RC522 Reader");

    // 初始化 RC522
    RC522_Init();
    Delay_ms(100);

    while (1)
    {
        // 读取卡片
        status = RC522_Request(PICC_REQIDL, uid);
        if (status == MI_OK)
        {
            // 防冲突，获取完整UID
            status = RC522_Anticoll(uid);
            if (status == MI_OK)
            {
                // 显示卡片UID
                sprintf(str, "Card: %02X%02X%02X%02X", uid[0], uid[1], uid[2], uid[3]);
                OLED_ShowString(2, 1, str);
                
                // 识别两张卡（示例UID，需替换为你的卡实际UID）
                if (uid[0] == 0x12 && uid[1] == 0x34 && uid[2] == 0x56 && uid[3] == 0x78)
                {
                    OLED_ShowString(3, 1, "Card 1         ");
                }
                else if (uid[0] == 0xAA && uid[1] == 0xBB && uid[2] == 0xCC && uid[3] == 0xDD)
                {
                    OLED_ShowString(3, 1, "Card 2         ");
                }
                else
                {
                    OLED_ShowString(3, 1, "Unknown Card   ");
                }
                
                Delay_ms(500); // 防抖延时
            }
        }
        else
        {
            OLED_ShowString(2, 1, "No Card        ");
            OLED_ShowString(3, 1, "               ");
        }
        
        Delay_ms(100);
    }
}

