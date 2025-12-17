# RFID嵌入式系统二阶段开发文档
## 小鸡身份追踪系统（简化版）

---

## 📋 项目概述

### 一阶段回顾
**当前功能：双模式RFID权限管理系统**
- 正常模式（卡片D4C8F505）：LED常亮，仅监控数据
- 警戒模式（卡片012C9902）：多传感器联动报警（光照/运动/距离检测）
- 应用场景：背包防盗、物品安全监控

### 二阶段目标
**小鸡身份追踪系统（演示版）**
- **核心理念**：通过RFID标识个体小鸡，实时监控活动状态
- **目标应用**：小鸡身份识别 + 步数统计 + 状态显示
- **技术特点**：简单易用，适合演示和学习

---

## 🎯 二阶段功能设计

### 1. RFID小鸡标识系统

#### 1.1 卡片类型（简化版）
```
类型A - 正常模式卡
  ├─ UID: 0xD4C8F505
  ├─ 功能：常规监控模式
  └─ 显示：传感器数据监控

类型B - 警戒模式卡
  ├─ UID: 0x012C9902
  ├─ 功能：多传感器联动报警
  └─ 显示：报警状态提示

类型C - 小鸡标识卡（新增）
  ├─ UID: 0x0425C712（可自定义）
  ├─ 功能：小鸡身份识别
  └─ 特色：播放See You Again旋律 + 步数统计
```

#### 1.2 小鸡模式使用流程
```
[刷小鸡标识卡] 
    ↓
[播放See You Again旋律] → 蜂鸣器音乐提示
    ↓
[显示小鸡信息] → OLED显示"Mode: CHICKEN"
    ↓
[开始计步] → MPU6050检测前后摆动
    ↓
[实时显示] → 步数 + 状态（无丢失）
    ↓
[步数增加时] → LED闪烁提示
```

---

### 2. MPU6050计步系统（简化版）

#### 2.1 计步算法

**原理：前后摆动检测**
```c
// 简化的计步参数
#define STEP_THRESHOLD       2000    // 加速度变化阈值
#define STEP_MIN_INTERVAL    300     // 最小步间隔(ms) - 防抖

// 检测逻辑
1. 读取Y轴和Z轴加速度
2. 计算与上次数据的差值
3. 差值超过阈值 → 判定为一步
4. 步数+1，LED闪烁提示
5. 更新OLED显示
```

**实现代码片段**
```c
// 检测前后摆动
accelChange = abs(accel[1] - lastAccel[1]) + abs(accel[2] - lastAccel[2]);

// 超过阈值且满足时间间隔
if (accelChange > STEP_THRESHOLD && (systemTick - lastStepTime) > STEP_MIN_INTERVAL)
{
    chickenStepCount++;           // 步数+1
    lastStepTime = systemTick;    // 更新时间
    stepLedFlashCounter = 4;      // 触发LED闪烁
}
0x000C-0x000D: 养殖天数 (2字节)
0x000E-0x008D: 每日步数数组 [30天] (4字节×30)
0x008E-0x008F: 数据校验CRC16 (2字节)
```

**掉电保护**
- 每100步写入一次EEPROM
- 每日零点自动备份当日数据
- CRC16校验防止数据损坏

---

### 3. OLED显示界面（小鸡模式）

#### 3.1 小鸡模式显示
```
┌──────────────────┐
│ Mode: CHICKEN    │  ← 模式标识
│ ID: Chick #001   │  ← 小鸡编号
│ Steps: 123       │  ← 实时步数
│ Status: No Lost  │  ← 状态：无丢失
└──────────────────┘
```

#### 3.2 显示更新逻辑
- **第1行**：显示当前模式（CHICKEN）
- **第2行**：小鸡身份标识
- **第3行**：实时步数（每次摆动自动更新）
- **第4行**：状态信息（固定显示"No Lost"）

---

## 🎵 音乐播放功能

### See You Again旋律
刷小鸡卡时，蜂鸣器自动播放See You Again主旋律片段：

```c
// 旋律片段（简化版）
It's been a long day without you my friend
And I'll tell you all about it when I see you again
```

**实现方式：**
- **无源蜂鸣器**：通过PWM输出不同频率，播放完整旋律
- **有源蜂鸣器**：用节奏beep模拟旋律（3短-2长-3短）

---

## 💡 LED闪烁提示

### 步数增加时LED闪烁
```
检测到摆动 → 步数+1 → LED快速闪烁4次（约200ms）
```

**闪烁逻辑：**
1. 检测到步数变化
2. 设置闪烁计数器 = 4
3. 每50ms切换LED状态（开/关）
4. 闪烁完成后LED保持关闭

---

## 🔧 硬件要求（无需新增）

### 现有硬件复用
- **RC522**：读取小鸡标识卡
- **MPU6050**：检测前后摆动，计算步数
- **OLED**：显示小鸡信息和步数
- **LED**：步数增加闪烁提示
- **蜂鸣器**：播放See You Again旋律

