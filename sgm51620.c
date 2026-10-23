///> @file sgm51620.c
///> @brief SGM51620 16位ADC SPI接口驱动示例
///>       外部参考电压, 单端输入, Gain=1, PGA旁路, 20SPS连续转换
///>       启动时自动诊断 VREF 实际值, 作为后续计算依据
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

// SPI设备节点
#define SPI_DEVICE "/dev/spidev4.1"
// SPI时钟频率 (1MHz，对于20SPS的ADC来说足够了，可根据实际硬件能力调高)
uint32_t spi_speed = 1000000;

// ===== ADC 配置参数（按硬件实际值修改） =====
// 标称 VREF: 诊断失败时的兜底值
#define VREF_NOMINAL     4.066f
// PGA 增益: 单端输入 MUX=1000~1011 时硬件强制旁路 PGA, 只允许 1/2/4
#define ADC_GAIN         1
// 单次 LSB (V) = 2 × VREF / Gain / 2^16
#define ADC_LSB_FROM_VREF(vref)  (2.0f * (vref) / (float)ADC_GAIN / 65536.0f)
// 20SPS 对应 ~50ms 转换时间, 加 10ms 余量避开内部振荡器 ±2.2% 误差
#define ADC_CONV_WAIT_US 60000

// 四通道配置宏定义（单端输入，AINN=AVSS, Gain=1, PGA_BYPASS=1 显式旁路）
#define CHANNEL_AIN0 0x81  // MUX=1000, GAIN=000, PGA_BYPASS=1
#define CHANNEL_AIN1 0x91  // MUX=1001, GAIN=000, PGA_BYPASS=1
#define CHANNEL_AIN2 0xA1  // MUX=1010, GAIN=000, PGA_BYPASS=1
#define CHANNEL_AIN3 0xB1  // MUX=1011, GAIN=000, PGA_BYPASS=1

// 通道名称数组（用于打印）
const char* channel_names[] = {"AIN0", "AIN1", "AIN2", "AIN3"};
// 通道配置数组
const uint8_t channel_configs[] = {CHANNEL_AIN0, CHANNEL_AIN1, CHANNEL_AIN2,
                                   CHANNEL_AIN3};
#define CHANNEL_COUNT 4  // 通道总数

// 系统监控模式: MUX=1100 测 (VREFPx-VREFNx)/4, GAIN=000, PGA_BYPASS=1
// 硬件自动用内部 2.048V 参考, 自动 bypass PGA, Gain=1
#define MUX_SYS_MON_EXTREF  0xC1

// 内部 2.048V 参考（系统监控模式自动使用, 用于 VREF 反推换算）
#define VREF_INTERNAL       2.048f
// 内部参考满量程 FSR = 2 × VREF_INTERNAL = 4.096V
// 1 LSB = FSR / 2^16 = 4.096 / 65536 V
#define VREF_INTERNAL_FSR   (2.0f * VREF_INTERNAL)

// 全局 SPI 文件描述符
static int spi_fd = -1;

// 全局识别得到的 VREF 实际值（启动诊断后更新）
static float g_vref_actual = VREF_NOMINAL;

// ===== 0-10V 输入分压校准 =====
// 硬件: V_in(0-10V) --[R1]-- AINx --[R2]-- GND
#define DIVIDER_R1     220.0f   // 串联电阻 (KΩ)
#define DIVIDER_R2     150.0f   // 并联电阻 (KΩ)
#define V_INPUT_FSR    10.0f    // 输入电压满量程 (V)
// 理论分压比 = R2 / (R1 + R2)
#define DIVIDER_RATIO_THEORY  (DIVIDER_R2 / (DIVIDER_R1 + DIVIDER_R2))

// 单点校准值（基于用户实测: 4V → 1.609127V，仅作参考,代码未使用）
// 系数 = 1.609127 / 4.0 = 0.402282
// #define DIVIDER_RATIO_MEASURED  0.402282f

// 两点校准结构（生产时实测填入，精度高于单点校准）
typedef struct {
    float v_adc_at_0v;    // 输入 0V 时的 ADC 读数（去掉 ADC offset）
    float v_adc_at_10v;   // 输入 10V 时的 ADC 读数（确定分压比 + gain）
} divider_calib_t;

