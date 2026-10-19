#include "stm32f4xx.h"
#include "application.h"

#include "SGM51242.h"
#include "spi.h"
#include "delay.h"
#include "usart.h"


void ADC_Single_Sampling(void)//Multi-Channel, No Repeat, ADC Conversion Sequence 
{
	u16 reg_val;
	u16 adc_seq;
	u16 adc_val;
	u8 CHX = 0xFF;
	u8 adc_channels = 8;
	
	u16 i;
	u8 data_chx;	
	double data_val;
	
  adc_seq = CHX;
	SGM51242_write_register(ADC_CONFIG, adc_seq); //ADC Pin Configuration Register,all channels are configured for ADC input
	reg_val = SGM51242_read_register(ADC_CONFIG); //Readback register;
	printf("ADC Configuration Register Value = 0x%x%x;\r\n",reg_val >> 8, reg_val & 0x0F);
	delay_us(2);
	
	SGM51242_write_register(ADC_SEQUENCE_CTRL, adc_seq); //ADC Sequence Register,all channels are included in the conversion sequence, sequence repetition disabled
	delay_us(2);
	
	SGM51242_read_ADC();// dummy convert
	
	for(i=0; i<adc_channels; i++)
	{
		adc_val = SGM51242_read_ADC();
		data_chx = adc_val >> 12;
		data_val = (double)(adc_val&0xFFF)/4095*2.5;
		printf("CH%d = %.3fV ",data_chx,data_val);
	} 
	printf("\r\n");
	
	// clear sequencer
	SGM51242_write_register(ADC_SEQUENCE_CTRL, 0x0000);
}

void ADC_Repeat_Sampling(void) //Multi-Channel, Repeating, ADC Conversion Sequence
{
	u16 reg_val;
	u16 adc_seq;
	u16 adc_val;
	u8 CHX = 0xFF;
	u8 adc_channels = 8;
	u16 adc_samples = 10;
	u16 i,j;
	u8 data_chx;	
	double data_val;
	
	
	adc_seq = CHX;
  SGM51242_write_register(ADC_CONFIG, adc_seq); //ADC Pin Configuration Register,all channels are configured for ADC input
	reg_val = SGM51242_read_register(ADC_CONFIG); //Readback register;
	printf("ADC Configuration Register Value = 0x%x%x;\r\n",reg_val >> 8, reg_val & 0x0F);
	delay_us(2);
	
	SGM51242_write_register(ADC_SEQUENCE_CTRL, 0x0200 | adc_seq); //ADC Sequence Register,all channels are included in the conversion sequence, sequence repetition disabled
	delay_us(2);

	SGM51242_read_ADC(); // dummy convert
	for(j=0; j<adc_samples; j++)
  {
		for(i=0; i<adc_channels; i++)
		{
			adc_val = SGM51242_read_ADC();
			data_chx = adc_val >> 12;
			data_val = (double)(adc_val&0xFFF)/4095*2.5;
			printf("CH%d = %.3fV ",data_chx,data_val);
			delay_us(3);
		} 
		printf("\r\n");
  }
	
	// clear sequencer
	SGM51242_write_register(ADC_SEQUENCE_CTRL, 0x0000);
}
