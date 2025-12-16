#include "BH1750.h"
#include "Delay.h"
#include "stm32f10x.h"

// Use PB6=SCL PB7=SDA (shared with OLED)
#define I2C_PORT_BH    GPIOB
#define I2C_SCL_PIN_BH GPIO_Pin_6
#define I2C_SDA_PIN_BH GPIO_Pin_7
#define I2C_RCC_BH     RCC_APB2Periph_GPIOB

#define BH1750_ADDR7   0x23
#define BH1750_ADDR_W  (BH1750_ADDR7 << 1)
#define BH1750_ADDR_R  ((BH1750_ADDR7 << 1) | 0x01)

// Commands
#define BH1750_POWER_ON    0x01
#define BH1750_RESET       0x07
#define BH1750_CONT_H_RES  0x10

static void I2C_Delay_BH(void)
{
    volatile uint16_t i = 40; // increase delay for more stable timing
    while(i--);
}

static void SCL_H_BH(void) { GPIO_SetBits(I2C_PORT_BH, I2C_SCL_PIN_BH); }
static void SCL_L_BH(void) { GPIO_ResetBits(I2C_PORT_BH, I2C_SCL_PIN_BH); }
static void SDA_H_BH(void) { GPIO_SetBits(I2C_PORT_BH, I2C_SDA_PIN_BH); }
static void SDA_L_BH(void) { GPIO_ResetBits(I2C_PORT_BH, I2C_SDA_PIN_BH); }
static uint8_t SDA_READ_BH(void) { return GPIO_ReadInputDataBit(I2C_PORT_BH, I2C_SDA_PIN_BH); }

static void SDA_OUT_BH(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = I2C_SDA_PIN_BH;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; // open-drain
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(I2C_PORT_BH, &GPIO_InitStructure);
}

static void SDA_IN_BH(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = I2C_SDA_PIN_BH;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // pull-up input
    GPIO_Init(I2C_PORT_BH, &GPIO_InitStructure);
}

static void I2C_Start_BH(void)
{
    SDA_OUT_BH();
    SDA_H_BH();
    SCL_H_BH();
    I2C_Delay_BH();
    SDA_L_BH();
    I2C_Delay_BH();
    SCL_L_BH();
}

static void I2C_Stop_BH(void)
{
    SDA_OUT_BH();
    SCL_L_BH();
    SDA_L_BH();
    I2C_Delay_BH();
    SCL_H_BH();
    I2C_Delay_BH();
    SDA_H_BH();
}

static uint8_t I2C_WaitAck_BH(void)
{
    uint32_t timeout = 0;
    SDA_IN_BH();
    SDA_H_BH();
    I2C_Delay_BH();
    SCL_H_BH();
    I2C_Delay_BH();
    while (SDA_READ_BH())
    {
        if (++timeout > 5000)
        {
            // Attempt a simple bus recovery: toggle SCL a few times while SDA released
            SCL_L_BH();
            for (int k = 0; k < 9; k++)
            {
                SCL_H_BH();
                I2C_Delay_BH();
                SCL_L_BH();
                I2C_Delay_BH();
            }
            I2C_Stop_BH();
            return 1;
        }
    }
    SCL_L_BH();
    return 0;
}

static void I2C_Ack_BH(void)
{
    SCL_L_BH();
    SDA_OUT_BH();
    SDA_L_BH();
    I2C_Delay_BH();
    SCL_H_BH();
    I2C_Delay_BH();
    SCL_L_BH();
}

static void I2C_NAck_BH(void)
{
    SCL_L_BH();
    SDA_OUT_BH();
    SDA_H_BH();
    I2C_Delay_BH();
    SCL_H_BH();
    I2C_Delay_BH();
    SCL_L_BH();
}

static void I2C_SendByte_BH(uint8_t data)
{
    uint8_t i;
    SDA_OUT_BH();
    SCL_L_BH();
    for (i = 0; i < 8; i++)
    {
        if (data & 0x80) SDA_H_BH(); else SDA_L_BH();
        data <<= 1;
        I2C_Delay_BH();
        SCL_H_BH();
        I2C_Delay_BH();
        SCL_L_BH();
        I2C_Delay_BH();
    }
}

static uint8_t I2C_ReadByte_BH(uint8_t ack)
{
    uint8_t i, data = 0;
    SDA_IN_BH();
    for (i = 0; i < 8; i++)
    {
        SCL_L_BH();
        I2C_Delay_BH();
        SCL_H_BH();
        data <<= 1;
        if (SDA_READ_BH()) data |= 1;
        I2C_Delay_BH();
    }
    if (ack) I2C_Ack_BH(); else I2C_NAck_BH();
    return data;
}

static void I2C_WriteCmd_BH(uint8_t cmd)
{
    I2C_Start_BH();
    I2C_SendByte_BH(BH1750_ADDR_W);
    if (I2C_WaitAck_BH()) { I2C_Stop_BH(); return; }
    I2C_SendByte_BH(cmd);
    I2C_WaitAck_BH();
    I2C_Stop_BH();
}

static int I2C_ReadBytes_BH(uint8_t *buf, uint8_t len)
{
    uint8_t i;
    I2C_Start_BH();
    I2C_SendByte_BH(BH1750_ADDR_R);
    if (I2C_WaitAck_BH()) { I2C_Stop_BH(); return -1; }
    for (i = 0; i < len; i++)
    {
        if (i == len - 1) buf[i] = I2C_ReadByte_BH(0);
        else buf[i] = I2C_ReadByte_BH(1);
    }
    I2C_Stop_BH();
    return 0;
}

void BH1750_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    // Enable GPIOB clock
    RCC_APB2PeriphClockCmd(I2C_RCC_BH, ENABLE);

    // Configure SCL/SDA as open-drain outputs and set high
    GPIO_InitStructure.GPIO_Pin = I2C_SCL_PIN_BH | I2C_SDA_PIN_BH;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(I2C_PORT_BH, &GPIO_InitStructure);

    SCL_H_BH(); SDA_H_BH();
    Delay_ms(10);

    // Power on and reset
    I2C_WriteCmd_BH(BH1750_POWER_ON);
    Delay_ms(10);
    I2C_WriteCmd_BH(BH1750_RESET);
    Delay_ms(10);
    // Start continuous high-res mode
    I2C_WriteCmd_BH(BH1750_CONT_H_RES);
}

int BH1750_ReadLux(float *lux)
{
    uint8_t buf[2];
    // Try a couple of times if bus is noisy or device not ready
    for (int attempt = 0; attempt < 3; attempt++)
    {
        if (I2C_ReadBytes_BH(buf, 2) == 0) break;
        // small delay and try to re-issue continuous mode
        Delay_ms(120);
        I2C_WriteCmd_BH(BH1750_CONT_H_RES);
        Delay_ms(10);
        if (attempt == 2) return -1;
    }
    uint16_t raw = (buf[0] << 8) | buf[1];
    // per datasheet: lux = raw / 1.2
    if (lux) *lux = raw / 1.2f;
    return 0;
}
