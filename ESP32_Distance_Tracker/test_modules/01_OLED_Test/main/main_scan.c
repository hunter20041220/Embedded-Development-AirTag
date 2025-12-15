/*
 * ESP32 I2C扫描器 - 检测OLED地址
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c_master.h"

#define TAG "I2C_SCAN"

#define SDA_GPIO  21
#define SCL_GPIO  22

void app_main(void)
{
    ESP_LOGI(TAG, "开始I2C扫描...");
    ESP_LOGI(TAG, "SDA=GPIO%d, SCL=GPIO%d", SDA_GPIO, SCL_GPIO);
    
    // 配置I2C总线
    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = SCL_GPIO,
        .sda_io_num = SDA_GPIO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    
    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));
    
    ESP_LOGI(TAG, "扫描I2C地址 0x00-0x7F...");
    ESP_LOGI(TAG, "");
    
    int found = 0;
    for (uint8_t addr = 0x00; addr < 0x80; addr++) {
        // 尝试探测设备
        esp_err_t ret = i2c_master_probe(bus_handle, addr, 1000);
        
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "✓ 找到设备！地址: 0x%02X", addr);
            found++;
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    ESP_LOGI(TAG, "");
    if (found == 0) {
        ESP_LOGE(TAG, "❌ 未找到任何I2C设备！");
        ESP_LOGE(TAG, "请检查：");
        ESP_LOGE(TAG, "  1. 接线是否正确");
        ESP_LOGE(TAG, "  2. OLED是否已上电");
        ESP_LOGE(TAG, "  3. 杜邦线接触是否良好");
    } else {
        ESP_LOGI(TAG, "✓ 扫描完成，找到 %d 个设备", found);
    }
    
    // 保持运行
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(5000));
        ESP_LOGI(TAG, "程序运行中...");
    }
}
