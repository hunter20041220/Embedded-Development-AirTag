#include "stm32f10x.h"
#include "OLED.h"
#include "RC522.h"
#include "BH1750.h"
#include "MPU6050.h"
#include "HCSR04.h"
#include "LED.h"
#include "Buzzer.h"
#include "Delay.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Global debug variable
uint8_t debug_step = 0;

// 系统状态定义
typedef enum {
    MODE_CHICKEN = 0,   // 小鸡模式
    MODE_ALERT = 1      // 警戒模式
} SystemMode;

// 报警状态
typedef struct {
    uint8_t lowLight;      // 低光照报警
    uint8_t fastMove;      // 快速移动报警
    uint8_t outdoor;       // 室外检测报警
} AlarmState;

// 全局变量
static SystemMode currentMode = MODE_CHICKEN;
static AlarmState alarmState = {0, 0, 0};
static uint32_t ledToggleTime = 0;
static uint8_t ledState = 0;
static int16_t lastAccel[3] = {0, 0, 0};
static uint8_t accelInitialized = 0;
static uint8_t fastMoveAlarmCounter = 0;  // 快速移动报警持续计数器

// 小鸡模式相关变量
static uint32_t chickenStepCount = 0;     // 小鸡步数计数
static uint32_t lastStepTime = 0;         // 上次计步时间（防抖）
static uint8_t stepLedFlashCounter = 0;   // LED闪烁计数器

// 阈值设定
#define LIGHT_THRESHOLD      50.0f   // 光照阈值 (lx) - 低于此值认为在背包内
#define ACCEL_CHANGE_THRESHOLD 3000  // 加速度变化阈值 - 超过此值认为快速移动（降低以提高灵敏度）
#define DISTANCE_CHANGE_THRESHOLD 50.0f // 距离变化阈值 (cm) - 超过此值认为环境变化大
#define FAST_MOVE_ALARM_DURATION 15  // 快速移动报警持续次数（约3秒）

// RFID卡片UID定义
#define CARD_CHICKEN_UID {0xD4, 0xC8, 0xF5, 0x05}  // 小鸡标识卡
#define CARD_ALERT_UID   {0x01, 0x2C, 0x99, 0x02}  // 警戒模式卡

// 函数声明
static void CheckRFID(void);
static void UpdateSensors(float *lux, int16_t *accel, float *distance);
static void CheckAlarms(float lux, int16_t *accel, float distance);
static void CheckChickenSteps(int16_t *accel);
static void UpdateDisplay(float lux, int16_t *accel);
static void ControlAlarm(void);
static uint8_t CompareUID(uint8_t *uid1, uint8_t *uid2);

int main(void)
{
    float lux = 0.0f;
    int16_t accel[3] = {0, 0, 0};
    float distance = 0.0f;
    MPU6050_Data mpu_data;
    uint8_t loopCounter = 0;

    // 初始化所有模块
    OLED_Init();
    OLED_Clear();
    OLED_ShowString(1, 1, "System Init...");
    
    RC522_Init();
    BH1750_Init();
    MPU6050_Init();
    HCSR04_Init();
    LED_Init();
    Buzzer_Init();
    
    Delay_ms(500);
    OLED_Clear();
    
    // 默认小鸡模式
    currentMode = MODE_CHICKEN;
    LED_Off();
    
    while (1)
    {
        // 每次循环检查RFID
        CheckRFID();
        
        // 每4个循环（约200ms）读取传感器数据
        loopCounter++;
        if (loopCounter >= 4)
        {
            loopCounter = 0;
            
            // 读取传感器
            UpdateSensors(&lux, accel, &distance);
            
            // 检查报警条件（仅在警戒模式）
            if (currentMode == MODE_ALERT)
            {
                CheckAlarms(lux, accel, distance);
            }
            else if (currentMode == MODE_CHICKEN)
            {
                // 小鸡模式：检测步数
                CheckChickenSteps(accel);
                alarmState.lowLight = 0;
                alarmState.fastMove = 0;
                alarmState.outdoor = 0;
            }
            
            // 更新显示
            UpdateDisplay(lux, accel);
        }
        
        // 每次循环都控制报警器（确保LED持续闪烁）
        ControlAlarm();
        
        Delay_ms(50);
    }
}

