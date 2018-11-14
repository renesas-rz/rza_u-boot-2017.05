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
U_BOOT_DEVICE(genmai_serial) = {
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
	pfc_set_pin_function(3, 0, ALT6, 0, 0);	/* P3_0 = TxD2 */
	pfc_set_pin_function(3, 2, ALT4, 0, 0);	/* P3_2 = RxD2 */

	/* QSPI_0 ch0 (booted in 1-bit, need to change to 4-bit) */
	pfc_set_pin_function(9, 2, ALT2, 0, 0);	/* P9_2 = SPBCLK_0 */
	pfc_set_pin_function(9, 3, ALT2, 0, 0);	/* P9_3 = SPBSSL_0 */
	pfc_set_pin_function(9, 4, ALT2, 0, 1);	/* P9_4 = SPBIO00_0 (bi dir) */
	pfc_set_pin_function(9, 5, ALT2, 0, 1);	/* P9_5 = SPBIO10_0 (bi dir) */
	pfc_set_pin_function(9, 6, ALT2, 0, 1);	/* P9_6 = SPBIO20_0 (bi dir) */
	pfc_set_pin_function(9, 7, ALT2, 0, 1);	/* P9_7 = SPBIO30_0 (bi dir) */

	/* QSPI_0 ch1 (4-bit interface for dual QSPI mode) */
	pfc_set_pin_function(2, 12, ALT4, 0, 1); /* P2_12 = SPBIO01_0 (bi dir) */
	pfc_set_pin_function(2, 13, ALT4, 0, 1); /* P2_13 = SPBIO11_0 (bi dir) */
	pfc_set_pin_function(2, 14, ALT4, 0, 1); /* P2_14 = SPBIO21_0 (bi dir) */
	pfc_set_pin_function(2, 15, ALT4, 0, 1); /* P2_15 = SPBIO31_0 (bi dir) */

	pfc_set_pin_function(1, 4, ALT1, 0, 1);	/* P1_4 = RIIC2SCL (bi dir) */
	pfc_set_pin_function(1, 5, ALT1, 0, 1);	/* P1_5 = RIIC2SDA (bi dir) */


	/* Ethernet */
	pfc_set_pin_function(1, 14, ALT4, 0, 0); /* P1_14 = ET_COL */
	pfc_set_pin_function(5, 9, ALT2, 0, 0);	/* P5_9 = ET_MDC */
	pfc_set_pin_function(3, 3, ALT2, 0, 1);	/* P3_3 = ET_MDIO (bi dir) */
	pfc_set_pin_function(3, 4, ALT2, 0, 0);	/* P3_4 = ET_RXCLK */
	pfc_set_pin_function(3, 5, ALT2, 0, 0);	/* P3_5 = ET_RXER */
	pfc_set_pin_function(3, 6, ALT2, 0, 0);	/* P3_6 = ET_RXDV */
	pfc_set_pin_function(2, 0, ALT2, 0, 0);	/* P2_0 = ET_TXCLK */
	pfc_set_pin_function(2, 1, ALT2, 0, 0);	/* P2_1 = ET_TXER */
	pfc_set_pin_function(2, 2, ALT2, 0, 0);	/* P2_2 = ET_TXEN */
	pfc_set_pin_function(2, 3, ALT2, 0, 0);	/* P2_3 = ET_CRS */
	pfc_set_pin_function(2, 4, ALT2, 0, 0);	/* P2_4 = ET_TXD0 */
	pfc_set_pin_function(2, 5, ALT2, 0, 0);	/* P2_5 = ET_TXD1 */
	pfc_set_pin_function(2, 6, ALT2, 0, 0);	/* P2_6 = ET_TXD2 */
	pfc_set_pin_function(2, 7, ALT2, 0, 0);	/* P2_7 = ET_TXD3 */
	pfc_set_pin_function(2, 8, ALT2, 0, 0);	/* P2_8 = ET_RXD0 */
	pfc_set_pin_function(2, 9, ALT2, 0, 0);	/* P2_9 = ET_RXD1 */
	pfc_set_pin_function(2, 10, ALT2, 0, 0); /* P2_10 = ET_RXD2 */
	pfc_set_pin_function(2, 11, ALT2, 0, 0); /* P2_11 = ET_RXD3 */
	//pfc_set_pin_function(4, 14, ALT8, 0, 0); /* P4_14 = IRQ6 (ET_IRQ) */ /* NOTE: u-boot doesn't enable interrupts */


	/* MMC */
#if 0 /*** PLEASE configure pins and 'ALTx' according to Tables 54.xx in the Hardware Manaul ***/
	pfc_set_pin_function(3, 8, ALT8, 0, 0);		/* MMC CD */
	pfc_set_pin_function(3, 10, ALT8, 0, 1);	/* MMC DAT1 (bi dir) */
	pfc_set_pin_function(3, 11, ALT8, 0, 1);	/* MMC DAT0 (bi dir) */
	pfc_set_pin_function(3, 12, ALT8, 0, 0);	/* MMC CLK */
	pfc_set_pin_function(3, 13, ALT8, 0, 1);	/* MMC CMD (bi dir) */
	pfc_set_pin_function(3, 14, ALT8, 0, 1);	/* MMC DAT3 (bi dir) */
	pfc_set_pin_function(3, 15, ALT8, 0, 1);	/* MMC DAT2 (bi dir)*/
	pfc_set_pin_function(4, 0, ALT8, 0, 1);		/* MMC DAT4 (bi dir)*/
	pfc_set_pin_function(4, 1, ALT8, 0, 1);		/* MMC DAT5 (bi dir) */
	pfc_set_pin_function(4, 2, ALT8, 0, 1);		/* MMC DAT6 (bi dir) */
	pfc_set_pin_function(4, 3, ALT8, 0, 1);		/* MMC DAT7 (bi dir) */
#endif

	/* SDRAM */
	pfc_set_pin_function(5, 8, ALT6, 0, 0);	/* P5_8 = CS2 */
	pfc_set_pin_function(7, 1, ALT1, 0, 0);	/* P7_1 = CS3 */
	for(i=0;i<=15;i++)
		pfc_set_pin_function(6, i, ALT1, 0, 1);	/* P6_0~15 = D0-D15 (bi dir) */
	pfc_set_pin_function(7, 2, ALT1, 0, 0);	/* P7_2 = RAS */
	pfc_set_pin_function(7, 3, ALT1, 0, 0);	/* P7_3 = CAS */
	pfc_set_pin_function(7, 4, ALT1, 0, 0);	/* P7_4 = CKE */
	pfc_set_pin_function(7, 5, ALT1, 0, 0);	/* P7_5 = RD/WR */
	pfc_set_pin_function(7, 6, ALT1, 0, 0);	/* P7_6 = WE0/DQMLL */
	pfc_set_pin_function(7, 7, ALT1, 0, 0);	/* P7_7 = WE1/DQMLU */
	for(i=9;i<=15;i++)
		pfc_set_pin_function(7, i, ALT1, 0, 0);	/* P7_9~15: A1-A7 */
	for(i=0;i<=15;i++)
		pfc_set_pin_function(8, i, ALT1, 0, 0);	/* P8_0~15 = A8-A23 */

	/* Parallel NOR Flash */
	/* Assumes previous SDRAM setup A1-A23,D0-D15,WE0 */
	pfc_set_pin_function(9, 0, ALT1, 0, 0);	/* P9_0 = A24 */
	pfc_set_pin_function(9, 1, ALT1, 0, 0);	/* P9_1 = A25 */
	pfc_set_pin_function(7, 8, ALT1, 0, 0);	/* P7_8 = RD */
	pfc_set_pin_function(7, 0, ALT1, 0, 0);	/* P7_0 = CS0 */

	/* LEDs */
	pfc_set_gpio(4, 10, GPIO_OUT); /* P4_10 = GPIO_OUT */
	pfc_set_gpio(4, 11, GPIO_OUT); /* P4_11 = GPIO_OUT */



	/**********************************************/
	/* Configure NOR Flash Chip Select (CS0, CS1) */
	/**********************************************/
	#define CS0WCR_D	0x00000b40
	#define CS0BCR_D	0x10000C00
	#define CS1WCR_D	0x00000b40
	#define CS1BCR_D	0x10000C00
	*(u32 *)CS0WCR = CS0WCR_D;
	*(u32 *)CS0BCR = CS0BCR_D;
	*(u32 *)CS1WCR = CS1WCR_D;
	*(u32 *)CS1BCR = CS1BCR_D;


	/**********************************************/
	/* Configure SDRAM (CS2, CS3)                 */
	/**********************************************/
	/* Even if you are only using CS2, we need to set up
	   the CS3 register CS3WCR because some bits are common for CS3 and CS2 */

	#define CS2BCR_D	0x00004C00	/* Type=SDRAM, 16-bit memory */
	#define CS2WCR_D	0x00000480	/* CAS Latency = 2 */
	#define CS3BCR_D	0x00004C00	/* Type=SDRAM, 16-bit memory */
	//#define CS3WCR_D	0x00002492	/*  */
	#define CS3WCR_D	1 << 13	|	/* (CS2,CS3) WTRP (1 cycles) */\
				1 << 10 |	/* (CS2,CS3) WTRCD (1 cycle) */\
				1 <<  7 |	/*     (CS3) A3CL (CAS Latency = 2) */\
				2 <<  3 |	/* (CS2,CS3) TRWL (2 cycles) */\
				2 <<  0		/* (CS2,CS3) WTRC (5 cycles) */
	#define SDCR_D		0x00120812	/* 13-bit row, 10-bit col, auto-refresh */

	/*
	 * You must refresh all rows within the amount of time specified in the memory spec.
	 * Total Refresh time =  [Number_of_rows] / [Clock_Source / Refresh Counter]
	 * 63.0ms =  [8192] /  [(66.6MHz / 16) / 32]
	 */
	#define RTCOR_D		0xA55A0020	/* Refresh Counter = 32 */
	#define RTCSR_D		0xA55A0010	/* Clock Source=CKIO/16 (CKIO=66MHz) */

	*(u32 *)CS2BCR = CS2BCR_D;
	*(u32 *)CS2WCR = CS2WCR_D;
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
	#define SDRAM_MODE_CS2 0x3FFFD040	/* CS2: CAS=2, burst write, 16bit bus */
	#define SDRAM_MODE_CS3 0x3FFFE040	/* CS3: CAS=2, burst write, 16bit bus */
	*(u16 *)SDRAM_MODE_CS2 = 0;
	*(u16 *)SDRAM_MODE_CS3 = 0;


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
#if !defined(CONFIG_BOOT_MODE0)
	printf(	"\t\t      SPI Flash Memory Map\n"
		"\t\t------------------------------------\n"
		"\t\t         Start      Size     SPI\n");
	printf(	"\t\tu-boot:  0x%08X 0x%06X 0\n", 0,CONFIG_ENV_OFFSET);
	printf(	"\t\t   env:  0x%08X 0x%06X 0\n", CONFIG_ENV_OFFSET, CONFIG_ENV_SIZE);
	printf(	"\t\t    DT:  0x%08X 0x%06X 0\n", CONFIG_ENV_OFFSET+CONFIG_ENV_SIZE,CONFIG_ENV_SECT_SIZE);
	printf(	"\t\tKernel:  0x%08X 0x%06X 0+1 (size*=2)\n",0x100000, 0x280000);
	printf(	"\t\trootfs:  0x%08X 0x%06X 0+1 (size*=2)\n",0x400000, 0x2000000-0x400000);
#endif

	/* Default addresses */
	#define DTB_ADDR_FLASH		"C0000"		/* Location of Device Tree in QSPI Flash (SPI flash offset) */
	#define DTB_ADDR_RAM		"20500000"	/* Internal RAM location to copy Device Tree */
	#define DTB_ADDR_SDRAM		"09800000"	/* External SDRAM location to copy Device Tree */

	#define MEM_ADDR_RAM		"0x20000000 0x00A00000"	/* System Memory for when using on-chip RAM (10MB) */
	#define MEM_ADDR_SDRAM		"0x08000000 0x08000000"	/* System Memory for when using external SDRAM RAM (128MB) */

	#define KERNEL_ADDR_FLASH	"0x18200000"	/* Flash location of xipImage or uImage binary */
	#define UIMAGE_ADDR_SDRAM	"09000000"	/* Address to copy uImage to in external SDRAM */
	#define UIMAGE_ADDR_SIZE	"0x400000"	/* Size of the uImage binary in Flash (4MB) */


	/* Default kernel command line options */
	setenv("cmdline_common", "ignore_loglevel earlyprintk earlycon");

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
	setenv("xImg", "qspi dual; setenv cmd bootx "KERNEL_ADDR_FLASH" ${addr_dtb}; run cmd");	/* Boot a XIP Kernel */
	setenv("uImg", "qspi dual; cp.b "KERNEL_ADDR_FLASH" "UIMAGE_ADDR_SDRAM" "UIMAGE_ADDR_SIZE"; bootm start "UIMAGE_ADDR_SDRAM" - "DTB_ADDR_SDRAM"; bootm loados ; bootm go");	/* Boot a uImage kernel */

	/* => run xa_boot */
	/* Boot XIP using internal RAM only, file system is AXFS, LCD dynamically allocated */
	setenv("xa_boot", "run dtb_read_ram dtb_mem_ram dtb_lcdfb_dyn; setenv bootargs ${cmdline_common} ${fs_axfs}; fdt chosen; run xImg");

	/* => run xsa_boot */
	/* Boot XIP using external 32MB SDRAM, file system is AXFS, LCD FB fixed to internal RAM */
	setenv("xsa_boot", "run dtb_read_sdram dtb_mem_sdram dtb_lcdfb_fixed; setenv bootargs ${cmdline_common} ${fs_axfs}; fdt chosen; run xImg");

	/* => run s_boot */
	/* Boot SDRAM uImage using external 32MB SDRAM, file system is squashfs, LCD FB fixed to internal RAM */
	setenv("s_boot", "run dtb_read_sdram dtb_mem_sdram dtb_lcdfb_fixed; setenv bootargs ${cmdline_common} ${fs_mtd}; fdt chosen; run uImg");

	/* => run sa_boot */
	/* Boot SDRAM uImage using external 32MB SDRAM, file system is AXFS, LCD FB fixed to internal RAM */
	setenv("sa_boot", "run dtb_read_sdram dtb_mem_sdram dtb_lcdfb_fixed; setenv bootargs ${cmdline_common} ${fs_axfs}; fdt chosen; run uImg");

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
#if 0
	if (value)	/* turn LED on */
		gpio_set(7,1,0);
	else		/* turn LED off */
		gpio_set(7,1,1);
#endif
}

u8 button_check_state(u8 sw)
{
	/* returns: 1 = button up
		    0 = button pressed
	*/
#if 0
	if (sw == 1)	/* SW 1 */
		return gpio_read(1, 9);
#endif
	return 1;
}

