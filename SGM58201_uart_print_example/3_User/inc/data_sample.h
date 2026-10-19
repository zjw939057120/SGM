#ifndef __DATA_SAMPLE_H
#define __DATA_SAMPLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx.h" 

void data_sample_init(void);
void data_sample_set_count(u16 cnt);
void data_sample_poll(void);
int32_t data_sample_get_last(void);

#ifdef __cplusplus
}
#endif

#endif /* __DATA_SAMPLE_H */

/*****END OF FILE****/
