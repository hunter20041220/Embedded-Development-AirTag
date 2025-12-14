# ESP32智能距离追踪器 - 完整开发指南

## 📋 项目概述

基于OpenHaystack的ESP32智能距离追踪器，实现物品到电脑的实时距离测量与多传感器监测。

### 🎯 核心功能
- ✅ BLE信号强度测距（RSSI）
- ✅ OLED实时显示距离与状态
- ✅ RFID门禁管理（RC522）
- ✅ 多传感器数据采集（温湿度、光照、运动、人体红外、超声波）
- ✅ 蜂鸣器报警系统
- ✅ 电脑终端实时监控
- ✅ 串口通信协议

---

## 📦 硬件清单

| 序号 | 模块名称 | 型号 | 数量 | 用途 |
|------|----------|------|------|------|
| 1 | 主控板 | ESP32-WROOM-32 | 1 | 核心控制器 |
| 2 | OLED显示屏 | SSD1306 (128x64, I2C) | 1 | 显示距离/状态 |
| 3 | RFID读卡器 | RC522 (13.56MHz) | 1 | 射频识别 |
| 4 | 蜂鸣器 | 有源/无源蜂鸣器 | 1 | 声音报警 |
| 5 | 加速度计 | MPU6050 (I2C) | 1 | 运动检测 |
| 6 | 温湿度传感器 | AM2302 (DHT22) | 1 | 环境监测 |
| 7 | 超声波测距 | HC-SR04 | 1 | 物理距离测量 |
| 8 | LED模块 | 3W白光LED | 1 | 照明/指示 |
| 9 | 人体红外 | HW-269 (HC-SR501) | 1 | 人体检测 |

---

## 🗂️ 项目结构

```
ESP32_Distance_Tracker/
├── README.md                          # 本文档
├── docs/                              # 详细文档
│   ├── 01_环境配置指南.md              # 软件安装与环境搭建
│   ├── 02_硬件接线说明.md              # 引脚定义与接线图
│   ├── 03_开发流程指南.md              # 开发步骤与调试方法
│   └── 04_常见问题解决.md              # 故障排查
│
├── test_modules/                      # 模块测试代码
│   ├── 01_OLED_Test/                  # OLED显示测试
│   │   ├── README.md                  # OLED使用文档
│   │   ├── main.c                     # 测试代码
│   │   └── CMakeLists.txt             # 构建文件
│   │
│   ├── 02_RFID_RC522_Test/            # RFID测试
│   │   ├── README.md
│   │   ├── main.c
│   │   └── CMakeLists.txt
│   │
│   ├── 03_Buzzer_Test/                # 蜂鸣器测试
│   │   ├── README.md
│   │   ├── main.c
│   │   └── CMakeLists.txt
│   │
│   ├── 04_MPU6050_Test/               # 加速度计测试
│   │   ├── README.md
│   │   ├── main.c
│   │   └── CMakeLists.txt
│   │
│   ├── 05_AM2302_Test/                # 温湿度测试
│   │   ├── README.md
│   │   ├── main.c
│   │   └── CMakeLists.txt
│   │
│   ├── 06_HCSR04_Test/                # 超声波测试
│   │   ├── README.md
│   │   ├── main.c
│   │   └── CMakeLists.txt
│   │
│   ├── 07_LED_Test/                   # LED测试
│   │   ├── README.md
│   │   ├── main.c
│   │   └── CMakeLists.txt
│   │
│   └── 08_HW269_Test/                 # 人体红外测试
│       ├── README.md
│       ├── main.c
│       └── CMakeLists.txt
│
├── firmware/                          # 完整固件
│   ├── main/
│   │   ├── main.c                     # 主程序
│   │   ├── ble_distance.c/h           # BLE测距模块
│   │   ├── oled_display.c/h           # OLED显示模块
│   │   ├── rfid_manager.c/h           # RFID管理模块
│   │   ├── sensors.c/h                # 传感器模块
│   │   ├── buzzer_alarm.c/h           # 蜂鸣器模块
│   │   ├── uart_protocol.c/h          # 串口通信协议
│   │   └── CMakeLists.txt
│   │
│   ├── CMakeLists.txt
│   └── sdkconfig                      # ESP-IDF配置
│
└── pc_monitor/                        # 电脑监控程序
    ├── monitor.py                     # Python监控脚本
    ├── requirements.txt               # Python依赖
    └── README.md                      # 使用说明
```

---

## 🚀 快速开始

### 第一步：环境配置
详见 [docs/01_环境配置指南.md](docs/01_环境配置指南.md)

**核心工具**：
- **ESP-IDF v5.0+** (ESP32官方开发框架)
- **VS Code + ESP-IDF插件** (推荐IDE)
- **Python 3.8+** (烧录工具与PC监控)
- **CH340/CP2102驱动** (USB串口驱动)

### 第二步：硬件接线
详见 [docs/02_硬件接线说明.md](docs/02_硬件接线说明.md)

