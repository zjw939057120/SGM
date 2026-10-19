#ifndef _COMMUNICATION_H_
#define _COMMUNICATION_H_

#include "stm32f4xx.h" 


typedef struct
{
	u8	head;
	u8	type;
	u8	len;
	u8	check;
	u8	data[32];
	
	u8	check_err;
	
} app_command_t;

extern app_command_t cmd_rx, cmd_tx;


typedef enum {
	APP_CMD_SPI = 3,
	APP_WR_REG,
	APP_RD_REG,
	APP_CONFIG_ALL,
	APP_SAMPLE_DATA
} app_cmd_type_t;

u8 ReceiveCommand(void);

u8 SendResponse(void);


void CommandExecution(void);

void print_cmd_rx(void);
void print_cmd_tx(void);


#endif	//_COMMUNICATION_H_
