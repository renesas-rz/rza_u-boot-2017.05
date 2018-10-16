/*
 * RZA2M SPI driver
 *
 */

//#define DEBUG
//#define DEBUG_DETAILED /* Print out all TX and RX data */


#include <common.h>
#include <malloc.h>
#include <spi.h>
#include <spi_flash.h>
#include <asm/io.h>
#include <wait_bit.h>

#if defined(CONFIG_RZA1)
 #define CONFIG_RZA_BASE_QSPI0		0x3FEFA000
#elif defined(CONFIG_RZA2)
 #define CONFIG_RZA_BASE_QSPI0		0x1F800000
#else
#error "Unknown Device"
#endif

struct stRzSpi{
	struct spi_slave	slave;

	volatile void*		pRegBase;
	u32			u32MaxSpeedHz;	// max_speed_hz
	unsigned int		unMode;

	u8			data_read_dual;	// single or dual chips in data read mode
	u8			this_cmd;	// What is the current SPI command.
	u8			cmd_tx_only;	// The SPI command will only TX data (no RX)
	u8			cmd_dual_tx_data;	// If using dual SPI flash, you need to send dual data
};


#define	WAIT_HZ			1000	// TEND time out rate.
#define	CMD_READ_ARRAY_FAST	0x0b	// EXT-READ dummy-cmd.

int qspi_disable_combine = 0;	// Don't combine responses from dual SPI flash
int qspi_combine_status_mode = 0; // Dual mode only, 0='OR results', 1='AND results'


/* NOTE: When "sf probe" is called, we might have to do some additional
   setup on the SPI Flash chip to get it out of QUAD mode. */
extern int qspi_reset_device(struct spi_flash *flash);


/* QSPI MODE */
#define READ_MODE		(0)
#define SPI_MODE		(1)

/* QSPI registers */
#define	QSPI_CMNCR		(0x0000)
#define	QSPI_SSLDR		(0x0004)
#define	QSPI_SPBCR		(0x0008)
#define	QSPI_DRCR		(0x000c)
#define	QSPI_DRCMR		(0x0010)
#define	QSPI_DREAR		(0x0014)
#define	QSPI_DROPR		(0x0018)
#define	QSPI_DRENR		(0x001c)
#define	QSPI_SMCR		(0x0020)
#define	QSPI_SMCMR		(0x0024)
#define	QSPI_SMADR		(0x0028)
#define	QSPI_SMOPR		(0x002c)
#define	QSPI_SMENR		(0x0030)

#define	QSPI_SMRDR0		(0x0038)
#define	QSPI_SMRDR1		(0x003c)
#define	QSPI_SMWDR0		(0x0040)
#define	QSPI_SMWDR1		(0x0044)
#define	QSPI_CMNSR		(0x0048)

#define	QSPI_DRDMCR		(0x0058)
#define	QSPI_DRDRENR		(0x005c)
#define	QSPI_SMDMCR		(0x0060)
#define	QSPI_SMDRENR		(0x0064)

#define	QSPI_PHYOFFSET1		(0x0080)
#define	QSPI_PHYOFFSET2		(0x0084)
#define	QSPI_PHYINT		(0x0088)


/* CMNCR */
#define	CMNCR_MD		(1u << 31)
#define	CMNCR_SFDE		(1u << 24)

#define	CMNCR_MOIIO3(x)		(((u32)(x) & 0x3) << 22)
#define	CMNCR_MOIIO2(x)		(((u32)(x) & 0x3) << 20)
#define	CMNCR_MOIIO1(x)		(((u32)(x) & 0x3) << 18)
#define	CMNCR_MOIIO0(x)		(((u32)(x) & 0x3) << 16)
#define	CMNCR_IO3FV(x)		(((u32)(x) & 0x3) << 14)
#define	CMNCR_IO2FV(x)		(((u32)(x) & 0x3) << 12)
#define	CMNCR_IO0FV(x)		(((u32)(x) & 0x3) << 8)

#define	CMNCR_CPHAT		(1u << 6)
#define	CMNCR_CPHAR		(1u << 5)
#define	CMNCR_SSLP		(1u << 4)
#define	CMNCR_CPOL		(1u << 3)
#define	CMNCR_BSZ(n)		(((u32)(n) & 0x3) << 0)

#define	OUT_0			(0u)
#define	OUT_1			(1u)
#define	OUT_REV			(2u)
#define	OUT_HIZ			(3u)

#define	BSZ_SINGLE		(0)
#define	BSZ_DUAL		(1)

