/*
 * Copyright (C) 2017 Renesas Electronics
 * Copyright (C) 2017 Chris Brandt
 *
 * This file is released under the terms of GPL v2 and any later version.
 * See the file COPYING in the root directory of the source tree for details.
 */

#include <common.h>
#include <asm/arch/rmobile.h>
#include <spi.h>
#include <spi_flash.h>

static int in_xip_mode = 0xff;


/* This function temporary disables the feature of OR-ing the results of
   commands together when using dual spi flash memory. When using single
   flash, it does nothing. */
void qspi_disable_auto_combine(void);


struct spi_flash_info {
	const char *name;
	u8	id[6];
	u8	id_len;
	u32	sector_size;
	u32	n_sectors;
	u16	page_size;
	u16	flags;
};

int spi_flash_cmd(struct spi_slave *spi, u8 cmd, void *response, size_t len);
struct spi_flash *spi_flash_probe(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int spi_mode);
int spi_flash_cmd_write(struct spi_slave *spi, const u8 *cmd, size_t cmd_len,
		const void *data, size_t data_len);
extern const struct spi_flash_info spi_flash_ids[];

#if defined(CONFIG_RZA1)
 #define CONFIG_RZA_BASE_QSPI0		0x3FEFA000
#elif defined(CONFIG_RZA2)
 #define CONFIG_RZA_BASE_QSPI0		0x1F800000
#else
#error "Unknown Device"
#endif

#define CMNCR_0	(CONFIG_RZA_BASE_QSPI0 + 0x000)	/* Common control register */
#define SSLDR_0 (CONFIG_RZA_BASE_QSPI0 + 0x004)	/* SSL Delay register */
#define DRCR_0	(CONFIG_RZA_BASE_QSPI0 + 0x00C)	/* Data Read Control Register */
#define DRCMR_0	(CONFIG_RZA_BASE_QSPI0 + 0x010)	/* Data Read Command Setting Register */
#define DREAR_0 (CONFIG_RZA_BASE_QSPI0 + 0x014)	/* Data read extended address setting register */
#define DRENR_0 (CONFIG_RZA_BASE_QSPI0 + 0x01C)	/* Data read enable setting register */
#define DROPR_0 (CONFIG_RZA_BASE_QSPI0 + 0x018)	/* Data read option setting register */
#define DMDMCR_0 (CONFIG_RZA_BASE_QSPI0 + 0x058)	/* SPI Mode Dummy Cycle Setting Register */
#define DRDRENR_0 (CONFIG_RZA_BASE_QSPI0 + 0x05C)	/* Data Read DDR Enable Register */

#define PHYCNT		(CONFIG_RZA_BASE_QSPI0 + 0x07C)	/* PHY Control Register */
#define PHYOFFSET1	(CONFIG_RZA_BASE_QSPI0 + 0x080)	/* PHY Offset Register 1 */
#define PHYOFFSET2	(CONFIG_RZA_BASE_QSPI0 + 0x084)	/* PHY Offset Register 2 */
#define PHYADJ1 	(CONFIG_RZA_BASE_QSPI0 + 0x070)	/* PHY Adjustment Register 1 */
#define PHYADJ2 	(CONFIG_RZA_BASE_QSPI0 + 0x074)	/* PHY Adjustment Register 2 */

struct read_mode {
	u8 cmd;
	char name[50];
};
#define READ_MODES 9
const struct read_mode modes[READ_MODES] = {
	{0x03, "Read Mode (3-byte Addr) (RZ/A1 reset value)"},
	{0x0C, "Fast Read Mode (4-byte Addr)"},
	{0x6C, "Quad Read Mode (4-byte Addr)"},
	{0xEC, "Quad I/O Read Mode (4-byte Addr)"},
	{0xEE, "Quad I/O DDR Read Mode (4-byte Addr)"},
	{0x0B, "Fast Read Mode (3-byte Addr)"},
	{0x6B, "Quad Read Mode (3-byte Addr)"},
	{0xEB, "Quad I/O Read Mode (3-byte Addr)"},
	{0xED, "Quad I/O DDR Read Mode (3-byte Addr)"},
};

/* If you are using a SPI Flash device that does not have 4-byte address
   commands (Flash size <= 16MB), then define ADDRESS_BYTE_SIZE to 3 in your board config file */
