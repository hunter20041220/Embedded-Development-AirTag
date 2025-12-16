/* 备份的DHT22测试程序 - 已禁用编译
 * 如需使用此测试，请将 main.c 改名或移除
 */

#if 0  // 禁用此文件编译

#include "stm32f10x.h"
#include "OLED.h"
#include "DHT22.h"
#include "Delay.h"
#include <stdio.h>

/* DHT22温湿度+OLED显示程序
 * 
 * DHT22接线:
 *   VCC  -> 3.3V
 *   DATA -> PB10 (必须加4.7K上拉电阻到VCC)
 *   GND  -> GND
 * 
 * OLED接线:
 *   VCC -> 3.3V
 *   GND -> GND
 *   SCL -> PB6
 *   SDA -> PB7
 */

uint8_t debug_step = 0;  // 全局变量记录调试步骤

int main(void)
{
    DHT22_Data dht_data;
    char str[20];
    uint8_t count = 0;
    
    // 初始化
    OLED_Init();
    OLED_Clear();
    OLED_ShowString(1, 1, "DHT22 Test");
    OLED_ShowString(2, 1, "Init...");
    
    DHT22_Init();
    Delay_ms(2000);  // 等待DHT22稳定
    
    OLED_Clear();
    OLED_ShowString(1, 1, "Reading...");
    
    while(1)
    {
        count++;
        debug_step = 0;
        
        // 读取DHT22数据
        if (DHT22_Read(&dht_data))
        {
            // 读取成功
            OLED_Clear();
            sprintf(str, "T:%.1fC", dht_data.temperature);
            OLED_ShowString(1, 1, str);
            
            sprintf(str, "H:%.1f%%", dht_data.humidity);
            OLED_ShowString(2, 1, str);
            
            sprintf(str, "OK %d", count);
            OLED_ShowString(3, 1, str);
        }
        else
        {
            // 读取失败，显示失败在哪一步
            sprintf(str, "Fail:%d", count);
            OLED_ShowString(2, 1, str);
            
            sprintf(str, "Step:%d", debug_step);
            OLED_ShowString(3, 1, str);
            
            OLED_ShowString(4, 1, "Check DHT22");
        }
        
        // DHT22采样间隔至少2秒
        Delay_ms(2000);
    }
}

#endif  // 结束禁用