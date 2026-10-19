#ifndef _SGM51242_H_
#define _SGM51242_H_

#include "stm32f4xx.h" 


#define NOP							            0x00
#define ADC_SEQUENCE_CTRL			      0x02
#define GENERAL_CTRL				        0x03
#define	ADC_CONFIG					        0x04
#define CH_PULL_DOWN_CONFIG			    0x06
#define	REG_READ_BACKE	            0x07
#define CH_PWR_DOWN_AND_REF_CTRL		0x0B
#define SOFT_RESET									0x0F



void SGM51242_interface_init(void);
void set_SGM51242_cs_low(void);
void set_SGM51242_cs_high(void);

u16 SGM51242_write_register(u16 reg_addr, u16 value);
u16 SGM51242_read_register(u16 addr);
u16 SGM51242_read_ADC(void);



#endif	//_SGM51242_H_
