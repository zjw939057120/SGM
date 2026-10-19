#include "stm32f4xx.h"

#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"

#include "main.h"

#include "usart.h"
#include "spi.h" 
#include "delay.h" 

#include "SGM51242.h" 
#include "application.h"

#define CONSOLE_OUTPUT


int main(void)
{ 

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	delay_init(168000000);
	
	uart_init(115200);	
	
	SGM51242_interface_init();
	
	
	#ifdef CONSOLE_OUTPUT
	printf("Hi, I am App!\r\n");
	#endif
	
	SGM51242_write_register(SOFT_RESET, 0x05AC);//Send Reset
	delay_ms(2);
	
	SGM51242_write_register(CH_PWR_DOWN_AND_REF_CTRL, 0x0200);//Enable Internal Reference
	delay_ms(2); //After enabling the internal reference, delay by 2ms and wait for the internal reference to be fully established
	
	printf("Single sampling test!\r\n");
	ADC_Single_Sampling();
	
	delay_ms(1000);	
	printf("Repeat sampling test!\r\n");
	ADC_Repeat_Sampling();
	
	while(1)
	{
     delay_ms(10);			
	}
	
}