#if ADDRESS_BYTE_SIZE == 3
 //#define ADDRESS_BYTE_SIZE 3	/* Addresses are 3-bytes (A0-A23) */
 #define FAST_READ 0x0B		/* Fast Read Mode (1-bit cmd, 1-bit addr, 1-bit data, 3-bytes of address) */
 #define QUAD_READ 0x6B		/* Quad Read Mode (1-bit cmd, 1-bit addr, 4-bit data, 3-bytes of address) */
 #define QUAD_IO_READ 0xEB	/* Quad I/O Read Mode (1-bit cmd, 4-bit addr, 4-bit data, 3-bytes of address) */
 #define QUAD_IO_DDR_READ 0xED	/* Quad I/O DDR Read Mode (1-bit cmd, 1-bit addr, 4-bit data, 3-bytes of address) */
#else
 #define ADDRESS_BYTE_SIZE 4	/* Addresses are 4-bytes (A0-A31) */
 #define FAST_READ 0x0C		/* Fast Read Mode (1-bit cmd, 1-bit addr, 1-bit data, 4-bytes of address) */
 #define QUAD_READ 0x6C		/* Quad Read Mode (1-bit cmd, 1-bit addr, 4-bit data, 4-bytes of address) */
 #define QUAD_IO_READ 0xEC	/* Quad I/O Read Mode (1-bit cmd, 4-bit addr, 4-bit data, 4-bytes of address) */
 #define QUAD_IO_DDR_READ 0xEE	/* Quad I/O DDR Read Mode (1-bit cmd, 1-bit addr, 4-bit data, 4-bytes of address) */
#endif

/* These should be filled out for each device */
u32 g_FAST_RD_DMY;		/* Fast Read Mode */
u32 g_QUAD_RD_DMY;		/* Quad Read Mode  */
u32 g_QUAD_IO_RD_DMY;		/* Quad I/O Read Mode  */
u32 g_QUAD_IO_DDR_RD_DMY;	/* Quad I/O DDR Read Mode  */
u32 g_QUAD_IO_RD_OPT;		/* Addtional option or 'mode' settings */

/***********************/
/* Macronix MX25L12845 */
/***********************/
int enable_quad_macronix(struct spi_flash *sf, u8 quad_addr, u8 quad_data )
{
	/* NOTE: This driver is reference from Spansion */
	/* NOTE: Due to the Flash size <=16MB, it used  3-bytes of address */
	/* NOTE: Once quad commands are enabled, you don't need to disable
		 them to use the non-quad mode commands, so we just always
		 leave them on. */
	int ret = 0;
	u8 data[5];
	u8 cmd;
	u8 spi_cnt = 1;
	u8 st_reg[2];
	u8 cfg_reg[2];

	/* Read ID Register (for cases where not all parts are the same) */
	//ret |= spi_flash_cmd(sf->spi, 0x9F, &data[0], 5);

	if (sf->spi->cs)
		spi_cnt = 2; /* Dual SPI Flash */

	/* Read Status register (RDSR1 05h) */
	qspi_disable_auto_combine();
	ret |= spi_flash_cmd(sf->spi, 0x05, st_reg, 1*spi_cnt);

	/* Read Configuration register (RDCR 15h) */
	qspi_disable_auto_combine();
	ret |= spi_flash_cmd(sf->spi, 0x15, cfg_reg, 1*spi_cnt);

#ifdef DEBUG
	printf("Initial Values:\n");
	for(cmd = 0; cmd < spi_cnt; cmd++) {
		printf("   SPI Flash %d:\n", cmd+1);
		printf("\tStatus register = %02X\n", st_reg[cmd]);
		printf("\tConfiguration register = %02X\n", cfg_reg[cmd]);
	}
#endif

	/* Skip SPI Flash configure if already correct */
	/* Note that if dual SPI flash, both have to be set */
	if ( (st_reg[0] != 0x40 ) ||
	     ((spi_cnt == 2) && (st_reg[1] != 0x40 ))) {

		data[0] = 0x40;	/* status reg: Set QUAD bit */
		data[1] = 0x00; /* confg reg: Don't Care */

		/* Send Write Enable (WREN 06h) */
		ret |= spi_flash_cmd(sf->spi, 0x06, NULL, 0);

		/* Send Write Registers (WRSR 01h) */
		cmd = 0x01;
		ret |= spi_flash_cmd_write(sf->spi, &cmd, 1, data, 2);

		/* Wait till WIP clears */
		/* Read Status register (RDSR1 05h) */
		do
			spi_flash_cmd(sf->spi, 0x05, &data[0], 1);
		while( data[0] & 0x01 );

	}

#ifdef DEBUG
	/* Read Status register (RDSR1 05h) */
	qspi_disable_auto_combine();
	ret |= spi_flash_cmd(sf->spi, 0x05, st_reg, 1*spi_cnt);

	/* Read Configuration register (RDCR 15h) */
	qspi_disable_auto_combine();
	ret |= spi_flash_cmd(sf->spi, 0x15, cfg_reg, 1*spi_cnt);

	printf("New Values:\n");
	for(cmd = 0; cmd < spi_cnt; cmd++) {
		printf("   SPI Flash %d:\n", cmd+1);
		printf("\tStatus register = %02X\n", st_reg[cmd]);
		printf("\tConfiguration register = %02X\n", cfg_reg[cmd]);
	}
#endif

	/* Finally, fill out the global settings for
	   Number of Dummy cycles between Address and Data */

	/* Macronix MX25L12845 */
	/* According to the Macronix spec (Table 5), dummy cycles
	   are needed for FAST READ, QUAD READ, and QUAD I/O READ commands */
	g_FAST_RD_DMY = 8;		/* Fast Read Mode: 8 cycles */
	g_QUAD_RD_DMY = 8;		/* Quad Read Mode  */
	g_QUAD_IO_RD_DMY = 6;		/* Quad I/O Read Mode: 6 cycles */
	g_QUAD_IO_DDR_RD_DMY = 6;	/* Quad I/O DDR Read Mode  (NOT SUPPORTED) */

	g_QUAD_IO_RD_OPT = 0; 		/* (NOT SUPPORTED) */

	return ret;
}