/* SSLDR */
#define	SSLDR_SPNDL(x)		(((u32)(x) & 0x7) << 16)
#define	SSLDR_SLNDL(x)		(((u32)(x) & 0x7) << 8)
#define	SSLDR_SCKDL(x)		(((u32)(x) & 0x7) << 0)

#define	SPBCLK_1_0		(0)
#define	SPBCLK_1_5		(0)
#define	SPBCLK_2_0		(1)
#define	SPBCLK_2_5		(1)
#define	SPBCLK_3_0		(2)
#define	SPBCLK_3_5		(2)
#define	SPBCLK_4_0		(3)
#define	SPBCLK_4_5		(3)
#define	SPBCLK_5_0		(4)
#define	SPBCLK_5_5		(4)
#define	SPBCLK_6_0		(5)
#define	SPBCLK_6_5		(5)
#define	SPBCLK_7_0		(6)
#define	SPBCLK_7_5		(6)
#define	SPBCLK_8_0		(7)
#define	SPBCLK_8_5		(7)

/* SPBCR */
#define	SPBCR_SPBR(x)		(((u32)(x) & 0xff) << 8)
#define	SPBCR_BRDV(x)		(((u32)(x) & 0x3) << 0)

/* DRCR (read mode) */
#define	DRCR_SSLN		(1u << 24)
#define	DRCR_RBURST(x)		(((u32)(x) & 0xf) << 16)
#define	DRCR_RCF		(1u << 9)
#define	DRCR_RBE		(1u << 8)
#define	DRCR_SSLE		(1u << 0)

/* DRCMR (read mode) */
#define	DRCMR_CMD(c)		(((u32)(c) & 0xff) << 16)
#define	DRCMR_OCMD(c)		(((u32)(c) & 0xff) << 0)

/* DREAR (read mode) */
#define	DREAR_EAV(v)		(((u32)(v) & 0xff) << 16)
#define	DREAR_EAC(v)		(((u32)(v) & 0x7) << 0)

/* DROPR (read mode) */
#define	DROPR_OPD3(o)		(((u32)(o) & 0xff) << 24)
#define	DROPR_OPD2(o)		(((u32)(o) & 0xff) << 16)
#define	DROPR_OPD1(o)		(((u32)(o) & 0xff) << 8)
#define	DROPR_OPD0(o)		(((u32)(o) & 0xff) << 0)

/* DRENR (read mode) */
#define	DRENR_CDB(b)		(((u32)(b) & 0x3) << 30)
#define	DRENR_OCDB(b)		(((u32)(b) & 0x3) << 28)
#define	DRENR_ADB(b)		(((u32)(b) & 0x3) << 24)
#define	DRENR_OPDB(b)		(((u32)(b) & 0x3) << 20)
#define	DRENR_DRDB(b)		(((u32)(b) & 0x3) << 16)
#define	DRENR_DME		(1u << 15)
#define	DRENR_CDE		(1u << 14)
#define	DRENR_OCDE		(1u << 12)
#define	DRENR_ADE(a)		(((u32)(a) & 0xf) << 8)
#define	DRENR_OPDE(o)		(((u32)(o) & 0xf) << 4)

/* SMCR (spi mode) */
#define	SMCR_SSLKP		(1u << 8)
#define	SMCR_SPIRE		(1u << 2)
#define	SMCR_SPIWE		(1u << 1)
#define	SMCR_SPIE		(1u << 0)

/* SMCMR (spi mode) */
#define	SMCMR_CMD(c)		(((u32)(c) & 0xff) << 16)
#define	SMCMR_OCMD(o)		(((u32)(o) & 0xff) << 0)

/* SMADR (spi mode) */

/* SMOPR (spi mode) */
#define	SMOPR_OPD3(o)		(((u32)(o) & 0xff) << 24)
#define	SMOPR_OPD2(o)		(((u32)(o) & 0xff) << 16)
#define	SMOPR_OPD1(o)		(((u32)(o) & 0xff) << 8)
#define	SMOPR_OPD0(o)		(((u32)(o) & 0xff) << 0)

/* SMENR (spi mode) */
#define	SMENR_CDB(b)		(((u32)(b) & 0x3) << 30)
#define	SMENR_OCDB(b)		(((u32)(b) & 0x3) << 28)
#define	SMENR_ADB(b)		(((u32)(b) & 0x3) << 24)
#define	SMENR_OPDB(b)		(((u32)(b) & 0x3) << 20)
#define	SMENR_SPIDB(b)		(((u32)(b) & 0x3) << 16)
#define	SMENR_DME		(1u << 15)
#define	SMENR_CDE		(1u << 14)
#define	SMENR_OCDE		(1u << 12)
#define	SMENR_ADE(b)		(((u32)(b) & 0xf) << 8)
#define	SMENR_OPDE(b)		(((u32)(b) & 0xf) << 4)
#define	SMENR_SPIDE(b)		(((u32)(b) & 0xf) << 0)

