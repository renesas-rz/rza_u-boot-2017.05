/*
 * Copyright (C) 2017 Renesas Electronics
 * Copyright (C) Chris Brandt
 *
 * This file is released under the terms of GPL v2 and any later version.
 * See the file COPYING in the root directory of the source tree for details.
 */

#include <common.h>
#include <asm/arch/r7s72100.h>
#include <asm/arch/sys_proto.h>
#include <dm/platform_data/serial_sh.h>
#include <i2c.h>
#include <spi_flash.h>
#include <netdev.h>
#include <asm/arch/mmc.h>

//#define DEBUG

DECLARE_GLOBAL_DATA_PTR;

/* Serial Console */
static const struct sh_serial_platdata serial_platdata = {
	.base = SCIF_CONSOLE_BASE,	/* SCIFx_BASE */
	.type = PORT_SCIF,
	.clk = CONFIG_SYS_CLK_FREQ,		/* P1 Clock */
};
U_BOOT_DEVICE(streamit_serial) = {
	.name = "serial_sh",
	.platdata = &serial_platdata,
};

int board_early_init_f(void)
{
	/* This function runs early in the boot process, before u-boot is relocated
	   to RAM (hence the '_f' in the function name stands for 'still running from
	   flash'). A temporary stack has been set up for us which is why we can
	   have this as C code. */

	int i;

	/* When booting from Parallel NOR, some pins need to be bi-directional */
	/* CS0, RD, A1-A15 */
	#if defined(CONFIG_BOOT_MODE0)
	  #define NOR_BIDIR 1
	  /* TODO: Replace '0' with 'NOR_BIDIR for those pins below */
	#else
	  #define NOR_BIDIR 0
	#endif

	/* =========== Pin Setup =========== */
	/* Adjust for your board as needed. */

	/* Serial Console */
	pfc_set_pin_function(7, 11, ALT5, 0, 0);/* P7_11 = TxD3 */
	pfc_set_pin_function(7, 10, ALT5, 0, 0);/* P7_11 = RxD3 */

	/* QSPI_0 ch0 (booted in 1-bit, need to change to 4-bit) */
	pfc_set_pin_function(4, 4, ALT2, 0, 0);	/* P4_4 = SPBCLK_0 */
	pfc_set_pin_function(4, 5, ALT2, 0, 0);	/* P4_5 = SPBSSL_0 */
	pfc_set_pin_function(4, 6, ALT2, 0, 1);	/* P4_6 = SPBIO00_0 (bi dir) */
	pfc_set_pin_function(4, 7, ALT2, 0, 1);	/* P4_7 = SPBIO10_0 (bi dir) */
	pfc_set_pin_function(4, 2, ALT2, 0, 1);	/* P4_2 = SPBIO20_0 (bi dir) */
	pfc_set_pin_function(4, 3, ALT2, 0, 1);	/* P4_3 = SPBIO30_0 (bi dir) */


	/* RIIC Ch 0 (EEPROM) */
	pfc_set_pin_function(1, 0, ALT1, 0, 1);	/* P1_0 = RIIC3SCL (bi dir) */
	pfc_set_pin_function(1, 1, ALT1, 0, 1);	/* P1_1 = RIIC3SDA (bi dir) */

	/* Enable USB 5 Volt supply */
	pfc_set_gpio(7, 1, GPIO_OUT); /* P7_1 = GPIO_OUT */
	gpio_set(7, 1, 1);

	/* Ethernet */
	pfc_set_pin_function(8, 14, ALT2, 0, 0); /* P8_14 = ET_COL */
	pfc_set_pin_function(9, 0, ALT2, 0, 0);	/* P9_0 = ET_MDC */
	pfc_set_pin_function(9, 1, ALT2, 0, 1);	/* P9_1 = ET_MDIO (bi dir) */
	pfc_set_pin_function(9, 2, ALT2, 0, 0);	/* P9_2 = ET_RXCLK */
	pfc_set_pin_function(9, 3, ALT2, 0, 0);	/* P9_3 = ET_RXER */
	pfc_set_pin_function(9, 4, ALT2, 0, 0);	/* P9_4 = ET_RXDV */
	pfc_set_pin_function(8, 4, ALT2, 0, 0);	/* P8_4 = ET_TXCLK */
	pfc_set_pin_function(8, 5, ALT2, 0, 0);	/* P8_5 = ET_TXER */
	pfc_set_pin_function(8, 6, ALT2, 0, 0);	/* P8_6 = ET_TXEN */
	pfc_set_pin_function(8, 15, ALT2, 0, 0);	/* P8_15 = ET_CRS */
	pfc_set_pin_function(8, 0, ALT2, 0, 0);	/* P8_0 = ET_TXD0 */
	pfc_set_pin_function(8, 1, ALT2, 0, 0);	/* P8_1 = ET_TXD1 */
	pfc_set_pin_function(8, 2, ALT2, 0, 0);	/* P8_2 = ET_TXD2 */
	pfc_set_pin_function(8, 3, ALT2, 0, 0);	/* P8_3 = ET_TXD3 */
	pfc_set_pin_function(8, 7, ALT2, 0, 0);	/* P8_7 = ET_RXD0 */
	pfc_set_pin_function(8, 8, ALT2, 0, 0);	/* P8_8 = ET_RXD1 */
	pfc_set_pin_function(8, 9, ALT2, 0, 0); /* P2_9 = ET_RXD2 */
	pfc_set_pin_function(8, 10, ALT2, 0, 0); /* P2_10 = ET_RXD3 */
	//pfc_set_pin_function(4, 14, ALT8, 0, 0); /* P4_14 = IRQ6 (ET_IRQ) */ /* NOTE: u-boot doesn't enable interrupts */

	/* Release Ethernet from reset  */
	pfc_set_gpio(2, 7, GPIO_OUT); /* P2_7 = GPIO_OUT */
	gpio_set(2, 7, 1);

#ifndef SDRAM_NONE /* If no SDRAM, skip this entire secton */

	/* SDRAM */
	pfc_set_pin_function(2, 0, ALT1, 0, 0);	/* P2_0 = CS3 */
	for(i=0;i<=15;i++)
		pfc_set_pin_function(5, i, ALT1, 0, 1);	/* P5_0~15 = D0-D15 (bi dir) */
	pfc_set_pin_function(2, 1, ALT1, 0, 0);	/* P2_1 = RAS */
	pfc_set_pin_function(2, 2, ALT1, 0, 0);	/* P2_2 = CAS */
	pfc_set_pin_function(2, 3, ALT1, 0, 0);	/* P2_3 = CKE */
	pfc_set_pin_function(2, 6, ALT1, 0, 0);	/* P2_6 = RD/WR */
	pfc_set_pin_function(2, 4, ALT1, 0, 0);	/* P2_4 = WE0/DQMLL */
	pfc_set_pin_function(2, 5, ALT1, 0, 0);	/* P2_5 = WE1/DQMLU */
#if SDRAM_SIZE_MB == 32
	for(i=0;i<=14;i++)
		pfc_set_pin_function(3, i, ALT1, 0, 0);	/* P3_0~14: A1-A15 */
#endif
#if SDRAM_SIZE_MB ==16
	for(i=0;i<=13;i++)
		pfc_set_pin_function(3, i, ALT1, 0, 0);	/* P3_0~13: A1-A14 */
#endif

	/* RED LED */
	pfc_set_gpio(7, 8, GPIO_OUT); /* P7_8 = GPIO_OUT */

	/* USER_SW */
	pfc_set_gpio(7, 9, GPIO_IN); /* P7_9 = GPIO_IN */

	/**********************************************/
	/* Configure SDRAM (CS3)                      */
	/**********************************************/
	/*
	 * NOTE: Because the Stream it board was laid out for a 32MB SDRAM chip, when using a 16MB chip
	 * the BA1 pin (pin 21) on the SDRAM needs to be lifted and then jumpered to its A12 (pin 36)
	 * so that it can be connected to the RZ's A13 signal. Pin 36 on this SDRAM chip is a NC, so
	 * driving it with A13 is not an issue.
	 *
	 * Using a 16MB SDRAM instead of a 32MB SDRAM has the benefit of freeing up the CEU clock (VIO_CLK)
	 * that is shared with the A15 signal on port P3_14.
	 */

//	#define CS2BCR_D	0x00004C00	/* Type=SDRAM, 16-bit memory */
//	#define CS2WCR_D	0x00000480	/* CAS Latency = 2 */
	#define CS3BCR_D	0x00004C00	/* Type=SDRAM, 16-bit memory */

#ifdef SDRAM_IS42_45S16800F
	/* ISSI 128Mb SYNCHRONOUS DRAM (16MByte)
	 *   IS42/45S16800F
	 *   2M x 16 x 4 Banks
	 */
	#define CS3WCR_D	2 << 13	|	/* (CS2,CS3) WTRP (2 cycles) */\
				2 << 10 |	/* (CS2,CS3) WTRCD (2 cycles) */\
				1 <<  7 |	/*     (CS3) A3CL (CAS Latency = 2) */\
				2 <<  3 |	/* (CS2,CS3) TRWL (2 cycles) */\
				3 <<  0		/* (CS2,CS3) WTRC (8 cycles) */
	#define SDCR_D	0x00090809	/* 12-bit row, 9-bit col, auto-refresh */
#endif

#ifdef SDRAM_W9825G6KH_6I
	/*
	 *   Winbond 256Mb SYNCHRONOUS DRAM (32MByte)
	 *   W9825G6KH-6I
	 *   4M x 16 x 4 Banks
	 */
	#define CS3WCR_D	2 << 13	|	/* (CS2,CS3) WTRP (2 cycles) */\
				2 << 10 |	/* (CS2,CS3) WTRCD (2 cycles) */\
				2 <<  7 |	/*     (CS3) A3CL (CAS Latency = 3) */\
				2 <<  3 |	/* (CS2,CS3) TRWL (2 cycles) */\
				3 <<  0		/* (CS2,CS3) WTRC (8 cycles) */
	#define SDCR_D	0x00090811	/* 13-bit row, 9-bit col, auto-refresh */
#endif

	/*
	 * You must refresh all rows within the amount of time specified in the memory spec.
	 * Total Refresh time =  [Number_of_rows] / [Clock_Source / Refresh Counter]
	 * 31.5ms =  [4096] /  [(66.6MHz / 4) / 128]
	 */
	#define RTCOR_D		0xA55A0080	/* Refresh Counter = 128 */
	#define RTCSR_D		0xA55A0008	/* Clock Source=CKIO/4 (CKIO=66MHz) */

	//*(u32 *)CS2BCR = CS2BCR_D;
	//*(u32 *)CS2WCR = CS2WCR_D;
	*(u32 *)CS3BCR = CS3BCR_D;
	*(u32 *)CS3WCR = CS3WCR_D;
	*(u32 *)SDCR = SDCR_D;
	*(u32 *)RTCOR = RTCOR_D;
	*(u32 *)RTCSR = RTCSR_D;

	/* wait */
	#define REPEAT_D 0x000033F1
	for (i=0;i<REPEAT_D;i++) {
		asm("nop");
	}

	/* The final step is to set the SDRAM Mode Register by written to a
	   specific address (the data value is ignored) */
	/* Check the hardware manual (table 8.15) if your settings differ */
	/*   Burst Length = 1 (fixed)
	 *   Burst Type = Sequential (fixed)
	 *   CAS Latency = 2 or 3 (see table 8.15)
	 *   Write Burst Mode = [burst read/single write] or [burst read/burst write] (see table 8.15)
	 */
#ifdef SDRAM_IS42_45S16800F
	#define SDRAM_MODE_CS3  0x3FFFE040	/* CS3: CAS=2, burst write, 16bit bus */
#endif
#ifdef SDRAM_W9825G6KH_6I
	#define SDRAM_MODE_CS3  0x3FFFE060	/* CS3: CAS=3, burst write, 16bit bus */
#endif
	//*(u16 *)SDRAM_MODE_CS2 = 0;
	*(u16 *)SDRAM_MODE_CS3 = 0;

#endif /* SDRAM_NONE */

	return 0;
}