/**********************/
/* Spansion S25FL512S */
/**********************/
int enable_quad_spansion(struct spi_flash *sf, u8 quad_addr, u8 quad_data )
{
	/* NOTE: Macronix and Windbond are similar to Spansion */
	/* NOTE: Once quad commands are enabled, you don't need to disable
		 them to use the non-quad mode commands, so we just always
		 leave them on. */
	int ret = 0;
	u8 data[5];
	u8 cmd;
	u8 spi_cnt = 1;
	u8 st_reg[2];
	u8 cfg_reg[2];

	/* Read ID Register (for cases where not all parts are the same) */
	//ret |= spi_flash_cmd(sf->spi, 0x9F, &data[0], 5);

	if (sf->spi->cs)
		spi_cnt = 2; /* Dual SPI Flash */

	/* Read Status register (RDSR1 05h) */
	qspi_disable_auto_combine();
	ret |= spi_flash_cmd(sf->spi, 0x05, st_reg, 1*spi_cnt);

	/* Read Configuration register (RDCR 35h) */
	qspi_disable_auto_combine();
	ret |= spi_flash_cmd(sf->spi, 0x35, cfg_reg, 1*spi_cnt);

#ifdef DEBUG
	printf("Initial Values:\n");
	for(cmd = 0; cmd < spi_cnt; cmd++) {
		printf("   SPI Flash %d:\n", cmd+1);
		printf("\tStatus register = %02X\n", st_reg[cmd]);
		printf("\tConfiguration register = %02X\n", cfg_reg[cmd]);
	}
#endif

	/* Skip SPI Flash configure if already correct */
	/* Note that if dual SPI flash, both have to be set */
	if ( (cfg_reg[0] != 0x02 ) ||
	     ((spi_cnt == 2) && (cfg_reg[1] != 0x02 ))) {

		data[0] = 0x00;	/* status reg: Don't Care */
		data[1] = 0x02; /* confg reg: Set QUAD, LC=00b */

		/* Send Write Enable (WREN 06h) */
		ret |= spi_flash_cmd(sf->spi, 0x06, NULL, 0);

		/* Send Write Registers (WRR 01h) */
		cmd = 0x01;
		ret |= spi_flash_cmd_write(sf->spi, &cmd, 1, data, 2);

		/* Wait till WIP clears */
		do
			spi_flash_cmd(sf->spi, 0x05, &data[0], 1);
		while( data[0] & 0x01 );

	}

#ifdef DEBUG
	/* Read Status register (RDSR1 05h) */
	qspi_disable_auto_combine();
	ret |= spi_flash_cmd(sf->spi, 0x05, st_reg, 1*spi_cnt);

	/* Read Configuration register (RDCR 35h) */
	qspi_disable_auto_combine();
	ret |= spi_flash_cmd(sf->spi, 0x35, cfg_reg, 1*spi_cnt);

	printf("New Values:\n");
	for(cmd = 0; cmd < spi_cnt; cmd++) {
		printf("   SPI Flash %d:\n", cmd+1);
		printf("\tStatus register = %02X\n", st_reg[cmd]);
		printf("\tConfiguration register = %02X\n", cfg_reg[cmd]);
	}
#endif

	/* Finally, fill out the global settings for
	   Number of Dummy cycles between Address and Data */

	/* Spansion S25FL512S */
	/* According to the Spansion spec (Table 8.5), dummy cycles
	   are needed when LC=00 (chip default) for FAST READ,
	   QUAD READ, and QUAD I/O READ commands */
	g_FAST_RD_DMY = 8;		/* Fast Read Mode: 8 cycles */
	g_QUAD_RD_DMY = 8;		/* Quad Read Mode  */
	g_QUAD_IO_RD_DMY = 4;		/* Quad I/O Read Mode: 4 cycles */
	g_QUAD_IO_DDR_RD_DMY = 6;	/* Quad I/O DDR Read Mode  (NOT SUPPORTED) */

	/* When sending a QUAD I/O READ command, and extra MODE field
	   is needed.
	     [[ Single Data Rate, Quad I/O Read, Latency Code=00b ]]
		<> command = 1-bit, 8 clocks
		<> Addr(32bit) = 4-bit, 8 clocks,
		<> Mode = 4-bit, 2 clocks
		<> Dummy = 4-bit, 4 clocks
		<> Data = 4-bit, 2 clocks x {length}
	    See "Figure 10.37 Quad I/O Read Command Sequence" in Spansion spec
	*/
	/* Use Option data registers to output '0' as the
	   'Mode' field by sending OPD3 (at 4-bit) between address
	   and dummy */
	g_QUAD_IO_RD_OPT = 1;

	return ret;
}

