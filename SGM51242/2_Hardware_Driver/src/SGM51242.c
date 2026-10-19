#include "spi.h"

#include "SGM51242.h"

#include "delay.h"


// use SPI1
// GPIOA pin4=CS, software control CS
// GPIOA pin5=SCLK
// GPIOA pin6=MISO
// GPIOA pin7=MOSI
void set_SGM51242_cs_low(void)
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_4);
}

void set_SGM51242_cs_high(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_4);
}


void SGM51242_interface_init(void)
{
	
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	
	// set sync pin as GPIO output
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	
	SPI1_Init(SPI_DataSize_16b, SPI_CPOL_High, SPI_CPHA_1Edge, SPI_BaudRatePrescaler_4);	
	set_SGM51242_cs_high();
	
}

u16 SGM51242_write_register(u16 addr, u16 value)
{
	u16 data_in = 0x0000;
	u16 data_out = 0x0000;
	
	data_in = addr;
	
	data_in = (data_in << 11) | (value & 0x07FF);
	
	set_SGM51242_cs_low();
	
	
	data_out = SPI1_ReadWrite(data_in);
	
	set_SGM51242_cs_high();
	
	return data_out;
	
}

u16 SGM51242_read_register(u16 addr)
{
	u16 data_in = 0x0000;
	u16 data_out = 0x0000;
	
	data_in = addr;
	data_in = (data_in << 2) | 0x03840;
	
	set_SGM51242_cs_low();

	
	data_out = SPI1_ReadWrite(data_in);
	
	set_SGM51242_cs_high();
	delay_us(1);
	
	set_SGM51242_cs_low();

	data_in = 0x0000;
	data_out = SPI1_ReadWrite(data_in);
	
	
	set_SGM51242_cs_high();
	return data_out;
	
}

u16 SGM51242_read_ADC(void)
{

	u16 data_out = 0x0000;
	
	
	set_SGM51242_cs_low();
	delay_us(6);
	
	data_out = SPI1_ReadWrite(0x0000);
	
	set_SGM51242_cs_high();
	delay_us(3);
	
	return data_out;
	
}
