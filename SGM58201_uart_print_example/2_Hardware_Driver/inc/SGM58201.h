#ifndef _SGM58201_H_
#define _SGM58201_H_

#include "stm32f4xx.h" 
#include "sgm58201_reg.h"

#define CS_LOW              GPIO_ResetBits(GPIOA, GPIO_Pin_4)
#define CS_HIGH             GPIO_SetBits(GPIOA, GPIO_Pin_4)


#define CMD_RESET           0x06
#define CMD_START_SYNC      0x08
#define CMD_POWERDOWN       0x02
#define CMD_RDATA           0x10
#define	CMD_RREG            0x20
#define CMD_WREG            0x40



#define ADDR_CFG_0          0x00
#define ADDR_CFG_1          0x01
#define ADDR_CFG_2          0x02
#define ADDR_CFG_3          0x03


void SGM58201_interface_init(void);
void SGM58201_send_cmd(u8 cmd);
u8 SGM58201_read_register(u8 reg_addr);
void SGM58201_write_register(u8 reg_addr, u8 value);
void SGM58201_write_multi_register(u8 reg_addr, u8 *buf, u8 num);
u8 SGM58201_get_drdy(void);
int32_t SGM58201_read_adc(void);
// u16  SGM58201_read_adc(u16 channel, u16 value);



#endif	/* _SGM58201_H_ */