/*******************/
/* Micron N25Q512A */
/*******************/
int enable_quad_micron(struct spi_flash *sf, u8 quad_addr, u8 quad_data )
{
/* NOTES:
	To use the QUAD commands, you need to enable dummy cycles for
	every type of FAST_READ command. While this is fine when the RZ-QSPI
	is running in XIP mode, but when you switch back to SPI mode to use
	something like "sf probe", it can't deal with those dummy cycles,
	therefore, we need to remove the dummy cycles during each
	"sf probe". See function qspi_reset_device().
	It should be noted that if the RZ/A1 is rebooted in XIP mode
	with those dummy cycles still enabled in the SPI flash, the reboot
	will still work because the RZ/A1 uses the legacy READ (0x03) command
	on reset, not FAST_READ */

	int ret = 0;
	u8 cmd;
	u8 vcfg_reg[2];

#ifdef DEBUG
	u8 st_reg[2];
	u16 nvcfg_reg[2];
	u8 tmp;

	/* Dual SPI Flash */
	if (sf->spi->cs) {
		/* Read Flag Status register (70h) */
		qspi_disable_auto_combine();	/* only lasts 1 call */
		ret |= spi_flash_cmd(sf->spi, 0x70, st_reg, 1*2);

		/* Read NONVOLATILE Configuration register (B5h) */
		qspi_disable_auto_combine();	/* only lasts 1 call */
		ret |= spi_flash_cmd(sf->spi, 0xB5, nvcfg_reg, 2*2);

		/* swap 2nd and 3rd bytes...because data for each
		   SPI flash comes in interleaved */
		tmp = ((u8 *)nvcfg_reg)[1];
		((u8 *)nvcfg_reg)[1] = ((u8 *)nvcfg_reg)[2];
		((u8 *)nvcfg_reg)[2] = tmp;

		/* Read VOLATILE Configuration register (85h) */
		qspi_disable_auto_combine();	/* only lasts 1 call */
		ret |= spi_flash_cmd(sf->spi, 0x85, vcfg_reg, 1*2);

	}
	else {
		/* Read Flag Status register (70h) */
		ret |= spi_flash_cmd(sf->spi, 0x70, st_reg, 1);

		/* Read NONVOLATILE Configuration register (B5h) */
		ret |= spi_flash_cmd(sf->spi, 0xB5, nvcfg_reg, 2);

		/* Read VOLATILE Configuration register (85h) */
		ret |= spi_flash_cmd(sf->spi, 0x85, vcfg_reg, 1);
	}

	printf("Initial Values:\n");
	for(tmp = 0; tmp <= sf->spi->cs ;tmp++) {
		printf("   SPI Flash %d:\n", tmp+1);
		printf("\tStatus register = %02X\n", st_reg[tmp]);
		printf("\tNonVolatile Configuration register = %04X\n", nvcfg_reg[tmp]);
		printf("\tVolatile Configuration register = %02X\n", vcfg_reg[tmp]);
	}
#endif

	/* To use the QUAD commands, we need dummy cycles after every
	   FAST_READ and FAST_READ_xxx commands */
	/* Send WRITE VOLATILE CONFIGURATION REGISTER (81h) */
	cmd = 0x81;
	if( quad_addr )
		vcfg_reg[0] = 0x5B;	/* Quad IO: 5 dummy cycles */
	else if( quad_data )
		vcfg_reg[0] = 0x3B;	/* Quad Read: 3 dummy cycles */
	else
		vcfg_reg[0] = 0x0B;	/* FAST_READ: 0 dummy cycles */

	ret |= spi_flash_cmd(sf->spi, 0x06, NULL, 0);	/* Send Write Enable (06h) */
	ret |= spi_flash_cmd_write(sf->spi, &cmd, 1, vcfg_reg, 1); /* send same to both flash */

#ifdef DEBUG
	ret |= spi_flash_cmd(sf->spi, 0x70, st_reg, 1);
	ret |= spi_flash_cmd(sf->spi, 0xB5, nvcfg_reg, 2);
	ret |= spi_flash_cmd(sf->spi, 0x85, vcfg_reg, 1);
	printf("New Values:\n");
	for(tmp = 0; tmp<1;tmp++) {
		printf("   SPI Flash %d:\n", tmp+1);
		printf("\tStatus register = %02X\n", st_reg[tmp]);
		printf("\tNonVolatile Configuration register = %04X\n", nvcfg_reg[tmp]);
		printf("\tVolatile Configuration register = %02X\n", vcfg_reg[tmp]);
	}
#endif

	/* Finally, fill out the global settings for
	   Number of Dummy cycles between Address and data */

	/* Assuming a 66MHz clock. Table 13 of n25q_512mb spec */
	g_FAST_RD_DMY = 0;		/* Fast Read Mode: 0 cycles */
	g_QUAD_RD_DMY = 3;		/* Quad Read Mode: 3 cycles  */
	g_QUAD_IO_RD_DMY = 5;		/* Quad I/O Read Mode: 5 cycles  */

	g_QUAD_IO_RD_OPT = 0;		/* Quad I/O Read Mode: no additional cycles */

	/* NOTE: This part can not run DDR at 66MHz */
	g_QUAD_IO_DDR_RD_DMY = 0;	/* Quad I/O DDR Read Mode  */

	/* HACK! */
	if( quad_addr )
	{
		/* When in QUAD I/O mode, sometimes the data is not correct.
		   It appears like the address gets corrupted. Therefore
		   we need to slow down the SPI clock in this mode. */
		/* This might be because the board this code was developed on
		   had lots of wire leads attached to the SPI flash pins */
#ifdef CONFIG_RZA1_BASE_QSPI0
		#define	QSPI_SPBCR (0x0008)
		*(u32 *)(CONFIG_RZA1_BASE_QSPI0 + QSPI_SPBCR) = 0x0300; /* 22.22 Mbps */
		printf("\nINFO: clock is now 22.22Mbps (see function %s)\n\n",__func__);
#endif
	}

	return ret;
}

