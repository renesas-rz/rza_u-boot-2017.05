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
#include <asm/arch/sh_sdhi.h>

//#define DEBUG

DECLARE_GLOBAL_DATA_PTR;

/* Serial Console */
static const struct sh_serial_platdata serial_platdata = {
	.base = SCIF_CONSOLE_BASE,	/* SCIFx_BASE */
	.type = PORT_SCIF,
	.clk = CONFIG_SYS_CLK_FREQ,		/* P1 Clock */
};
U_BOOT_DEVICE(rskrza1_serial) = {
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
	/* Specific for the RZ/H on the RSK board. Adjust for your board as needed. */

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

	/* RIIC Ch 3 */
	pfc_set_pin_function(1, 6, ALT1, 0, 1);	/* P1_6 = RIIC3SCL (bi dir) */
	pfc_set_pin_function(1, 7, ALT1, 0, 1);	/* P1_7 = RIIC3SDA (bi dir) */

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
#ifdef CONFIG_SH_MMCIF
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

#ifdef CONFIG_SH_SDHI
	pfc_set_pin_function(3, 8, ALT7, 0, 0);		/* SD_CD_1 */
	pfc_set_pin_function(3, 9, ALT7, 0, 0);		/* SD_WP_1 */
	pfc_set_pin_function(3, 10, ALT7, 0, 1);	/* SD_D1_1 (bi dir) */
	pfc_set_pin_function(3, 11, ALT7, 0, 1);	/* SD_D0_1 (bi dir) */
	pfc_set_pin_function(3, 12, ALT7, 0, 0);	/* SD_CLK_1 */
	pfc_set_pin_function(3, 13, ALT7, 0, 1);	/* SD_CMD_1 (bi dir) */
	pfc_set_pin_function(3, 14, ALT7, 0, 1);	/* SD_D3_1 (bi dir) */
	pfc_set_pin_function(3, 15, ALT7, 0, 1);	/* SD_D2_1 (bi dir) */
#endif

	/* SDRAM */
	pfc_set_pin_function(5, 8, ALT6, 0, 0);	/* P5_8 = CS2 */
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

	/* LED 0 */
	pfc_set_gpio(7, 1, GPIO_OUT); /* P7_1 = GPIO_OUT */

	/* SW1 */
	pfc_set_gpio(1, 9, GPIO_IN); /* P1_9 = GPIO_IN */
	/* SW2 */
	pfc_set_gpio(1, 8, GPIO_IN); /* P1_8 = GPIO_IN */
	/* SW3 */
	pfc_set_gpio(1, 11, GPIO_IN); /* P1_11 = GPIO_IN */


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
	/* [[ RZ/A1H RSK BOARD ]]
	* Note : This configuration is invalid for a single SDRAM and is a
	*      : known limitation of the RSK+ board. The port pin used by
	*      : CS3 is configured for LED0. To allow SDRAM operation CS2
	*      : and CS3 must be configured to SDRAM. Option link R164 must
	*      : NOT be fitted to avoid a Data Bus conflict on the SDRAM
	*      : and expansion buffers. In a new application with one SDRAM
	*      : always connect the SDRAM to CS3. On this RSK+ CS3 can not
	*      : be used in another configuration including the expansion
	*      : headers unless the SDRAM is completely disabled. For other
	*      : external memory mapped devices CS1 is available for use
	*      : with the expansion headers.
	*      : See the hardware manual Bus State Controller
	*/
	/* Additionally, even though we are only using CS2, we need to set up
	   the CS3 register CS3WCR because some bits are common for CS3 and CS2 */

	#define CS2BCR_D	0x00004C00	/* Type=SDRAM, 16-bit memory */
	#define CS2WCR_D	0x00000480	/* CAS Latency = 2 */
	#define CS3BCR_D	0x00004C00	/* Type=SDRAM, 16-bit memory */
	//#define CS3WCR_D	0x00004492	/*  */
	#define CS3WCR_D	2 << 13	|	/* (CS2,CS3) WTRP (2 cycles) */\
				1 << 10 |	/* (CS2,CS3) WTRCD (1 cycle) */\
				1 <<  7 |	/*     (CS3) A3CL (CAS Latency = 2) */\
				2 <<  3 |	/* (CS2,CS3) TRWL (2 cycles) */\
				2 <<  0		/* (CS2,CS3) WTRC (5 cycles) */
	#define SDCR_D		0x00110811	/* 13-bit row, 9-bit col, auto-refresh */

	/*
	 * You must refresh all rows within the amount of time specified in the memory spec.
	 * Total Refresh time =  [Number_of_rows] / [Clock_Source / Refresh Counter]
	 * 63.0ms =  [8192] /  [(66.6MHz / 4) / 128]
	 */
	#define RTCOR_D		0xA55A0080	/* Refresh Counter = 128 */
	#define RTCSR_D		0xA55A0008	/* Clock Source=CKIO/4 (CKIO=66MHz) */

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

#ifdef CONFIG_SH_SDHI
	ret = sh_sdhi_init(CONFIG_SYS_SH_SDHI1_BASE, RZ_SDHI_CHANNEL,
			   SH_SDHI_QUIRK_32BIT_BUF);
#endif
	return ret;
}

