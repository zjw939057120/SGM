#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "main.h"
#include "usart.h"
#include "spi.h"
#include "delay.h"
#include "SGM58201.h"

int adc_code = 0;
int gain = 1;
float vref = 2.048;
float vin = 0.0;
float avdd = 0.0;

int main(void)
{
    delay_init(168000000);

    /* 波特率为115200，串口打开接收中断，并回显接收数据*/
    uart_init(115200);

    /* 初始化SGM58201用到的MCU外设 */
    SGM58201_interface_init();

    /* RESET */
    SGM58201_send_cmd(CMD_RESET);

    /* 延时等待SGM58201复位结束, >58us */
    delay_ms(5);

    /* MUX: 1101 , 输入通道(AVDD-AVSS)/4
     * GAIN: 000, 增益为1
     * PGA_BP: 0, 使能PGA
     */
    SGM58201_write_register(ADDR_CFG_0, 0xD0);

    /* DR: 000,
     * MODE: 00,
     * CM: 1, 连续转换模式
     * TS: 0, 禁用温度传感器
     * BCS: 0, 烧毁电流源关闭
     */
    SGM58201_write_register(ADDR_CFG_1, 0x04);

    /* 启动连续转换 */
    SGM58201_send_cmd(CMD_START_SYNC);

    /* VERF：00, 内部2.048V基准
     * 50/60：00, 无抑制
     * PSW：0, 低侧电源开关-关
     * IDAC：000，IDAC1、2关断
     */
    SGM58201_write_register(ADDR_CFG_2, 0x00);

    /* I1MUX：000
     * I2MUX：000
     * DRDYM：0
     * REVERSE：0 */
    SGM58201_write_register(ADDR_CFG_3, 0x00);


    while (1)
    {

        /* 轮询DRDY */
        while (SGM58201_get_drdy() == 1);

        /* 读取结果为码值 */
        adc_code = SGM58201_read_adc();

        /* 计算模拟电压 */
        vin = adc_code * vref / gain / 8388608;

        /* 计算模拟AVDD */
        avdd = vin * 4;

        /* 串口格式化发送 */
        printf("%f\n", avdd);
    }
}