// 4 通道独立校准数据（先用占位值，测完后填实际值）
static const divider_calib_t g_divider_calib[CHANNEL_COUNT] = {
    [0] = {.v_adc_at_0v = 0.000f, .v_adc_at_10v = 4.000f},  // AIN0 待测
    [1] = {.v_adc_at_0v = 0.000f, .v_adc_at_10v = 4.000f},  // AIN1 待测
    [2] = {.v_adc_at_0v = 0.000f, .v_adc_at_10v = 4.000f},  // AIN2 待测
    [3] = {.v_adc_at_0v = 0.000f, .v_adc_at_10v = 4.000f},  // AIN3 待测
};

// V_adc → V_in(0-10V) 两点校准（精度高, 消掉电阻精度 + ADC offset）
// 温度补偿在应用层做, 这里只输出原始校准后的 V_in
static inline float adc_to_input_calib(float v_adc, const divider_calib_t* cal) {
    float span = cal->v_adc_at_10v - cal->v_adc_at_0v;
    if (span < 1e-6f) return 0.0f;  // 防止除零
    return (v_adc - cal->v_adc_at_0v) / span * V_INPUT_FSR;
}

// 函数声明
int spi_init(void);
int spi_transfer(uint8_t* tx_buf, uint8_t* rx_buf, int len);
void adc_write_reg(uint8_t reg_addr, uint8_t data);
uint8_t adc_read_reg(uint8_t reg_addr);
void adc_set_channel(uint8_t channel_cfg);
void adc_start_conversion(void);
int16_t adc_read_data(void);
float adc_convert_to_temp(int16_t raw_data);
float adc_identify_vref(void);

// SPI初始化（打开设备并配置参数）
int spi_init(void) {
  uint8_t mode = SPI_MODE_1;  // 对应原代码 CPOL=0, CPHA=1
  uint8_t bits = 8;           // 8位数据位

  spi_fd = open(SPI_DEVICE, O_RDWR);
  if (spi_fd < 0) {
    perror("Failed to open SPI device");
    return -1;
  }

  // 配置SPI模式
  if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
    perror("Failed to set SPI mode");
    close(spi_fd);
    return -1;
  }

  // 配置数据位
  if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
    perror("Failed to set SPI bits per word");
    close(spi_fd);
    return -1;
  }

  // 配置时钟频率
  if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed) < 0) {
    perror("Failed to set SPI max speed");
    close(spi_fd);
    return -1;
  }

  printf("SPI initialized successfully (Device: %s, Speed: %d Hz, Mode: 1)\n",
         SPI_DEVICE, spi_speed);
  return 0;
}

// SPI传输函数（全双工）
int spi_transfer(uint8_t* tx_buf, uint8_t* rx_buf, int len) {
  struct spi_ioc_transfer tr = {
      .tx_buf = (unsigned long)tx_buf,
      .rx_buf = (unsigned long)rx_buf,
      .len = len,
      .delay_usecs = 0,  // 片选间隔时间，如遇时序问题可适当增加(如50)
      .speed_hz = spi_speed,
      .bits_per_word = 8,
  };

  // 如果rx_buf为NULL，则说明是纯写操作
  if (rx_buf == NULL) {
    uint8_t dummy_rx[len];
    tr.rx_buf = (unsigned long)dummy_rx;
  }

  int ret = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
  if (ret < 1) {
    perror("SPI transfer failed");
  }
  return ret;
}

// 写入ADC配置寄存器（WREG命令：0100 rrnn）
void adc_write_reg(uint8_t reg_addr, uint8_t data) {
  uint8_t tx[2];
  tx[0] = 0x40 | ((reg_addr & 0x03) << 2) | 0x00;  // WREG命令
  tx[1] = data;                                    // 寄存器数据
  spi_transfer(tx, NULL, 2);
  usleep(1000);
}

// 读取ADC配置寄存器（RREG命令：0010 rrnn）
uint8_t adc_read_reg(uint8_t reg_addr) {
  uint8_t tx[2] = {0};
  uint8_t rx[2] = {0};

  tx[0] = 0x20 | ((reg_addr & 0x03) << 2) | 0x00;  // RREG命令
  tx[1] = 0x00;                                    // 发送空字节获取数据

  spi_transfer(tx, rx, 2);
  usleep(1000);
  return rx[1];
}

// 设置ADC输入通道（通过配置REG0x00的MUX位）
void adc_set_channel(uint8_t channel_cfg) {
  adc_write_reg(0x00, channel_cfg);
  // 发送START/SYNC命令重启转换
  adc_start_conversion();
}