#define	ADE_23_16		(0x4)
#define	ADE_23_8		(0x6)
#define	ADE_23_0		(0x7)
#define	ADE_31_0		(0xf)

#define	BITW_1BIT		(0)
#define	BITW_2BIT		(1)
#define	BITW_4BIT		(2)

#define	SPIDE_16BITS_DUAL	(0x8)
#define	SPIDE_32BITS_DUAL	(0xc)
#define	SPIDE_64BITS_DUAL	(0xf)
#define	SPIDE_8BITS	(0x8)
#define	SPIDE_16BITS	(0xc)
#define	SPIDE_32BITS	(0xf)

#define	OPDE_3			(0x8)
#define	OPDE_3_2		(0xc)
#define	OPDE_3_2_1		(0xe)
#define	OPDE_3_2_1_0		(0xf)

/* SMRDR0 (spi mode) */
/* SMRDR1 (spi mode) */
/* SMWDR0 (spi mode) */
/* SMWDR1 (spi mode) */

/* CMNSR (spi mode) */
#define	CMNSR_SSLF		(1u << 1)
#define	CMNSR_TEND		(1u << 0)

/* DRDMCR (read mode) */
#define	DRDMCR_DMDB(b)		(((u32)(b) & 0x3) << 16)
#define	DRDMCR_DMCYC(b)		(((u32)(b) & 0x7) << 0)

/* DRDRENR (read mode) */
#define	DRDRENR_ADDRE		(1u << 8)
#define	DRDRENR_OPDRE		(1u << 4)
#define	DRDRENR_DRDRE		(1u << 0)

/* SMDMCR (spi mode) */
#define	SMDMCR_DMDB(b)		(((u32)(b) & 0x3) << 16)
#define	SMDMCR_DMCYC(b)		(((u32)(b) & 0x7) << 0)

/* SMDRENR (spi mode) */
#define	SMDRENR_ADDRE		(1u << 8)
#define	SMDRENR_OPDRE		(1u << 4)
#define	SMDRENR_SPIDRE		(1u << 0)

#define	QSPI_BASE_CLK		(133333333)

/*
 *  FlashROM Chip Commands
 */

#define	CMD_READ_ID		(0x9f)	/* (REMS) Read Electronic Manufacturer Signature */
#define	CMD_PP			(0x02)	/* Page Program (3-byte address) */
#define	CMD_QPP			(0x32)	/* Quad Page Program (3-byte address) */
#define	CMD_READ		(0x03)	/* Read (3-byte address) */
#define	CMD_FAST_READ		(0x0b)	/* Fast Read (3-byte address) */
#define	CMD_DOR			(0x3b)	/* Read Dual Out (3-byte address) */
#define	CMD_QOR			(0x6b)	/* Read Quad Out (3-byte address) */
#define	CMD_SE			(0xd8)	/* Sector Erase */

#define	CMD_4READ		(0x13)	/* Read (4-byte address) */
#define	CMD_4FAST_READ		(0x0c)	/* Fast Read (4-byte address) */
#define	CMD_4PP			(0x12)	/* Page Program (4-byte address) */
#define	CMD_4SE			(0xdc)	/* Sector Erase */

static inline struct stRzSpi* to_rz_spi(struct spi_slave* sl)
{
	return container_of(sl, struct stRzSpi, slave);
}

static inline u32 qspi_read32(struct stRzSpi* stRzSpi, int nOff)
{
	return readl((u32)stRzSpi->pRegBase + nOff);
}

static inline u8 qspi_read8(struct stRzSpi* stRzSpi, int nOff)
{
	return readb((u32)stRzSpi->pRegBase + nOff);
}

static inline u16 qspi_read16(struct stRzSpi* stRzSpi, int nOff)
{
	return readw((u32)stRzSpi->pRegBase + nOff);
}

static inline void qspi_write32(struct stRzSpi* stRzSpi, u32 u32Value, int nOff)
{
	writel(u32Value, (u32)stRzSpi->pRegBase + nOff);
}

static inline void qspi_write8(struct stRzSpi* stRzSpi, u8 u8Value, int nOff)
{
	writeb(u8Value, (u32)stRzSpi->pRegBase + nOff);
}

static inline void qspi_write16(struct stRzSpi* stRzSpi, u16 u16Value, int nOff)
{
	writew(u16Value, (u32)stRzSpi->pRegBase + nOff);
}


