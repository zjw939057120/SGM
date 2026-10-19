#ifndef __SGM58201_H__
#define __SGM58201_H__

/************************** Constant Definitions *****************************/
#define SGM58201_BASEADDR              0x00
#define SGM58201_CFG0_ADDR             (SGM58201_BASEADDR + 0x00)
#define SGM58201_CFG1_ADDR             (SGM58201_BASEADDR + 0x01)
#define SGM58201_CFG2_ADDR             (SGM58201_BASEADDR + 0x02)
#define SGM58201_CFG3_ADDR             (SGM58201_BASEADDR + 0x03)

#define SGM58201_CFG0_MUX_SHIFT        4
#define SGM58201_CFG0_MUX_MASK         0xf0

#define SGM58201_CFG0_GAIN_SHIFT       1
#define SGM58201_CFG0_GAIN_MASK        0xe

#define SGM58201_CFG0_PGA_SHIFT        0
#define SGM58201_CFG0_PGA_MASK         0x1

#define SGM58201_CFG1_DR_SHIFT         5
#define SGM58201_CFG1_DR_MASK          0xe0

#define SGM58201_CFG1_MODE_SHIFT       3
#define SGM58201_CFG1_MODE_MASK        0x18

#define SGM58201_CFG1_CM_SHIFT         2
#define SGM58201_CFG1_CM_MASK          0x4

#define SGM58201_CFG1_TS_SHIFT         1
#define SGM58201_CFG1_TS_MASK          0x2

#define SGM58201_CFG1_BCS_SHIFT        0
#define SGM58201_CFG1_BCS_MASK         0x1

#define SGM58201_CFG2_VREF_SHIFT       6
#define SGM58201_CFG2_VREF_MASK        0xc0

#define SGM58201_CFG2_FIR_SHIFT        4
#define SGM58201_CFG2_FIR_MASK         0x30

#define SGM58201_CFG2_PSW_SHIFT        3
#define SGM58201_CFG2_PSW_MASK         0x8

#define SGM58201_CFG2_IDAC_SHIFT       0
#define SGM58201_CFG2_IDAC_MASK        0x7

#define SGM58201_CFG3_I1MUX_SHIFT      5
#define SGM58201_CFG3_I1MUX_MASK       0xe0

#define SGM58201_CFG3_I2MUX_SHIFT      2
#define SGM58201_CFG3_I2MUX_MASK       0x1c

#define SGM58201_CFG3_DRDYM_SHIFT      1
#define SGM58201_CFG3_DRDYM_MASK       0x2


#endif