int board_late_init(void)
{
	u8 mac[6];
	u8 tmp[1];

	/* Read Mac Address and set*/
	i2c_init(CONFIG_SYS_I2C_SPEED, 0);
	i2c_set_bus_num(CONFIG_SYS_I2C_MODULE);

	/*
	 * PORT EXPANDER
	 *
	 * PX1.0  LED1             O  1  0=ON, 1=OFF
	 * PX1.1  LED2             O  1  0=ON, 1=OFF
	 * PX1.2  LED3             O  1  0=ON, 1=OFF
	 * PX1.3  NOR_A25          O  0  Bit #25 of NOR Flash Addressing
	 * PX1.4  PMOD1_RST        O  1  Reset for PMOD channel 1
	 * PX1.5  PMOD2_RST        O  1  Reset for PMOD channel 2
	 * PX1.6  SD_CONN_PWR_EN   O  0  Enable power supply for external SD card
	 * PX1.7  SD_MMC_PWR_EN    O  1  Enable power supply for MMC card
	 *
	 * PX2.0  PX1_EN0          O  0  0=LCD, 1=DV
	 * PX2.1  PX1_EN1          O  1  0=General Data, 1=Ethernet
	 * PX2.2  TFT_CS           O  0  Chip select for TFT
	 * PX2.3  PX1_EN3          O  0  0=PWM timer channels, 1=ADC/DAC I/O lines
	 * PX2.4  USB_OVR_CURRENT  I  1  Signal from USB power controller (over-current)
	 * PX2.5  USB_PWR_ENA      O  0  Enable power supply for USB channel 0
	 * PX2.6  USB_PWR_ENB      O  0  Enable power supply for USB channel 1
	 * PX2.7  PX1_EN7          O  0  0=A18-A21, 1=SGOUT0-SGOUT4
	 */

	/* init PX01(IC34) */
	tmp[0] = 0x00;
	i2c_write(0x20, 3, 1, tmp, 1); /* config */
	tmp[0] = 0x37;
	i2c_write(0x20, 1, 1, tmp, 1); /* output */

	/*init PX02(IC35) */
	tmp[0] = 0x10;
	i2c_write(0x21, 3, 1, tmp, 1); /* config */
	tmp[0] = 0x12;
	i2c_write(0x21, 1, 1, tmp, 1); /* output */

	/* Read MAC address from EEPROM */
	i2c_read(CONFIG_SYS_I2C_EEPROM_ADDR,
		 2,				/* Address Offset of 2 */
		 CONFIG_SYS_I2C_EEPROM_ADDR_LEN,
		 mac, 6);			/* 6 byte MAC address */

	if (is_valid_ethaddr(mac))
		eth_setenv_enetaddr("ethaddr", mac);

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
	#define MEM_ADDR_SDRAM		"0x08000000 0x02000000"	/* System Memory for when using external SDRAM RAM (32MB) */

	#define KERNEL_ADDR_FLASH	"0x18200000"	/* Flash location of xipImage or uImage binary */
	#define UIMAGE_ADDR_SDRAM	"09000000"	/* Address to copy uImage to in external SDRAM */
	#define UIMAGE_ADDR_SIZE	"0x400000"	/* Size of the uImage binary in Flash (4MB) */


	/* Default kernel command line options */
	setenv("cmdline_common", "ignore_loglevel earlyprintk earlycon=scif,0xE8008000");

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

	/* => run xm_boot */
	/* Boot XIP using internal RAM only, file system is MTD (cramfs-XIP), LCD dynamically allocated */
	setenv("xm_boot", "run dtb_read_ram dtb_mem_ram dtb_lcdfb_dyn; setenv bootargs ${cmdline_common} ${fs_mtd}; fdt chosen; run xImg");

	/* => run xsm_boot */
	/* Boot XIP using external 32MB SDRAM, file system is MTD (cramfs-XIP), LCD FB fixed to internal RAM */
	setenv("xsm_boot", "run dtb_read_sdram dtb_mem_sdram dtb_lcdfb_fixed; setenv bootargs ${cmdline_common} ${fs_mtd}; fdt chosen; run xImg");

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
	if (value)	/* turn LED on */
		gpio_set(7,1,0);
	else		/* turn LED off */
		gpio_set(7,1,1);
}

