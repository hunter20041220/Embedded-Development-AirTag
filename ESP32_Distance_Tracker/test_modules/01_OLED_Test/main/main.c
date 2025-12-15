/*
 * ESP32 OLED 显示 Hello World
 * 硬件连接：
 * - OLED VCC  -> ESP32 3.3V
 * - OLED GND  -> ESP32 GND
 * - OLED SCL  -> ESP32 GPIO22
 * - OLED SDA  -> ESP32 GPIO21
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ssd1306.h"

#define TAG "OLED"

// I2C引脚定义
#define SDA_GPIO  21
#define SCL_GPIO  22
#define RESET_GPIO  -1  // 无复位引脚

// OLED参数
#define WIDTH  128
#define HEIGHT 64

void app_main(void)
{
    SSD1306_t dev;
    
    ESP_LOGI(TAG, "开始初始化OLED...");
    
    // 初始化I2C和OLED
    i2c_master_init(&dev, SDA_GPIO, SCL_GPIO, RESET_GPIO);
    i2c_init(&dev, WIDTH, HEIGHT);
    
    ESP_LOGI(TAG, "OLED初始化完成");
    
    // 清屏
    ssd1306_clear_screen(&dev, false);
    
    // 显示Hello World
    ssd1306_display_text(&dev, 0, "  Hello World!  ", 16, false);
    ssd1306_display_text(&dev, 2, "  ESP32 + OLED  ", 16, false);
    ssd1306_display_text(&dev, 4, "   I2C @ 0x3C   ", 16, false);
    ssd1306_display_text(&dev, 6, "   Working! :)  ", 16, false);
    
    ESP_LOGI(TAG, "Hello World已显示到OLED屏幕");
    
    // 保持运行
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