### 第三步：模块逐个测试
按顺序测试每个模块，确保硬件正常：
1. [OLED显示测试](test_modules/01_OLED_Test/README.md)
2. [RFID读卡测试](test_modules/02_RFID_RC522_Test/README.md)
3. [蜂鸣器测试](test_modules/03_Buzzer_Test/README.md)
4. [MPU6050测试](test_modules/04_MPU6050_Test/README.md)
5. [AM2302测试](test_modules/05_AM2302_Test/README.md)
6. [HC-SR04测试](test_modules/06_HCSR04_Test/README.md)
7. [LED测试](test_modules/07_LED_Test/README.md)
8. [HW-269测试](test_modules/08_HW269_Test/README.md)

### 第四步：烧录完整固件
```bash
cd firmware
idf.py build
idf.py -p COM3 flash monitor
```

### 第五步：启动PC监控
```bash
cd pc_monitor
pip install -r requirements.txt
python monitor.py
```

---

## 📊 系统架构

```
┌─────────────────────────────────────────────────────────┐
│                  ESP32智能追踪器                         │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  输入模块              核心处理              输出模块    │
│  ┌──────────┐       ┌──────────┐        ┌──────────┐  │
│  │ RFID读卡 │──────►│          │───────►│ OLED显示 │  │
│  │ RC522    │       │  ESP32   │        │ SSD1306  │  │
│  │          │       │          │        │          │  │
│  │ MPU6050  │──────►│ FreeRTOS │───────►│ 蜂鸣器   │  │
│  │ 加速度计 │       │ 多任务   │        │ 报警     │  │
│  │          │       │          │        │          │  │
│  │ AM2302   │──────►│ BLE测距  │───────►│ LED指示  │  │
│  │ 温湿度   │       │ RSSI     │        │          │  │
│  │          │       │          │        │          │  │
│  │ HC-SR04  │──────►│ 串口通信 │───────►│ UART输出 │  │
│  │ 超声波   │       │ 协议     │        │ to PC    │  │
│  │          │       │          │        │          │  │
│  │ HW-269   │──────►│ NVS存储  │        │          │  │
│  │ 人体红外 │       │          │        │          │  │
│  └──────────┘       └──────────┘        └──────────┘  │
│                                                         │
└─────────────────────────────────────────────────────────┘
                            │
                            │ USB串口
                            ▼
                    ┌───────────────┐
                    │  电脑终端     │
                    │  Python监控   │
                    │  实时显示     │
                    └───────────────┘
```

---

## 🎓 开发路线图

### 阶段1：环境搭建（1天）
- [ ] 安装ESP-IDF
- [ ] 配置VS Code
- [ ] 安装驱动程序
- [ ] 烧录Hello World验证

### 阶段2：模块测试（3-4天）
- [ ] OLED显示测试
- [ ] RFID读卡测试
- [ ] 传感器测试（MPU6050、AM2302、HC-SR04）
- [ ] 输出测试（蜂鸣器、LED）
- [ ] 人体红外测试

### 阶段3：功能开发（4-5天）
- [ ] BLE测距功能
- [ ] 多传感器融合
- [ ] OLED多页面显示
- [ ] RFID门禁管理
- [ ] 串口通信协议
- [ ] 报警逻辑

### 阶段4：系统集成（2天）
- [ ] 整合所有模块
- [ ] 优化任务调度
- [ ] 电源管理
- [ ] 稳定性测试

### 阶段5：文档与演示（1天）
- [ ] 编写用户手册
- [ ] 制作演示视频
- [ ] 准备答辩PPT

**总计：10-12天**

---

## 🔌 快速接线参考

| 模块 | 引脚 | ESP32 GPIO |
|------|------|-----------|
| OLED SSD1306 | SDA/SCL | GPIO21/GPIO22 |
| RC522 RFID | MOSI/MISO/SCK/CS/RST | GPIO23/19/18/5/4 |
| MPU6050 | SDA/SCL | GPIO21/GPIO22 (共享) |
| AM2302 | DATA | GPIO15 |
| HC-SR04 | Trig/Echo | GPIO25/GPIO26 |
| 蜂鸣器 | Signal | GPIO27 |
| LED | PWM | GPIO32 |
| HW-269 | OUT | GPIO33 |

详细接线见 [docs/02_硬件接线说明.md](docs/02_硬件接线说明.md)

---

## 📞 技术支持

**遇到问题？**
1. 查看 [docs/04_常见问题解决.md](docs/04_常见问题解决.md)
2. 检查各模块的README文档
3. 查看ESP-IDF官方文档：https://docs.espressif.com/

---

## 📝 版本信息

- **版本**：v1.0.0
- **创建日期**：2025-12-14
- **ESP-IDF版本**：v5.0+
- **兼容硬件**：ESP32-WROOM-32

---

## 📄 许可证

本项目基于OpenHaystack，遵循AGPL-3.0 License。

---

**祝开发顺利！🚀**