u8 button_check_state(u8 sw)
{
	/* returns: 1 = button up
		    0 = button pressed
	*/

	if (sw == 1)	/* SW 1 */
		return gpio_read(1, 9);
	if (sw == 2)	/* SW 2 */
		return gpio_read(1, 8);
	if (sw == 3)	/* SW 3 */
		return gpio_read(1, 11);
	return 1;
}


/* Change I/O mux */
/* This command is only relevant to the RSK board */
static char io_mux_help_text[] =
	"Change the I/O Multiplexers by changing the output of I2C Port Expander 2.\n"
	"Usage: io_mux [a|b|c|d]\n"
	"\t   a: PX1_EN0=L: P10/P11 = LCD(CN44)\n"
	"\t   b: PX1_EN0=H: P10/P11 = CEU(CN41) and RSPI-1(CN25,CN26)\n"
	"\n"
	"\t   c: PX1_EN1=L: P2/P3 = IO(JA1), RSPI-4(CN15), SIM(CN4)\n"
	"\t   d: PX1_EN1=H: P2/P3 = Ethernet\n"
	"\n"
	"\tCurrent PX1_EN0 = #\n"
	"\t        PX1_EN1 = #\n";
int do_io_mux(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u8 value;

	/* PORT EXPANDER 2 (IC35) on I2C bus 3 */
#define INPUT_PORT_REG 0
#define OUTPUT_PORT_REG 1
#define POLARITY_INV_REG 2
#define CONFIG_REG 3

	/* read current value of output register */
	i2c_set_bus_num(3);
	i2c_read(0x21, OUTPUT_PORT_REG, 1, &value, sizeof(value));

	/* need at least two arguments */
	if (argc <= 1) {

		/* replace the curent values in the help text */
		io_mux_help_text[sizeof(io_mux_help_text) - 24] = value & 1?'H':'L';
		io_mux_help_text[sizeof(io_mux_help_text) - 3] = value & 2?'H':'L';
		goto usage;
	}

	if( argv[1][0] == 'a' ) {
		value &= ~1;
	}
	else if( argv[1][0] == 'b' ) {
		value |= 1;
	}
	else if( argv[1][0] == 'c' ) {
		value &= ~2;
	}
	else if( argv[1][0] == 'd' ) {
		value |= 2;
	}
	else {
		printf("Invalid option\n");
		goto usage;
	}

	/* write new value to output register */
	i2c_write(0x21, OUTPUT_PORT_REG, 1, &value, sizeof(value)); /* output */

	return 0;

usage:
	return CMD_RET_USAGE;
}
U_BOOT_CMD(
	io_mux,	CONFIG_SYS_MAXARGS, 1, do_io_mux,
	"Change io mux", io_mux_help_text
);

/* Upstream Testing */
/* This command is used for upstream driver testing. Since the kernel
 * assumes that peripheral clocks and pin settings are at their chip
 * defaults, this command resets them back in order to uncover any
 * conditions that the u-boot hides by running first. */
