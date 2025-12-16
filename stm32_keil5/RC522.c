#include "RC522.h"
#include "stm32f10x.h"
#include "Delay.h"

// RC522接线：PA5=SCK, PA6=MISO, PA7=MOSI, PA4=CS, PA3=RST
#define RC522_CS_PIN      GPIO_Pin_4
#define RC522_RST_PIN     GPIO_Pin_3
#define RC522_PORT        GPIOA

#define RC522_CS_LOW()    GPIO_ResetBits(RC522_PORT, RC522_CS_PIN)
#define RC522_CS_HIGH()   GPIO_SetBits(RC522_PORT, RC522_CS_PIN)
#define RC522_RST_LOW()   GPIO_ResetBits(RC522_PORT, RC522_RST_PIN)
#define RC522_RST_HIGH()  GPIO_SetBits(RC522_PORT, RC522_RST_PIN)

static void RC522_SPI_Init(void);
static uint8_t RC522_SPI_Transfer(uint8_t data);
static void RC522_WriteReg(uint8_t addr, uint8_t val);
static uint8_t RC522_ReadReg(uint8_t addr);
static void RC522_SetBitMask(uint8_t reg, uint8_t mask);
static void RC522_ClearBitMask(uint8_t reg, uint8_t mask);
static void RC522_AntennaOn(void);

static void RC522_SPI_Init(void)
{
    SPI_InitTypeDef SPI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    // 使能时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_GPIOA, ENABLE);

    // 配置SPI引脚：PA5=SCK, PA6=MISO, PA7=MOSI
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置CS和RST为推挽输出
    GPIO_InitStructure.GPIO_Pin = RC522_CS_PIN | RC522_RST_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(RC522_PORT, &GPIO_InitStructure);

    RC522_CS_HIGH();
    RC522_RST_HIGH();

    // 配置SPI1
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStructure);

    SPI_Cmd(SPI1, ENABLE);
}

static uint8_t RC522_SPI_Transfer(uint8_t data)
{
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI1, data);
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    return SPI_I2S_ReceiveData(SPI1);
}

static void RC522_WriteReg(uint8_t addr, uint8_t val)
{
    RC522_CS_LOW();
    RC522_SPI_Transfer((addr << 1) & 0x7E);
    RC522_SPI_Transfer(val);
    RC522_CS_HIGH();
}

static uint8_t RC522_ReadReg(uint8_t addr)
{
    uint8_t val;
    RC522_CS_LOW();
    RC522_SPI_Transfer(((addr << 1) & 0x7E) | 0x80);
    val = RC522_SPI_Transfer(0x00);
    RC522_CS_HIGH();
    return val;
}

static void RC522_SetBitMask(uint8_t reg, uint8_t mask)
{
    uint8_t tmp = RC522_ReadReg(reg);
    RC522_WriteReg(reg, tmp | mask);
}

static void RC522_ClearBitMask(uint8_t reg, uint8_t mask)
{
    uint8_t tmp = RC522_ReadReg(reg);
    RC522_WriteReg(reg, tmp & (~mask));
}

static void RC522_AntennaOn(void)
{
    uint8_t temp = RC522_ReadReg(TxControlReg);
    if (!(temp & 0x03))
    {
        RC522_SetBitMask(TxControlReg, 0x03);
    }
}

void RC522_Reset(void)
{
    RC522_WriteReg(CommandReg, PCD_RESETPHASE);
    Delay_ms(10);
}

void RC522_Init(void)
{
    RC522_SPI_Init();
    RC522_RST_LOW();
    Delay_ms(10);
    RC522_RST_HIGH();
    Delay_ms(10);

    RC522_Reset();

    RC522_WriteReg(TModeReg, 0x8D);
    RC522_WriteReg(TPrescalerReg, 0x3E);
    RC522_WriteReg(TReloadRegL, 30);
    RC522_WriteReg(TReloadRegH, 0);
    RC522_WriteReg(TxASKReg, 0x40);
    RC522_WriteReg(ModeReg, 0x3D);

    RC522_AntennaOn();
}

uint8_t RC522_ToCard(uint8_t command, uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint16_t *backLen)
{
    uint8_t status = MI_ERR;
    uint8_t irqEn = 0x00;
    uint8_t waitIRq = 0x00;
    uint8_t lastBits;
    uint8_t n;
    uint16_t i;

    switch (command)
    {
    case PCD_AUTHENT:
        irqEn = 0x12;
        waitIRq = 0x10;
        break;
    case PCD_TRANSCEIVE:
        irqEn = 0x77;
        waitIRq = 0x30;
        break;
    default:
        break;
    }

    RC522_WriteReg(CommIEnReg, irqEn | 0x80);
    RC522_ClearBitMask(CommIrqReg, 0x80);
    RC522_SetBitMask(FIFOLevelReg, 0x80);

    RC522_WriteReg(CommandReg, PCD_IDLE);

    for (i = 0; i < sendLen; i++)
    {
        RC522_WriteReg(FIFODataReg, sendData[i]);
    }

    RC522_WriteReg(CommandReg, command);
    if (command == PCD_TRANSCEIVE)
    {
        RC522_SetBitMask(BitFramingReg, 0x80);
    }

    i = 2000;
    do
    {
        n = RC522_ReadReg(CommIrqReg);
        i--;
    } while ((i != 0) && !(n & 0x01) && !(n & waitIRq));

    RC522_ClearBitMask(BitFramingReg, 0x80);

    if (i != 0)
    {
        if (!(RC522_ReadReg(ErrorReg) & 0x1B))
        {
            status = MI_OK;

            if (n & irqEn & 0x01)
            {
                status = MI_NOTAGERR;
            }

            if (command == PCD_TRANSCEIVE)
            {
                n = RC522_ReadReg(FIFOLevelReg);
                lastBits = RC522_ReadReg(ControlReg) & 0x07;
                if (lastBits)
                {
                    *backLen = (n - 1) * 8 + lastBits;
                }
                else
                {
                    *backLen = n * 8;
                }

                if (n == 0)
                {
                    n = 1;
                }
                if (n > 16)
                {
                    n = 16;
                }

                for (i = 0; i < n; i++)
                {
                    backData[i] = RC522_ReadReg(FIFODataReg);
                }
            }
        }
        else
        {
            status = MI_ERR;
        }
    }

    return status;
}

uint8_t RC522_Request(uint8_t reqMode, uint8_t *TagType)
{
    uint8_t status;
    uint16_t backBits;

    RC522_WriteReg(BitFramingReg, 0x07);

    TagType[0] = reqMode;
    status = RC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);

    if ((status != MI_OK) || (backBits != 0x10))
    {
        status = MI_ERR;
    }

    return status;
}

uint8_t RC522_Anticoll(uint8_t *serNum)
{
    uint8_t status;
    uint8_t i;
    uint8_t serNumCheck = 0;
    uint16_t unLen;

    RC522_WriteReg(BitFramingReg, 0x00);
    serNum[0] = PICC_ANTICOLL;
    serNum[1] = 0x20;

    status = RC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

    if (status == MI_OK)
    {
        for (i = 0; i < 4; i++)
        {
            serNumCheck ^= serNum[i];
        }
        if (serNumCheck != serNum[i])
        {
            status = MI_ERR;
        }
    }

    return status;
}
