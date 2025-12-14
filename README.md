# OpenHaystack 智能AirTag防丢器 - 从零开始开发教程

## 📋 目录

- [一、项目简介](#一项目简介)
- [二、硬件准备清单](#二硬件准备清单)
- [三、软件环境搭建](#三软件环境搭建)
- [四、开发路线图](#四开发路线图)
- [五、详细开发步骤](#五详细开发步骤)
- [六、常见问题解决](#六常见问题解决)

---

## 一、项目简介

### 1.1 什么是这个项目？

这是一个基于 **OpenHaystack** 的增强型智能防丢器项目，结合了：
- 🌍 **全球定位**：利用 Apple Find My 网络（借助附近的 iPhone）
- 📏 **近距离监测**：实时显示与手机的距离
- 🔔 **智能报警**：超出安全距离自动蜂鸣
- 📺 **OLED显示**：实时显示状态信息
- 🎫 **RFID识别**：刷卡解除报警

### 1.2 核心功能

| 功能 | 描述 | 应用场景 |
|-----|------|---------|
| Find My 定位 | 全球范围找回丢失物品 | 钥匙丢在咖啡厅，2小时后查看位置 |
| BLE 距离测量 | 实时计算与手机距离 | 随身包超出10米自动提醒 |
| 蜂鸣报警 | 超距/震动/暗光触发 | 包被移动或环境变暗时报警 |
| OLED 显示 | 显示距离、方位、状态 | 查看实时信息无需手机 |
| RFID 管理 | 刷卡解除报警 | 快速静音，防误触 |

---

## 二、硬件准备清单

### 2.1 必备硬件

| 序号 | 名称 | 规格型号 | 数量 | 价格参考 | 用途 |
|-----|------|---------|-----|---------|------|
| 1 | **ESP32 开发板** | ESP32-WROOM-32 | 1 | ¥20-30 | 主控制器（BLE+WiFi） |
| 2 | **OLED 显示屏** | 0.96寸 128x64 I2C (SSD1306) | 1 | ¥10-15 | 显示距离/状态 |
| 3 | **RFID 读卡器** | RC522 模块 + S50卡片 | 1套 | ¥8-12 | 刷卡解除报警 |
| 4 | **加速度传感器** | MPU6050 (I2C) | 1 | ¥8-12 | 震动检测 |
| 5 | **光照传感器** | BH1750 (I2C) | 1 | ¥5-8 | 环境光检测 |
| 6 | **蜂鸣器** | 有源/无源蜂鸣器 5V | 1 | ¥2-5 | 声音报警 |
| 7 | **LED 灯** | 红/绿/蓝 3mm LED | 各1 | ¥1 | 状态指示 |
| 8 | **按键开关** | 轻触按键 6x6mm | 2-3个 | ¥1 | 模式切换/静音 |
| 9 | **电阻** | 220Ω、10KΩ | 若干 | ¥1 | LED限流、上拉 |
| 10 | **面包板** | 830孔 | 1 | ¥5 | 原型搭建 |
| 11 | **杜邦线** | 公对公/公对母/母对母 | 各20根 | ¥5-10 | 连线 |
| 12 | **USB数据线** | Micro USB / Type-C | 1 | ¥5 | 烧录固件 |

**总预算**：约 ¥70-110

### 2.2 可选硬件（增强功能）

| 名称 | 规格 | 价格 | 用途 |
|-----|------|-----|------|
| 锂电池 + 充电模块 | 3.7V 1000mAh + TP4056 | ¥15 | 便携供电 |
| 外壳 | 3D打印/亚克力 | ¥10-30 | 封装保护 |
| GPS 模块 | NEO-6M (可选) | ¥20 | 本地GPS定位 |

### 2.3 接口引脚规划表（ESP32）

| 模块 | 接口类型 | ESP32引脚 | 说明 |
|-----|---------|----------|------|
| **OLED SSD1306** | I2C | SDA=GPIO21, SCL=GPIO22 | 与其他I2C设备共享 |
| **MPU6050** | I2C | SDA=GPIO21, SCL=GPIO22 | 共享I2C总线 |
| **BH1750** | I2C | SDA=GPIO21, SCL=GPIO22 | 共享I2C总线 |
| **RC522 RFID** | SPI | MOSI=GPIO23, MISO=GPIO19, SCK=GPIO18, SDA(CS)=GPIO5, RST=GPIO4 | 独立SPI |
| **蜂鸣器** | GPIO | GPIO25 | PWM控制音调 |
| **LED红** | GPIO | GPIO32 | 报警指示 |
| **LED绿** | GPIO | GPIO33 | 正常指示 |
| **LED蓝** | GPIO | GPIO27 | BLE连接指示 |
| **按键1** | GPIO | GPIO0 (BOOT) | 模式切换 |
| **按键2** | GPIO | GPIO34 (INPUT_ONLY) | 静音/确认 |
| **串口调试** | UART0 | TX=GPIO1, RX=GPIO3 | USB串口 |

> **注意**：GPIO34-39 只能作为输入，不能配置内部上拉电阻

---

## 三、软件环境搭建

### 3.1 开发环境总览

| 工具 | 用途 | 系统要求 |
|-----|------|---------|
| **macOS 电脑** | 运行 OpenHaystack 客户端 | macOS 11+ (Big Sur或更高) |
| **Windows/Linux 电脑** | ESP32 固件开发 | Windows 10+ / Ubuntu 20.04+ |
| **ESP-IDF** | ESP32 开发框架 | v4.2 或 v4.4 |
| **Python** | 烧录工具 | Python 3.7+ |
| **VS Code + PlatformIO** | IDE（可选） | 推荐使用 |

---

### 3.2 macOS 端环境（OpenHaystack 客户端）

#### 步骤 1：安装 Xcode
```bash
# 从 App Store 安装 Xcode（约12GB）
# 安装命令行工具
xcode-select --install
```

#### 步骤 2：下载 OpenHaystack
```bash
# 克隆项目
cd ~/Documents
git clone https://github.com/seemoo-lab/openhaystack.git
cd openhaystack
```

#### 步骤 3：编译 OpenHaystack 应用
```bash
# 打开 Xcode 项目
open OpenHaystack/OpenHaystack.xcodeproj

# 在 Xcode 中：
# 1. 选择菜单 Product → Build (⌘B)
# 2. 等待编译完成（首次需要5-10分钟）
```

#### 步骤 4：安装 Mail 插件
```bash
# 临时禁用 Gatekeeper
sudo spctl --master-disable

# 打开 OpenHaystack 应用（会自动安装插件）
open ~/Library/Developer/Xcode/DerivedData/OpenHaystack-xxx/Build/Products/Debug/OpenHaystack.app

# 打开 Mail → 偏好设置 → 通用 → 管理插件
# 勾选 "OpenHaystackMail.mailbundle"

# 重启 Mail 应用

# 重新启用 Gatekeeper
sudo spctl --master-enable
```

> **如果"管理插件"按钮不显示**：
> ```bash
> sudo defaults write "/Library/Preferences/com.apple.mail" EnableBundles 1
> ```

---

### 3.3 Windows/Linux 端环境（ESP32 固件开发）

#### 方案 A：使用 ESP-IDF（官方推荐）

**Windows 安装**：
```powershell
# 1. 下载 ESP-IDF 离线安装器
# https://dl.espressif.com/dl/esp-idf/?idf=4.4

# 2. 运行安装器，选择：
#    - ESP-IDF v4.4
#    - 安装路径：C:\esp-idf
#    - 勾选 "Add to PATH"

# 3. 安装完成后，桌面会出现 "ESP-IDF 4.4 CMD" 快捷方式
```

**Linux 安装**：
```bash
# 安装依赖
sudo apt-get install git wget flex bison gperf python3 python3-pip python3-venv \
  cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0

# 克隆 ESP-IDF
mkdir -p ~/esp
cd ~/esp
git clone -b v4.4 --recursive https://github.com/espressif/esp-idf.git

# 安装工具链
cd esp-idf
./install.sh esp32

# 设置环境变量（添加到 ~/.bashrc）
echo 'alias get_idf=". $HOME/esp/esp-idf/export.sh"' >> ~/.bashrc
source ~/.bashrc
```

**测试安装**：
```bash
# Windows: 打开 "ESP-IDF 4.4 CMD"
# Linux: 先运行 get_idf

# 检查版本
idf.py --version
# 应显示：ESP-IDF v4.4.x
```

#### 方案 B：使用 PlatformIO（更简单）

```bash
# 1. 安装 VS Code
# https://code.visualstudio.com/

# 2. 安装 PlatformIO 插件
# 在 VS Code 中：Extensions → 搜索 "PlatformIO IDE" → Install

# 3. 重启 VS Code，等待 PlatformIO 自动安装工具链（约5-10分钟）
```

---

### 3.4 驱动安装

#### ESP32 串口驱动

**检测芯片型号**：
- 看开发板上的 USB 转串口芯片（通常是 CP2102 或 CH340）

**Windows 驱动下载**：
- **CP2102**：https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers
- **CH340**：http://www.wch.cn/downloads/CH341SER_EXE.html

**Linux 驱动**：
```bash
# CP2102（一般已内置）
lsmod | grep cp210x

# CH340
sudo apt-get install linux-headers-$(uname -r)
# 重启后自动加载

# 添加用户到串口组
sudo usermod -a -G dialout $USER
# 注销重新登录生效
```

**测试连接**：
```bash
# Windows (PowerShell)
Get-PnpDevice -Class Ports

# Linux
ls /dev/ttyUSB*
# 应显示：/dev/ttyUSB0
```

---

### 3.5 Python 环境配置

```bash
# 检查 Python 版本
python --version
# 或
python3 --version
# 需要 3.7 以上

# 安装必要的包
pip install esptool pyserial

# 测试 esptool
esptool.py --help
```

---

## 四、开发路线图

### 阶段总览

```mermaid
graph LR
    A[阶段1: 环境搭建] --> B[阶段2: 硬件连接测试]
    B --> C[阶段3: Find My 基础功能]
    C --> D[阶段4: 传感器集成]
    D --> E[阶段5: 增强功能开发]
    E --> F[阶段6: 系统集成测试]
```

---

### 🎯 阶段 1：环境搭建与工具验证（1天）

**目标**：确保所有软硬件环境正常工作

| 任务 | 验收标准 | 时间 |
|-----|---------|------|
| 安装 macOS OpenHaystack | 应用能正常打开，Mail 插件激活 | 2h |
| 安装 ESP-IDF / PlatformIO | `idf.py --version` 正常显示 | 2h |
| 驱动安装与测试 | 设备管理器/`ls /dev` 能识别 ESP32 | 1h |
| 烧录 Blink 示例 | LED 闪烁 | 1h |

**验收代码**（测试 ESP32）：
```c
// 在 ESP-IDF 项目中创建 main/blink.c
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LED_PIN GPIO_NUM_2

void app_main() {
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    
    while(1) {
        gpio_set_level(LED_PIN, 1);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        gpio_set_level(LED_PIN, 0);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
```

**烧录命令**：
```bash
# ESP-IDF
cd Firmware/ESP32
idf.py -p COM3 flash monitor  # Windows
idf.py -p /dev/ttyUSB0 flash monitor  # Linux

# 按 Ctrl+] 退出监视器
```

---

### 🎯 阶段 2：硬件模块逐个测试（2天）

**目标**：验证每个硬件模块能独立工作

#### 2.1 OLED 显示测试（2h）

**接线**：
```
OLED VCC  → ESP32 3.3V
OLED GND  → ESP32 GND
OLED SDA  → ESP32 GPIO21
OLED SCL  → ESP32 GPIO22
```

**测试代码**（需要安装 ESP-IDF 的 SSD1306 库）：
```bash
# 安装组件
cd ~/esp/esp-idf/components
git clone https://github.com/nopnop2002/esp-idf-ssd1306.git ssd1306
```

**验收标准**：OLED 显示 "Hello ESP32"

#### 2.2 RFID 读卡测试（2h）

**接线**：
```
RC522 VCC  → ESP32 3.3V
RC522 GND  → ESP32 GND
RC522 MOSI → ESP32 GPIO23
RC522 MISO → ESP32 GPIO19
RC522 SCK  → ESP32 GPIO18
RC522 SDA  → ESP32 GPIO5
RC522 RST  → ESP32 GPIO4
```

**测试代码**（使用 arduino-esp32 的 MFRC522 库）：
```c
// 安装库：pio lib install "miguelbalboa/MFRC522"
// 刷卡后串口输出卡号
```

**验收标准**：刷卡时串口打印 UID

#### 2.3 传感器测试（2h）

**MPU6050 加速度计**：
- 检测到震动时 LED 亮起
  
**BH1750 光照传感器**：
- 遮挡后串口显示亮度值下降

#### 2.4 蜂鸣器与 LED 测试（1h）

**接线**：
```
蜂鸣器 +  → ESP32 GPIO25
蜂鸣器 -  → GND

LED 红正极 → 220Ω → GPIO32
LED 负极   → GND
（绿、蓝类似）
```

**验收标准**：蜂鸣器能发出不同音调，LED 能控制开关

---

### 🎯 阶段 3：OpenHaystack Find My 功能（1天）

**目标**：实现基础的 Find My 网络定位

#### 3.1 生成密钥对（30min）

**在 macOS OpenHaystack 中**：
1. 打开 OpenHaystack 应用
2. 点击 "+" 添加新配件
3. 名称输入 "ESP32测试"，选择图标
4. 右键点击配件 → "Copy Base64 Public Key"
5. 保存密钥（格式如 `AgEBQ...==`）

#### 3.2 烧录 Find My 固件（1h）

```bash
cd Firmware/ESP32

# 编译固件
idf.py build

# 烧录固件和密钥
./flash_esp32.sh -p /dev/ttyUSB0 "你的Base64密钥"
# Windows: 修改脚本中的端口为 COM3

# 重启设备
esptool.py -p /dev/ttyUSB0 --after hard_reset read_mac
```

#### 3.3 验证定位功能（2h）

**测试步骤**：
1. ESP32 开机后等待 30 秒（开始BLE广播）
2. 用 iPhone 靠近（距离 < 5 米）
3. 等待 10-30 分钟（iPhone 上传位置）
4. 在 macOS OpenHaystack 中点击 "刷新"
5. 地图上应显示 ESP32 位置

**调试命令**：
```bash
# 查看 ESP32 日志
idf.py -p /dev/ttyUSB0 monitor

# 应看到：
# [BLE] Advertising with key: AgEBQ...
# [BLE] Advertisement started
```

**常见问题**：
- ❌ 30分钟后仍无位置 → 检查 iPhone 蓝牙是否开启、是否登录 iCloud
- ❌ 日志显示 BLE 初始化失败 → 检查密钥格式是否正确

---

### 🎯 阶段 4：传感器数据采集与处理（2天）

**目标**：实现多传感器数据融合

#### 4.1 I2C 多设备通信（1天）

**任务清单**：
- [ ] 初始化 I2C 总线（GPIO21/22）
- [ ] 扫描 I2C 设备地址
- [ ] 同时读取 OLED(0x3C)、MPU6050(0x68)、BH1750(0x23)
- [ ] 数据在 OLED 上实时显示

**核心代码框架**：
```c
#include "driver/i2c.h"

#define I2C_MASTER_SCL_IO    22
#define I2C_MASTER_SDA_IO    21
#define I2C_MASTER_FREQ_HZ   100000

void i2c_master_init() {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_NUM_0, &conf);
    i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
}
```

#### 4.2 BLE RSSI 距离测量（1天）

**原理**：通过 BLE 信号强度（RSSI）估算距离

**公式**：
$$
距离(米) = 10^{\frac{RSSI_{测量} - RSSI_{1米}}{10 \times n}}
$$

其中：
- $RSSI_{1米}$：标定值（约 -59 dBm）
- $n$：环境因子（室内取 2.5-3）

**实现步骤**：
1. 在 ESP32 上添加 BLE Server（用于近距离通信）
2. 手机端 App 连接并读取 RSSI
3. 计算距离并显示

---

### 🎯 阶段 5：增强功能开发（3天）

#### 5.1 中断系统（1天）

**中断类型**：
1. **GPIO 中断**（按键）
2. **MPU6050 运动中断**（硬件中断引脚）
3. **定时器中断**（周期扫描）

**示例代码**：
```c
#include "driver/gpio.h"

#define BUTTON_PIN GPIO_NUM_0

void IRAM_ATTR button_isr_handler(void* arg) {
    // 切换模式
    xQueueSendFromISR(button_queue, &gpio_num, NULL);
}

void setup_interrupts() {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config(&io_conf);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, button_isr_handler, NULL);
}
```

#### 5.2 RFID 权限管理（1天）

**功能设计**：
- 授权卡：UID 写入白名单
- 刷授权卡：解除报警
- 未授权卡：记录并拒绝

**数据结构**：
```c
typedef struct {
    uint8_t uid[4];
    char owner[20];
    bool is_authorized;
} rfid_card_t;

rfid_card_t whitelist[10];
```

#### 5.3 多模式状态机（1天）

**模式定义**：
```c
typedef enum {
    MODE_STANDBY,      // 待机（仅 Find My）
    MODE_TRACKING,     // 跟踪（距离监测）
    MODE_ALARM,        // 报警（超距/震动/暗光）
    MODE_SILENT        // 静音（RFID解除）
} system_mode_t;
```

**状态转换逻辑**：
```
待机 → [BLE连接] → 跟踪
跟踪 → [超距] → 报警
报警 → [刷卡] → 静音 → 待机
```

---

### 🎯 阶段 6：系统集成与优化（2天）

#### 6.1 多任务调度（FreeRTOS）

**任务优先级分配**：
```c
xTaskCreate(ble_advertising_task, "BLE_ADV", 4096, NULL, 5, NULL);  // 最高
xTaskCreate(sensor_read_task, "SENSOR", 4096, NULL, 4, NULL);
xTaskCreate(oled_display_task, "OLED", 4096, NULL, 3, NULL);
xTaskCreate(rfid_scan_task, "RFID", 2048, NULL, 2, NULL);
```

#### 6.2 电源管理

**低功耗策略**：
- 待机模式：CPU 降频到 80MHz，关闭 WiFi
- 深度睡眠：保留 BLE 广播，其他外设断电
- 定时唤醒：每 5 秒检测一次传感器

**代码示例**：
```c
#include "esp_pm.h"

void enable_power_save() {
    esp_pm_config_esp32_t pm_config = {
        .max_freq_mhz = 80,
        .min_freq_mhz = 40,
        .light_sleep_enable = true
    };
    esp_pm_configure(&pm_config);
}
```

#### 6.3 综合测试用例

| 测试项 | 操作步骤 | 预期结果 |
|-------|---------|---------|
| Find My 定位 | ESP32 移动到 500 米外 | 30分钟内 macOS 显示新位置 |
| 近距离跟踪 | 手机连接，距离 > 10 米 | OLED 显示距离，蜂鸣器响 |
| 震动报警 | 摇晃 ESP32 | LED 红灯闪烁，蜂鸣 |
| 暗光报警 | 遮挡 BH1750 | 串口输出 "Dark detected" |
| RFID 解除 | 刷授权卡 | 蜂鸣器停止，LED 绿灯亮 |
| 中断响应 | 按下按键 | 模式切换，OLED 显示变化 |

---

## 五、详细开发步骤

### 步骤 1：创建项目结构

```bash
cd ~/Documents/openhaystack/Firmware/ESP32

# 创建增强版项目文件夹
mkdir enhanced-airtag
cd enhanced-airtag

# 初始化 ESP-IDF 项目
idf.py create-project main

# 项目结构
enhanced-airtag/
├── CMakeLists.txt
├── main/
│   ├── CMakeLists.txt
│   ├── main.c
│   ├── ble_handler.c      # BLE 功能
│   ├── sensor_handler.c   # 传感器处理
│   ├── rfid_handler.c     # RFID 逻辑
│   ├── display_handler.c  # OLED 显示
│   └── config.h           # 配置文件
└── components/            # 外部组件
    ├── ssd1306/
    ├── mfrc522/
    └── mpu6050/
```

### 步骤 2：移植 OpenHaystack BLE 代码

**复制原始 Find My 代码**：
```bash
# 复制 openhaystack.c 中的 BLE 广播部分
cp ../main/openhaystack.c main/ble_handler.c

# 提取关键函数：
# - esp_ble_adv_init()
# - esp_ble_adv_start()
# - esp_gap_cb()
```

**修改广播参数**（保留 Find My 兼容性）：
```c
static esp_ble_adv_params_t ble_adv_params = {
    .adv_int_min = 0x0640,  // 1000ms
    .adv_int_max = 0x0C80,  // 2000ms
    .adv_type = ADV_TYPE_NONCONN_IND,
    .own_addr_type = BLE_ADDR_TYPE_RANDOM,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};
```

### 步骤 3：集成传感器库

**安装组件**：
```bash
cd components

# OLED SSD1306
git clone https://github.com/nopnop2002/esp-idf-ssd1306.git ssd1306

# MPU6050
git clone https://github.com/natanaeljr/esp32-MPU6050.git mpu6050

# BH1750
git clone https://github.com/UncleRus/esp-idf-lib.git
cp -r esp-idf-lib/components/bh1750 ./
rm -rf esp-idf-lib

# MFRC522
git clone https://github.com/abobija/esp-idf-rc522.git rc522
```

**在 main/CMakeLists.txt 中注册**：
```cmake
idf_component_register(
    SRCS "main.c" "ble_handler.c" "sensor_handler.c" "rfid_handler.c" "display_handler.c"
    INCLUDE_DIRS "."
    REQUIRES ssd1306 mpu6050 bh1750 rc522
)
```

### 步骤 4：实现核心功能模块

#### 4.1 配置文件（config.h）

```c
#ifndef CONFIG_H
#define CONFIG_H

// 引脚定义
#define PIN_I2C_SDA        21
#define PIN_I2C_SCL        22
#define PIN_SPI_MOSI       23
#define PIN_SPI_MISO       19
#define PIN_SPI_SCK        18
#define PIN_RC522_CS       5
#define PIN_RC522_RST      4
#define PIN_BUZZER         25
#define PIN_LED_RED        32
#define PIN_LED_GREEN      33
#define PIN_LED_BLUE       27
#define PIN_BUTTON_MODE    0
#define PIN_BUTTON_MUTE    34

// 报警阈值
#define DISTANCE_THRESHOLD  10.0   // 米
#define LIGHT_THRESHOLD     10.0   // lux
#define MOTION_THRESHOLD    2000   // mg

// BLE 参数
#define BLE_SCAN_INTERVAL   5000   // ms

#endif
```

#### 4.2 主程序框架（main.c）

```c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "config.h"

extern void ble_init();
extern void sensor_init();
extern void rfid_init();
extern void display_init();

void app_main() {
    ESP_LOGI("MAIN", "Enhanced AirTag Starting...");
    
    // 初始化硬件
    ble_init();
    sensor_init();
    rfid_init();
    display_init();
    
    // 创建任务
    xTaskCreate(ble_task, "BLE", 4096, NULL, 5, NULL);
    xTaskCreate(sensor_task, "SENSOR", 4096, NULL, 4, NULL);
    xTaskCreate(rfid_task, "RFID", 2048, NULL, 2, NULL);
    xTaskCreate(display_task, "DISPLAY", 4096, NULL, 3, NULL);
    
    ESP_LOGI("MAIN", "All tasks started");
}
```

#### 4.3 传感器处理（sensor_handler.c）

```c
#include "driver/i2c.h"
#include "mpu6050.h"
#include "bh1750.h"
#include "config.h"

static mpu6050_handle_t mpu6050;
static bh1750_t bh1750;

void sensor_init() {
    // 初始化 I2C
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = PIN_I2C_SDA,
        .scl_io_num = PIN_I2C_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    i2c_param_config(I2C_NUM_0, &conf);
    i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
    
    // 初始化 MPU6050
    mpu6050 = mpu6050_create(I2C_NUM_0, MPU6050_I2C_ADDRESS);
    mpu6050_wake_up(mpu6050);
    
    // 初始化 BH1750
    bh1750_init_desc(&bh1750, BH1750_ADDR_LO, I2C_NUM_0, PIN_I2C_SDA, PIN_I2C_SCL);
    bh1750_setup(&bh1750, BH1750_MODE_CONTINUOUS, BH1750_RES_HIGH);
}

void sensor_task(void *arg) {
    mpu6050_acce_value_t acce;
    float lux;
    
    while(1) {
        // 读取加速度
        mpu6050_get_acce(mpu6050, &acce);
        float motion = sqrt(acce.acce_x*acce.acce_x + 
                           acce.acce_y*acce.acce_y + 
                           acce.acce_z*acce.acce_z);
        
        // 读取光照
        bh1750_read(&bh1750, &lux);
        
        // 判断报警条件
        if (motion > MOTION_THRESHOLD) {
            trigger_alarm(ALARM_MOTION);
        }
        if (lux < LIGHT_THRESHOLD) {
            trigger_alarm(ALARM_DARK);
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
```

#### 4.4 OLED 显示（display_handler.c）

```c
#include "ssd1306.h"
#include "config.h"

static SSD1306_t oled;

void display_init() {
    i2c_master_init(&oled, PIN_I2C_SDA, PIN_I2C_SCL, 0x3C);
    ssd1306_init(&oled, 128, 64);
    ssd1306_clear_screen(&oled, false);
}

void display_task(void *arg) {
    char buffer[32];
    
    while(1) {
        ssd1306_clear_screen(&oled, false);
        
        // 显示模式
        ssd1306_display_text(&oled, 0, "Mode: Tracking", 14, false);
        
        // 显示距离
        sprintf(buffer, "Dist: %.1fm", get_distance());
        ssd1306_display_text(&oled, 2, buffer, strlen(buffer), false);
        
        // 显示传感器数据
        sprintf(buffer, "Light: %.0flux", get_light());
        ssd1306_display_text(&oled, 4, buffer, strlen(buffer), false);
        
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
```

### 步骤 5：编译与烧录

```bash
# 配置项目
idf.py menuconfig
# Component config → Bluetooth → 勾选 "Bluetooth"

# 编译
idf.py build

# 烧录（需要先在 OpenHaystack 中生成密钥）
idf.py -p COM3 flash

# 注入公钥
esptool.py --port COM3 write_flash 0x3F0000 public_key.bin

# 监视日志
idf.py -p COM3 monitor
```

### 步骤 6：开发手机端 App（可选）

**使用 Flutter（跨平台）**：
```dart
// 利用 openhaystack-mobile 项目
cd openhaystack-mobile

// 安装依赖
flutter pub get

// 运行
flutter run
```

**功能增强**：
- 添加 BLE 距离监测页面
- 实时显示传感器数据
- 报警通知推送

---

## 六、常见问题解决

### 问题 1：ESP32 无法识别

**症状**：设备管理器/`ls /dev` 找不到 COM 口

**解决方案**：
```bash
# Windows：
# 1. 检查驱动是否安装（设备管理器 → 端口）
# 2. 更换 USB 数据线（必须是数据线，非充电线）
# 3. 尝试不同 USB 口

# Linux：
sudo dmesg | grep tty
# 查看是否有权限问题：
sudo chmod 666 /dev/ttyUSB0
```

### 问题 2：编译错误 `esp_bt.h not found`

**原因**：未启用 Bluetooth 组件

**解决方案**：
```bash
idf.py menuconfig
# → Component config 
# → Bluetooth 
# → [*] Bluetooth（勾选）
# → [*] Bluetooth controller (enabled)
```

### 问题 3：Find My 无法定位

**排查步骤**：
1. **检查密钥格式**：
   ```bash
   # 密钥应为 28 字节 Base64 编码
   echo "你的密钥" | base64 -d | wc -c
   # 应输出 28
   ```

2. **检查 BLE 广播**：
   ```bash
   # 使用 nRF Connect App (iOS/Android)
   # 搜索名为 "Find My" 或无名的设备
   # Manufacturer Data 应包含 0x004C（Apple）
   ```

3. **检查 iPhone 设置**：
   - 蓝牙：已开启
   - iCloud：已登录
   - 定位服务：已启用
   - 查找网络：已加入

4. **等待时间**：首次定位可能需要 **30-60 分钟**

### 问题 4：I2C 设备初始化失败

**症状**：`i2c_master_cmd_begin() returned 0x107`

**解决方案**：
```bash
# 1. 检查接线（SDA/SCL 是否接反）
# 2. 添加上拉电阻（4.7kΩ 到 3.3V）
# 3. 扫描 I2C 地址：
idf.py menuconfig
# → Component config → I2C → I2C address scan
# 重新编译烧录，查看 log 输出的地址
```

### 问题 5：OLED 显示乱码

**原因**：I2C 时钟过快或电压不足

**解决方案**：
```c
// 降低 I2C 时钟频率
.master.clk_speed = 100000,  // 从 400kHz 降到 100kHz

// 确保 VCC 接 3.3V（非 5V）
```

### 问题 6：RFID 读卡不稳定

**优化方案**：
```c
// 1. 增加读卡重试次数
for (int retry = 0; retry < 3; retry++) {
    if (rc522_read_card(&uid) == ESP_OK) break;
    vTaskDelay(100);
}

// 2. 调整天线增益
rc522_set_antenna_gain(0x07 << 4);  // 最大增益
```

### 问题 7：内存不足 `ESP_ERR_NO_MEM`

**解决方案**：
```c
// 1. 减少任务栈大小
xTaskCreate(task, "NAME", 2048, ...);  // 从 4096 降到 2048

// 2. 启用 PSRAM（如果开发板支持）
idf.py menuconfig
// → Component config → ESP32-specific → Support for external, SPI-connected RAM

// 3. 使用静态分配
static uint8_t task_stack[2048];
xTaskCreateStatic(..., task_stack, ...);
```

### 问题 8：BLE 和 WiFi 冲突

**症状**：启用 WiFi 后 BLE 广播停止

**原因**：ESP32 的蓝牙和 WiFi 共享射频

**解决方案**：
```c
// 使用蓝牙和 WiFi 共存模式
esp_coex_preference_set(ESP_COEX_PREFER_BT);

// 或者分时复用
void app_main() {
    ble_adv_start();
    vTaskDelay(30000);  // BLE 广播 30 秒
    
    wifi_init();
    wifi_connect();
    // 上传数据
    wifi_disconnect();
    
    // 回到 BLE 模式
}
```

---

## 附录

### A. 参考资料

**官方文档**：
- [ESP-IDF 编程指南](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [OpenHaystack GitHub](https://github.com/seemoo-lab/openhaystack)
- [Apple Find My Network Accessory 规范](https://developer.apple.com/find-my/)

**论文**：
- [Who Can _Find My_ Devices? (PETS 2021)](https://arxiv.org/abs/2103.02282)

**视频教程**：
- [ESP32 完整入门教程](https://www.youtube.com/watch?v=xPlN_Tk3VLQ)
- [OpenHaystack DIY AirTag](https://www.youtube.com/watch?v=_WXSudhndgM)

### B. 工具链快速参考

| 命令 | 功能 |
|-----|------|
| `idf.py build` | 编译项目 |
| `idf.py -p PORT flash` | 烧录固件 |
| `idf.py -p PORT monitor` | 查看日志 |
| `idf.py menuconfig` | 配置项目 |
| `idf.py fullclean` | 清理编译缓存 |
| `esptool.py --port PORT erase_flash` | 擦除 Flash |
| `esptool.py --port PORT read_mac` | 读取 MAC 地址 |

### C. 硬件采购链接（参考）

| 平台 | 优势 | 链接 |
|-----|------|------|
| 淘宝 | 价格便宜、发货快 | 搜索 "ESP32开发板套件" |
| 立创商城 | 品质保证、技术支持 | [szlcsc.com](https://www.szlcsc.com) |
| 亚马逊 | 海外品牌、售后好 | 搜索 "ESP32 DevKit" |

### D. 项目展示与包装建议

**文档准备**：
- 系统框图（用 draw.io 绘制）
- 接线图（用 Fritzing 设计）
- 演示视频（3-5 分钟）
- 测试报告（Excel 记录）

**答辩 PPT 大纲**：
1. 项目背景与意义（2 分钟）
2. 技术原理（Find My 网络工作流程）（3 分钟）
3. 硬件设计（引脚分配、传感器选型）（3 分钟）
4. 软件架构（多任务调度、中断处理）（4 分钟）
5. 功能演示（现场测试）（5 分钟）
6. 总结与展望（2 分钟）

**外观优化**：
- 3D 打印外壳（推荐使用 Fusion 360 设计）
- 亚克力透明外壳（便宜且美观）
- PCB 定制（如果时间允许，使用立创 EDA）

---

## 🎉 结语

恭喜你完成开发教程的阅读！这个项目涉及的知识点非常全面：
- ✅ 嵌入式系统开发（ESP32）
- ✅ 蓝牙协议（BLE）
- ✅ 多传感器融合
- ✅ RTOS 多任务编程
- ✅ 中断与外设驱动
- ✅ 逆向工程应用（Find My 网络）

**开发时间规划**：
- 完整开发：**7-10 天**（每天 4-6 小时）
- 快速原型：**3-5 天**（使用现有库，跳过优化）

**遇到问题？**
- 查看项目 Issues：https://github.com/seemoo-lab/openhaystack/issues
- ESP32 中文社区：https://www.esp32.com/
- 在本项目提交 Issue 或讨论

**祝你课设顺利！🚀**

---

> **最后更新**：2024-12-12  
> **作者**：基于 OpenHaystack 项目整理  
> **许可证**：遵循原项目 AGPL-3.0 License
