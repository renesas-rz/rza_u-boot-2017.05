/*
 * Configuration settings for the Renesas RZATEMPLATE board
 *
 */

#ifndef __RZATEMPLATE_H
#define __RZATEMPLATE_H

#define CONFIG_ARCH_RMOBILE_BOARD_STRING	"RZATEMPLATE"
#define CONFIG_MACH_TYPE			0xFFFFFFFF /* Boot Linux using Device Tree */

/* Board Clock */
#define CONFIG_SYS_CLK_FREQ	66000000 /* P1 clock frequency (XTAL=24MHz) */

/* Set all system clocks to full speed */
/* On reset, the CPU will be running at 1/2 speed */
/* In the Hardware Manual, see Table 6.4 Settable Frequency Ranges */
#define FRQCR_D		0x1012
	/* PLL(x88), I:G:B:P1:P0 = 22:11:5.5:2.75:1.375 */
	/* CKIO:Output at time usually,                 */
	/* Output when bus right is opened,             */
	/* output at standby"L"                         */
	/* Clockin  = 24MHz,                            */
	/* I  Clock = 528MHz,                           */
	/* G  Clock = 264MHz                            */
	/* B  Clock = 132MHz,                           */
	/* P1 Clock = 66MHz,                            */
	/* P0 Clock = 33MHz                             */

#define CKIOSEL_D	0x0000; /* CKIO Output = B Clock  */

/* Serial Console */
#define CONFIG_SCIF_CONSOLE		/* Enable Serial Console Driver */
#define SCIF_CONSOLE_BASE SCIF2_BASE	/* ## Select the SCIF channel ## */
#define CONFIG_BAUDRATE		115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ CONFIG_BAUDRATE }

/* Miscellaneous */
#define CONFIG_SYS_LONGHELP		/* undef to save memory	*/
#define CONFIG_SYS_CBSIZE	256	/* Boot Argument Buffer Size */
#define CONFIG_SYS_PBSIZE	256	/* Print Buffer Size (default=384) */
#define CONFIG_SYS_MAXARGS	16	/* max number of command args */
#define CONFIG_SYS_ARM_CACHE_WRITETHROUGH
#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs */
#define CONFIG_CMDLINE_EDITING		/* add command line history	*/
#define __io	/* Support in,out functions on RAM. See arch/arm/include/asm/io.h */
#define CONFIG_ARCH_CPU_INIT


/* Internal RAM Size */
/*
 * u-boot will be relocated to internal RAM during boot.
 * RZ/A2M = 4MB
 */
#define CONFIG_SYS_SDRAM_BASE		0x80000000
#define CONFIG_SYS_SDRAM_SIZE		(4 * 1024 * 1024)
#define CONFIG_SYS_INIT_SP_ADDR         (CONFIG_SYS_SDRAM_BASE + CONFIG_SYS_SDRAM_SIZE - 1*1024*1024)
#define _HIDE_CONFIG_SYS_INIT_SP_ADDR         0x80400000 /* Internal RAM @ 4MB (not used) */
#define	_HIDE_CONFIG_LOADADDR			CONFIG_SYS_SDRAM_BASE	/* default load address (not used) */
#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 4*1024*1024)

#define CONFIG_NR_DRAM_BANKS		1	/* must be set to 1 */


/* Boot Mode Address */
/*
 * You can choose to boot from:
 * 1. QSPI flash                  (base address = 0x18000000)
 * 2.
 * 3. RAM (pre-loaded with JTAG)  (base address = 0x80020000)
 */
#define BOOT_MODE 1	/* << MAKE YOUR SELECTION HERE >> */


/* QSPI Flash Boot */
#if (BOOT_MODE == 1)
  #define CONFIG_SYS_TEXT_BASE		0x20000000
  #define CONFIG_ENV_IS_IN_SPI_FLASH
  #define CONFIG_ENV_OFFSET		0x80000
  #define CONFIG_ENV_SECT_SIZE		0x10000		/* smallest erase sector size */
  #define CONFIG_ENV_SIZE		CONFIG_ENV_SECT_SIZE
  #define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET)

/* RAM Boot
 *
 * Build a version that can be downloaded to RAM directly and run
 * in order to be used to program QSPI flash for the first time.
 */
#elif (BOOT_MODE == 3)
  #define CONFIG_SYS_TEXT_BASE		0x80020000
  #define CONFIG_ENV_IS_NOWHERE		/* Store ENV in RAM memory only */
  #define CONFIG_ENV_SIZE		0x1000

  #define CONFIG_ENV_OFFSET		0 /* dummy */
  #define CONFIG_ENV_SECT_SIZE		0 /* dummy */

