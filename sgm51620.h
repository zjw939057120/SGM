#ifndef __SGM51620_H__
#define __SGM51620_H__

#include <stdint.h>

#if __cplusplus
extern "C" {
#endif

/**
 * @brief 指令字地址
 * 
 */
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

int SGM51620_SPI_Init(uint16_t width, uint16_t cpol, uint16_t cpha, uint32_t rate);
 
uint8_t SGM51620_SPI_ReadWrite(uint8_t TxData);

void SGM51620_interface_init(void);

void SGM51620_send_cmd(uint8_t cmd);

void SGM51620_write_register(uint8_t reg_addr, uint8_t value);

int32_t SGM51620_read_adc(void);

void SGM51620_init(void);

#if __cplusplus
}
#endif
#endif