/* Dummy cycles are need for the quad mode FAST_READ commands,
   but they get applied to ever type of FAST_READ command.
   Since the "sf" commands in u-boot knows nothing about dummy
   cycles, we need to shut them off if we see a "sf probe" */
int remove_dummy_micron(struct spi_flash *sf)
{
	int ret;
	u8 cmd;
	u8 cfg_reg;

#ifdef DEBUG
	/* Read VOLATILE Configuration register (85h) */
	ret = spi_flash_cmd(sf->spi, 0x85, &cfg_reg, 1);
	printf("%s: Initial Volatile Configuration register = %02X\n", __func__, cfg_reg);
#endif

	/* Send Write Enable (06h) */
	ret = spi_flash_cmd(sf->spi, 0x06, NULL, 0);

	/* Set Volatile Configuration Register to default value */
	/* Send WRITE VOLATILE CONFIGURATION REGISTER (81h) */
	cmd = 0x81;
	cfg_reg = 0xFB;
	ret |= spi_flash_cmd_write(sf->spi, &cmd, 1, &cfg_reg, 1);

#ifdef DEBUG
	/* Read Volatile Configuration register (85h) */
	ret = spi_flash_cmd(sf->spi, 0x85, &cfg_reg, 1);
	printf("%s: New Volatile Configuration register = %02X\n", __func__, cfg_reg);
#endif

	return ret;
}

