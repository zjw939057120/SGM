#include "spi.h"

#include "SGM58201.h"

// use SPI1
// GPIOA pin4=CS, software control CS
// GPIOA pin5=SCLK
// GPIOA pin6=MISO
// GPIOA pin7=MOSI

/* 初始化片选引脚和SPI外设 */
void SGM58201_interface_init(void)
{
	
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	// set sync pin as GPIO output, PA4
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_SetBits(GPIOA, GPIO_Pin_4);
	
	// set drdy pin as GPIO input, PE2
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	// SPI Mode 1，SCLK = 84/64MHz = 1.3MHz
	SPI1_Init(SPI_DataSize_8b, SPI_CPOL_Low, SPI_CPHA_2Edge, SPI_BaudRatePrescaler_64);	
}

/* RESET-0x06, START-0x08 ... */
void SGM58201_send_cmd(u8 cmd)
{
	CS_LOW;
	
	/* 发送命令 */
	SPI1_ReadWrite(cmd);

	CS_HIGH;
}

void SGM58201_write_register(u8 reg_addr, u8 value)
{
	u8 cmd = 0;
	/* 使用WREG命令写寄存器，将value写入选中的寄存器，
     * 以给定地址为起始地址，写入1字节数据。
     */
	
	cmd = CMD_WREG | (reg_addr << 2) | 0;
	
	CS_LOW;
	
	/* 发送WREG命令 */
	SPI1_ReadWrite(cmd);

	/* 发送寄存器数据 */
	SPI1_ReadWrite(value);

	CS_HIGH;
}


void SGM58201_write_multi_register(u8 reg_addr, u8 *buf, u8 num)
{
	u8 cmd = 0;
	/* 使用WREG命令写寄存器，将buf写入选中的寄存器，
     * 以给定地址为起始地址，写入num字节数据。
     */
	u8 num_bits = (num-1) & 0x3;
	int i = 0;
	cmd = CMD_WREG | (reg_addr << 2) | num_bits;
	
	CS_LOW;
	
	/* 发送WREG命令 */
	SPI1_ReadWrite(cmd);

	/* 发送寄存器数据 */
	for (i=0; i<num; i++)
	{
		SPI1_ReadWrite(buf[i]);
	}
	CS_HIGH;
}

u8 SGM58201_read_register(u8 reg_addr)
{
	u8 cmd = 0;
	u8 value;
	/* 使用RREG命令读寄存器
     * 以给定地址为起始地址，读出1字节数据。
     */
	
	cmd = CMD_RREG | (reg_addr << 2) | 0;
	
	CS_LOW;
	
	/* 发送RREG命令 */
	SPI1_ReadWrite(cmd);

	/* 读取寄存器数据 */
	value = SPI1_ReadWrite(0x00);

	CS_HIGH;

	return value;
}


u8 SGM58201_get_drdy(void)
{
	return GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2);
}


int32_t SGM58201_read_adc(void)
{
	u8 buf[3] = {0};
	int i = 0;
	int32_t data = 0;

	CS_LOW;
	/* 在DRDY有效后读取数据，可以不使用读命令 */
	for(i=0; i<3; i++)
	 /* 由于未使用读命令，因此读取过程中SDI应保持低电平，所以发送0x00 */
		buf[i] = SPI1_ReadWrite(0x00);
	
	/* 拼接数据 */
	data = (buf[0] << 16) | (buf[1] << 8) | buf[2];
	
	/* 用int32来表示24位补码需要扩展符号位 */
	data = (data << 8) >> 8;
	
	CS_HIGH;

	return data;
}