static int qspi_wait_for_tend(struct stRzSpi* pstRzSpi)
{
	unsigned long timebase;

	timebase = get_timer(0);
	do{
		if(qspi_read32(pstRzSpi, QSPI_CMNSR) & CMNSR_TEND){
			break;
		}
	}while(get_timer(timebase) < WAIT_HZ);

	if (!(qspi_read32(pstRzSpi, QSPI_CMNSR) & CMNSR_TEND)){
		printf("%s: wait timeout\n", __func__);
		return -1;
	}

	return 0;
}



/**
 * qspi_disable_auto_combine
 *
 * This function is useful only when you are in dual SPI flash mode
 * and you want to send a command but not have the results from
 * the 2 devices OR-ed together (becase you need to check each SPI
 * Flash individually).
 *
 * Just remember that you need to send a buffer size big enough to handle
 * the results from both SPI flashes.
 *
 * This only effects the very next command sent, after that it will
 * automatically reset.
 *
 * NOTE: You will need to add a prototype of this function in your
 * code to use it (it's not in any header file).
 */
void qspi_disable_auto_combine(void)
{
	qspi_disable_combine = 1;
}

static inline int qspi_is_ssl_negated(struct stRzSpi* pstRzSpi)
{
	return !(qspi_read32(pstRzSpi, QSPI_CMNSR) & CMNSR_SSLF);
}

static int qspi_set_config_register(struct stRzSpi* pstRzSpi)
{
	u32 value;

	debug("call %s:\n", __func__);

	/* Check if SSL still low */
	if (!qspi_is_ssl_negated(pstRzSpi)){
		/* Clear the SSL from the last data read operation */
		qspi_write32(pstRzSpi, qspi_read32(pstRzSpi,QSPI_DRCR) | DRCR_SSLN,
			QSPI_DRCR);

		/* Disable Read & Write */
		qspi_write32(pstRzSpi, 0, QSPI_SMCR);
	}

	/* NOTES: Set swap (SFDE) so the order of bytes D0 to D7 in the SPI RX/TX FIFO are always in the
	   same order (LSB=D0, MSB=D7) regardless if the SPI did a byte,word, dwrod fetch */
	value =
		CMNCR_MD|	       		/* spi mode */
		CMNCR_SFDE|			/* swap */
		CMNCR_MOIIO3(OUT_HIZ)|
		CMNCR_MOIIO2(OUT_HIZ)|
		CMNCR_MOIIO1(OUT_HIZ)|
		CMNCR_MOIIO0(OUT_HIZ)|
		CMNCR_IO3FV(OUT_HIZ)|
		CMNCR_IO2FV(OUT_HIZ)|
		CMNCR_IO0FV(OUT_HIZ)|
		CMNCR_BSZ(BSZ_SINGLE);

	/* dual memory? */
	if (pstRzSpi->slave.cs)
		value |= CMNCR_BSZ(BSZ_DUAL);	/* s-flash x 2 */

	/* set common */
	qspi_write32(pstRzSpi, value, QSPI_CMNCR);

	/* flush read-cache */
	qspi_write32(pstRzSpi, qspi_read32(pstRzSpi, QSPI_DRCR) | DRCR_RCF,
		QSPI_DRCR);

	/* setup delay */

	qspi_write32(pstRzSpi,
		SSLDR_SPNDL(7)|	/* next access delay */
		SSLDR_SLNDL(7)|	/* SPBSSL negate delay */
		SSLDR_SCKDL(7),	/* clock delay */
		QSPI_SSLDR);

	//qspi_write32(pstRzSpi, 0x21511144, QSPI_PHYOFFSET1);
	//qspi_write32(pstRzSpi, 0x00000431, QSPI_PHYOFFSET2);
	//qspi_write32(pstRzSpi, 0x07070002, QSPI_PHYINT);

	return 0;
}


