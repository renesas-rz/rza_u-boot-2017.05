/*
 * Copyright (C) 2018 Renesas Electronics Corporation
*/

#ifndef _RZA2_REGS_H
#define _RZA2_REGS_H



#ifndef __ASSEMBLY__
/* Pin Function Controller Driver Prototype */
/* See file arch/arm/mach-rmobile/pfc-r7s72100.h */
enum pfc_pin_alt_mode {ALT1=1, ALT2, ALT3, ALT4, ALT5, ALT6, ALT7, ALT8};
enum pfc_pin_gpio_mode {GPIO_OUT=0, GPIO_IN=1};
void pfc_set_gpio(u8 n, u8 b, u8 d);
void gpio_set(u8 n, u8 b, u8 v);
u8 gpio_read(u8 n, u8 b);
enum pfc_pin_port_name {P0=0, P1,P2,P3,P4,P5,P6,P7,P8,P9,PA,PB,PC,PD,PE,PF,PG,PH,PJ,PK,PL,PM};
void pfc_set_pin_function(u8 port, u8 pin, u8 func);

#endif /* __ASSEMBLY__ */


/*
 *  Register bases.
 */
#define RZA2_WDT_BASE            (0xFCFE7000)
#define RZA2_STBCR_BASE          (0xFCFE0020)
//#define RZA2_PCTR_BASE           (0xFCFE3000)
#define RZA2_OST_BASE            (0xE803B000)
#define RZA2_BCR_BASE            (0x1F000000)
#define RZA2_SDRAM_BASE          (0x1F000000)

/* Serial SCIF bases */
#define SCIF0_BASE			0xE8007000
#define SCIF1_BASE			0xE8007800
#define SCIF2_BASE			0xE8008000
#define SCIF3_BASE			0xE8008800
#define SCIF4_BASE			0xE8009000

/* RIIC bases */
#define CONFIG_SH_I2C_BASE0		0xE803A000
#define CONFIG_SH_I2C_BASE1		0xE803A400
#define CONFIG_SH_I2C_BASE2		0xE803A800
#define CONFIG_SH_I2C_BASE3		0xE803AC00

/* QSPI bases */
#define CONFIG_RZA2_BASE_QSPI0		0x1F800000

/* Clock Registers */
#define FRQCR				0xFCFE0010
#define CKIOSEL				0xFCFE0100
#define SCLKSEL				0xFCFE0104

/* HyperBus */
#define HYPER_BASE 0x1F400000
#define HYPER_CSR (HYPER_BASE + 0x00)
#define HYPER_IEN (HYPER_BASE + 0x04)
#define HYPER_ISR (HYPER_BASE + 0x08)
#define HYPER_MCR0 (HYPER_BASE + 0x20)
#define HYPER_MCR1 (HYPER_BASE + 0x24)
#define HYPER_MTR0 (HYPER_BASE + 0x30)
#define HYPER_MTR1 (HYPER_BASE + 0x34)


/* Watchdog Registers */
#define WTCSR (RZA2_WDT_BASE + 0x00) /* Watchdog Timer Control Register */
#define WTCNT (RZA2_WDT_BASE + 0x02) /* Watchdog Timer Counter Register */
#define WRCSR (RZA2_WDT_BASE + 0x04) /* Watchdog Reset Control Register */

/* OS Timer Registers */
#define OSTM0CMP (RZA2_OST_BASE + 0x000)
#define OSTM0CNT (RZA2_OST_BASE + 0x004)
#define OSTM0TE  (RZA2_OST_BASE + 0x010)
#define OSTM0TS  (RZA2_OST_BASE + 0x014)
#define OSTM0TT  (RZA2_OST_BASE + 0x018)
#define OSTM0CTL (RZA2_OST_BASE + 0x020)
#define OSTM1CMP (RZA2_OST_BASE + 0x400)
#define OSTM1CNT (RZA2_OST_BASE + 0x404)
#define OSTM1TE  (RZA2_OST_BASE + 0x410)
#define OSTM1TS  (RZA2_OST_BASE + 0x414)
#define OSTM1TT  (RZA2_OST_BASE + 0x418)
#define OSTM1CTL (RZA2_OST_BASE + 0x420)

/* Standby controller registers */
#define STBCR1 (RZA2_STBCR_BASE + 0x000)
#define STBCR2 (RZA2_STBCR_BASE + 0x004)
#define STBCR3 (RZA2_STBCR_BASE + 0x400)
#define STBCR4 (RZA2_STBCR_BASE + 0x404)
#define STBCR5 (RZA2_STBCR_BASE + 0x408)
#define STBCR6 (RZA2_STBCR_BASE + 0x40c)
#define STBCR7 (RZA2_STBCR_BASE + 0x410)
#define STBCR8 (RZA2_STBCR_BASE + 0x414)
#define STBCR9 (RZA2_STBCR_BASE + 0x418)
#define STBCR10 (RZA2_STBCR_BASE + 0x41c)
#define STBCR11 (RZA2_STBCR_BASE + 0x420)
#define STBCR12 (RZA2_STBCR_BASE + 0x424)
#define STBCR13 (RZA2_STBCR_BASE + 0x450)

/* P0n Pin Function Control Register */
/* Port0 Control register */
/* Port1 Control register */
/* Port2 Control register */
/* Port3 Control register */
/* Port4 Control register */
/* Port5 Control register */
/* Port6 Control register */
/* Port7 Control register */
/* Port8 Control register */
/* Port9 Control register */
/* PortA Control register */
/* PortB Control register */
/* PortC Control register */
/* PortD Control register */
/* PortE Control register */
/* PortF Control register */
/* PortG Control register */
/* PortH Control register */
/* PortJ Control register */
/* PortK Control register */
/* PortL Control register */
/* PortM Control register */


