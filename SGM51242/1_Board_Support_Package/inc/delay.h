#ifndef _DELAY_H_
#define _DELAY_H_

#include "stm32f4xx.h" 




void delay(u32 val);

void delay_init(u32 SysClk);
void delay_us(u32 nus);
void delay_ms(u32 nms);


#endif	//_DELAY_H_
