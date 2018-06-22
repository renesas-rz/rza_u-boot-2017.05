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
U_BOOT_DEVICE(grpeach_serial) = {
	.name = "serial_sh",
	.platdata = &serial_platdata,
};

int board_early_init_f(void)
{
	/* This function runs early in the boot process, before u-boot is relocated
	   to RAM (hence the '_f' in the function name stands for 'still running from
	   flash'). A temporary stack has been set up for us which is why we can
	   have this as C code. */

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
	pfc_set_pin_function(6, 3, ALT7, 0, 0);	/*TxD2*/
	pfc_set_pin_function(6, 2, ALT7, 0, 0);	/*RxD2*/

	/* QSPI_0 ch0 (booted in 1-bit, need to change to 4-bit) */
	pfc_set_pin_function(9, 2, ALT2, 0, 0);	/* P9_2 = SPBCLK_0 */
	pfc_set_pin_function(9, 3, ALT2, 0, 0);	/* P9_3 = SPBSSL_0 */
	pfc_set_pin_function(9, 4, ALT2, 0, 1);	/* P9_4 = SPBIO00_0 (bi dir) */
	pfc_set_pin_function(9, 5, ALT2, 0, 1);	/* P9_5 = SPBIO10_0 (bi dir) */
	pfc_set_pin_function(9, 6, ALT2, 0, 1);	/* P9_6 = SPBIO20_0 (bi dir) */
	pfc_set_pin_function(9, 7, ALT2, 0, 1);	/* P9_7 = SPBIO30_0 (bi dir) */


	/* Ethernet */
	pfc_set_pin_function(1, 14, ALT4, 0, 0); /* P1_14 = ET_COL */
	pfc_set_pin_function(5, 9, ALT2, 0, 0);	/* P5_9 = ET_MDC */
	pfc_set_pin_function(3, 3, ALT2, 0, 1);	/* P3_3 = ET_MDIO (bi dir) */
	pfc_set_pin_function(3, 4, ALT2, 0, 0);	/* P3_4 = ET_RXCLK */
	pfc_set_pin_function(3, 5, ALT2, 0, 0);	/* P3_5 = ET_RXER */
	pfc_set_pin_function(3, 6, ALT2, 0, 0);	/* P3_6 = ET_RXDV */
	pfc_set_pin_function(3, 0, ALT2, 0, 0);	/* P2_0 = ET_TXCLK */
	pfc_set_pin_function(10, 1, ALT4, 0, 0);/* P2_1 = ET_TXER */
	pfc_set_pin_function(10, 2, ALT4, 0, 0);/* P2_2 = ET_TXEN */
	pfc_set_pin_function(10, 3, ALT4, 0, 0);/* P2_3 = ET_CRS */
	pfc_set_pin_function(10, 4, ALT4, 0, 0);/* P2_4 = ET_TXD0 */
	pfc_set_pin_function(10, 5, ALT4, 0, 0);/* P2_5 = ET_TXD1 */
	pfc_set_pin_function(10, 6, ALT4, 0, 0);/* P2_6 = ET_TXD2 */
	pfc_set_pin_function(10, 7, ALT4, 0, 0);/* P2_7 = ET_TXD3 */
	pfc_set_pin_function(10, 8, ALT4, 0, 0);/* P2_8 = ET_RXD0 */
	pfc_set_pin_function(10, 9, ALT4, 0, 0);/* P2_9 = ET_RXD1 */
	pfc_set_pin_function(10, 10, ALT4, 0, 0);/* P2_10 = ET_RXD2 */
	pfc_set_pin_function(10, 11, ALT4, 0, 0);/* P2_11 = ET_RXD3 */
	//pfc_set_pin_function(4, 14, ALT8, 0, 0); /* P4_14 = IRQ6 (ET_IRQ) */ /* NOTE: u-boot doesn't enable interrupts */

	/* Ethernet - GPIO toggle reset*/
	pfc_set_gpio(4, 2, GPIO_OUT); /* P4_2 = GPIO, 0 */
	gpio_set(4, 2, 0);
	gpio_set(4, 2, 1);

	/* USB - Enable 5v VBUS supply */
	pfc_set_gpio(4, 1, GPIO_OUT); /* P4_2 = GPIO, 0 */
	gpio_set(4, 1, 0);

	/* LEDs */
	pfc_set_gpio(6, 12, GPIO_OUT); /* P6_12 = LED_USER */
	gpio_set(6, 12, 1);	/* LED_USER ON */
	pfc_set_gpio(6, 13, GPIO_OUT); /* P6_13 = LED_R */
	gpio_set(6, 13, 0);	/* LED_R OFF */
	pfc_set_gpio(6, 14, GPIO_OUT); /* P6_14 = LED_G */
	gpio_set(6, 14, 0);	/* LED_G OFF */
	pfc_set_gpio(6, 15, GPIO_OUT); /* P6_15 = LED_B */
	gpio_set(6, 15, 0);	/* LED_B OFF */

	/* SW0 */
	pfc_set_gpio(6, 0, GPIO_IN); /* P6_0 = GPIO_IN */

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
	printf(	"\t\tKernel:  0x%08X 0x%06X 0\n",0x100000, 0x50000);
	printf(	"\t\tRootfs:  0x%08X 0x%06X 0\n",0x600000, 0xA0000);

#endif

	/* Default addresses */
	#define DTB_ADDR_FLASH		"C0000"		/* Location of Device Tree in QSPI Flash (SPI flash offset) */
	#define DTB_ADDR_RAM		"20500000"	/* Internal RAM location to copy Device Tree */

	#define MEM_ADDR_RAM		"0x20000000 0x00A00000"	/* System Memory for when using on-chip RAM (10MB) */

	#define KERNEL_ADDR_FLASH	"0x18100000"	/* Flash location of xipImage or uImage binary */


	/* Default kernel command line options */
	setenv("cmdline_common", "ignore_loglevel earlyprintk earlycon=scif,0xE8008000"); /* Linux-4.9 */
	//setenv("cmdline_common", "console=ttySC2,115200 ignore_loglevel earlyprintk"); /* Linux-3.14 */

	/* Root file system choices */
	setenv("fs_axfs", "rootfstype=axfs rootflags=physaddr=0x18600000");
	setenv("fs_mtd",  "root=/dev/mtdblock0");

	/* LCD Frame buffer location */
	setenv("dtb_lcdfb_dyn",   "fdt set /display@fcff7400 fb_phys_addr <0x00000000>");	/* Dynamically allocate during boot */

	/* Read DTB from Flash into either internal on-chip RAM or external SDRAM */
	setenv("dtb_read_ram",   "sf probe 0; sf read "DTB_ADDR_RAM" "DTB_ADDR_FLASH" 8000; fdt addr "DTB_ADDR_RAM" ; setenv addr_dtb "DTB_ADDR_RAM"");

	/* Set the system memory address and size. This overrides the setting in Device Tree */
	setenv("dtb_mem_ram",   "fdt memory "MEM_ADDR_RAM"");		/* Use internal RAM for system memory */

	/* Kernel booting operations */
	setenv("xImg", "qspi single; setenv cmd bootx "KERNEL_ADDR_FLASH" ${addr_dtb}; run cmd");	/* Boot a XIP Kernel */

	/* => run xa_boot */
	/* Boot XIP using internal RAM only, file system is AXFS, LCD dynamically allocated */
	setenv("xa_boot", "run dtb_read_ram dtb_mem_ram dtb_lcdfb_dyn; setenv bootargs ${cmdline_common} ${fs_axfs}; fdt chosen; run xImg");

	/* => run xu_boot */
	/* Boot XIP using internal RAM only, file system is on USB, LCD dynamically allocated */
	setenv("xu_boot", "run dtb_read_ram dtb_mem_ram dtb_lcdfb_dyn; setenv bootargs ${cmdline_common} root=/dev/sda1 rootwait; fdt chosen; run xImg");


	/* Boot XIP using internal RAM */
	/* Rootfs is a AXFS image in memory mapped QSPI */
	/* => run xa_boot */
	/* Read out DT blob */
	setenv("xa1", "sf probe 0; sf read 20500000 C0000 8000");
	/* Change memory address in DTB */
	setenv("xa2", "fdt addr 20500000 ; fdt memory 0x20000000 0x00A00000"); /* 10MB RAM */
	setenv("xa3", "qspi single");
	setenv("xaargs", "console=ttySC2,115200 console=tty0 ignore_loglevel root=/dev/null rootflags=physaddr=0x18600000 earlyprintk rz_irq_trim");
	setenv("xa2_boot", "run xa1 xa2 xa3; setenv bootargs ${xaargs}; fdt chosen; bootx 18100000 20500000");


	gpio_set(6, 12, 0);	/* LED_USER OFF */

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
		gpio_set(6, 12, 1);	/* LED_USER ON */
	else		/* turn LED off */
		gpio_set(6, 12, 0);	/* LED_USER OFF */

}

u8 button_check_state(u8 sw)
{
	/* returns: 1 = button up
		    0 = button pressed
	*/
	/* SW0 */
	return gpio_read(6, 0);
}