/* Bus State Contoller registers */
#define CMNCR  (RZA2_BCR_BASE + 0x00)
#define CS0BCR (RZA2_BCR_BASE + 0x04)
#define CS0WCR (RZA2_BCR_BASE + 0x28)
#define CS1BCR (RZA2_BCR_BASE + 0x08)
#define CS1WCR (RZA2_BCR_BASE + 0x2c)
#define CS2BCR (RZA2_BCR_BASE + 0x0c)
#define CS2WCR (RZA2_BCR_BASE + 0x30)
#define CS3BCR (RZA2_BCR_BASE + 0x10)
#define CS3WCR (RZA2_BCR_BASE + 0x34)

/* SDRAM controller registers */
#define SDCR   (RZA2_SDRAM_BASE + 0x4c)
#define RTCOR  (RZA2_SDRAM_BASE + 0x58)
#define RTCSR  (RZA2_SDRAM_BASE + 0x50)


/* PL310 L2 Cache */
#define PL310_BASE 0x1F003000
#define PL310_REG(reg) ( *(volatile uint32_t *)(PL310_BASE + reg) )

/* registers */
#define REG0_CACHE_ID              0x000
#define REG0_CACHE_TYPE            0x004
#define REG1_CONTROL               0x100
#define REG1_AUX_CONTROL           0x104
#define REG1_TAG_RAM_CONTROL       0x108
#define REG1_DATA_RAM_CONTROL      0x10c
#define REG2_EV_COUNTER_CTRL       0x200
#define REG2_EV_COUNTER1_CFG       0x204
#define REG2_EV_COUNTER0_CFG       0x208
#define REG2_EV_COUNTER1           0x20c
#define REG2_EV_COUNTER0           0x210
#define REG2_INT_MASK              0x214
#define REG2_INT_MASK_STATUS       0x218
#define REG2_INT_RAW_STATUS        0x21c
#define REG2_INT_CLEAR             0x220
#define REG7_CACHE_SYNC            0x730
#define REG7_INV_PA                0x770
#define REG7_INV_WAY               0x77c
#define REG7_CLEAN_PA              0x7b0
#define REG7_CLEAN_INDEX           0x7b8
#define REG7_CLEAN_WAY             0x7bc
#define REG7_CLEAN_INV_PA          0x7f0
#define REG7_CLEAN_INV_INDEX       0x7f8
#define REG7_CLEAN_INV_WAY         0x7fc
#define REG9_D_LOCKDOWN0           0x900
#define REG9_I_LOCKDOWN0           0x904
#define REG9_D_LOCKDOWN1           0x908
#define REG9_I_LOCKDOWN1           0x90c
#define REG9_D_LOCKDOWN2           0x910
#define REG9_I_LOCKDOWN2           0x914
#define REG9_D_LOCKDOWN3           0x918
#define REG9_I_LOCKDOWN3           0x91c
#define REG9_D_LOCKDOWN4           0x920
#define REG9_I_LOCKDOWN4           0x924
#define REG9_D_LOCKDOWN5           0x928
#define REG9_I_LOCKDOWN5           0x92c
#define REG9_D_LOCKDOWN6           0x930
#define REG9_I_LOCKDOWN6           0x934
#define REG9_D_LOCKDOWN7           0x938
#define REG9_I_LOCKDOWN7           0x93c
#define REG9_LOCK_LINE_EN          0x950
#define REG9_UNLOCK_WAY            0x954
#define REG12_ADDR_FILTERING_START 0xc00
#define REG12_ADDR_FILTERING_END   0xc04
#define REG15_DEBUG_CTRL           0xf40
#define REG15_PREFETCH_CTRL        0xf60
#define REG15_POWER_CTRL           0xf80


/* Power-Down Registers (Chapter 55) */
#define SWRSTCR1 0xFCFE0460	/* Software reset control register 1 */
#define SWRSTCR2 0xFCFE0464	/* Software reset control register 2 */
#define SYSCR1 0xFCFE0400	/* System control register 1 */
#define SYSCR2 0xFCFE0404	/* System control register 2 */
#define SYSCR3 0xFCFE0408	/* System control register 3 */
#define CPUSTS 0xFCFE0018	/* CPU status register */
#define STBREQ1 0xFCFE0030	/* Standby request register 1 */
#define STBREQ2 0xFCFE0034	/* Standby request register 2 */
#define STBREQ3 0xFCFE0038	/* Standby request register 3 */
#define STBACK1 0xFCFE0040	/* Standby acknowledge register 1 */
#define STBACK2 0xFCFE0044	/* Standby acknowledge register 2 */
#define STBACK3 0xFCFE0048	/* Standby acknowledge register 3 */
#define RRAMKP 0xFCFFC000	/* On-chip data-retention RAM area setting register */
#define DSCTR 0xFCFFC002	/* Deep standby control register */
#define DSSSR 0xFCFFC004	/* Deep standby cancel source select register */
#define DSESR 0xFCFFC006	/* Deep standby cancel edge select register */
#define DSFR 0xFCFFC008		/* Deep standby cancel source flag register */
#define XTALCTR 0xFCFF1810	/* XTAL crystal oscillator gain control register */

#endif				/* _RZA2_REGS_H */