// 发送START/SYNC命令启动转换
void adc_start_conversion(void) {
  uint8_t tx[1] = {0x08};  // START/SYNC命令：0000100x
  spi_transfer(tx, NULL, 1);
  usleep(1000);
}

// 读取ADC转换结果（16位）
int16_t adc_read_data(void) {
  uint8_t tx[3] = {0};
  uint8_t rx[3] = {0};

  tx[0] = 0x10;  // RDATA命令：0001xxxx
  tx[1] = 0x00;  // 空字节
  tx[2] = 0x00;  // 空字节

  spi_transfer(tx, rx, 3);

  // 组合2字节数据（MSB先）
  return (int16_t)((rx[1] << 8) | rx[2]);
}

// 将温度传感器原始数据转换为温度值（℃）
float adc_convert_to_temp(int16_t raw_data) {
  // 温度数据为14位，左对齐在16位结果中
  int16_t temp_data = raw_data >> 2;
  // 1LSB = 0.03125℃，负数为二进制补码
  if (temp_data & 0x2000) {  // 最高位为1（负数）
    temp_data = -(~temp_data + 1);
  }
  return temp_data * 0.03125f;
}

// ===== VREF 自动诊断函数 =====
// 原理（规格书 P28 "System Monitor"）：
//   1) MUX=1100 把 (VREFPx - VREFNx)/4 接到 ADC 输入
//   2) 此模式硬件自动用内部 2.048V 参考, 自动 bypass PGA, Gain=1
//   3) 1 LSB = 2 × VREF_int / Gain / 2^16 = 4.096 / 65536 V
//   4) 测得 raw 时, 输入电压 = raw × 4.096 / 65536 = (VREFPx - VREFNx) / 4
//   5) 反推: VREF_actual = (VREFPx - VREFNx) = raw × 4 × 4.096 / 65536
//                                    = raw × 16.384 / 65536
// 要求: VREF[1:0] = 01 (REFP0/REFN0) 或 10 (REFP1/REFN1)
// 返回: 实际 VREF (V); 失败返回 -1.0f (g_vref_actual 保持不变)
float adc_identify_vref(void) {
  // 1. 检查当前 VREF[1:0] 配置, 内部参考/AVSS 参考无法诊断
  uint8_t reg2 = adc_read_reg(0x02);
  uint8_t vref_sel = (reg2 >> 6) & 0x03;

  if (vref_sel == 0x00) {
    printf("[VREF-DIAG] SKIP: 当前 VREF[1:0]=00 (内部 2.048V), 无外部参考可诊断\n");
    return -1.0f;
  }
  if (vref_sel == 0x03) {
    printf("[VREF-DIAG] SKIP: 当前 VREF[1:0]=11 (AVDD-AVSS), 无固定参考可诊断\n");
    return -1.0f;
  }

  // 2. 保存 REG0 当前配置（诊断后恢复）
  uint8_t saved_reg0 = adc_read_reg(0x00);

  // 3. 切到系统监控模式: 测当前 VREF[1:0] 配的参考对
  adc_write_reg(0x00, MUX_SYS_MON_EXTREF);

  // 4. 第一次转换有通道切换延迟, 取 16 次平均
  const int N = 16;
  int32_t samples[16];
  int32_t sum = 0;
  for (int i = 0; i < N; i++) {
    adc_start_conversion();
    usleep(ADC_CONV_WAIT_US);
    samples[i] = adc_read_data();
    sum += samples[i];
  }
  int16_t avg_raw = (int16_t)(sum / N);

  // 5. 换算: VREF_actual = avg_raw × 4 × (2 × VREF_int) / 65536
  //      推导: 1 LSB = 2 × VREF_int / 65536
  //            input = raw × 1 LSB
  //            VREF_actual = input × 4  (系统监控 ÷4 输入, 反推 ×4)
  // 测试: raw=0x4656(18006) → 18006 × 4 × 4.096 / 65536 = 4.501V
  float vref_actual = (float)avg_raw * 4.0f * VREF_INTERNAL_FSR / 65536.0f;

  // 6. 合理性检查（0.5~5.5V, 规格书 VREF 范围 0.75V~AVDD）
  int valid = 1;
  if (vref_actual < 0.5f) {
    printf("[VREF-DIAG] WARN: 识别 VREF=%.3fV 过小, REFP/REFN 可能悬空或短路\n",
           vref_actual);
    valid = 0;
  } else if (vref_actual > 5.5f) {
    printf("[VREF-DIAG] WARN: 识别 VREF=%.3fV 超过 AVDD+0.1V, 违反规格书绝对最大额定值\n",
           vref_actual);
    valid = 0;
  }

  // 7. 恢复 REG0 原配置
  adc_write_reg(0x00, saved_reg0);
  adc_start_conversion();
  usleep(ADC_CONV_WAIT_US);

  if (valid) {
    printf("[VREF-DIAG] OK: VREF=%.3fV (raw_avg=0x%04X, %d samples)\n",
           vref_actual, (uint16_t)avg_raw, N);
    return vref_actual;
  } else {
    return -1.0f;
  }
}