int do_upstream_testing(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

	/* need at least two arguments */
	if (argc <= 1)
		goto usage;

	if( argv[1][0] == 'c' ) {
		/* turn everything off except SCIF2 and QSPI */
		*(u8 *)0xfcfe0420 = 0xff;
		*(u8 *)0xfcfe0424 = 0xdf;
		*(u8 *)0xfcfe0428 = 0xff;
		*(u8 *)0xfcfe042c = 0xff;
		*(u8 *)0xfcfe0430 = 0xff;
		*(u8 *)0xfcfe0434 = 0xff;
		*(u8 *)0xfcfe0438 = 0xf3;
		*(u8 *)0xfcfe043c = 0xff;
		*(u8 *)0xfcfe0440 = 0xff;
		*(u8 *)0xfcfe0444 = 0xff;
		printf("All clocks disabled except SCIF2 and QSPI\n");
	}

	if( argv[1][0] == 'p' ) {
		/* QSPI_0 ch0 (booted in 1-bit, need to change to 4-bit) */
		//pfc_set_gpio(9, 2, GPIO_IN);	/* P9_2 = SPBCLK_0 */
		//pfc_set_gpio(9, 3, GPIO_IN);	/* P9_3 = SPBSSL_0 */
		//pfc_set_gpio(9, 4, GPIO_IN);	/* P9_4 = SPBIO00_0 (bi dir) */
		//pfc_set_gpio(9, 5, GPIO_IN);	/* P9_5 = SPBIO10_0 (bi dir) */
		//pfc_set_gpio(9, 6, GPIO_IN);	/* P9_6 = SPBIO20_0 (bi dir) */
		//pfc_set_gpio(9, 7, GPIO_IN);	/* P9_7 = SPBIO30_0 (bi dir) */

		/* QSPI_0 ch1 (4-bit interface for dual QSPI mode) */
		//pfc_set_gpio(2, 12, GPIO_IN); /* P2_12 = SPBIO01_0 (bi dir) */
		//pfc_set_gpio(2, 13, GPIO_IN); /* P2_13 = SPBIO11_0 (bi dir) */
		//pfc_set_gpio(2, 14, GPIO_IN); /* P2_14 = SPBIO21_0 (bi dir) */
		//pfc_set_gpio(2, 15, GPIO_IN); /* P2_15 = SPBIO31_0 (bi dir) */

		/* RIIC Ch 3 */
		pfc_set_gpio(1, 6, GPIO_IN);	/* P1_6 = RIIC3SCL (bi dir) */
		pfc_set_gpio(1, 7, GPIO_IN);	/* P1_7 = RIIC3SDA (bi dir) */

		/* Ethernet */
		pfc_set_gpio(1, 14, GPIO_IN); /* P1_14 = ET_COL */
		pfc_set_gpio(5, 9, GPIO_IN);	/* P5_9 = ET_MDC */
		pfc_set_gpio(3, 3, GPIO_IN);	/* P3_3 = ET_MDIO (bi dir) */
		pfc_set_gpio(3, 4, GPIO_IN);	/* P3_4 = ET_RXCLK */
		pfc_set_gpio(3, 5, GPIO_IN);	/* P3_5 = ET_RXER */
		pfc_set_gpio(3, 6, GPIO_IN);	/* P3_6 = ET_RXDV */
		pfc_set_gpio(2, 0, GPIO_IN);	/* P2_0 = ET_TXCLK */
		pfc_set_gpio(2, 1, GPIO_IN);	/* P2_1 = ET_TXER */
		pfc_set_gpio(2, 2, GPIO_IN);	/* P2_2 = ET_TXEN */
		pfc_set_gpio(2, 3, GPIO_IN);	/* P2_3 = ET_CRS */
		pfc_set_gpio(2, 4, GPIO_IN);	/* P2_4 = ET_TXD0 */
		pfc_set_gpio(2, 5, GPIO_IN);	/* P2_5 = ET_TXD1 */
		pfc_set_gpio(2, 6, GPIO_IN);	/* P2_6 = ET_TXD2 */
		pfc_set_gpio(2, 7, GPIO_IN);	/* P2_7 = ET_TXD3 */
		pfc_set_gpio(2, 8, GPIO_IN);	/* P2_8 = ET_RXD0 */
		pfc_set_gpio(2, 9, GPIO_IN);	/* P2_9 = ET_RXD1 */
		pfc_set_gpio(2, 10, GPIO_IN); /* P2_10 = ET_RXD2 */
		pfc_set_gpio(2, 11, GPIO_IN); /* P2_11 = ET_RXD3 */
		//pfc_set_gpio(4, 14, GPIO_IN); /* P4_14 = IRQ6 (ET_IRQ) */ /* NOTE: u-boot doesn't enable interrupts */

		/* LED 0 */
		pfc_set_gpio(7, 1, GPIO_IN); /* P7_1 = GPIO_OUT */

		/* SW1 */
		pfc_set_gpio(1, 9, GPIO_IN); /* P1_9 = GPIO_IN */
		/* SW2 */
		pfc_set_gpio(1, 8, GPIO_IN); /* P1_8 = GPIO_IN */
		/* SW3 */
		pfc_set_gpio(1, 11, GPIO_IN); /* P1_11 = GPIO_IN */
		printf("All pins reset to GPIO except SCIF2, QSPI, and SDRAM\n");
	}

	return 0;

usage:
	return CMD_RET_USAGE;
}
static char upstream_testing_help_text[] =
	"Turn off clocks and reset pins to default status\n"
	"Usage: upstream_testing [c|p]\n"
	"\t   c: disable clocks\n"
	"\t   p: reset pinux";
U_BOOT_CMD(
	upstream_testing,	CONFIG_SYS_MAXARGS, 1, do_upstream_testing,
	"Turn off clocks and reset pins to default status", upstream_testing_help_text
);
