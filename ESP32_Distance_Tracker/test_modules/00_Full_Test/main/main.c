/*
 * ESP32 综合测试 - OLED + LED
 * 
 * 硬件连接：
 * OLED:
 * - OLED VCC  -> ESP32 3.3V
 * - OLED GND  -> ESP32 GND
 * - OLED SCL  -> ESP32 GPIO22
 * - OLED SDA  -> ESP32 GPIO21
 * 
 * LED:
 * - LED长脚(正极) -> 220Ω电阻 -> ESP32 GPIO15
 * - LED短脚(负极) -> ESP32 GND
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "ssd1306.h"

#define TAG "FULL_TEST"

// OLED引脚定义
#define SDA_GPIO  21
#define SCL_GPIO  22
#define RESET_GPIO  -1

// LED引脚定义
#define LED_GPIO  15

// OLED参数
#define WIDTH  128
#define HEIGHT 64

void app_main(void)
{
    SSD1306_t dev;
    
    ESP_LOGI(TAG, "=== ESP32综合测试程序 ===");
    ESP_LOGI(TAG, "测试项目: OLED显示 + LED闪烁");
    
    // 1. 配置LED
    ESP_LOGI(TAG, "初始化LED (GPIO%d)...", LED_GPIO);
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level(LED_GPIO, 1);  // LED先亮，表示电源稳定
    ESP_LOGI(TAG, "LED初始化完成，已点亮");
    
    // 延迟1秒，让电源稳定
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 2. 初始化OLED
    ESP_LOGI(TAG, "初始化OLED (SDA=%d, SCL=%d)...", SDA_GPIO, SCL_GPIO);
    i2c_master_init(&dev, SDA_GPIO, SCL_GPIO, RESET_GPIO);
    i2c_init(&dev, WIDTH, HEIGHT);
    ESP_LOGI(TAG, "OLED初始化完成");
    
    // 3. 清屏并显示内容
    ssd1306_clear_screen(&dev, false);
    ssd1306_display_text(&dev, 0, "  ESP32 TEST    ", 16, false);
    ssd1306_display_text(&dev, 2, "  OLED: OK!     ", 16, false);
    ssd1306_display_text(&dev, 3, "  LED : OK!     ", 16, false);
    ssd1306_display_text(&dev, 5, " Power Stable   ", 16, false);
    ssd1306_display_text(&dev, 7, "  All Working!  ", 16, false);
    
    ESP_LOGI(TAG, "OLED显示完成");
    ESP_LOGI(TAG, "==========================");
    ESP_LOGI(TAG, "进入循环：LED每秒闪烁一次");
    
    // 4. 主循环：LED闪烁
    int count = 0;
    while(1) {
        // LED亮
        gpio_set_level(LED_GPIO, 1);
        ESP_LOGI(TAG, "LED ON - 次数: %d", count);
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        // LED灭
        gpio_set_level(LED_GPIO, 0);
        ESP_LOGI(TAG, "LED OFF");
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        count++;
        
        // 每10次更新一次OLED显示的计数
        if (count % 10 == 0) {
            char buf[32];
            snprintf(buf, sizeof(buf), " Count: %d", count);
            ssd1306_display_text(&dev, 7, buf, strlen(buf), false);
        }
    }
}