static int qspi_set_ope_mode(struct stRzSpi* pstRzSpi, int mode)
{
	int ret;
	u32 cmncr = qspi_read32(pstRzSpi, QSPI_CMNCR);
	u32 drcr;

	debug("call %s: mode=%d(%s)\n", __func__, mode, mode==SPI_MODE ? "SPI": "XIP");

	if((mode == SPI_MODE) && (cmncr & CMNCR_MD)){
		debug("Already in correct mode\n");
		return 0;
	}
	if((mode != SPI_MODE) && !(cmncr & CMNCR_MD)){
		debug("Already in correct mode\n");
		return 0;
	}

	if(!qspi_is_ssl_negated(pstRzSpi)){
		/* SSL still low from last transfer */
		/* Disable Read & Write */
		qspi_write32(pstRzSpi, 0, QSPI_SMCR);
		/* Clear the SSL from the last data read operation */
		qspi_write32(pstRzSpi, qspi_read32(pstRzSpi,QSPI_DRCR) | DRCR_SSLN,
			QSPI_DRCR);
	}

	ret = qspi_wait_for_tend(pstRzSpi);
	if(ret){
		printf("%s: hw busy\n", __func__);
		return ret;
	}

	if(mode == SPI_MODE){
		// SPI mode.
		cmncr |= CMNCR_MD;

		/* cs=0 is single chip, cs=1 is dual chip */
		if( pstRzSpi->slave.cs )
			cmncr |= BSZ_DUAL;
		else
			cmncr &= ~BSZ_DUAL;

		qspi_write32(pstRzSpi, cmncr, QSPI_CMNCR);

	}else{

		/* End the transfer by putting SSL low */
		qspi_write32(pstRzSpi, DRCR_SSLE, QSPI_DRCR);

		// EXT-READ mode.
		cmncr &= ~CMNCR_MD;

		// put back in same mode (1 or 2 chips) as was originally
		if( pstRzSpi->data_read_dual )
			cmncr |= BSZ_DUAL;
		else
			cmncr &= ~BSZ_DUAL;

		qspi_write32(pstRzSpi, cmncr, QSPI_CMNCR);

		// Flush cache
		drcr = qspi_read32(pstRzSpi, QSPI_DRCR);
		drcr |= DRCR_RCF;
		qspi_write32(pstRzSpi, drcr, QSPI_DRCR);
	}

	return 0;
}


const u8 SPIDE_for_single[5] = {0x0,
		0x8,	// 8-bit transfer (1 bytes)
		0xC,	// 16-bit transfer (2 bytes)
		0x0,	// 24-bit transfers are invalid!
		0xF};	// 32-bit transfer (3-4 bytes)
const u8 SPIDE_for_dual[9] = {0,
		0x0,	// 8-bit transfers are invalid!
		0x8,	// 16-bit transfer (1 byte)
		0x0,	// 24-bit transfers are invalid!
		0xC,	// 32-bit transfer (4 bytes)
		0x0,	// 40-bit transfers are invalid!
		0x0,	// 48-bit transfers are invalid!
		0x0,	// 56-bit transfers are invalid!
		0xF};	// 64-bit transfer (8 bytes)


