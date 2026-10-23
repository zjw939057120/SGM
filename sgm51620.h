#ifndef __SGM51620_H__
#define __SGM51620_H__

#include <stdint.h>

#if __cplusplus
extern "C" {
#endif

void SGM51620_read_adc(void);

int SGM51620_init(void);

#if __cplusplus
}
#endif
#endif