// 检查RFID卡片并切换模式
static void CheckRFID(void)
{
    uint8_t uid[5];
    uint8_t status;
    uint8_t chickenUID[] = CARD_CHICKEN_UID;
    uint8_t alertUID[] = CARD_ALERT_UID;
    
    status = RC522_Request(PICC_REQIDL, uid);
    if (status == MI_OK)
    {
        status = RC522_Anticoll(uid);
        if (status == MI_OK)
        {
            // 比对UID并切换模式
            if (CompareUID(uid, chickenUID))
            {
                if (currentMode != MODE_CHICKEN)
                {
                    currentMode = MODE_CHICKEN;
                    LED_Off();
                    OLED_Clear();
                    
                    // 蜂鸣器响一声
                    Buzzer_Beep(200);
                    
                    Delay_ms(100);
                }
            }
            else if (CompareUID(uid, alertUID))
            {
                if (currentMode != MODE_ALERT)
                {
                    currentMode = MODE_ALERT;
                    OLED_Clear();
                    Delay_ms(100);
                }
            }
            
            Delay_ms(500); // 防抖
        }
    }
}

// 读取传感器数据
static void UpdateSensors(float *lux, int16_t *accel, float *distance)
{
    MPU6050_Data mpu_data;
    
    // 读取光照
    if (BH1750_ReadLux(lux) != 0)
    {
        *lux = 0.0f;
    }
    
    // 读取加速度
    if (MPU6050_Read(&mpu_data))
    {
        accel[0] = mpu_data.accel_x;
        accel[1] = mpu_data.accel_y;
        accel[2] = mpu_data.accel_z;
    }
    
    // 读取距离
    *distance = HCSR04_GetDistance();
}

// 检查报警条件
static void CheckAlarms(float lux, int16_t *accel, float distance)
{
    static float lastDistance = 0.0f;
    int32_t accelChange;  // 使用32位避免溢出
    
    // 1. 检查低光照（背包内）
    if (lux < LIGHT_THRESHOLD)
    {
        alarmState.lowLight = 1;
    }
    else
    {
        alarmState.lowLight = 0;
    }
    
    // 2. 检查快速移动（加速度突变）- 改进的检测逻辑
    if (accelInitialized)
    {
        // 计算三轴加速度变化的绝对值之和
        accelChange = abs(accel[0] - lastAccel[0]) + 
                      abs(accel[1] - lastAccel[1]) + 
                      abs(accel[2] - lastAccel[2]);
        
        // 检测到快速移动
        if (accelChange > ACCEL_CHANGE_THRESHOLD)
        {
            alarmState.fastMove = 1;
            fastMoveAlarmCounter = FAST_MOVE_ALARM_DURATION;  // 重置持续计数器
        }
        else
        {
            // 持续报警机制：检测到快速移动后持续报警一段时间
            if (fastMoveAlarmCounter > 0)
            {
                fastMoveAlarmCounter--;
                alarmState.fastMove = 1;
            }
            else
            {
                alarmState.fastMove = 0;
            }
        }
    }
    else
    {
        // 首次读取，初始化历史数据
        accelInitialized = 1;
        alarmState.fastMove = 0;
    }
    
    // 3. 检查室外/距离变化（超声波）
    if (lastDistance > 0)
    {
        float distChange = distance - lastDistance;
        if (distChange < 0) distChange = -distChange;
        
        if (distChange > DISTANCE_CHANGE_THRESHOLD)
        {
            alarmState.outdoor = 1;
        }
        else
        {
            alarmState.outdoor = 0;
        }
    }
    
    // 更新历史数据
    lastAccel[0] = accel[0];
    lastAccel[1] = accel[1];
    lastAccel[2] = accel[2];
    lastDistance = distance;
}

// 小鸡模式：检测X轴加速度变化计步
#define STEP_THRESHOLD 500        // 加速度变化阈值（X轴）
#define STEP_MIN_INTERVAL 10      // 最小步间隔（循环次数），防止重复计数
static void CheckChickenSteps(int16_t *accel)
{
    static uint32_t loopCounter = 0;
    int32_t accelChange;
    
    loopCounter++;  // 循环计数器
    
    if (!accelInitialized)
    {
        // 首次初始化
        accelInitialized = 1;
        lastAccel[0] = accel[0];
        return;
    }
    
    // 计算X轴加速度变化
    accelChange = abs(accel[0] - lastAccel[0]);
    
    // 检测到变化且超过防抖间隔
    if (accelChange > STEP_THRESHOLD && (loopCounter - lastStepTime) > STEP_MIN_INTERVAL)
    {
        chickenStepCount++;
        lastStepTime = loopCounter;
        stepLedFlashCounter = 2;  // LED闪烁2次（快闪）
        Buzzer_Beep(50);          // 蜂鸣器响50ms
    }
    
    // 更新历史数据
    lastAccel[0] = accel[0];
}

