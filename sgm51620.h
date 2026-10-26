#ifndef __SGM51620_H__
#define __SGM51620_H__

#include <stdint.h>

#if __cplusplus
extern "C" {
#endif

// ADC 原始数据缓存
static uint16_t g_adc_raw[12] = {0};

// ===== 0-10V 输入分压校准 =====
// 硬件: V_in(0-10V) --[R1]-- INx --[R2]-- GND
#define DIVIDER_R1     220.0f   // 串联电阻 (KΩ)
#define DIVIDER_R2     150.0f   // 并联电阻 (KΩ)
#define V_INPUT_FSR    10.0f    // 输入电压满量程 (V)
// 理论分压比 = R2 / (R1 + R2)
#define DIVIDER_RATIO_THEORY  (DIVIDER_R2 / (DIVIDER_R1 + DIVIDER_R2))

// 两点校准结构（生产时实测填入，精度高于单点校准）
typedef struct {
    float v_adc_at_0v;    // 输入 0V 时的 ADC 读数（去掉 ADC offset）
    float v_adc_at_10v;   // 输入 10V 时的 ADC 读数（确定分压比 + gain）
} divider_calib_t;

// 8 通道独立校准数据（先用占位值，测完后填实际值）
static const divider_calib_t g_divider_calib[12] = {
    [0] = {.v_adc_at_0v = 0.000f, .v_adc_at_10v = 4.000f},  // IN0 待测
    [1] = {.v_adc_at_0v = 0.000f, .v_adc_at_10v = 4.000f},  // IN1 待测
    [2] = {.v_adc_at_0v = 0.000f, .v_adc_at_10v = 4.000f},  // IN2 待测
    [3] = {.v_adc_at_0v = 0.000f, .v_adc_at_10v = 4.000f},  // IN3 待测
    [4] = {.v_adc_at_0v = 0.000f, .v_adc_at_10v = 4.000f},  // IN4 待测
    [5] = {.v_adc_at_0v = 0.000f, .v_adc_at_10v = 4.000f},  // IN5 待测
    [6] = {.v_adc_at_0v = 0.000f, .v_adc_at_10v = 4.000f},  // IN6 待测
    [7] = {.v_adc_at_0v = 0.000f, .v_adc_at_10v = 4.000f},  // IN7 待测
    [8] = {.v_adc_at_0v = 0.000f, .v_adc_at_10v = 4.000f},  // IN8 待测
    [9] = {.v_adc_at_0v = 0.000f, .v_adc_at_10v = 4.000f},  // IN9 待测
    [10] = {.v_adc_at_0v = 0.000f, .v_adc_at_10v = 4.000f},  // IN10 待测
    [11] = {.v_adc_at_0v = 0.000f, .v_adc_at_10v = 4.000f},  // IN11 待测
    
};

float adc_to_input_calib(float v_adc, const divider_calib_t* cal);

/**
 * @brief 将原始 ADC 读数转换为电压值
 * 
 * @param ch 通道号
 * @return float 电压值（单位：V）
 */
float SGM51620_raw_to_voltage(uint8_t ch);

void SGM51620_read_adc(void);

int SGM51620_init(void);

#if __cplusplus
}
#endif
#endif