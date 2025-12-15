/*
 * ESP32 LED闪烁测试
 * 
 * 硬件连接：
 * - LED长脚(正极) -> 限流电阻(220Ω或330Ω) -> ESP32 GPIO15
 * - LED短脚(负极) -> ESP32 GND
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define TAG "LED_TEST"

// LED引脚定义
#define LED_GPIO  15  // 使用GPIO15控制LED

void app_main(void)
{
    ESP_LOGI(TAG, "LED测试程序启动");
    ESP_LOGI(TAG, "LED引脚: GPIO%d", LED_GPIO);
    
    // 配置GPIO为输出模式
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    
    ESP_LOGI(TAG, "GPIO配置完成，开始闪烁...");
    
    int count = 0;
    while(1) {
        // LED亮
        gpio_set_level(LED_GPIO, 1);
        ESP_LOGI(TAG, "LED ON - 次数: %d", count);
        vTaskDelay(pdMS_TO_TICKS(500));  // 延时500ms
        
        // LED灭
        gpio_set_level(LED_GPIO, 0);
        ESP_LOGI(TAG, "LED OFF");
        vTaskDelay(pdMS_TO_TICKS(500));  // 延时500ms
        
        count++;
    }
}
