#include "DHT22.h"
#include "Delay.h"

extern uint8_t debug_step;  // 外部调试步骤变量

// DHT22引脚定义 (PB10) - PC13可能是RTC引脚有问题
#define DHT22_PORT      GPIOB
#define DHT22_PIN       GPIO_Pin_10
#define DHT22_RCC       RCC_APB2Periph_GPIOB

// 引脚操作宏
#define DHT22_HIGH()    GPIO_SetBits(DHT22_PORT, DHT22_PIN)
#define DHT22_LOW()     GPIO_ResetBits(DHT22_PORT, DHT22_PIN)
#define DHT22_READ()    GPIO_ReadInputDataBit(DHT22_PORT, DHT22_PIN)

// 设置引脚模式
static void DHT22_SetOutput(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = DHT22_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DHT22_PORT, &GPIO_InitStructure);
}

static void DHT22_SetInput(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = DHT22_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  // 浮空输入（已有外部上拉电阻）
    GPIO_Init(DHT22_PORT, &GPIO_InitStructure);
}

void DHT22_Init(void)
{
    // 使能GPIO时钟
    RCC_APB2PeriphClockCmd(DHT22_RCC, ENABLE);
    
    // 初始化为输出高电平
    DHT22_SetOutput();
    DHT22_HIGH();
    Delay_ms(1000);  // 上电后等待1秒稳定
}

uint8_t DHT22_Read(DHT22_Data *data)
{
    uint8_t i, j;
    uint8_t buffer[5] = {0};  // 5字节数据缓冲区
    uint32_t timeout;
    
    data->valid = 0;
    debug_step = 1;
    
    // 1. 发送起始信号: 主机拉低1-10ms
    DHT22_SetOutput();
    DHT22_LOW();
    Delay_ms(1);  // 拉低1ms即可
    DHT22_HIGH();
    Delay_us(20);  // 拉高20us
    
    // 2. 切换为输入模式，释放总线
    DHT22_SetInput();
    
    debug_step = 2;
    // 3. 等待DHT22拉低（响应信号，80us）
    timeout = 100;
    while (DHT22_READ() == 1 && timeout > 0) { timeout--; }
    if (timeout == 0) return 0;  // Step 2失败
    
    debug_step = 3;
    // 4. 等待DHT22拉高（响应信号，80us）
    timeout = 100;
    while (DHT22_READ() == 0 && timeout > 0) { timeout--; }
    if (timeout == 0) return 0;  // Step 3失败
    
    debug_step = 4;
    // 5. 等待DHT22再次拉低（准备发送数据）
    timeout = 100;
    while (DHT22_READ() == 1 && timeout > 0) { timeout--; }
    if (timeout == 0) return 0;  // Step 4失败
    
    debug_step = 5;
    // 6. 读取40位数据（5字节）
    for (i = 0; i < 5; i++)
    {
        for (j = 0; j < 8; j++)
        {
            // 等待数据位起始（低电平50us）
            timeout = 100;
            while (DHT22_READ() == 0 && timeout > 0) { timeout--; }
            if (timeout == 0) return 0;  // Step 5失败
            
            // 延时30us后读取电平判断数据位
            // 数据0: 50us低+26-28us高
            // 数据1: 50us低+70us高
            Delay_us(30);
            
            buffer[i] <<= 1;
            if (DHT22_READ() == 1)  // 如果还是高电平，则为'1'
            {
                buffer[i] |= 1;
            }
            
            // 等待高电平结束
            timeout = 100;
            while (DHT22_READ() == 1 && timeout > 0) { timeout--; }
        }
    }
    
    // 7. 恢复输出模式
    DHT22_SetOutput();
    DHT22_HIGH();
    
    // 8. 校验数据
    uint8_t checksum = buffer[0] + buffer[1] + buffer[2] + buffer[3];
    if (checksum != buffer[4])
        return 0;  // 校验失败
    
    // 9. 解析数据
    // 湿度: buffer[0]高字节, buffer[1]低字节
    data->humidity = ((buffer[0] << 8) | buffer[1]) / 10.0f;
    
    // 温度: buffer[2]高字节, buffer[3]低字节
    uint16_t temp_raw = (buffer[2] << 8) | buffer[3];
    if (temp_raw & 0x8000)  // 负温度
    {
        temp_raw &= 0x7FFF;
        data->temperature = -(temp_raw / 10.0f);
    }
    else
    {
        data->temperature = temp_raw / 10.0f;
    }
    
    data->valid = 1;
    return 1;
}