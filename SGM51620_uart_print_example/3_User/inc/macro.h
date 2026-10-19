#ifndef __MACRO_H
#define __MACRO_H

#ifdef __cplusplus
extern "C" {
#endif

// Generic mask/shift macros
#define FIELD_GET(data, mask, shift) \
	(((data) & (mask)) >> (shift))
#define FIELD_SET(data, mask, shift, value) \
	(((data) & (~(mask))) | (((value) << (shift)) & (mask)))



#ifdef __cplusplus
}
#endif

#endif /* __MACRO_H */

/*****END OF FILE****/
