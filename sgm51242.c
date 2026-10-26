#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include "sgm51242.h"
#include "sgm51620.h"

#define SPI_DEVICE "/dev/spidev4.0"

static int spi_fd = -1;

/*
 * 打开 SPI 设备并配置模式、位宽、速率。
 * 对应原来的 SPI1_Init。
 */
int SGM51242_SPI_Init(uint16_t width, uint16_t cpol, uint16_t cpha, uint32_t speed_hz)
{
    uint8_t mode = 0;
    uint8_t bits;

    if (spi_fd >= 0) {
        close(spi_fd);
        spi_fd = -1;
    }

    spi_fd = open(SPI_DEVICE, O_RDWR);
    if (spi_fd < 0) {
        perror("open " SPI_DEVICE);
        return -1;
    }

    if (cpol) {
        mode |= SPI_CPOL;
    }
    if (cpha) {
        mode |= SPI_CPHA;
    }

    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
        perror("SPI_IOC_WR_MODE");
        goto fail;
    }

    bits = (uint8_t)width;   /* 8 或 16 */
    if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
        perror("SPI_IOC_WR_BITS_PER_WORD");
        goto fail;
    }

    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed_hz) < 0) {
        perror("SPI_IOC_WR_MAX_SPEED_HZ");
        goto fail;
    }

    return 0;

fail:
    close(spi_fd);
    spi_fd = -1;
    return -1;
}

/*
 * 对应原来的 SPI1_ReadWrite：
 * 发送 TxData，同时返回 RxData。
 *
 */
uint16_t SGM51242_SPI_ReadWrite(uint16_t TxData)
{
    uint16_t rx = 0;

    if (spi_fd < 0) {
        return 0;
    }

    struct spi_ioc_transfer tr;
    memset(&tr, 0, sizeof(tr));

    tr.tx_buf        = (unsigned long)&TxData;
    tr.rx_buf        = (unsigned long)&rx;
    tr.len           = sizeof(uint16_t);
    tr.bits_per_word = 16;

    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr) < 0) {
        return 0;
    }

    return rx;
}

void SGM51242_interface_init(void)
{
    /*
     * 原来 STM32 的调用类似：
     * SPI1_Init(SPI_DataSize_16b, SPI_CPOL_High, SPI_CPHA_1Edge, SPI_BaudRatePrescaler_4);
     *
     * Linux 下需要把 prescaler 换成实际 SPI 时钟频率，例如 21MHz：
     */
    if (SGM51242_SPI_Init(16, 1, 0, 21000000) < 0) {
        fprintf(stderr, "SGM51242 SPI init failed\n");
        return;
    }

    /* 可选：发送一次 dummy 数据，类似原来 SPI1_ReadWrite(0xFF); */
    SGM51242_SPI_ReadWrite(0xFF);
}

uint16_t SGM51242_write_register(uint16_t addr, uint16_t value)
{
	uint16_t data_in = 0x0000;
	uint16_t data_out = 0x0000;
	
	data_in = addr;
	
	data_in = (data_in << 11) | (value & 0x07FF);
	
	
	
	data_out = SGM51242_SPI_ReadWrite(data_in);
	
	
	return data_out;
	
}

uint16_t SGM51242_read_register(uint16_t addr)
{
	uint16_t data_in = 0x0000;
	uint16_t data_out = 0x0000;
	
	data_in = addr;
	data_in = (data_in << 2) | 0x03840;
	

	
	data_out = SGM51242_SPI_ReadWrite(data_in);
	
	usleep(1);
	

	data_in = 0x0000;
	data_out = SGM51242_SPI_ReadWrite(data_in);
	
	
	return data_out;
	
}

uint16_t SGM51242_read_ADC(void)
{

	uint16_t data_out = 0x0000;
	
	
	usleep(6);
	
	data_out = SGM51242_SPI_ReadWrite(0x0000);
	
	usleep(3);
	
	return data_out;
	
}

float SGM51242_raw_to_voltage(uint8_t ch)
{
  if (ch < 3 || ch > 12) {
    return 0.0f;
  }
  float v_adc = (float)(g_adc_raw[ch]&0xFFF)/4095*4.066f;
  return adc_to_input_calib(v_adc, &g_divider_calib[ch]);
}

