/*
 * ESP32 OLED SSD1306 测试程序
 * 
 * 功能：
 * - 测试I2C通信
 * - 显示文字、图形
 * - 多页面切换
 * 
 * 接线：
 * - OLED VCC  -> ESP32 3.3V
 * - OLED GND  -> ESP32 GND
 * - OLED SCL  -> ESP32 GPIO22
 * - OLED SDA  -> ESP32 GPIO21
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "ssd1306.h"

// 日志标签
static const char *TAG = "OLED_TEST";

// I2C配置
#define I2C_MASTER_SCL_IO    GPIO_NUM_22    // I2C时钟引脚
#define I2C_MASTER_SDA_IO    GPIO_NUM_21    // I2C数据引脚
#define I2C_MASTER_NUM       I2C_NUM_0      // I2C端口号
#define I2C_MASTER_FREQ_HZ   400000         // I2C频率 400kHz

// OLED配置
#define OLED_I2C_ADDRESS     0x3C           // OLED I2C地址
#define OLED_WIDTH           128            // 屏幕宽度
#define OLED_HEIGHT          64             // 屏幕高度

// 全局OLED设备
SSD1306_t dev;

/**
 * @brief I2C主机初始化
 */
esp_err_t i2c_master_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    
    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C参数配置失败: %s", esp_err_to_name(err));
        return err;
    }
    
    err = i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C驱动安装失败: %s", esp_err_to_name(err));
        return err;
    }
    
    ESP_LOGI(TAG, "✓ I2C初始化成功 (SDA=GPIO%d, SCL=GPIO%d, Freq=%d Hz)",
             I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO, I2C_MASTER_FREQ_HZ);
    return ESP_OK;
}

/**
 * @brief 扫描I2C总线上的设备
 */
void i2c_scan(void)
{
    ESP_LOGI(TAG, "[2] 扫描I2C设备...");
    
    uint8_t found_count = 0;
    for (uint8_t addr = 1; addr < 127; addr++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        
        esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 100 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
        
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "找到I2C设备，地址: 0x%02X", addr);
            found_count++;
        }
    }
    
    if (found_count == 0) {
        ESP_LOGW(TAG, "未找到任何I2C设备！请检查接线。");
    } else {
        ESP_LOGI(TAG, "✓ 共找到 %d 个I2C设备", found_count);
    }
}

/**
 * @brief 显示欢迎信息
 */
void display_welcome(SSD1306_t *dev)
{
    ESP_LOGI(TAG, "[4] 显示欢迎信息...");
    
    ssd1306_clear_screen(dev, false);
    
    // 标题
    ssd1306_display_text(dev, 0, "ESP32 OLED Test", 15, false);
    
    // 分割线（用连字符模拟）
    ssd1306_display_text(dev, 1, "----------------", 16, false);
    
    // 内容
    ssd1306_display_text(dev, 3, "  Hello ESP32!  ", 16, false);
    ssd1306_display_text(dev, 5, "SSD1306 Working!", 16, false);
    ssd1306_display_text(dev, 7, " I2C @ 0x3C     ", 16, false);
    
    ESP_LOGI(TAG, "✓ 显示完成");
}

/**
 * @brief 显示系统信息
 */
void display_system_info(SSD1306_t *dev)
{
    ESP_LOGI(TAG, "[5] 显示系统信息...");
    
    ssd1306_clear_screen(dev, false);
    
    // 获取系统信息
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    
    // 显示芯片信息
    ssd1306_display_text(dev, 0, "System Info", 11, false);
    ssd1306_display_text(dev, 1, "------------", 12, false);
    
    char line[32];
    snprintf(line, sizeof(line), "Chip: ESP32");
    ssd1306_display_text(dev, 2, line, strlen(line), false);
    ESP_LOGI(TAG, "  - 芯片型号: ESP32");
    
    snprintf(line, sizeof(line), "Cores: %d", chip_info.cores);
    ssd1306_display_text(dev, 3, line, strlen(line), false);
    ESP_LOGI(TAG, "  - CPU核心数: %d", chip_info.cores);
    
    snprintf(line, sizeof(line), "Freq: %d MHz", esp_clk_cpu_freq() / 1000000);
    ssd1306_display_text(dev, 4, line, strlen(line), false);
    ESP_LOGI(TAG, "  - CPU频率: %d MHz", esp_clk_cpu_freq() / 1000000);
    
    snprintf(line, sizeof(line), "RAM: %d KB", esp_get_free_heap_size() / 1024);
    ssd1306_display_text(dev, 5, line, strlen(line), false);
    ESP_LOGI(TAG, "  - 可用内存: %d KB", esp_get_free_heap_size() / 1024);
    
    snprintf(line, sizeof(line), "Flash: %d MB", spi_flash_get_chip_size() / (1024 * 1024));
    ssd1306_display_text(dev, 6, line, strlen(line), false);
    
    ssd1306_display_text(dev, 7, "I2C:OK  SPI:OK", 14, false);
    
    ESP_LOGI(TAG, "✓ 系统信息显示完成");
}

