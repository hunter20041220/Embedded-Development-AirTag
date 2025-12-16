# STM32F103引脚定义与硬件接线

## 📋 目录

- [引脚总览](#引脚总览)
- [模块详细接线](#模块详细接线)
- [引脚分配原则](#引脚分配原则)
- [电源管理](#电源管理)
- [接线注意事项](#接线注意事项)

---

## 引脚总览

### STM32F103引脚功能分配表

| 序号 | 模块名称 | 接口类型 | STM32引脚 | 复用功能 | 说明 |
|------|----------|---------|-----------|----------|------|
| 1 | **OLED SSD1306** | I2C (软件模拟) | PB6(SCL), PB7(SDA) | - | 显示屏 (已测试✅) |
| 2 | **MPU6050** | I2C1 (硬件) | PB8(SCL), PB9(SDA) | I2C1 | 加速度+陀螺仪 |
| 3 | **RC522 RFID** | SPI1 | PA5(SCK), PA6(MISO), PA7(MOSI), PA4(CS), PA3(RST) | SPI1 | RFID读卡器 |
| 4 | **AM2302 (DHT22)** | 单总线 | PC13 | GPIO | 温湿度传感器 |
| 5 | **HC-SR04** | GPIO | PB0(Trig), PB1(Echo) | TIM3_CH3,CH4 | 超声波测距 |
| 6 | **蜂鸣器** | PWM | PA8 | TIM1_CH1 | 声音报警 |
| 7 | **LED** | PWM/GPIO | PA15 | GPIO | 状态指示灯 |
| 8 | **HW-269** | ADC | PA0 | ADC1_IN0 | 电池电量检测 |
| 9 | **串口调试** | USART1 | PA9(TX), PA10(RX) | USART1 | USB转串口 |
| 10 | **BH1750** | I2C | PB6(SCL), PB7(SDA) | - | 光照强度传感器 (可与OLED共享I2C) |
| 11 | **HCSR501 (PIR)** | GPIO/IRQ | PB11(OUT) | EXTI11 | 被动红外运动传感器 |

### 引脚资源占用统计

- **GPIOA**: PA0(ADC), PA3(RST), PA4(CS), PA5-PA7(SPI1), PA8(PWM), PA9-PA10(USART1), PA15(LED)
- **GPIOB**: PB0-PB1(SR04), PB6-PB7(OLED I2C), PB8-PB9(MPU6050 I2C1)
 - **GPIOB**: PB0-PB1(HC-SR04), PB6-PB7(OLED/BH1750 I2C), PB8-PB9(MPU6050 I2C1), PB11(HCSR501 OUT)
- **GPIOC**: PC13(DHT22)

---

## 模块详细接线

### 1. OLED SSD1306 显示屏 (I2C软件模拟) ✅已测试

**模块规格**：
- 尺寸：0.96寸 128x64
- 接口：I2C (4针)
- I2C地址：0x78 (8位写地址) / 0x3C (7位地址)
- 电压：3.3V

**接线表**：

| OLED引脚 | STM32引脚 | 引脚功能 | 说明 |
|----------|----------|----------|------|
| VCC | 3.3V | 电源 | 供电 |
| GND | GND | 地 | 接地 |
| SCL | **PB6** | GPIO开漏输出 | I2C时钟线 (软件模拟) |
| SDA | **PB7** | GPIO开漏输出 | I2C数据线 (软件模拟) |

**接线示意图**：
```
OLED SSD1306         STM32F103
┌──────────┐         ┌──────┐
│ VCC      │────────►│ 3.3V │
│ GND      │────────►│ GND  │
│ SCL      │◄───────►│ PB6  │ 开漏输出 50MHz
│ SDA      │◄───────►│ PB7  │ 开漏输出 50MHz
└──────────┘         └──────┘
```

**代码配置** (Hardware/OLED/OLED.c)：
```c
#define OLED_W_SCL(x) GPIO_WriteBit(GPIOB, GPIO_Pin_6, (BitAction)(x))
#define OLED_W_SDA(x) GPIO_WriteBit(GPIOB, GPIO_Pin_7, (BitAction)(x))

// GPIO配置：开漏输出 50MHz
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
```

**注意事项**：
- ✅ 已测试通过，可正常显示
- ⚠️ 使用软件模拟I2C，不占用硬件I2C资源
- ⚠️ 电源必须接3.3V，不能接5V

---

### 2. MPU6050 加速度计+陀螺仪 (I2C1硬件接口)

**模块规格**：
- 6轴传感器 (3轴加速度 + 3轴陀螺仪)
- 接口：I2C
- I2C地址：0x68 (AD0→GND) / 0x69 (AD0→VCC)
- 电压：3.3V-5V

**接线表**：

| MPU6050引脚 | STM32引脚 | 引脚功能 | 说明 |
|-------------|----------|----------|------|
| VCC | 3.3V | 电源 | 供电 |
| GND | GND | 地 | 接地 |
| SCL | **PB8** | I2C1_SCL | I2C时钟 (硬件I2C) |
| SDA | **PB9** | I2C1_SDA | I2C数据 (硬件I2C) |
| AD0 | GND | GPIO | 地址选择 (0x68) |
| INT | (可选PA1) | GPIO输入 | 中断输出 (可选) |

**接线示意图**：
```
MPU6050              STM32F103
┌──────────┐         ┌──────┐
│ VCC      │────────►│ 3.3V │
│ GND      │────────►│ GND  │
│ SCL      │◄───────►│ PB8  │ I2C1_SCL (复用推挽 50MHz)
│ SDA      │◄───────►│ PB9  │ I2C1_SDA (复用推挽 50MHz)
│ AD0      │────────►│ GND  │ (I2C地址 = 0x68)
│ INT      │────────►│ PA1  │ (可选，数据就绪中断)
└──────────┘         └──────┘
```

**代码配置** (Hardware/MPU6050/MPU6050.c)：
```c
// 开启I2C1时钟和GPIOB时钟
RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

// GPIO配置：复用推挽输出 50MHz
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;  // 复用开漏
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;

// I2C配置
I2C_InitStructure.I2C_ClockSpeed = 400000; // 400kHz快速模式
I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
```

**注意事项**：
- ✅ 使用硬件I2C1，不与OLED冲突 (OLED用软件模拟)
- ⚠️ AD0接GND时地址为0x68
- ⚠️ 需要上拉电阻 (大多数模块已内置)

---

### 3. RC522 RFID读卡器 (SPI1接口)

**模块规格**：
- 频率：13.56MHz
- 通信距离：3-5cm
- 接口：SPI
- 电压：3.3V (⚠️不能接5V)

**接线表**：

| RC522引脚 | STM32引脚 | 引脚功能 | 说明 |
|-----------|----------|----------|------|
| VCC (3.3V) | 3.3V | 电源 | 供电 |
| GND | GND | 地 | 接地 |
| RST | **PA3** | GPIO输出 | 复位引脚 |
| IRQ | (不接) | - | 中断 (可选) |
| MISO | **PA6** | SPI1_MISO | SPI主入从出 |
| MOSI | **PA7** | SPI1_MOSI | SPI主出从入 |
| SCK | **PA5** | SPI1_SCK | SPI时钟 |
| SDA (CS) | **PA4** | GPIO输出 | SPI片选 |

**接线示意图**：
```
RC522                STM32F103
┌──────────┐         ┌──────┐
│ VCC(3.3V)│────────►│ 3.3V │ ⚠️ 必须3.3V
│ GND      │────────►│ GND  │
│ RST      │◄────────│ PA3  │ GPIO推挽输出
│ IRQ      │         │  -   │ (不接)
│ MISO     │────────►│ PA6  │ SPI1_MISO
│ MOSI     │◄────────│ PA7  │ SPI1_MOSI
│ SCK      │◄────────│ PA5  │ SPI1_SCK
│ SDA(CS)  │◄────────│ PA4  │ GPIO推挽输出
└──────────┘         └──────┘
```

**代码配置** (Hardware/RC522/RC522.c)：
```c
// SPI1配置
RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_GPIOA, ENABLE);

// PA5(SCK), PA7(MOSI) - 复用推挽输出
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;

// PA6(MISO) - 浮空输入
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;

// PA3(RST), PA4(CS) - 推挽输出
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4;
```

**注意事项**：
- ⚠️ **只能用3.3V供电，接5V会烧毁！**
- ⚠️ SDA引脚实际是片选CS，不是I2C的SDA
- ⚠️ 读卡最佳距离：2-3cm

---

### 4. AM2302 (DHT22) 温湿度传感器 (单总线)

**模块规格**：
- 温度范围：-40~80℃ (±0.5℃)
- 湿度范围：0-100% RH (±2%)
- 接口：单总线 (1-Wire)
- 电压：3.3V-5V

**接线表**：

| AM2302引脚 | STM32引脚 | 引脚功能 | 说明 |
|------------|----------|----------|------|
| VCC (+) | 3.3V 或 5V | 电源 | 供电 |
| DATA | **PC13** | GPIO推挽/开漏 | 数据线 |
| GND (-) | GND | 地 | 接地 |

**接线示意图**：
```
AM2302               STM32F103
┌──────────┐         ┌──────┐
│ VCC (+)  │────────►│ 3.3V │ (或5V)
│ DATA     │◄───────►│ PC13 │ 推挽输出 (可切换输入)
│ GND (-)  │────────►│ GND  │
└──────────┘         └──────┘
      │
      └──[4.7K上拉]──► VCC (模块可能已内置)
```

**代码配置** (Hardware/DHT22/DHT22.c)：
```c
// PC13配置 - 推挽输出 (读取时切换为输入)
RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

// 输出模式 (发送起始信号)
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;

// 读取时切换为输入模式
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // 上拉输入
```

**注意事项**：
- ⚠️ 读取间隔必须 ≥ 2秒
- ⚠️ 需要精确的微秒级延时
- ⚠️ PC13是内置LED引脚 (部分开发板)

---

### 5. HC-SR04 超声波测距模块 (GPIO)

**模块规格**：
- 测距范围：2cm - 400cm
- 精度：±3mm
- 接口：GPIO (2针)
- 电压：5V

**接线表**：

| HC-SR04引脚 | STM32引脚 | 引脚功能 | 说明 |
|-------------|----------|----------|------|
| VCC | 5V | 电源 | 供电 |
| GND | GND | 地 | 接地 |
| Trig | **PB0** | GPIO输出 | 触发脉冲 (可用TIM3_CH3) |
| Echo | **PB1** | GPIO输入 | 回响脉冲 (可用TIM3_CH4输入捕获) |

**接线示意图**：
```
HC-SR04              STM32F103
┌──────────┐         ┌──────┐
│ VCC      │────────►│ 5V   │
│ GND      │────────►│ GND  │
│ Trig     │◄────────│ PB0  │ 推挽输出 (TIM3_CH3可选PWM)
│ Echo     │────────►│ PB1  │ 浮空/上拉输入 (TIM3_CH4输入捕获)
└──────────┘         └──────┘
```

**代码配置** (Hardware/HCSR04/HCSR04.c)：
```c
// PB0 - Trig输出
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;

// PB1 - Echo输入 (可配置为TIM3_CH4输入捕获)
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;

// 高级方案：使用TIM3输入捕获测量Echo脉宽
// PB1 = TIM3_CH4, 配置为输入捕获模式
```

**测距计算公式**：
```c
// 距离(cm) = 脉宽(us) * 声速(340m/s) / 2
// 距离 = 脉宽 * 0.017
distance_cm = pulse_width_us * 0.017;
```

**注意事项**：
- ⚠️ Trig需要10us以上高电平触发
- ⚠️ Echo脉宽 = 距离 * 58us/cm
- ⚠️ 建议用定时器输入捕获测量脉宽

---

### BH1750 光照强度传感器 (I2C)

**模块规格**：
- 接口：I2C
- 测量范围：0 ~ 65535 lx (16-bit)
- I2C地址：0x23 (默认) / 0x5C (ADDR引脚拉高时，具体请参考模块数据表)
- 电压：3.3V - 5V（优先使用3.3V）

**接线表**：

| BH1750引脚 | STM32引脚 | 引脚功能 | 说明 |
|-----------|----------|----------|------|
| VCC | 3.3V | 电源 | 供电 |
| GND | GND | 地 | 接地 |
| SCL | **PB6** | I2C时钟 | 与OLED共用I2C时钟线（软件I2C或硬件I2C视情况） |
| SDA | **PB7** | I2C数据 | 与OLED共用I2C数据线 |
| ADDR | GND/VCC | 地址选择 | 可选，接GND为0x23，接VCC为另一个地址 |

**接线示意图**：
```
BH1750               STM32F103
┌──────────┐         ┌──────┐
│ VCC      │────────►│ 3.3V │
│ GND      │────────►│ GND  │
│ SCL      │◄───────►│ PB6  │ I2C时钟 (与OLED共享)
│ SDA      │◄───────►│ PB7  │ I2C数据 (与OLED共享)
│ ADDR     │────────►│ GND/VCC │ 地址选择
└──────────┘         └──────┘
```

**代码/硬件注意**：
- 如果OLED已使用软件I2C (PB6/PB7)，BH1750可以直接共享同一总线，注意不同I2C设备地址。
- 确保I2C总线有上拉电阻（通常4.7k），模块若内置则无需外接。
- 常用读数流程：PowerOn -> Reset -> 读取测量寄存器（16位），转换为lx。

---

### HCSR501 被动红外运动传感器 (PIR) (GPIO / 中断)

**模块规格**：
- 类型：被动红外传感器模块
- 接口：数字输出 (OUT)
- 电压：3.3V - 5V（依据模块规格）

**接线表**：

| HCSR501引脚 | STM32引脚 | 引脚功能 | 说明 |
|------------|----------|----------|------|
| VCC | 3.3V 或 5V | 电源 | 供电（以模块规格为准） |
| GND | GND | 地 | 接地 |
| OUT | **PB11** | 数字输出 / 外部中断 | 高电平表示检测到运动，建议使用EXTI中断检测 |

**接线示意图**：
```
HCSR501              STM32F103
┌──────────┐         ┌──────┐
│ VCC      │────────►│ 3.3V │ (或5V，参照模块)
│ GND      │────────►│ GND  │
│ OUT      │────────►│ PB11 │ 数字输入 (可配置为 EXTI11)
└──────────┘         └──────┘
```

**代码配置建议**：
- 将 `PB11` 配置为输入并启用外部中断（EXTI11），以实现低延迟检测：
  - 输入模式：`GPIO_Mode_IN_FLOATING` 或 `GPIO_Mode_IPD/ IPU`（根据模块输出电平）
  - 配置 `EXTI_Line11` 和 NVIC 中断优先级以处理触发事件
- 注意模块上通常有灵敏度和延时可调电位器，先用短延时和高灵敏度测试，必要时调整。
- 输出为脉冲/高电平时表示检测到运动，长时间高电平说明持续检测到活动。


### 6. 蜂鸣器 (有源/无源两种)

**模块规格**：
- 类型：有源蜂鸣器 (默认) 或 无源蜂鸣器
- 接口：3针 (VCC, I/O, GND)
- 电压：3.3V-5V (根据蜂鸣器规格)
- 驱动方式：
  - **有源蜂鸣器**：高电平直接响 (GPIO控制)
  - **无源蜂鸣器**：需要PWM驱动 (2-5kHz，典型2.7kHz)

**接线表**：

| 蜂鸣器引脚 | STM32引脚 | 引脚功能 | 说明 |
|-----------|----------|----------|------|
| VCC | 3.3V 或 5V | 电源 | 供电 (根据蜂鸣器规格选择) |
| I/O | **PA8** | GPIO/TIM1_CH1 | 控制引脚 (有源用GPIO，无源用PWM) |
| GND | GND | 地 | 接地 |

**接线示意图**：
```
蜂鸣器               STM32F103
┌──────────┐         ┌──────┐
│ VCC      │────────►│ 5V   │ (或3.3V，看蜂鸣器规格)
│ I/O      │◄────────│ PA8  │ 有源：GPIO推挽输出
│ GND      │────────►│ GND  │ 无源：TIM1_CH1 PWM
└──────────┘         └──────┘
```

**代码配置** (Hardware/Buzzer/Buzzer.c)：

**方式1: 有源蜂鸣器 (默认，推荐测试)**
```c
// Buzzer.h 中设置：
#define BUZZER_TYPE  BUZZER_TYPE_ACTIVE  // 有源蜂鸣器

// PA8配置为推挽输出
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;

// 控制函数
Buzzer_On();   // 高电平，响
Buzzer_Off();  // 低电平，停
Buzzer_Beep(200);  // 响200ms后自动停
Buzzer_BeepTimes(3, 200);  // 响3次，每次200ms
```

**方式2: 无源蜂鸣器 (需要PWM)**
```c
// Buzzer.h 中设置：
#define BUZZER_TYPE  BUZZER_TYPE_PASSIVE  // 无源蜂鸣器

// PA8配置为TIM1_CH1 PWM输出
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // 复用推挽
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;

// TIM1 PWM配置 (2.7kHz频率)
TIM_TimeBaseStructure.TIM_Period = 999; // ARR
TIM_TimeBaseStructure.TIM_Prescaler = 26; // 72MHz/(26+1)/1000 = 2.67kHz
TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
TIM_OCInitStructure.TIM_Pulse = 500; // 占空比50%

// 控制函数
Buzzer_On();   // 启动PWM
Buzzer_Off();  // 停止PWM
```

**如何判断蜂鸣器类型**：
- **有源蜂鸣器**：
  - 底部有电路板或元件
  - 直接接3.3V/5V和GND会响
  - 只需高低电平控制
  
- **无源蜂鸣器**：
  - 底部只有一个振动膜
  - 直接接电源不会响
  - 需要PWM驱动 (2-5kHz方波)

**注意事项**：
- ⚠️ **先用有源模式测试**，如果不响再改成无源模式
- ⚠️ 电流较大时建议加三极管驱动
- ⚠️ 确认VCC引脚电压 (3.3V或5V，看蜂鸣器规格贴纸)
- ⚠️ 无源蜂鸣器频率不对声音会很小或不响

---

### 7. LED指示灯 (GPIO控制)

**模块规格**：
- 类型：普通LED
- 电流：10-20mA
- 电压：1.8-3.3V
- 接口：GPIO (1针)

**接线表**：

| LED引脚 | STM32引脚 | 引脚功能 | 说明 |
|---------|----------|----------|------|
| 正极 (+) | 220Ω电阻 → **PA15** | GPIO输出 | LED控制 |
| 负极 (-) | GND | 地 | 接地 |

**接线示意图**：
```
LED                  STM32F103
┌──────────┐         ┌──────┐
│ 正极 (+) │◄─[220Ω]─│ PA15 │ 推挽输出
│ 负极 (-) │────────►│ GND  │
└──────────┘         └──────┘
```

**代码配置** (Hardware/LED/LED.c)：
```c
// PA15配置为推挽输出
RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; // 低速即可
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;

// LED控制宏定义
#define LED_ON()  GPIO_ResetBits(GPIOA, GPIO_Pin_15) // 低电平点亮
#define LED_OFF() GPIO_SetBits(GPIOA, GPIO_Pin_15)   // 高电平熄灭
```

**注意事项**：
- ⚠️ 必须加限流电阻 (220Ω-1kΩ)
- ⚠️ GPIO输出电流最大25mA
- ⚠️ 大功率LED需要三极管/MOS管驱动

---

### 8. HW-269 电池电量检测 (ADC)

**模块规格**：
- 类型：电压检测模块
- 输入电压：0-25V
- 输出电压：0-3.3V (分压后)
- 接口：模拟输入 (ADC)

**接线表**：

| HW-269引脚 | STM32引脚 | 引脚功能 | 说明 |
|-----------|----------|----------|------|
| VCC | 电池正极 | - | 被测电压 |
| GND | GND | 地 | 接地 |
| OUT | **PA0** | ADC1_IN0 | 分压后电压 (0-3.3V) |

**接线示意图**：
```
HW-269               STM32F103
┌──────────┐         ┌──────┐
│ VCC      │◄────────│ 电池+ │ (5V-12V)
│ GND      │────────►│ GND   │
│ OUT      │────────►│ PA0   │ ADC1_IN0模拟输入
└──────────┘         └──────┘
```

**代码配置** (Hardware/HW269/HW269.c)：
```c
// PA0配置为模拟输入
RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE);

GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; // 模拟输入
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;

// ADC1配置
ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
ADC_InitStructure.ADC_ScanConvMode = DISABLE;
ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;

// 电压计算公式
// 实际电压 = (ADC值 / 4095) * 3.3V * 分压系数
float voltage = (ADC_Value / 4095.0) * 3.3 * 7.5; // 假设分压系数7.5
```

**注意事项**：
- ⚠️ OUT引脚电压不能超过3.3V
- ⚠️ 需要校准分压系数
- ⚠️ 建议使用滤波算法 (移动平均)

---

### 9. 串口调试 (USART1)

**接口规格**：
- 波特率：115200 (默认)
- 数据位：8位
- 停止位：1位
- 校验：无

**接线表**：

| 功能 | STM32引脚 | 引脚功能 | 说明 |
|------|----------|----------|------|
| TX | **PA9** | USART1_TX | 串口发送 |
| RX | **PA10** | USART1_RX | 串口接收 |

**接线示意图**：
```
USB转TTL             STM32F103
┌──────────┐         ┌──────┐
│ VCC      │ ────────│ 3.3V │ (可选，USB供电)
│ GND      │────────►│ GND  │
│ TXD      │────────►│ PA10 │ USART1_RX
│ RXD      │◄────────│ PA9  │ USART1_TX
└──────────┘         └──────┘
```

**代码配置** (System/USART/USART.c)：
```c
// PA9(TX), PA10(RX)配置
RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

// PA9 - TX 复用推挽输出
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;

// PA10 - RX 浮空输入
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;

// USART1配置
USART_InitStructure.USART_BaudRate = 115200;
USART_InitStructure.USART_WordLength = USART_WordLength_8b;
USART_InitStructure.USART_StopBits = USART_StopBits_1;
USART_InitStructure.USART_Parity = USART_Parity_No;
```

---

## 引脚分配原则

### STM32F103可用引脚分类

✅ **通用GPIO (推荐使用)**：
- PA0-PA15 (部分有复用功能)
- PB0-PB15 (部分有复用功能)
- PC13-PC15 (仅3个引脚，大封装才有PC0-PC12)

⚠️ **特殊功能引脚 (谨慎使用)**：
- **PA13/PA14**：SWD调试接口 (SWDIO/SWCLK) - **禁止占用**
- **PA9/PA10**：USART1 - 串口调试已占用
- **PA11/PA12**：USB (D-/D+) - USB功能冲突
- **PB2**：BOOT1引脚
- **PB3/PB4**：JTAG接口 (释放后可用)

❌ **禁止使用的引脚**：
- **PA13/PA14**：SWD调试 (占用后无法下载程序)

### 引脚分配策略

1. **I2C设备优先用硬件I2C**：
   - I2C1: PB8/PB9 (MPU6050)
   - I2C2: PB10/PB11 (备用)

2. **SPI设备优先用硬件SPI**：
   - SPI1: PA5/PA6/PA7 (RC522)
   - SPI2: PB13/PB14/PB15 (备用)

3. **定时器引脚用于PWM**：
   - TIM1_CH1: PA8 (蜂鸣器PWM)
   - TIM3_CH3/CH4: PB0/PB1 (超声波输入捕获)

4. **ADC引脚用于模拟输入**：
   - ADC1_IN0: PA0 (电池电量检测)

5. **独立GPIO用于单总线/简单IO**：
   - PC13: DHT22温湿度
   - PA15: LED指示灯
   - PA3/PA4: RC522的RST和CS

---

## 电源管理

### 电源引脚连接

| 引脚 | 连接 | 说明 |
|------|------|------|
| VDD | 3.3V | 数字电源 (多个VDD引脚全部连接) |
| VSS | GND | 数字地 (多个VSS引脚全部连接) |
| VDDA | 3.3V | 模拟电源 (ADC供电) |
| VSSA | GND | 模拟地 |
| VBAT | 3.3V 或 电池 | RTC备份电源 (可选) |

### 去耦电容配置

每个VDD引脚附近放置去耦电容：
- **100nF (0.1uF) 陶瓷电容** x 4-6个 (每个VDD一个)
- **10uF 钽电容** x 1个 (总电源处)

### 电源分配方案

```
                     5V USB供电
                       │
                       ├──► AMS1117-3.3V ──► 3.3V (STM32 VDD, OLED, RC522, MPU6050)
                       │                     │
                       │                     ├──► 100nF × 6 (去耦电容)
                       │                     └──► 10uF × 1
                       │
                       └──► HC-SR04 (5V供电)
```

**注意事项**：
- ⚠️ STM32和大部分传感器用3.3V
- ⚠️ HC-SR04超声波需要5V供电
- ⚠️ RC522只能用3.3V (接5V会烧毁)
- ⚠️ 总电流估算：STM32 50mA + OLED 20mA + 传感器 100mA ≈ 200mA

---

## 接线注意事项

### 关键注意事项 ⚠️

1. **电压匹配**：
   - STM32 GPIO耐压：3.3V (5V容忍输入)
   - RC522必须3.3V供电
   - HC-SR04需要5V供电

2. **调试接口保护**：
   - 禁止占用PA13/PA14 (SWD)
   - 占用后无法下载程序

3. **I2C上拉电阻**：
   - 硬件I2C需要外部上拉 (4.7kΩ)
   - 软件I2C用开漏输出自带上拉

4. **SPI片选信号**：
   - RC522的SDA实际是CS片选
   - 空闲时保持高电平

5. **定时器冲突**：
   - 检查PWM/输入捕获是否占用同一定时器

### 推荐接线顺序

1. ✅ 先接电源和地 (VDD, GND)
2. ✅ 再接调试接口 (USART1)
3. ✅ 逐个测试外设模块
4. ✅ 最后集成所有模块

### 测试建议

- **第一阶段**：单独测试每个模块 (Tests目录)
- **第二阶段**：两两组合测试 (检查引脚冲突)
- **第三阶段**：完整集成测试 (App目录)

---

## 引脚快速查询表

### 按引脚编号排序

| 引脚 | 功能1 | 功能2 | 复用功能 | 已分配 |
|------|-------|-------|----------|--------|
| PA0 | GPIO | ADC1_IN0 | HW-269电量检测 | ✅ |
| PA1 | GPIO | TIM2_CH2 | (MPU6050 INT可选) | 可选 |
| PA3 | GPIO | - | RC522 RST | ✅ |
| PA4 | GPIO | - | RC522 CS | ✅ |
| PA5 | GPIO | SPI1_SCK | RC522 SCK | ✅ |
| PA6 | GPIO | SPI1_MISO | RC522 MISO | ✅ |
| PA7 | GPIO | SPI1_MOSI | RC522 MOSI | ✅ |
| PA8 | GPIO | TIM1_CH1 | 蜂鸣器PWM | ✅ |
| PA9 | USART1_TX | TIM1_CH2 | 串口调试TX | ✅ |
| PA10 | USART1_RX | TIM1_CH3 | 串口调试RX | ✅ |
| PA13 | SWDIO | - | SWD调试 | ❌禁用 |
| PA14 | SWCLK | - | SWD调试 | ❌禁用 |
| PA15 | GPIO | - | LED指示灯 | ✅ |
| PB0 | GPIO | TIM3_CH3 | SR04 Trig | ✅ |
| PB1 | GPIO | TIM3_CH4 | SR04 Echo | ✅ |
| PB6 | GPIO | I2C1_SCL | OLED SCL (软件I2C) | ✅ |
| PB7 | GPIO | I2C1_SDA | OLED SDA (软件I2C) | ✅ |
| PB8 | GPIO | I2C1_SCL | MPU6050 SCL (硬件I2C) | ✅ |
| PB9 | GPIO | I2C1_SDA | MPU6050 SDA (硬件I2C) | ✅ |
| PC13 | GPIO | - | DHT22数据线 | ✅ |

### 按功能模块排序

| 模块 | 引脚 | 状态 |
|------|------|------|
| OLED | PB6, PB7 | ✅已测试 |
| MPU6050 | PB8, PB9 | 待测试 |
| RC522 | PA3-PA7 | 待测试 |
| DHT22 | PC13 | 待测试 |
| HC-SR04 | PB0, PB1 | 待测试 |
| 蜂鸣器 | PA8 | 待测试 |
| LED | PA15 | 待测试 |
| HW-269 | PA0 | 待测试 |
| 串口 | PA9, PA10 | 系统必需 |

---

## 附录：代码模板

### 1. GPIO输出初始化模板
```c
void GPIO_Output_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 开启时钟
    if (GPIOx == GPIOA) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    if (GPIOx == GPIOB) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    if (GPIOx == GPIOC) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    
    // 配置为推挽输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin;
    GPIO_Init(GPIOx, &GPIO_InitStructure);
}
```

### 2. GPIO输入初始化模板
```c
void GPIO_Input_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIOMode_TypeDef mode)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 开启时钟
    if (GPIOx == GPIOA) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    if (GPIOx == GPIOB) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    if (GPIOx == GPIOC) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    
    // 配置为输入模式 (GPIO_Mode_IN_FLOATING / GPIO_Mode_IPU / GPIO_Mode_IPD)
    GPIO_InitStructure.GPIO_Mode = mode;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin;
    GPIO_Init(GPIOx, &GPIO_InitStructure);
}
```

---

**文档版本**: v1.0  
**创建日期**: 2025-12-15  
**适用芯片**: STM32F103C8T6/CB/RB  
**开发环境**: Keil MDK-ARM 5.x + STM32标准库  
**状态**: OLED模块已测试通过 ✅
