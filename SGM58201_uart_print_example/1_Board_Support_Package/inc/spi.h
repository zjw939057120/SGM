#ifndef __SPI_H
#define __SPI_H


#include "stm32f4xx.h"


 	    													  
void SPI1_Init(u16 width, u16 cpol, u16 cpha, u16 rate);	//初始化SPI1口
 
u16 SPI1_ReadWrite(u16 TxData);//SPI1总线读写一个字节

void SPI2_Init(u16 width, u16 cpol, u16 cpha, u16 rate);	//初始化SPI2口
  
u16 SPI2_ReadWrite(u16 TxData);//SPI1总线读写一个字节
		 
#endif // __SPI_H

