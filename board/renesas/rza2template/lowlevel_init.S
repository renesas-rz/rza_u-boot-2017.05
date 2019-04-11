/*
 * Copyright (C) 2018 Renesas Electronics Corporation
 * Copyright (C) 2018 Chris Brandt
 *
 * This file is released under the terms of GPL v2 and any later version.
 * See the file COPYING in the root directory of the source tree for details.
 */
#include <config.h>
#include <version.h>
#include <asm/arch/r7s9210.h>
#include <asm/macro.h>

.macro  dummy_read32, addr
        ldr     r4, =\addr
	ldr	r5, [r4]
.endm

.macro  dummy_read8, addr
        //ldr     r4, =\addr
        //ldrb    r5, [r4]
	ldrb    r6, [r4]
.endm


	.global	lowlevel_init

	.text
	.align	2

lowlevel_init:

	/* PL310 init */
	write32 (PL310_BASE + REG15_POWER_CTRL), 0x00000001	/* Power Control Register: Standby mode enable */
	dummy_read32 (PL310_BASE + REG15_POWER_CTRL)		/* dummy read */

	/* Set clocks */
	write16	FRQCR, FRQCR_D
	write16 CKIOSEL, CKIOSEL_D

	/* Enable all peripherals(Standby Control) */
	write8 STBCR3, STBCR3_D
	dummy_read8 STBCR3
	write8 STBCR4, STBCR4_D
	dummy_read8 STBCR4
	write8 STBCR5, STBCR5_D
	dummy_read8 STBCR5
	write8 STBCR6, STBCR6_D
	dummy_read8 STBCR6
	write8 STBCR7, STBCR7_D
	dummy_read8 STBCR7
	write8 STBCR8, STBCR8_D
	dummy_read8 STBCR8
	write8 STBCR9, STBCR9_D
	dummy_read8 STBCR9
	write8 STBCR10, STBCR10_D
	dummy_read8 STBCR10
	write8 STBCR11, STBCR11_D
	dummy_read8 STBCR11

	/* Disable WDT */
	write16	WTCSR, WTCSR_D
	write16	WTCNT, WTCNT_D

	/* Disable Register Bank interrupts */
	/* write16 IBNR, IBNR_D */

	/* For serial booting, enable read ahead caching to speed things up */
#define DRCR  0x1F80000C
	write32 DRCR, 0x00010300	/* Read Burst ON, Length=2, Flush cache */

	/* Enable all internal RAM */
	write8 SYSCR1, 0xFF
	write8 SYSCR2, 0xFF
	write8 SYSCR3, 0x0F
	dummy_read8 SYSCR3

	nop
	/* back to arch calling code */
	mov	pc, lr

	.align 4