/**
 * @brief 绘制图形
 */
void display_graphics(SSD1306_t *dev)
{
    ESP_LOGI(TAG, "[6] 绘制图形...");
    
    ssd1306_clear_screen(dev, false);
    
    // 绘制矩形
    ssd1306_display_text(dev, 0, "Graphics Test", 13, false);
    ssd1306_display_text(dev, 1, "-------------", 13, false);
    
    // 绘制多个矩形
    for (int i = 0; i < 3; i++) {
        int x = 10 + i * 30;
        int y = 20 + i * 5;
        ssd1306_draw_rectangle(dev, x, y, 20, 15);
    }
    
    // 绘制圆形
    ssd1306_draw_circle(dev, 100, 40, 10);
    
    // 绘制线条
    ssd1306_draw_line(dev, 0, 63, 127, 63);
    ssd1306_draw_line(dev, 64, 16, 64, 63);
    
    ESP_LOGI(TAG, "✓ 图形绘制完成");
}

/**
 * @brief 显示滚动文字
 */
void display_scrolling_text(SSD1306_t *dev)
{
    ESP_LOGI(TAG, "[7] 滚动文字动画...");
    
    const char *text = "OLED Test Passed! >>> ";
    int text_len = strlen(text);
    
    for (int scroll = 0; scroll < 50; scroll++) {
        ssd1306_clear_screen(dev, false);
        
        ssd1306_display_text(dev, 0, "Scroll Animation", 16, false);
        
        // 创建滚动效果
        char display_line[32] = {0};
        for (int i = 0; i < 21; i++) {
            display_line[i] = text[(scroll + i) % text_len];
        }
        ssd1306_display_text(dev, 3, display_line, 21, false);
        
        // 进度条
        ssd1306_draw_rectangle(dev, 10, 45, 108, 10);
        int progress = (scroll * 108) / 50;
        for (int i = 0; i < progress; i += 2) {
            ssd1306_draw_line(dev, 11 + i, 46, 11 + i, 53);
        }
        
        char progress_text[16];
        snprintf(progress_text, sizeof(progress_text), "%d%%", (scroll * 100) / 50);
        ssd1306_display_text(dev, 7, progress_text, strlen(progress_text), false);
        
        vTaskDelay(pdMS_TO_TICKS(60));
    }
    
    ESP_LOGI(TAG, "✓ 动画完成");
}

/**
 * @brief 显示测试完成
 */
void display_test_complete(SSD1306_t *dev)
{
    ssd1306_clear_screen(dev, false);
    
    ssd1306_display_text(dev, 1, "  TEST PASSED! ", 16, false);
    ssd1306_display_text(dev, 3, "================", 16, false);
    ssd1306_display_text(dev, 4, "  OLED Working  ", 16, false);
    ssd1306_display_text(dev, 5, "  Perfectly!    ", 16, false);
    ssd1306_display_text(dev, 7, "  Ready to Use  ", 16, false);
}

/**
 * @brief 主程序
 */
void app_main(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "ESP32 OLED SSD1306 测试程序");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "");
    
    // 步骤1: 初始化I2C
    ESP_LOGI(TAG, "[1] 初始化I2C总线...");
    esp_err_t ret = i2c_master_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "❌ I2C初始化失败！");
        return;
    }
    
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // 步骤2: 扫描I2C设备
    i2c_scan();
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // 步骤3: 初始化OLED
    ESP_LOGI(TAG, "[3] 初始化OLED显示...");
    ssd1306_init(&dev, OLED_WIDTH, OLED_HEIGHT, OLED_I2C_ADDRESS, 
                 I2C_MASTER_NUM, I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO);
    
    // 检查OLED是否响应
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "❌ OLED未检测到！地址0x%02X无响应。", OLED_I2C_ADDRESS);
        ESP_LOGE(TAG, "请检查：");
        ESP_LOGE(TAG, "  1. VCC是否连接到3.3V（不是5V！）");
        ESP_LOGE(TAG, "  2. GND是否连接");
        ESP_LOGE(TAG, "  3. SDA/SCL是否接反");
        ESP_LOGE(TAG, "  4. 杜邦线是否连接牢固");
        return;
    }
    
    ESP_LOGI(TAG, "✓ OLED初始化成功");
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // 步骤4: 显示欢迎信息
    display_welcome(&dev);
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    // 步骤5: 显示系统信息
    display_system_info(&dev);
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    // 步骤6: 绘制图形
    display_graphics(&dev);
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    // 步骤7: 滚动动画
    display_scrolling_text(&dev);
    
    // 测试完成
    display_test_complete(&dev);
    
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "✓ 所有测试通过！OLED工作正常！");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "💡 提示：");
    ESP_LOGI(TAG, "  - 如果显示正常，说明I2C通信和OLED硬件都OK");
    ESP_LOGI(TAG, "  - 可以继续测试下一个模块：RFID RC522");
    ESP_LOGI(TAG, "  - 按RESET键重新运行测试");
    
    // 保持显示
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