static int qspi_send_data(struct stRzSpi *pstRzSpi,
	const u8* pcnu8DataBuff, unsigned int unDataLength, unsigned int keep_cs_low)
{
	int ret;
	u32 smcr;
	u32 smenr = 0;
	u32 smwdr0;
	u32 smwdr1 = 0;
	int unit;
	int sslkp = 1;

	debug("call %s:\n", __func__);

	/* wait spi transfered */
	if ((ret = qspi_wait_for_tend(pstRzSpi)) < 0) {
		printf("%s: prev xmit timeout\n", __func__);
		return ret;
	}

	while (unDataLength > 0) {

		if( pstRzSpi->slave.cs ) {
			/* Dual memory */
			if (unDataLength >= 8)
				unit = 8;
			else
				unit = unDataLength;

			if( unit & 1 ) {
				printf("ERROR: Can't send odd number of bytes in dual memory mode\n");
				return -1;
			}

			if( unit == 6 )
				unit = 4; /* 6 byte transfers not supported */

			smenr &= ~0xF;	/* clear SPIDE bits */
			smenr |= SPIDE_for_dual[unit];
		}
		else {
			/* Single memory */
			if (unDataLength >= 4)
				unit = 4;
			else
				unit = unDataLength;
			if(unit == 3)
				unit = 2;	/* 3 byte transfers not supported */

			smenr &= ~0xF;	/* clear SPIDE bits */
			smenr |= SPIDE_for_single[unit];
		}

		if( !pstRzSpi->slave.cs ) {
			/* Single memory */

			/* set data */
			smwdr0 = (u32)*pcnu8DataBuff++;
			if (unit >= 2)
			smwdr0 |= (u32)*pcnu8DataBuff++ << 8;
			if (unit >= 3)
			smwdr0 |= (u32)*pcnu8DataBuff++ << 16;
			if (unit >= 4)
			smwdr0 |= (u32)*pcnu8DataBuff++ << 24;
		}
		else
		{
			/* Dual memory */
			if( unit == 8 ) {
				/* Note that SMWDR1 gets sent out first
				   when sending 8 bytes */
				smwdr1 = (u32)*pcnu8DataBuff++;
				smwdr1 |= (u32)*pcnu8DataBuff++ << 8;
				smwdr1 |= (u32)*pcnu8DataBuff++ << 16;
				smwdr1 |= (u32)*pcnu8DataBuff++ << 24;
			}
			/* sending 2 bytes */
			smwdr0 = (u32)*pcnu8DataBuff++;
			smwdr0 |= (u32)*pcnu8DataBuff++ << 8;

			/* sending 4 bytes */
			if( unit >= 4) {
				smwdr0 |= (u32)*pcnu8DataBuff++ << 16;
				smwdr0 |= (u32)*pcnu8DataBuff++ << 24;
			}
		}

		/* Write data to send */
		if (unit == 2){
			qspi_write16(pstRzSpi, (u16)smwdr0, QSPI_SMWDR0);
		}
		else if (unit == 1){
			qspi_write8(pstRzSpi, (u8)smwdr0, QSPI_SMWDR0);
		}
		else {
			qspi_write32(pstRzSpi, smwdr0, QSPI_SMWDR0);
		}

		if( unit == 8 ) {
			/* Dual memory only */
			qspi_write32(pstRzSpi, smwdr1, QSPI_SMWDR1);
		}

		unDataLength -= unit;
		if (unDataLength <= 0) {
			sslkp = 0;
		}

		/* set params */
		qspi_write32(pstRzSpi, 0, QSPI_SMCMR);
		qspi_write32(pstRzSpi, 0, QSPI_SMADR);
		qspi_write32(pstRzSpi, 0, QSPI_SMOPR);
		qspi_write32(pstRzSpi, smenr, QSPI_SMENR);

		/* start spi transfer */
//		smcr = SMCR_SPIE|SMCR_SPIWE;
		smcr = SMCR_SPIE|SMCR_SPIWE|SMCR_SPIRE;
		if (sslkp || keep_cs_low)
			smcr |= SMCR_SSLKP;
		qspi_write32(pstRzSpi, smcr, QSPI_SMCR);

		/* wait spi transfered */
		if ((ret = qspi_wait_for_tend(pstRzSpi)) < 0) {
			printf("%s: data send timeout\n", __func__);
			return ret;
		}

	}
	return 0;
}

/* This function is called when "sf probe" is issued, meaning that
   the user wants to access the deivcce in normal single wire SPI mode.
   Since some SPI devices have to have special setups to enable QSPI mode
   or DDR QSPI mode, this function is used to reset those settings
   back to normal.

   This is a 'weak' function because it is intended that each board
   implements its own function to overide this one. */
struct spi_flash;
__attribute__((weak))
int qspi_reset_device(struct spi_flash *flash)
{
	printf("Warning: You should implement your own qspi_reset_device function\n");
	return 0;
}

/**
 * Required function for u-boot SPI subsystem
 */
struct spi_slave* spi_setup_slave(unsigned int unBus, unsigned int unCs,
	unsigned int unMaxHz, unsigned int unMode)
{
	struct stRzSpi* pstRzSpi;

	debug("call %s: bus(%d) cs(%0d) maxhz(%d) mode(%d))\n",
		__func__, unBus, unCs, unMaxHz, unMode);

	pstRzSpi = spi_alloc_slave(struct stRzSpi, unBus, unCs);
	if(pstRzSpi == NULL){
		printf("%s: malloc error.\n", __func__);
		return NULL;
	}

	pstRzSpi->slave.bus		= unBus;
	pstRzSpi->slave.cs		= unCs;
	pstRzSpi->pRegBase		= (void*)CONFIG_RZA_BASE_QSPI0;
	pstRzSpi->u32MaxSpeedHz		= unMaxHz;
	pstRzSpi->unMode		= unMode;

	/* Save if we were using 1 or 2 chips in data read mode
	   (so we can put it back when we're done) */
	if( qspi_read32(pstRzSpi, QSPI_CMNCR) & BSZ_DUAL )
		pstRzSpi->data_read_dual = 1;
	else
		pstRzSpi->data_read_dual = 0;

	if( pstRzSpi->slave.cs )
		printf("SF: Dual SPI mode\n");

	qspi_set_config_register(pstRzSpi);

	return &pstRzSpi->slave;
}

/**
 * Required function for u-boot SPI subsystem
 */
void spi_free_slave(struct spi_slave* pstSpiSlave)
{
	debug("call %s: bus(%d) cs(%d)\n",
	__func__, pstSpiSlave->bus, pstSpiSlave->cs);

	free(to_rz_spi(pstSpiSlave));
}

/**
 * Required function for u-boot SPI subsystem
 */