int board_init(void)
{
	gd->bd->bi_boot_params = (CONFIG_SYS_SDRAM_BASE + 0x100);
	return 0;
}

#ifdef CONFIG_SH_ETHER
int board_eth_init(bd_t *bis)
{
	int ret = -ENODEV;
	ret = sh_eth_initialize(bis);
	return ret;
}
#endif

int board_mmc_init(bd_t *bis)
{
	int ret = 0;
#ifdef CONFIG_SH_MMCIF
	ret = mmcif_mmc_init();
#endif
	return ret;
}

int board_late_init(void)
{
	printf(	"\t\t      SPI Flash Memory Map\n"
		"\t\t------------------------------------\n"
		"\t\t         Start      Size     SPI\n");
	printf(	"\t\tu-boot:  0x%08X 0x%06X 0\n", 0,CONFIG_ENV_OFFSET);
	printf(	"\t\t   env:  0x%08X 0x%06X 0\n", CONFIG_ENV_OFFSET, CONFIG_ENV_SIZE);
	printf(	"\t\t    DT:  0x%08X 0x%06X 0\n", 0x000C0000,0x040000);
	printf(	"\t\tKernel:  0x%08X 0x%06X 0\n", 0x200000, 0x600000);
	printf(	"\t\trootfs:  0x%08X 0x%06X 0\n", 0x800000, 0x2000000-0x400000);


	/* Default addresses */
	#define DTB_ADDR_FLASH		"C0000"		/* Location of Device Tree in QSPI Flash (SPI flash offset) */
	#define DTB_ADDR_RAM		"20200000"	/* Internal RAM location to copy Device Tree */
	#define DTB_ADDR_SDRAM		"0CF00000"	/* External SDRAM location to copy Device Tree */

	#define MEM_ADDR_RAM		"0x20000000 0x00300000"	/* System Memory for when using on-chip RAM (3MB) */
#if SDRAM_SIZE_MB == 32
	#define MEM_ADDR_SDRAM		"0x0C000000 0x02000000"	/* System Memory for when using external SDRAM RAM (32MB) */
#elif SDRAM_SIZE_MB == 16
	#define MEM_ADDR_SDRAM		"0x0C000000 0x01000000"	/* System Memory for when using external SDRAM RAM (16MB) */
#elif SDRAM_SIZE_MB == 0
	#define MEM_ADDR_SDRAM		"0x0C000000 0x00000000"	/* Not using SDRAM */
#endif

	#define KERNEL_ADDR_FLASH	"0x18200000"	/* Flash location of xipImage or uImage binary */
	#define UIMAGE_ADDR_SDRAM	"0C800000"	/* Address to copy uImage to in external SDRAM */
	#define UIMAGE_ADDR_SIZE	"0x400000"	/* Size of the uImage binary in Flash (4MB) */


	/* Default kernel command line options */
	setenv("cmdline_common", "ignore_loglevel earlyprintk earlycon=scif,0xE8008800"); /* Linux-4.9 */
	//setenv("cmdline_common", "console=ttySC3,115200 ignore_loglevel earlyprintk"); /* Linux-3.14 */

	/* Root file system choices */
	setenv("fs_axfs", "rootfstype=axfs rootflags=physaddr=0x18800000");
	setenv("fs_mtd",  "root=/dev/mtdblock0");

	/* LCD Frame buffer location */
	setenv("dtb_lcdfb_fixed", "fdt set /display@fcff7400 fb_phys_addr <0x60000000>");	/* Fixed address */
	setenv("dtb_lcdfb_dyn",   "fdt set /display@fcff7400 fb_phys_addr <0x00000000>");	/* Dynamically allocate during boot */

	/* Read DTB from Flash into either internal on-chip RAM or external SDRAM */
	setenv("dtb_read_ram",   "sf probe 0; sf read "DTB_ADDR_RAM" "DTB_ADDR_FLASH" 8000; fdt addr "DTB_ADDR_RAM" ; setenv addr_dtb "DTB_ADDR_RAM"");
	setenv("dtb_read_sdram", "sf probe 0; sf read "DTB_ADDR_SDRAM" "DTB_ADDR_FLASH" 8000; fdt addr "DTB_ADDR_SDRAM" ; setenv addr_dtb "DTB_ADDR_SDRAM"");

	/* Set the system memory address and size. This overrides the setting in Device Tree */
	setenv("dtb_mem_ram",   "fdt memory "MEM_ADDR_RAM"");		/* Use internal RAM for system memory */
	setenv("dtb_mem_sdram", "fdt memory "MEM_ADDR_SDRAM"");		/* Use external SDRAM for system memory */

	/* Kernel booting operations */
	setenv("xImg", "qspi single; setenv cmd bootx "KERNEL_ADDR_FLASH" ${addr_dtb}; run cmd");	/* Boot a XIP Kernel */
	setenv("uImg", "qspi single; cp.b "KERNEL_ADDR_FLASH" "UIMAGE_ADDR_SDRAM" "UIMAGE_ADDR_SIZE"; bootm start "UIMAGE_ADDR_SDRAM" - "DTB_ADDR_SDRAM"; bootm loados ; bootm go");	/* Boot a uImage kernel */

	/* => run xa_boot */
	/* Boot XIP using internal RAM only, file system is AXFS, LCD dynamically allocated */
	setenv("xa_boot", "run dtb_read_ram; run dtb_mem_ram; run dtb_lcdfb_dyn; setenv bootargs ${cmdline_common} ${fs_axfs}; fdt chosen; run xImg");

	/* => run xsa_boot */
	/* Boot XIP using external 32MB SDRAM, file system is AXFS, LCD FB fixed to internal RAM */
	setenv("xsa_boot", "run dtb_read_sdram; run dtb_mem_sdram; run dtb_lcdfb_fixed; setenv bootargs ${cmdline_common} ${fs_axfs}; fdt chosen; run xImg");

	/* => run s_boot */
	/* Boot SDRAM uImage using external 32MB SDRAM, file system is squashfs, LCD FB fixed to internal RAM */
	setenv("s_boot", "run dtb_read_sdram; run dtb_mem_sdram; run dtb_lcdfb_fixed; setenv bootargs ${cmdline_common} ${fs_mtd}; fdt chosen; run uImg");

	/* => run sa_boot */
	/* Boot SDRAM uImage using external 32MB SDRAM, file system is AXFS, LCD FB fixed to internal RAM */
	setenv("sa_boot", "run dtb_read_sdram; run dtb_mem_sdram; run dtb_lcdfb_fixed; setenv bootargs ${cmdline_common} ${fs_axfs}; fdt chosen; run uImg");

	return 0;
}