float lsb_actual = 0.0f;

void SGM51620_read_adc(void) {
    // 读取温度传感器数据（温度传感器强制使用内部 2.048V 参考, 与 VREF 配置无关）
    adc_write_reg(0x01, 0x06);  // REG1: TS=1 启用温度传感器
    adc_start_conversion();
    usleep(ADC_CONV_WAIT_US);
    int16_t temp_raw = adc_read_data();
    float temperature = adc_convert_to_temp(temp_raw);
    // 切回 ADC 连续转换模式（写寄存器自动触发一次转换, 连续模式继续）
    adc_write_reg(0x01, 0x04);
    usleep(ADC_CONV_WAIT_US);  // 等第一次转换

    // 循环读取四路ADC通道
    for (int i = 0; i < CHANNEL_COUNT; i++) {
      // 设置当前通道
      adc_set_channel(channel_configs[i]);
      usleep(ADC_CONV_WAIT_US);

      // 读取当前通道ADC数据
      int16_t adc_raw = adc_read_data();

      // 用识别得到的 VREF 计算 AINx 实际电压
      float v_adc = (float)adc_raw * lsb_actual;
      // 反推 0-10V 原始输入（温度补偿在应用层做, 这里只做分压反推）
      float v_in = adc_to_input_calib(v_adc, &g_divider_calib[i]);
      printf("[%s] Raw: 0x%04X, V_adc=%.4fV, V_in=%.3fV\n",
             channel_names[i], adc_raw, v_adc, v_in);
      usleep(100000);
    }

    printf("[TEMP] %.2f℃\n", temperature);
    printf("--------------------------------------------\n");
    usleep(1000000);  // 暂停1秒后继续循环
}

int SGM51620_init(void) {
  printf("SGM51620R ADC 四通道读取 Demo (Hardware SPI via /dev/spidev4.1) %s\n", __TIME__);
  printf("=============================================================\n");
  printf("Nominal VREF=%.3fV, Gain=%d (fallback if diag fails)\n",
         VREF_NOMINAL, ADC_GAIN);
  printf("Mode: External REF0/REFN0, 20SPS Normal, 50+60Hz Rejection ON\n");
  printf("=============================================================\n");

  // 1. 初始化SPI
  if (spi_init() < 0) {
    return -1;
  }

  // 2. 全局配置ADC
  // REG1: DR=000(20SPS), MODE=00(Normal), CM=1(连续转换), TS=0, BCS=0
  adc_write_reg(0x01, 0x04);
  // REG2: VREF=01(外部 REFP0/REFN0), 50/60=01(同时抑制 50Hz+60Hz 工频干扰),
  //       PSW=0, IDAC=000(关闭)
  //       0x50 = 0b0101 0000
  adc_write_reg(0x02, 0x50);
  // REG3: IDAC 路由禁用, DRDYM=0(nDRDY 专用引脚指示数据 ready)
  adc_write_reg(0x03, 0x00);

  // 3. 启动转换（连续转换模式只需发一次 START/SYNC）
  adc_start_conversion();
  usleep(ADC_CONV_WAIT_US);

  // 4. 自动诊断 VREF 实际值
  float vref_identified = adc_identify_vref();
  if (vref_identified > 0.0f) {
    g_vref_actual = vref_identified;
  } else {
    printf("[VREF-DIAG] 诊断失败, 使用标称值 %.3fV\n", VREF_NOMINAL);
  }

  // 5. 用识别的 VREF 算 LSB（之后所有电压计算都用这个）
  lsb_actual = ADC_LSB_FROM_VREF(g_vref_actual);
  printf("[VREF-USED] VREF=%.3fV, 1LSB=%.3f uV\n", g_vref_actual, lsb_actual * 1e6f);
  printf("=============================================================\n");

  return 0;
}