void SGM51242_ADC_Single_Sampling(void)
{
	uint16_t reg_val;
	uint16_t adc_seq;
	uint16_t adc_val;
	uint8_t CHX = 0xFF;
	uint8_t adc_channels = 8;
	
	uint16_t i;
	uint8_t data_chx;
    uint8_t chx_offset = 4;	
	// double data_val;
	
  adc_seq = CHX;
	SGM51242_write_register(ADC_CONFIG, adc_seq); //ADC Pin Configuration Register,all channels are configured for ADC input
	reg_val = SGM51242_read_register(ADC_CONFIG); //Readback register;
	// printf("ADC Configuration Register Value = 0x%x%x;\r\n",reg_val >> 8, reg_val & 0x0F);
	usleep(2);
	
	SGM51242_write_register(ADC_SEQUENCE_CTRL, adc_seq); //ADC Sequence Register,all channels are included in the conversion sequence, sequence repetition disabled
	usleep(2);
	
	SGM51242_read_ADC();// dummy convert
	
	for(i=0; i<adc_channels; i++)
	{
		adc_val = SGM51242_read_ADC();
		data_chx = adc_val >> 12;
		// data_val = (double)(adc_val&0xFFF)/4095*4.066f;
        // 保存原始数据到缓存
        switch (data_chx) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7: {
            // 保存原始数据到缓存
            g_adc_raw[data_chx + chx_offset] = adc_val;
        }
            break;
        default:
            break;
        }
		printf("CH%d = %.3fV ",data_chx,SGM51242_raw_to_voltage(data_chx + chx_offset));
	} 
	printf("\r\n");
    fflush(stdout);
	
	// clear sequencer
	SGM51242_write_register(ADC_SEQUENCE_CTRL, 0x0000);
}

void SGM51242_ADC_Repeat_Sampling(void)
{
	uint16_t reg_val;
	uint16_t adc_seq;
	uint16_t adc_val;
	uint8_t CHX = 0xFF;
	uint8_t adc_channels = 8;
	uint16_t adc_samples = 10;
	uint16_t i,j;
	uint8_t data_chx;	
	double data_val;
	
	
	adc_seq = CHX;
  SGM51242_write_register(ADC_CONFIG, adc_seq); //ADC Pin Configuration Register,all channels are configured for ADC input
	reg_val = SGM51242_read_register(ADC_CONFIG); //Readback register;
	// printf("ADC Configuration Register Value = 0x%x%x;\r\n",reg_val >> 8, reg_val & 0x0F);
	usleep(2);
	
	SGM51242_write_register(ADC_SEQUENCE_CTRL, 0x0200 | adc_seq); //ADC Sequence Register,all channels are included in the conversion sequence, sequence repetition disabled
	usleep(2);

	SGM51242_read_ADC(); // dummy convert
	for(j=0; j<adc_samples; j++)
  {
		for(i=0; i<adc_channels; i++)
		{
			adc_val = SGM51242_read_ADC();
			data_chx = adc_val >> 12;
			data_val = (double)(adc_val&0xFFF)/4095*4.066f;
			printf("CH%d = %.3fV ",data_chx,data_val);
			usleep(3);
		} 
		printf("\r\n");
  }
	
	// clear sequencer
	SGM51242_write_register(ADC_SEQUENCE_CTRL, 0x0000);
}

void SGM51242_init(void)
{
	SGM51242_interface_init();
	
	SGM51242_write_register(SOFT_RESET, 0x05AC);//Send Reset
	usleep(2*1000);
	
	// SGM51242_write_register(CH_PWR_DOWN_AND_REF_CTRL, 0x0200);//Enable Internal Reference
	// usleep(2*1000); //After enabling the internal reference, delay by 2ms and wait for the internal reference to be fully established
	
    SGM51242_write_register(CH_PWR_DOWN_AND_REF_CTRL, 0x0000);//Enable External Reference
	usleep(2*1000); //After enabling the external reference, delay by 2ms and wait for the external reference to be fully established
}