/* This function is called when "sf probe" is issued, meaning that
   the user wants to access the device in normal single wire SPI mode.
   Since some SPI devices have to have special setups to enable QSPI mode
   or DDR QSPI mode, this function is used to reset those settings
   back to normal single wire FAST_READ mode. */
int qspi_reset_device(struct spi_flash *sf)
{
	int ret = 0;
	const struct spi_flash_info *id;

	/* If we are already in SPI mode, just return */
	if ( !in_xip_mode )
		return 0;

	id = &spi_flash_ids[0];
	while (id->name) {
		if ( !strcmp(sf->name, id->name) )
			break;
		id++;
	}

	if( !id->name ) {
		printf("ERROR: Cannot find SPI Flash in ID table");
		return 1;
	}

	/* Spansion, Macronix and Windbond don't really need anything done */

	/* Check for Micron "N25Q512" */
	if ( (id->id[0] == 0x20) && (id->id[1] == 0xba) )
		ret = remove_dummy_micron(sf);

	in_xip_mode = 0;

	return ret;
}

/* QUAD SPI MODE */
int do_qspi(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct spi_flash *my_spi_flash;
	int ret = 0;
	int i;
	u8 cmd;
	u8 dual_chip;
	u8 quad_data;
	u8 quad_addr;
	u8 ddr;
	u32 dmdmcr, drenr, cmncr, drcmr, dropr, drdrenr;
	const struct spi_flash_info *id;

	/* need at least 1 argument (single/dual) */
	if (argc < 2)
		goto usage;

	if ( strcmp(argv[1], "single") == 0)
		dual_chip = 0;
	else if ( strcmp(argv[1], "dual") == 0)
		dual_chip = 1;
	else
		goto usage;

	if ( argc <= 2 )
		quad_addr = 1;
	else if ( strcmp(argv[2], "a1") == 0)
		quad_addr = 0;
	else if ( strcmp(argv[2], "a4") == 0)
		quad_addr = 1;
	else
		goto usage;

	if ( argc <= 3 )
		quad_data = 1;
	else if ( strcmp(argv[3], "d1") == 0)
		quad_data = 0;
	else if ( strcmp(argv[3], "d4") == 0)
		quad_data = 1;
	else
		goto usage;

	if ( argc <= 4 )
		ddr = 0;
	else if ( strcmp(argv[4], "sdr") == 0)
		ddr = 0;
	else if ( strcmp(argv[4], "ddr") == 0)
		ddr = 1;
	else
		goto usage;

	/* checks */
	if( quad_addr && !quad_data )
		return CMD_RET_USAGE;
	if( ddr && !quad_addr )
		return CMD_RET_USAGE;

	/* Read initial register values */
	dmdmcr = *(volatile u32 *)DMDMCR_0;
	drenr = *(volatile u32 *)DRENR_0;
	cmncr = *(volatile u32 *)CMNCR_0;
	drcmr = *(volatile u32 *)DRCMR_0;
	dropr = *(volatile u32 *)DROPR_0;
	drdrenr = *(volatile u32 *)DRDRENR_0;

	printf("Current Mode: ");
	cmd = (drcmr >> 16) & 0xFF;
	for( i=0; i < READ_MODES; i++) {
		if( modes[i].cmd == cmd )
			printf("%s\n",modes[i].name);
	}

	/* bus=0, cs=0, speed=1000000 */
	if( dual_chip )
		my_spi_flash = spi_flash_probe(0, 1, 1000000, SPI_MODE_3);
	else
		my_spi_flash = spi_flash_probe(0, 0, 1000000, SPI_MODE_3);

	if (!my_spi_flash) {
		printf("Failed to initialize SPI flash.");
		return 1;
	}

	/* The only thing we have access to is the string name of the
	 * device. Use that name to search the ID table to find the chip
	 * ID */
	id = &spi_flash_ids[0];
	while (id->name) {
		if ( !strcmp(my_spi_flash->name, id->name) )
			break;
		id++;
	}

	if( !id->name ) {
		printf("ERROR: Cannot find SPI Flash in ID table");
		spi_flash_free(my_spi_flash);	/* Done with SPI Flash */
		return 1;
	}

#ifdef DEBUG
	printf("JEDIC ID = %02X%02X%02x, ext = %02X%02X",
		id->id[0],id->id[1],id->id[2],id->id[3],id->id[4]);
	if (id->id_len == 6)
		printf("%02X",id->id[5]);
	printf("\n");
#endif

	/* For Quad Mode operation, extra setup is needed in the SPI
	   Flash devices */
	if( id->id[0] == 0x01 )
		ret = enable_quad_spansion(my_spi_flash, quad_addr, quad_data);
	else if( id->id[0] == 0xc2 )
		ret = enable_quad_macronix(my_spi_flash, quad_addr, quad_data);
	else if( id->id[0] == 0x20 )
		ret = enable_quad_micron(my_spi_flash, quad_addr, quad_data);	/* Micron/ST */
	else
	{
		printf("ERROR: SPI Flash support needs to be added to function %s() in file %s\n",__func__, __FILE__);
		spi_flash_free(my_spi_flash);	/* Done with SPI Flash */
		return 1;
	}

	/* Done with SPI Flash */
	spi_flash_free(my_spi_flash);

	if ( ret )
	{
		printf("Failed to set SPI Flash Configuration register.\n");
		return 1;
	}

	/***************************/
	/* Set up RZ SPI Registers */
	/***************************/

	/* Set data pins HIGH when not being used. This helps make sure that
	if you go from dual chip to single chip, only FF will get
	transferred out to the second chip. */
	cmncr &= ~0x00FF0000UL;
	cmncr |=  0x00550000UL;

	/* Enable data swap (SFDE) */
	/* Keeps the endian order of bytes the same on the internal bus
	   regardless of how you fetched them over SPI */
	cmncr |= 0x01000000UL;

	if( dual_chip ) {
		/* Switch to dual memory */
		cmncr |= 0x00000001UL;
	}
	else {
		/* Switch to single memory */
		cmncr &= ~0x00000001UL;
	}

	/* 1-bit address, 4-bit data */
	if( quad_data && !quad_addr ) {
		/* Set read cmd to Quad Read */
		drcmr = (u32)QUAD_READ << 16;

		/* width: 1-bit cmd, 1-bit addr, 4-bit data */
#if (ADDRESS_BYTE_SIZE == 4)
		/* address: 32 bits */
		drenr = 0x00024f00UL;
#else /* ADDRESS_BYTE_SIZE == 3 */
		/* address: 24 bits */
		drenr = 0x00024700UL;
#endif
		/* Add extra Dummy cycles between address and data */
		dmdmcr = 0x00020000 | (g_QUAD_RD_DMY-1); /* 4 bit width, x cycles */
		drenr |= 0x00008000; /* Set Dummy Cycle Enable (DME) */
	}

	/* 1-bit address, 1-bit data */
	if( !quad_data && !quad_addr ) {
		/* Set read cmd to FAST Read */
		drcmr = (u32)FAST_READ << 16;

		/* width: 1-bit cmd, 1-bit addr, 1-bit data */
#if (ADDRESS_BYTE_SIZE == 4)
		/* address: 32 bits */
		drenr = 0x00004f00;
#else /* ADDRESS_BYTE_SIZE == 3 */
		/* address: 24 bits */
		drenr = 0x00004700;
#endif
		/* Add extra Dummy cycles between address and data */
		dmdmcr = 0x00000000 | (g_FAST_RD_DMY-1); /* 1 bit width, x cycles */
		drenr |= 0x00008000; /* Set Dummy Cycle Enable (DME) */
	}

	/* 4-bit address, 4-bit data */
	if( quad_addr ) {
		/* Set read cmd to Quad I/O */
		drcmr = (u32)QUAD_IO_READ << 16;

		/* width: 1-bit cmd, 4-bit addr, 4-bit data */
#if (ADDRESS_BYTE_SIZE == 4)
		/* address: 32 bits */
		drenr = 0x02024f00;
#else /* ADDRESS_BYTE_SIZE == 3 */
		/* address: 24 bits */
		drenr = 0x02024700;
#endif

		/* Use Option data registers to output 0x00 to write the
		   'mode' byte by sending OPD3 (at 4-bit) between address
		   and dummy */
		if ( g_QUAD_IO_RD_OPT ) {
			dropr = 0x00000000;
			drenr |= 0x00200080;	// send OPD3(8-bit) at 4-bit width (2 cycles total)
		}

		/* Add extra Dummy cycles between address and data */
		dmdmcr = 0x00020000 | (g_QUAD_IO_RD_DMY-1); /* 4 bit size, x cycles */

		drenr |= 0x00008000; /* Set Dummy Cycle Enable (DME) */
	}

	if ( ddr ) {
#if defined(CONFIG_RZA1)
		if (strcmp(CONFIG_ARCH_RMOBILE_BOARD_STRING, "RSKRZA1"))
			printf( "WARNING: DDR mode doesn't actually work yet on the RSKRZA1 board.\n"
				"   The Spansion SPI flash has an extra phase in the command stream\n"
				"   that we can't account for.\n");
#endif

		/* Set read cmd to Read DDR Quad I/O */
		drcmr = (u32)QUAD_IO_DDR_READ << 16;

		/* Address, option and data all 4-bit DDR */
		drdrenr = 0x00000111;

		/* According to the Spansion spec (Table 8.5), dummy cycles
		   are needed when LC=00b for READ DDR QUAD I/O commands */
		/* Add extra Dummy cycles between address and data */
		dmdmcr = 0x00020000 | (g_QUAD_IO_DDR_RD_DMY-1); /* 4 bit size, x cycles */
		drenr |= 0x00008000; /* Set Dummy Cycle Enable (DME) */
	}
	else {
		drdrenr = 0;
	}

	/* Set new register values */
	*(volatile u32 *)DMDMCR_0 = dmdmcr;
	*(volatile u32 *)DRENR_0 = drenr;
	*(volatile u32 *)CMNCR_0 = cmncr;
	*(volatile u32 *)DRCMR_0 = drcmr;
	*(volatile u32 *)DROPR_0 = dropr;
	*(volatile u32 *)DRDRENR_0 = drdrenr;

	/* Allow 32MB of SPI addressing (POR default is only 16MB) */
	*(volatile u32 *)DREAR_0 = 0x00000001;

	/* Turn Read Burst on, Burst Length=2 units (also set cache flush) */
	/* Keep SSL low (SSLE=1) in case the next transfer is continuous with
	   our last...saves on address cycle. */
	*(u32 *)DRCR_0 = 0x00010301;
	asm("nop");
	*(volatile u32 *)DRCR_0;	/* Read must be done after cache flush */

#ifdef CONFIG_RZA2
	/* Remove SSL delay to improve performance */
	*(volatile u32 *)SSLDR_0 = 0;

	/* RZ/A2M Timing Adjustments */
	if (ddr == 0) {
		/* PHYOFFSET1:DDRTMG = b'11 */
		*(volatile u32 *)PHYOFFSET1 |= 3 << 28;

		/* For SDR, sequence in Figure 20.28(1) */
		*(volatile u32 *)PHYADJ2 = 0xA5390000;
		*(volatile u32 *)PHYADJ1 = 0x80000000;
		*(volatile u32 *)PHYADJ2 = 0x00008080;
		*(volatile u32 *)PHYADJ1 = 0x80000022;
		*(volatile u32 *)PHYADJ2 = 0x00008080;
		*(volatile u32 *)PHYADJ1 = 0x80000024;
		*(volatile u32 *)PHYCNT |= 3 << 16;	/* PHYCNT.CKSEL[1:0] to “11” */
		*(volatile u32 *)PHYADJ2 = 0x00000000;
		*(volatile u32 *)PHYADJ1 = 0x80000032;
	} else {
		/* PHYCNT:PHYMEM = b'01 */
		*(volatile u32 *)PHYCNT = (*(volatile u32 *)PHYCNT & ~0x3) | 0x1;
		/* PHYOFFSET1:DDRTMG = b'10 */
		*(volatile u32 *)PHYOFFSET1 = (*(volatile u32 *)PHYOFFSET1 & ~0x30000000) | 0x2 << 28;
	}

	/* Do some dummy reads (out of order) to help clean things up */
	*(volatile u32 *)0x20000010;
	*(volatile int *)0x20000000;
#endif

#ifdef CONFIG_RZA1
	/* Do some dummy reads (out of order) to help clean things up */
	*(volatile u32 *)0x18000010;
	*(volatile int *)0x18000000;
#endif

	printf("New Mode: ");
	cmd = (*(volatile long *)DRCMR_0 >> 16) & 0xFF;
	for( i=0; i < READ_MODES; i++) {
		if( modes[i].cmd == cmd )
			printf("%s\n",modes[i].name);
	}

	in_xip_mode = 1;

	return 0;
usage:
	return CMD_RET_USAGE;
}
static char qspi_help_text[] =
	"Set the XIP Mode for QSPI\n"
	"Usage: qspi [single|dual] [a1|(a4)] [d1|(d4)] [(sdr)|ddr]\n"
	"  (xx) means defualt value if not specified\n"
	"  'a4' requries 'd4' to be set\n"
	"  'ddr' requries 'd4' and 'a4' to be set\n";
U_BOOT_CMD(
	qspi,	CONFIG_SYS_MAXARGS,	1,	do_qspi,
	"Change QSPI XIP Mode", qspi_help_text
);