int dram_init(void)
{
	#if (1 !=  CONFIG_NR_DRAM_BANKS)
	# error CONFIG_NR_DRAM_BANKS must set 1 in this board.
	#endif
	/* SDRAM setup is already done in board_early_init_f */
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = CONFIG_SYS_SDRAM_SIZE;
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE * CONFIG_NR_DRAM_BANKS;

	return 0;
}

const struct rmobile_sysinfo sysinfo = {
	CONFIG_ARCH_RMOBILE_BOARD_STRING
};

void reset_cpu(ulong addr)
{
	/* If you have board specific stuff to do, you can do it
	here before you reboot */

	/* Dummy read (must read WRCSR:WOVF at least once before clearing) */
	*(volatile u8 *)(WRCSR) = *(u8 *)(WRCSR);

	*(volatile u16 *)(WRCSR) = 0xA500;     /* Clear WOVF */
	*(volatile u16 *)(WRCSR) = 0x5A5F;     /* Reset Enable */
	*(volatile u16 *)(WTCNT) = 0x5A00;     /* Counter to 00 */
	*(volatile u16 *)(WTCSR) = 0xA578;     /* Start timer */

	while(1); /* Wait for WDT overflow */
}

void led_set_state(unsigned short value)
{
	if (value)	/* turn LED on */
		gpio_set(7,8,0);
	else		/* turn LED off */
		gpio_set(7,8,1);
}

u8 button_check_state(u8 sw)
{
	/* returns: 1 = button up
		    0 = button pressed
	*/

	return gpio_read(7, 9);
}