// GPIO组数量
#define GPIO_GROUP_COUNT 11
// GPIO组定义
uint32_t gpio_group[GPIO_GROUP_COUNT][3] = {
    //SGM51620
    {272,273,274},//AI1_A AI1_B AI1_C
    {275,276,277},//AI2_A AI2_B AI2_C
    {278,279,280},//AI3_A AI3_B AI3_C
    {281,282,283},//AI4_A AI4_B AI4_C
    //SGM51242
    {284,285,286},//AI5_A AI5_B AI5_C
    {287,256,257},//AI6_A AI6_B AI6_C
    {258,259,260},//AI7_A AI7_B AI7_C
    {261,262,263},//AI8_A AI8_B AI8_C
    {264,265,266},//AI9_A AI9_B AI9_C
    {267,268,269},//AI10_A AI10_B AI10_C
    {270,271,52}//AI11_A AI11_B AI11_C
};

enum GPIO_MODE {
    E_GPIO_MODE_OFF = 0,
    E_GPIO_MODE_DI = 1,
    E_GPIO_MODE_AI = 2,
    E_GPIO_MODE_UI = 3
};

/**
 * @brief 设置GPIO组模式
 * 
 * @param group GPIO组指针
 * @param mode
 * 模式1：DI开关信号输入，GPIO配置：011，短接输入：电压为0V，断开输入：电压为：4.13V
 * 模式2：AI电流信号输入，GPIO配置：101，0mA输入：电压为0V，10mA输入：电压为2.7V，20mA输入：电压为：4.13V
 * 模式3：UI电压信号输入，GPIO配置：110，0V输入：电压为0V，5V输入：电压为2.7V，10V输入：电压为：4.13V
 */
void gpio_group_mode(uint32_t *group,uint8_t mode){
    printf("GPIO=[%d,%d,%d],mode=[%d]\n", group[0], group[1] ,group[2],mode);
    char gpio_cmd[128] = {0};
        snprintf(gpio_cmd, sizeof(gpio_cmd), "echo %d > /sys/class/gpio/gpio%d/value", mode == E_GPIO_MODE_DI ? 0 : 1, group[0]);
        system(gpio_cmd);
        usleep(1000*10);
        snprintf(gpio_cmd, sizeof(gpio_cmd), "echo %d > /sys/class/gpio/gpio%d/value", mode == E_GPIO_MODE_AI ? 0 : 1, group[1]);
        system(gpio_cmd);
        usleep(1000*10);
        snprintf(gpio_cmd, sizeof(gpio_cmd), "echo %d > /sys/class/gpio/gpio%d/value", mode == E_GPIO_MODE_UI ? 0 : 1, group[2]);
        system(gpio_cmd);
        usleep(1000*10);
}

void gpio_init(uint8_t mode) {
    char gpio_cmd[128] = {0};
    for (int i = 0; i < GPIO_GROUP_COUNT; i++) {
        printf("初始化GPIO=[%d,%d,%d]\n", gpio_group[i][0],gpio_group[i][1],gpio_group[i][2]);
        for (int j = 0; j < 3; j++) {
        // 导出GPIO引脚
        snprintf(gpio_cmd, sizeof(gpio_cmd), "echo %d > /sys/class/gpio/export", gpio_group[i][j]);
        system(gpio_cmd);
        usleep(1000*10);
        // 设置GPIO方向为输出
        snprintf(gpio_cmd, sizeof(gpio_cmd), "echo out > /sys/class/gpio/gpio%d/direction", gpio_group[i][j]);
        system(gpio_cmd);
        usleep(1000*10);
        }
        //模式1：DI开关信号输入
        //模式2：AI电流信号输入
        //模式3：UI电压信号输入
        gpio_group_mode(gpio_group[i],mode);
    }
}

int main(int argc, char *argv[]) 
{
    // 初始化GPIO
	gpio_init(E_GPIO_MODE_UI);
	// 初始化SGM51242
    SGM51242_init();
    usleep(1000*1000);

    // 初始化SGM51620
    SGM51620_init();
    usleep(1000*1000);

	while(1)
	{
    // 读取SGM51242数据
    usleep(100*1000);
    SGM51242_ADC_Single_Sampling();

    // 读取SGM51620数据
    usleep(100*1000);
    SGM51620_read_adc();
	}
	return 0;
}