#else
  #error "huh?"
#endif

#define CONFIG_ENV_OVERWRITE	1

/* Malloc */
#define CONFIG_SYS_MALLOC_LEN		(256 * 1024)
#define CONFIG_SYS_MONITOR_LEN		(128 * 1024)

/* Kernel Boot */
#define CONFIG_BOOTARGS			"ignore_loglevel"

/* SPI Flash Device Selection */
/* Enabled using menuconfig:
 * Device Drivers > SPI Flash Support > Legacy SPI Flash Interface support
 */
#define CONFIG_SPI_FLASH_SPANSION
#define CONFIG_SPI_FLASH_MACRONIX
#define CONFIG_SPI_FLASH_BAR		/* For SPI Flash bigger than 16MB */


/* SECT_NOR_FLASH */
/* Parallel NOR Flash */
#define CONFIG_SYS_FLASH_BASE		0x00000000
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	512
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE }
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_FLASH_BASE
#define CONFIG_MTD_NOR_FLASH
/* SECT_NOR_FLASH_END */

/* SECT_I2C */
/* I2C configuration */
/* Enabled using menuconfig:
 * Device Drivers > I2C support > Renesas RZ/A RIIC Driver
 */
#ifdef CONFIG_RZA_RIIC
 #define CONFIG_HARD_I2C
 #define CONFIG_I2C_MULTI_BUS
 #define CONFIG_SYS_MAX_I2C_BUS		4
 #define CONFIG_SYS_I2C_SPEED		100000 /* 100 kHz */
 #define CONFIG_CMD_EEPROM			/* add eeprom command */
 #define CONFIG_SYS_I2C_EEPROM_ADDR	0x50
 #define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1
 #define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	4	/* 16-Byte Write Mode */
 #define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10	/* 10ms of delay */
 #define CONFIG_SYS_I2C_MODULE		3
#endif
/* SECT_I2C_END */

/* SECT_ETHERNET */
/* Network interface */
#define CONFIG_SH_ETHER
#define CONFIG_SH_ETHER_USE_PORT	0
#define CONFIG_SH_ETHER_PHY_ADDR	0
#define CONFIG_SH_ETHER_PHY_MODE PHY_INTERFACE_MODE_MII
#define CONFIG_PHYLIB
#define CONFIG_BITBANGMII
#define CONFIG_BITBANGMII_MULTI
#define CONFIG_IPADDR		192.168.0.55
#define CONFIG_SERVERIP		192.168.0.1
/* SECT_ETHERNET_END */

/* SECT_SDMMC */
/* SD/MMC */
#define CONFIG_CMD_MMC			1
#define CONFIG_MMC			1
#define CONFIG_GENERIC_MMC		1
#define CONFIG_SH_SDHI			1
#define CONFIG_SH_SDHI_FREQ		(CONFIG_SYS_CLK_FREQ*2) /* B clk */
/* SECT_SDMMC_END */

/*
 * Lowlevel configuration
 */
/* Disable WDT */
#define WTCSR_D		0xA518
#define WTCNT_D		0x5A00

/* Enable all peripheral clocks */
#define STBCR2_D	0x6A
#define STBCR3_D	0x80 /* OSTM0, OSTM1, OSTM3, MTU3, CAN-FD, ADC, GPT */
#define STBCR4_D	0x00 /* SCIF0, SCIF1, SCIF2, SCIF3, SCIF4, SCI0, SCI1,IrDA */
#define STBCR5_D	0x31 /* A/D, CEU, RTC0, RTC1, JCU */
#define STBCR6_D	0x80 /* VIN, ETHER0, ETHER1, EtherPTR, EtherM, USB0, USB1 */
#define STBCR7_D	0x10 /* IMR-LS2, DAVE-2D, MIPI, SSIF0, SSIF1, SSIF2, SSIF3  */
#define STBCR8_D	0x05 /* IIC0, IIC1, IIC2, IIC3, SPIBSC, VDC6 */
#define STBCR9_D	0x10 /* RSPI0, RSPI1, RSPI2, HYPER, OCTA, SPDIF, DRP */
#define STBCR10_D	0x00 /* TSIP, SDHI00, SDHI01, SDHI10, SDHI11 */
#define STBCR11_D	0x3F /* POE3, POEG */



#endif	/* __RZATEMPLATE_H */
