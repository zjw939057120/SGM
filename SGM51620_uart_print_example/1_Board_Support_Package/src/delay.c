#include "stm32f4xx.h"
#include "delay.h"

// 普通定时
// 168MH系统频率下，delay(41667) 大约1ms
// 168MH系统频率下，delay(42) 大约1us
void delay(u32 val)
{
	u32 i;
	
	for(i=0; i<val; i++) {;}
}



// 精确定时

static u32 cnt_us=0;							//1us延时count			   
static u32 cnt_ms=0;							//1ms延时count

//初始化延迟函数

//SYSTICK的时钟固定为AHB时钟的1/8
//SYSCLK:系统时钟频率，默认168MHz
void delay_init(u32 SysClk)
{
 	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8); 
	cnt_us=SysClk/8000000;			//每个us需要的systick时钟数
	cnt_ms=SysClk/8000;				//每个ms需要的systick时钟数   
}	


//延时nus
//nus为要延时的us数.	
//注意:nus的值,不要大于798915us(最大值即2^24/fac_us@fac_us=21)
void delay_us(u32 nus)
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*cnt_us; 				//时间加载	  		 
	SysTick->VAL=0x00;        				//清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ; //开始倒数 	 
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));	//等待时间到达   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk; //关闭计数器
	SysTick->VAL =0X00;       				//清空计数器 
}
//延时nms
//注意nms的范围
//SysTick->LOAD为24位寄存器,所以,最大延时为:
//nms<=0xffffff*8*1000/SYSCLK
//SYSCLK单位为Hz,nms单位为ms
//对168M条件下,nms<=798ms 
//超频到248M的时候,delay_xms最大只能延时541ms左右
void delay_ms(u32 nms)
{	 		  	  
	u32 temp;		   
	SysTick->LOAD=nms*cnt_ms;			//时间加载(SysTick->LOAD为24bit)
	SysTick->VAL =0x00;           			//清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;          //开始倒数 
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));	//等待时间到达   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;       //关闭计数器
	SysTick->VAL =0X00;     		  		//清空计数器	  	    
} 


void delay_sec(u32 sec)
{
	u32 val = sec*10;
	for(;val>0;val--)
	{
		delay_ms(100);
	}
}