int spi_claim_bus(struct spi_slave* pstSpiSlave)
{
	debug("call %s: bus(%d) cs(%d)\n",
		__func__, pstSpiSlave->bus, pstSpiSlave->cs);

#if 0  /* TODO: fix, or add to qspi_reset_device() */
	/* Force to Bank 0 (to keep dual flashes in sync) */
	ret = spi_flash_write_common(flash, &flash->bank_write_cmd, 1,
				    &curr_bank, 1);
#endif

	return 0;
}

/**
 * Required function for u-boot SPI subsystem
 */
void spi_release_bus(struct spi_slave* pstSpiSlave)
{
	debug("call %s: bus(%d) cs(%d)\n",
		__func__, pstSpiSlave->bus, pstSpiSlave->cs);
}

/**
 * Required function for u-boot SPI subsystem
 */
int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	debug("call %s: bus(%d) cs(%d)\n", __func__, bus, cs);

	/* Use cs=1 to mean we want to use 2 spi flashes */
	if (!bus && (cs ==0 || cs==1) )
		return 1;
	else
		return 0;
}

/**
 * Required function for u-boot SPI subsystem
 */
void spi_cs_activate(struct spi_slave* pstSpiSlave)
{
	debug("call %s: bus(%d) cs(%d)\n",
		__func__, pstSpiSlave->bus, pstSpiSlave->cs);
}

/**
 * Required function for u-boot SPI subsystem
 */
void spi_cs_deactivate(struct spi_slave* pstSpiSlave)
{
	debug("call %s: bus(%d) cs(%d)\n",
		__func__, pstSpiSlave->bus, pstSpiSlave->cs);
}


/**
 * Required function for u-boot SPI subsystem
 */