**无需额外硬件！** 完全使用现有传感器实现。

---

## 💻 软件实现说明

### 主要代码修改

#### 1. Buzzer.c - 添加音乐播放
```c
// 新增函数
void Buzzer_PlaySeeYouAgain(void);  // 播放See You Again

// 音符定义
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
// ... 更多音符
    uint8_t dayOfWeek;
} RTC_DateTime;

void RTC_Init(void);
void RTC_SetDateTime(RTC_DateTime *dt);
void RTC_GetDateTime(RTC_DateTime *dt);
uint32_t RTC_GetTimestamp(void);  // Unix时间戳
uint8_t RTC_GetDaysSince(uint32_t startTime);
```

#### 模块4: EEPROM存储 (`EEPROM_Storage.c/.h`)
```c
// 数据存储结构
typedef struct {
    uint32_t uid;                    // 产品UID
    uint32_t startTime;              // 开始时间戳
    uint32_t totalSteps;             // 总步数
    uint16_t days;                   // 养殖天数
// 旋律数据结构
Note melody[] = {
    {NOTE_G4, 500},  // It's been a long day...
    {NOTE_A4, 250},
    // ... 更多音符
};
```

#### 2. main.c - 添加小鸡模式
```c
// 新增模式枚举
typedef enum {
    MODE_NORMAL = 0,
    MODE_ALERT = 1,
    MODE_CHICKEN = 2    // 小鸡模式
} SystemMode;

// 新增变量
static uint32_t chickenStepCount = 0;      // 步数计数
static uint8_t stepLedFlashCounter = 0;    // LED闪烁计数

// 小鸡卡UID（需要根据实际卡片修改）
#define CARD_CHICKEN_UID {0x04, 0x25, 0xC7, 0x12}
```

#### 3. 步数检测函数
```c
static void CheckChickenSteps(int16_t *accel)
{
    // 计算Y轴和Z轴加速度变化
    int32_t accelChange = abs(accel[1] - lastAccel[1]) + 
                          abs(accel[2] - lastAccel[2]);
    
    // 超过阈值 → 步数+1
    if (accelChange > STEP_THRESHOLD && 
        (systemTick - lastStepTime) > STEP_MIN_INTERVAL)
    {
        chickenStepCount++;
        stepLedFlashCounter = 4;  // 触发LED闪烁
    }
}
```

---

## 🔄 使用流程

### 快速开始
```
1. 上电启动 → 默认正常模式
2. 刷小鸡标识卡 → 蜂鸣器播放See You Again
3. OLED显示小鸡模式信息
4. 前后摆动MPU6050 → 步数自动增加
5. 每次步数增加 → LED闪烁提示
```

### 小鸡卡UID配置
**重要**：需要根据你的实际RFID卡片修改UID

在 [main.c](stm32_keil5/main.c#L50) 中修改：
```c
#define CARD_CHICKEN_UID {0x04, 0x25, 0xC7, 0x12}
// 改为你的卡片实际UID，例如：
// #define CARD_CHICKEN_UID {0xAA, 0xBB, 0xCC, 0xDD}
```

**如何获取卡片UID**：
1. 刷卡时观察OLED或串口输出
2. 或临时添加打印代码查看UID

---

## 🛠️ 开发进度

### ✅ 已完成功能
- [x] See You Again旋律播放
- [x] 小鸡模式切换
- [x] 前后摆动步数检测
- [x] 步数实时显示
- [x] LED闪烁提示
- [x] 状态显示（无丢失）

### 💡 演示效果
1. **音乐提示**：刷卡瞬间播放旋律
2. **实时计步**：摆动即可看到步数增加
3. **视觉反馈**：LED闪烁 + OLED更新
4. **简单易用**：无需额外配置

---

## 📊 测试方法

### 步数检测测试
1. 刷小鸡卡进入模式
2. 手持设备前后摆动（模拟鸡走路）
3. 观察OLED步数变化
4. 观察LED是否闪烁

### 参数调整
如果步数不准确，可调整阈值：

在 [main.c](stm32_keil5/main.c#L258) 中修改：
```c
#define STEP_THRESHOLD 2000       // 增大=更难触发，减小=更容易触发
#define STEP_MIN_INTERVAL 300     // 防抖时间(ms)
```

---

## 🚀 总结

**简化版特点**：
- ✅ 无需新增硬件
- ✅ 功能演示完整
- ✅ 代码结构清晰
- ✅ 易于调试修改

**核心实现**：
1. RFID识别：小鸡身份标识
2. 音乐播放：See You Again旋律
3. 步数统计：MPU6050摆动检测
4. 视觉反馈：OLED显示 + LED闪烁

**应用价值**：
- 学习嵌入式多模式切换
- 理解传感器数据处理
- 掌握音乐播放原理
- 为后续扩展打基础

---

**文档版本**: v2.0 (简化版)  
**更新日期**: 2025-12-17  
**作者**: Embedded Development Team  
**项目代号**: Chicken Tracker - Simplified Edition