// 更新OLED显示
static void UpdateDisplay(float lux, int16_t *accel)
{
    char str[32];
    static uint8_t lastMode = 0xFF;
    static float lastLux = -1.0f;
    static int16_t lastDispAccel = -32768;
    static uint8_t lastAlarmType = 0xFF;
    static uint32_t lastStepDisplay = 0xFFFFFFFF;
    uint8_t currentAlarmType = 0;
    
    // 第1行：显示当前模式（仅在模式变化时更新）
    if (lastMode != currentMode)
    {
        lastMode = currentMode;
        if (currentMode == MODE_CHICKEN)
        {
            OLED_ShowString(1, 1, "Mode: CHICKEN  ");
        }
        else
        {
            OLED_ShowString(1, 1, "Mode: ALERT    ");
        }
    }
    
    // 小鸡模式的特殊显示
    if (currentMode == MODE_CHICKEN)
    {
        // 第2行：小鸡标识
        OLED_ShowString(2, 1, "ID: Chick #001  ");
        
        // 第3行：步数（仅在变化时更新）
        if (lastStepDisplay != chickenStepCount)
        {
            lastStepDisplay = chickenStepCount;
            sprintf(str, "Steps: %u    ", (unsigned int)chickenStepCount);
            OLED_ShowString(3, 1, str);
        }
        
        // 第4行：状态
        OLED_ShowString(4, 1, "Status: No Lost ");
        return;
    }
    
    // 警戒模式显示
    // 第2行：光照强度（仅在值变化时更新）
    if (lastLux != lux)
    {
        lastLux = lux;
        sprintf(str, "Lux:%5.0f lx   ", lux);
        OLED_ShowString(2, 1, str);
    }
    
    // 第3行：加速度X轴（仅在值变化超过阈值时更新）
    if (abs(accel[0] - lastDispAccel) > 100)
    {
        lastDispAccel = accel[0];
        sprintf(str, "Ax:%6d      ", accel[0]);
        OLED_ShowString(3, 1, str);
    }
    
    // 第4行：报警状态（仅在状态变化时更新）
    if (currentMode == MODE_ALERT && (alarmState.lowLight || alarmState.fastMove || alarmState.outdoor))
    {
        if (alarmState.lowLight)
            currentAlarmType = 1;
        else if (alarmState.fastMove)
            currentAlarmType = 2;
        else if (alarmState.outdoor)
            currentAlarmType = 3;
    }
    
    if (lastAlarmType != currentAlarmType)
    {
        lastAlarmType = currentAlarmType;
        
        if (currentAlarmType == 1)
            OLED_ShowString(4, 1, "ALARM: Low Light");
        else if (currentAlarmType == 2)
            OLED_ShowString(4, 1, "ALARM: Fast Move");
        else if (currentAlarmType == 3)
            OLED_ShowString(4, 1, "ALARM: Outdoor  ");
        else
            OLED_ShowString(4, 1, "Status: OK      ");
    }
}

// 控制报警器（LED和蜂鸣器）
static void ControlAlarm(void)
{
    static uint8_t toggleCounter = 0;
    uint8_t alarmTriggered = 0;
    
    if (currentMode == MODE_ALERT)
    {
        alarmTriggered = alarmState.lowLight || alarmState.fastMove || alarmState.outdoor;
    }
    
    if (alarmTriggered)
    {
        // LED闪烁（每次调用递增，每3次切换，即150ms切换一次）
        toggleCounter++;
        if (toggleCounter >= 3)
        {
            toggleCounter = 0;
            if (ledState)
            {
                LED_Off();
                ledState = 0;
            }
            else
            {
                LED_On();
                ledState = 1;
            }
        }
        
        // 蜂鸣器响
        Buzzer_On();
    }
    else if (currentMode == MODE_CHICKEN)
    {
        // 小鸡模式：步数增加时LED闪烁
        if (stepLedFlashCounter > 0)
        {
            stepLedFlashCounter--;
            toggleCounter++;
            if (toggleCounter >= 1)  // 快速闪烁
            {
                toggleCounter = 0;
                if (ledState)
                {
                    LED_Off();
                    ledState = 0;
                }
                else
                {
                    LED_On();
                    ledState = 1;
                }
            }
        }
        else
        {
            // 无步数变化时LED关闭
            LED_Off();
            ledState = 0;
            toggleCounter = 0;
        }
        Buzzer_Off();
    }
    else
    {
        // 其他模式
        toggleCounter = 0;
        ledState = 0;
        LED_Off();
        Buzzer_Off();
    }
}

// 比较两个UID
static uint8_t CompareUID(uint8_t *uid1, uint8_t *uid2)
{
    return (uid1[0] == uid2[0] && uid1[1] == uid2[1] && 
            uid1[2] == uid2[2] && uid1[3] == uid2[3]);
}

