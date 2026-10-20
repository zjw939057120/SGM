#ifndef __SGM51242_H__
#define __SGM51242_H__

#if __cplusplus
extern "C" {
#endif

/**
 * @brief 指令字地址
 * 
 */
#define NOP							            0x00
#define ADC_SEQUENCE_CTRL			      0x02
#define GENERAL_CTRL				        0x03
#define	ADC_CONFIG					        0x04
#define CH_PULL_DOWN_CONFIG			    0x06
#define	REG_READ_BACKE	            0x07
#define CH_PWR_DOWN_AND_REF_CTRL		0x0B
#define SOFT_RESET									0x0F

int SPI4_Init(uint16_t width, uint16_t cpol, uint16_t cpha, uint32_t rate);
 
uint16_t SPI4_ReadWrite(uint16_t TxData);

void SGM51242_interface_init(void);

uint16_t SGM51242_write_register(uint16_t reg_addr, uint16_t value);
uint16_t SGM51242_read_register(uint16_t addr);
uint16_t SGM51242_read_ADC(void);

/**
 * @brief 单通道采样
 * 
 */
void SGM51242_ADC_Single_Sampling(void);//Multi-Channel ADC Conversion Sequence,No Repeat
/**
 * @brief 多通道采样
 * 
 */
void SGM51242_ADC_Repeat_Sampling(void);//Multi-Channel ADC Conversion Sequence, Repeat
#if __cplusplus
}
#endif
#endif