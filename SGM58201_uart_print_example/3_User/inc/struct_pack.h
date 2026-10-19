#ifndef __STRUCT_PACK_H
#define __STRUCT_PACK_H

#ifdef __cplusplus
extern "C" {
#endif


#include "stm32f4xx.h" 

static __INLINE u8 unpack_uint8(u8 **ptr)
{
    u8 ret;
    u8 *ret_ptr = (u8 *)&ret;

    ret_ptr[0] = *(*ptr)++;
    return ret;
}

static __INLINE int8_t unpack_int8(u8 **ptr)
{
    int8_t ret;
    u8 *ret_ptr = (u8 *)&ret;

    ret_ptr[0] = *(*ptr)++;
    return ret;
}

static __INLINE u16 unpack_uint16(u8 **ptr)
{
    u16 ret;
    u8 *ret_ptr = (u8 *)&ret;

    ret_ptr[1] = *(*ptr)++;
    ret_ptr[0] = *(*ptr)++;
    return ret;
}

static __INLINE int16_t unpack_int16(u8 **ptr)
{
    int16_t ret;
    u8 *ret_ptr = (u8 *)&ret;

    ret_ptr[1] = *(*ptr)++;
    ret_ptr[0] = *(*ptr)++;
    return ret;
}

static __INLINE u32 unpack_uint32(u8 **ptr)
{
    u32 ret;
    u8 *ret_ptr = (u8 *)&ret;

    ret_ptr[3] = *(*ptr)++;
    ret_ptr[2] = *(*ptr)++;
    ret_ptr[1] = *(*ptr)++;
    ret_ptr[0] = *(*ptr)++;
    return ret;
}


static __INLINE int32_t unpack_int32(u8 **ptr)
{
    int32_t ret;
    u8 *ret_ptr = (u8 *)&ret;

    ret_ptr[3] = *(*ptr)++;
    ret_ptr[2] = *(*ptr)++;
    ret_ptr[1] = *(*ptr)++;
    ret_ptr[0] = *(*ptr)++;
    return ret;
}


static __INLINE float unpack_float(u8 **ptr)
{
    float ret;
    u8 *ret_ptr = (u8 *)&ret;

    ret_ptr[3] = *(*ptr)++;
    ret_ptr[2] = *(*ptr)++;
    ret_ptr[1] = *(*ptr)++;
    ret_ptr[0] = *(*ptr)++;
    return ret;
}

static __INLINE void pack_int8(u8 **ptr, int8_t value)
{
    u8 *value_ptr = (u8 *)&value;

    *(*ptr)++ = value_ptr[0];
}

static __INLINE void pack_uint16(u8 **ptr, u16 value)
{
    u8 *value_ptr = (u8 *)&value;

    *(*ptr)++ = value_ptr[1];
    *(*ptr)++ = value_ptr[0];
}

static __INLINE void pack_int16(u8 **ptr, int16_t value)
{
    u8 *value_ptr = (u8 *)&value;

    *(*ptr)++ = value_ptr[1];
    *(*ptr)++ = value_ptr[0];
}

static __INLINE void pack_uint32(u8 **ptr, u32 value)
{
    u8 *value_ptr = (u8 *)&value;

    *(*ptr)++ = value_ptr[3];
    *(*ptr)++ = value_ptr[2];
    *(*ptr)++ = value_ptr[1];
    *(*ptr)++ = value_ptr[0];
}

static __INLINE void pack_int32(u8 **ptr, int32_t value)
{
    u8 *value_ptr = (u8 *)&value;

    *(*ptr)++ = value_ptr[3];
    *(*ptr)++ = value_ptr[2];
    *(*ptr)++ = value_ptr[1];
    *(*ptr)++ = value_ptr[0];
}

static __INLINE void pack_float(u8 **ptr, float value)
{
    u8 *value_ptr = (u8 *)&value;

    *(*ptr)++ = value_ptr[3];
    *(*ptr)++ = value_ptr[2];
    *(*ptr)++ = value_ptr[1];
    *(*ptr)++ = value_ptr[0];
}


#ifdef __cplusplus
}
#endif

#endif /* __STRUCT_PACK_H */

/*****END OF FILE****/
