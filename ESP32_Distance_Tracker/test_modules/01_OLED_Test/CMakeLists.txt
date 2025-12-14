# OLED SSD1306测试项目配置idf_component_register(












project(${PROJECT_NAME})# 定义项目include($ENV{IDF_PATH}/tools/cmake/project.cmake)# 包含ESP-IDF构建系统set(PROJECT_NAME "01_OLED_Test")# 设置项目名称cmake_minimum_required(VERSION 3.16)    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES driver ssd1306
)
