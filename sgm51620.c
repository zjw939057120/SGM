#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include "sgm51620.h"

#define SPI_DEVICE "/dev/spidev4.1"

static int spi_fd = -1;

/*
 * 打开 SPI 设备并配置模式、位宽、速率。
 * 对应原来的 SPI1_Init。
 */
int SGM51620_SPI_Init(uint16_t width, uint16_t cpol, uint16_t cpha, uint32_t speed_hz)
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
uint8_t SGM51620_SPI_ReadWrite(uint8_t TxData)
{
    uint8_t rx = 0;

    if (spi_fd < 0) {
        return 0;
    }

    struct spi_ioc_transfer tr;
    memset(&tr, 0, sizeof(tr));

    tr.tx_buf        = (unsigned long)&TxData;
    tr.rx_buf        = (unsigned long)&rx;
    tr.len           = sizeof(uint8_t);
    tr.bits_per_word = 8;

    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr) < 0) {
        return 0;
    }

    return rx;
}

void SGM51620_interface_init(void)
{
    /*
     * 原来 STM32 的调用类似：
     * SPI1_Init(SPI_DataSize_8b, SPI_CPOL_Low, SPI_CPHA_2Edge, SPI_BaudRatePrescaler_64);
     *
     * Linux 下需要把 prescaler 换成实际 SPI 时钟频率，例如 21MHz：
     */
    if (SGM51620_SPI_Init(8, 0, 1, 21000000) < 0) {
        fprintf(stderr, "SGM51620 SPI init failed\n");
        return;
    }

    /* 可选：发送一次 dummy 数据，类似原来 SPI1_ReadWrite(0xFF); */
    SGM51620_SPI_ReadWrite(0xFF);
}

/* RESET-0x06, START-0x08 ... */
void SGM51620_send_cmd(uint8_t cmd)
{
	/* 发送命令 */
	SGM51620_SPI_ReadWrite(cmd);
}

void SGM51620_write_register(uint8_t reg_addr, uint8_t value)
{
	uint8_t cmd = 0;
	/* 使用WREG命令写寄存器，将value写入选中的寄存器，
     * 以给定地址为起始地址，写入1字节数据。
     */
	
	cmd = CMD_WREG | (reg_addr << 2) | 0;
	
	/* 发送WREG命令 */
	SGM51620_SPI_ReadWrite(cmd);

	/* 发送寄存器数据 */
	SGM51620_SPI_ReadWrite(value);
}

int32_t SGM51620_read_adc(void)
{
	uint8_t buf[3] = {0};
	int i = 0;
	int32_t data = 0;

	/* 在DRDY有效后读取数据，可以不使用读命令 */
	for(i=0; i<3; i++){
	 /* 由于未使用读命令，因此读取过程中SDI应保持低电平，所以发送0x00 */
		buf[i] = SGM51620_SPI_ReadWrite(0x00);
        printf("buf[%d] = 0x%x ", i, buf[i]);
    }
    printf("\n");
	
	/* 拼接数据 */
	data = (buf[0] << 16) | (buf[1] << 8) | buf[2];
	
	/* 用int32来表示24位补码需要扩展符号位 */
	data = (data << 8) >> 8;

	return data;
}

void SGM51620_init(void)
{
    /* 初始化SGM51620用到的MCU外设 */
    SGM51620_interface_init();

    /* RESET */
    SGM51620_send_cmd(CMD_RESET);

    /* 延时等待SGM51620复位结束, >58us */
    usleep(1000*5);

    /* MUX: 1101 , 输入通道(AVDD-AVSS)/4
     * GAIN: 000, 增益为1
     * PGA_BP: 0, 使能PGA
     */
    SGM51620_write_register(ADDR_CFG_0, 0xD0);

    /* DR: 000,
     * MODE: 00,
     * CM: 1, 连续转换模式
     * TS: 0, 禁用温度传感器
     * BCS: 0, 烧毁电流源关闭
     */
    SGM51620_write_register(ADDR_CFG_1, 0x04);

    /* 启动连续转换 */
    SGM51620_send_cmd(CMD_START_SYNC);

    /* VERF：00, 内部2.048V基准
     * 50/60：00, 无抑制
     * PSW：0, 低侧电源开关-关
     * IDAC：000，IDAC1、2关断
     */
    SGM51620_write_register(ADDR_CFG_2, 0x00);

    /* I1MUX：000
     * I2MUX：000
     * DRDYM：0
     * REVERSE：0 */
    SGM51620_write_register(ADDR_CFG_3, 0x00);
}