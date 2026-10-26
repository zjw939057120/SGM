#ifndef __SGM51620_H__
#define __SGM51620_H__

#include <stdint.h>

#if __cplusplus
extern "C" {
#endif

// GPIO模式定义
#define GPIO_MODE_DI 1
#define GPIO_MODE_AI 2
#define GPIO_MODE_UI 3

typedef struct {
    uint8_t mode;//GPIO模式
    uint32_t gpio[3];//GPIO引脚
} gpio_group_t;

// GPIO组数量
#define GPIO_GROUP_COUNT 12
// GPIO组定义（实体定义在 sgm51620.c）
extern gpio_group_t g_gpio_group[GPIO_GROUP_COUNT];

// ADC 原始数据缓存（实体定义在 sgm51620.c）
extern uint16_t g_adc_raw[GPIO_GROUP_COUNT];

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

// 8 通道独立校准数据（先用占位值，测完后填实际值）（实体定义在 sgm51620.c）
extern const divider_calib_t g_divider_calib[12];

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