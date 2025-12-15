# ESP32 LED测试项目

## 项目说明
测试ESP32控制LED灯的基本功能，实现LED闪烁。

## 硬件连接

### 方案1：使用板载LED（推荐）
- 无需外接，直接使用ESP32开发板上的LED（通常连接到GPIO2）

### 方案2：外接普通LED（5mm/3mm小LED）
```
ESP32 GPIO2 → 限流电阻(220Ω-1kΩ) → LED正极(长脚)
LED负极(短脚) → ESP32 GND
```

### ⚠️ 方案3：3W大功率LED（需要驱动电路）
**不能直接连接ESP32！** 需要三极管/MOS管驱动：
```
ESP32 GPIO2 → 1kΩ电阻 → 三极管基极(B)
三极管发射极(E) → GND
三极管集电极(C) → LED负极
LED正极 → 限流电阻 → 3.3V/5V电源
```

## 编译和烧录
```bash
cd test_modules/02_LED_Test
idf.py build
idf.py -p COM3 flash monitor
```

## 预期效果
- LED每500ms闪烁一次
- 串口输出闪烁次数