int spi_xfer(struct spi_slave* pstSpiSlave, unsigned int bitlen,
	const void* dout, void* din, unsigned long flags)
{
	struct stRzSpi *pstRzSpi	= to_rz_spi(pstSpiSlave);
	unsigned char* pbTxData		= (unsigned char*)dout;
	unsigned int len		= (bitlen + 7) / 8;
	int ret				= 0;
	int keep_cs_low			= flags & SPI_XFER_END ? 0 : 1;
	unsigned char dual_cmd[12];

	int i, j;

	u32 smenr, offset = 0;
	u32 wlen = dout ? (bitlen / 8) : 0;
	u32 rlen = din ? (bitlen / 8) : 0;
	static u8 cmdcopy[20];
	static u8 cmdlen = 0;
	static u8 cmd_sent = false;

	debug("call %s: bus(%d) cs(%d) bitlen(%d) dout=(0x%08x) din=(0x%08x), flag=(%d)\n",
		__func__, pstSpiSlave->bus, pstSpiSlave->cs,
		bitlen, (u32)dout, (u32)din, (u32)flags);

	if (qspi_wait_for_tend(pstRzSpi) < 0) {
		printf("%s: error waiting for end of last transfer\n", __func__);
		return -1;
	}

	/* If this is the beginning of a command, just save the command
	 * and do nothing else */
	if (flags & SPI_XFER_BEGIN) {

		debug("  SPI command = 0x%02X\n", *pbTxData);

		if ((ret = qspi_wait_for_tend(pstRzSpi)) < 0) {
			printf("%s: error wait for poll (%d)\n", __func__, ret);
			return -1;
		}

		if (wlen == 0) {
			printf("%s: Can't start a new command without any TX data \n", __func__);
			return -1;

		}

		/* reset */
		pstRzSpi->cmd_dual_tx_data = 0;
		pstRzSpi->this_cmd = 0;
		pstRzSpi->cmd_tx_only = 1;

		pstRzSpi->this_cmd = *pbTxData;

		/* Will this command require to receive data? */
		switch (pstRzSpi->this_cmd) {
				case 0x06:	/* write enable */
				case 0xD8:	/* block erase */
				case 0x02:	/* page program */
					break;

				case 0x17: /* Write Bank register (CMD_BANKADDR_BRWR) */
				case 0xC5: /* Write Bank register (CMD_EXTNADDR_WREAR) */
				case 0x01: /* Write Status and configuration */
				case 0xB1: /* Write NonVolatile Configuration register (Micron) */
				case 0x81: /* Write Volatile Configuration register (Micron) */

					if (pstRzSpi->slave.cs) {
						/* Dual SPI Flash */
						/* Duplicate the command to both SPI flash */
						pstRzSpi->cmd_dual_tx_data = 1;

						for(i=0,j=0;i<len;i++) {
							dual_cmd[j++] = pbTxData[i];
							dual_cmd[j++] = pbTxData[i];
						}
						len *= 2;
						pbTxData = dual_cmd;
					}

					pstRzSpi->cmd_tx_only = 1;
					break;

				case 0x9F:	/* Read ID */
				case 0x05:	/* Read Status Register */
				case 0xC8:	/* Read Extended Address Register */
				case 0x15:	/* Read Configuration Register */
				case 0x03:	/* Read */
				case 0x0B:	/* Fast Read */
					pstRzSpi->cmd_tx_only = 0;
					break;

				default:
					printf("%s: Error, unknown command 0x%02X\n",__func__,pstRzSpi->this_cmd);
					return -1;
		}


		/* If this is a dual SPI Flash, we need to send the same
		   command to both chips. */
		/* TODO*/
		if (pstRzSpi->slave.cs) {
			printf("%s: Dual QSPI not supported\n", __func__);
			return -1;
		}


		if (pstRzSpi->cmd_tx_only) {
			ret = qspi_set_ope_mode(pstRzSpi, SPI_MODE);
			if(ret){
				printf("%s: Unknown SPI mode\n", __func__);
				return -1;
			}
			if ((ret = qspi_wait_for_tend(pstRzSpi)) < 0) {
				printf("%s: error wait for poll (%d)\n", __func__, ret);
				return -1;
			}
		}
		else {
			/* copy the command for later */
			memcpy(cmdcopy, dout, wlen);
			cmdlen = wlen;
			cmd_sent = false;
		}
	}


	if (pstRzSpi->cmd_tx_only) {
		ret = qspi_send_data(pstRzSpi, pbTxData, len, keep_cs_low);

#ifdef DEBUG_DETAILED
		if (ret < 0) {
			printf("%s: Error Send Command (%x)\n", __func__, ret);
		} else {
			int nIndex;
			debug("send cmd : ");
			for (nIndex = 0; nIndex < len; nIndex++) {
				debug(" %02x", *(pbTxData + nIndex));
			}
			if(flags & SPI_XFER_END) debug(" <END>");
		}			debug("\n");
#endif

	}
	else {
		/* If we are not going to read any data, then we are done */
		if (rlen == 0)
			return 0;

		if( !cmd_sent ) {
			/* Send the command using XIP mode */

			/* Flush read cache */
			qspi_write32(pstRzSpi,
				qspi_read32(pstRzSpi, QSPI_DRCR) | DRCR_RCF,
				QSPI_DRCR);

			//qspi_write32(pstRzSpi, cmncr & ~CMNCR_MD, QSPI_CMNCR);
			qspi_write32(pstRzSpi, 0x01FFF300, QSPI_CMNCR);
			smenr = 0;

			if (cmdlen >= 1) {	/* Command(1) */
				qspi_write32(pstRzSpi, SMCMR_CMD(cmdcopy[0]), QSPI_DRCMR);	// set command
				smenr |= DRENR_CDE;
			}
			if (cmdlen >= 4)		/* Address(3) */
				smenr |= DRENR_ADE(7);

			offset = (cmdcopy[1] << 16) | (cmdcopy[2] << 8) | (cmdcopy[3] << 0);

			if (cmdlen >= 5) {	/* Dummy(n) */
				writel(8 * (cmdlen - 4) - 1,  pstRzSpi->pRegBase + QSPI_DRDMCR);
				smenr |= DRENR_DME;
			}
			qspi_write32(pstRzSpi, 0, QSPI_DROPR);

			qspi_write32(pstRzSpi, smenr, QSPI_DRENR);

			cmd_sent = 1;
		}

		if (cmdlen == 1)
			offset = 0;

		memcpy_fromio(din, 0x20000000 + offset, rlen);

#ifdef	DEBUG_DETAILED
		{
			int nIndex;
			debug("recv : ");
			for(nIndex = 0; nIndex < len; nIndex++){
				debug(" %02x", *((u8 *)din + nIndex));
				if(nIndex > 100) {
					printf("\n\tStopped after displaying 100 bytes\n");
					break;
				}
			}
			if(flags & SPI_XFER_END) debug(" <END>");
			debug("\n");
		}
#endif

	}

	if(flags & SPI_XFER_END){
		/* Make sure CS goes back low (it might have been left high
		   from the last transfer). It's tricky because basically,
		   you have to disable RD and WR, then start a dummy transfer. */
		qspi_write32(pstRzSpi, 1 , QSPI_SMCR);
		if ((ret = qspi_wait_for_tend(pstRzSpi)) < 0) {
			printf("%s: error wait for poll (%d)\n", __func__, ret);
		}
		return 0;
	}

	return ret;
}



