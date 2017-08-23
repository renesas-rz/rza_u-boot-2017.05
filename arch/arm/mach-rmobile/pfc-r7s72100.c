/*
 * Copyright (C) 2017 Renesas Electronics
 * Copyright (C) Chris Brandt
 *
 * This file is released under the terms of GPL v2 and any later version.
 * See the file COPYING in the root directory of the source tree for details.
 */


#include <common.h>
#include <asm/arch/r7s72100.h>


/* Port Function Registers */
#define PORTn_base 0xFCFE3000
#define Pn(n)	(PORTn_base + 0x0000 + n * 4)	/* Port register R/W */
#define PSRn(n)	(PORTn_base + 0x0100 + n * 4)	/* Port set/reset register R/W */
#define PPRn(n)	(PORTn_base + 0x0200 + n * 4)	/* Port pin read register R */
#define PMn(n)	(PORTn_base + 0x0300 + n * 4)	/* Port mode register R/W */
#define PMCn(n)	(PORTn_base + 0x0400 + n * 4)	/* Port mode control register R/W */
#define PFCn(n)	(PORTn_base + 0x0500 + n * 4)	/* Port function control register R/W */
#define PFCEn(n)	(PORTn_base + 0x0600 + n * 4)	/* Port function control expansion register R/W */
#define PNOTn(n)	(PORTn_base + 0x0700 + n * 4)	/* Port NOT register W */
#define PMSRn(n)	(PORTn_base + 0x0800 + n * 4)	/* Port mode set/reset register R/W */
#define PMCSRn(n)	(PORTn_base + 0x0900 + n * 4)	/* Port mode control set/reset register R/W */
#define PFCAEn(n)	(PORTn_base + 0x0A00 + n * 4)	/* Port Function Control Additional Expansion register R/W */
#define PIBCn(n)	(PORTn_base + 0x4000 + n * 4)	/* Port input buffer control register R/W */
#define PBDCn(n)	(PORTn_base + 0x4100 + n * 4)	/* Port bi-direction control register R/W */
#define PIPCn(n)	(PORTn_base + 0x4200 + n * 4)	/* Port IP control register R/W */

const u32 alt_settings[9][3] = {
	/* PFCAEn, PFCEn, PFCn */
	{0,0,0},/* dummy */
	{0,0,0},/* 1st alternative function */
	{0,0,1},/* 2nd alternative function */
	{0,1,0},/* 3rd alternative function */
	{0,1,1},/* 4th alternative function */
	{1,0,0},/* 5th alternative function */
	{1,0,1},/* 6th alternative function */
	{1,1,0},/* 7th alternative function */
	{1,1,1},/* 8th alternative function */
};

/* Arguments:
   n = port(1-11)
   b = bit(0-15)
   d = direction('GPIO_IN','GPIO_OUT')
*/
void pfc_set_gpio(u8 n, u8 b, u8 d)
{
	*(u32 *)PMCSRn(n) = 1UL<<(b+16);		// Pin as GPIO
	*(u32 *)PMSRn(n) = 1UL<<(b+16) | (u32)d << b;	// Set pin IN/OUT

	if( d == GPIO_IN )
		*(u16 *)PIBCn(n) |= 1UL << b; 		// Enable input buffer
	else
		*(u16 *)PIBCn(n) &= ~(1UL << b); 	// Disable input buffer
}

/* Arguments:
   n = port(1-11)
   b = bit(0-15)
   v = value (0 or 1)
*/
void gpio_set(u8 n, u8 b, u8 v)
{
	/* The pin should have been configured as GPIO_OUT using pfc_set_gpio */

	/* Use the 'Port Set and Reset Register' to only effect the pin bit */
	/* Upper WORD is the bit mask, the lower WORD is the desire pin level */
	*(u32 *)PSRn(n) = 1UL<<(b+16) | (u32)v<< b;	// Set pin to 0/1
}

/* Arguments:
   n = port(1-11)
   b = bit(0-15)
   return = current pin level (0 or 1);
*/
u8 gpio_read(u8 n, u8 b)
{
	/* The pin should have been configured as GPIO_IN using pfc_set_gpio */
	return ( *(u16 *)PPRn(n) >> b ) & 0x0001;
}


/* Arguments:
    n = port number (P1-P11)
    b = bit number (0-15)
    alt = Alternative mode ('ALT1'-'ALT7')
    inbuf =  Input buffer (0=disabled, 1=enabled)
    bi = Bidirectional mode (0=disabled, 1=enabled)
*/
void pfc_set_pin_function(u16 n, u16 b, u16 alt, u16 inbuf, u16 bi)
{
	u16 value;

	/* Set PFCAEn */
	value = *(u16 *)PFCAEn(n);
	value &= ~(1UL<<b); // clear
	value |= (alt_settings[alt][0] & 1UL) << b; // set(maybe)
	*(u16 *)PFCAEn(n) = value;

	/* Set PFCEn */
	value = *(u16 *)PFCEn(n);
	value &= ~(1UL<<b); // clear
	value |= (alt_settings[alt][1] & 1UL) << b; // set(maybe)
	*(u16 *)PFCEn(n) = value;

	/* Set PFCn */
	value = *(u16 *)PFCn(n);
	value &= ~(1UL<<b); // clear
	value |= (alt_settings[alt][2] & 1UL) << b; // set(maybe)
	*(u16 *)PFCn(n) = value;

	/* Set Pn */
	/* Not used for alternative mode */
	/* NOTE: PIP must be set to '0' for the follow peripherals and Pn must be set instead
		<> Multi-function timer pulse unit
		<> LVDS output interface
		<> Serial sound interface
	   For those, use this to set Pn: *(u32 *)PSRn(n) = 1UL<<(b+16) | direction<<(b);
	*/

	/* Set PIBCn */
	value = *(u16 *)PIBCn(n);
	value &= ~(1UL<<b); // clear
	value |= inbuf << b; // set(maybe)
	*(u16 *)PIBCn(n) = value;

	/* Set PBDCn */
	value = *(u16 *)PBDCn(n);
	value &= ~(1UL<<b); // clear
	value |= bi << b; // set(maybe)
	*(u16 *)PBDCn(n) = value;

	/* Alternative mode '1' (not GPIO '0') */
	*(u32 *)PMCSRn(n) |= 1UL<<(b+16) | 1UL<<(b);

	/* Set PIPCn so pin used for function '1'(not GPIO '0') */
	*(u16 *)PIPCn(n) |= 1UL <<b;
}
