/*------------------------------------------------------------------------/
/  Bitbanging MMCv3/SDv1/SDv2 (in SPI mode) control module for PFF
/-------------------------------------------------------------------------/
/
/  Copyright (C) 2014, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/--------------------------------------------------------------------------/
 Features:

 * Very Easy to Port
   It uses only 4-6 bit of GPIO port. No interrupt, no SPI port is used.

 * Platform Independent
   You need to modify only a few macros to control GPIO ports.

/-------------------------------------------------------------------------*/


#include "diskio.h"


/*-------------------------------------------------------------------------*/
/* Platform dependent macros and functions needed to be modified           */
/*-------------------------------------------------------------------------*/

//#include <hardware.h>			/* Include hardware specific declareation file here */
#include <stdint.h>
#include "80c186eb.h"
#include "io.h"
#include "delay.h"
#include "pcb_bitstorage.h"
#include "pcb_storage.h"
#include "sd_asm.h"

#define	INIT_PORT()	init_spi()	/* Initialize MMC control port (CS/CLK/DI:output, DO:input) */
#define DLY_US(n)	dly_us(n)	/* Delay n microseconds */
#define	FORWARD(d)	//forward(d)	/* Data in-time processing function (depends on the project) */

#define	CS_H()		spi_cs_1()	/* Set MMC CS "high" */
#define CS_L()		spi_cs_0()	/* Set MMC CS "low" */
#define	CK_L()		spi_sck_0()	/* Set MMC SCLK "low" */



/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

/* Definitions for MMC/SDC command */
#define CMD0	(0x40+0)	/* GO_IDLE_STATE */
#define CMD1	(0x40+1)	/* SEND_OP_COND (MMC) */
#define	ACMD41	(0xC0+41)	/* SEND_OP_COND (SDC) */
#define CMD8	(0x40+8)	/* SEND_IF_COND */
#define CMD16	(0x40+16)	/* SET_BLOCKLEN */
#define CMD17	(0x40+17)	/* READ_SINGLE_BLOCK */
#define CMD24	(0x40+24)	/* WRITE_BLOCK */
#define CMD55	(0x40+55)	/* APP_CMD */
#define CMD58	(0x40+58)	/* READ_OCR */

/* Card type flags (CardType) */
#define CT_MMC				0x01	/* MMC ver 3 */
#define CT_SD1				0x02	/* SD ver 1 */
#define CT_SD2				0x04	/* SD ver 2 */
#define CT_SDC				(CT_SD1|CT_SD2)	/* SD */
#define CT_BLOCK			0x08	/* Block addressing */



//static
//uint8_t CardType;			/* b0:MMC, b1:SDv1, b2:SDv2, b3:Block addressing */


#define SPI_LANES(f) \
	f(spi_cs_0, 0xff56, "and $0xdf") \
	f(spi_cs_1, 0xff56, "or $0x20") \
	f(spi_sck_0, 0xff56, "and $0xfd")

#define DECL_STUFF(name, addr, op) \
void name(void) { \
	asm volatile ("mov $" # addr ", %%dx\n" \
				  "in %%dx, %%ax\n" \
				  op ", %%al\n" \
				  "out %%ax, %%dx" ::: "ax", "dx"); \
}

SPI_LANES(DECL_STUFF)

/*-----------------------------------------------------------------------*/
/* Deselect the card and release SPI bus                                 */
/*-----------------------------------------------------------------------*/

static
void release_spi (void)
{
	CS_H();
	rcvr_mmc();
}


/*-----------------------------------------------------------------------*/
/* Send a command packet to MMC                                          */
/*-----------------------------------------------------------------------*/

static
uint8_t send_cmd (
	uint8_t cmd,		/* Command uint8_t */
	uint32_t arg		/* Argument */
)
{
	uint8_t n, res;


	if (cmd & 0x80) {	/* ACMD<n> is the command sequense of CMD55-CMD<n> */
		cmd &= 0x7F;
		res = send_cmd(CMD55, 0);
		if (res > 1) return res;
	}

	/* Select the card */
	CS_H(); rcvr_mmc();
	CS_L(); rcvr_mmc();

	/* Send a command packet */
	xmit_mmc(cmd);					/* Start + Command index */
	xmit_mmc((uint8_t)(arg >> 24));	/* Argument[31..24] */
	xmit_mmc((uint8_t)(arg >> 16));	/* Argument[23..16] */
	xmit_mmc((uint8_t)(arg >> 8));		/* Argument[15..8] */
	xmit_mmc((uint8_t)arg);			/* Argument[7..0] */
	n = 0x01;						/* Dummy CRC + Stop */
	if (cmd == CMD0) n = 0x95;		/* Valid CRC for CMD0(0) */
	if (cmd == CMD8) n = 0x87;		/* Valid CRC for CMD8(0x1AA) */
	xmit_mmc(n);

	/* Receive a command response */
	n = 10;								/* Wait for a valid response in timeout of 10 attempts */
	do {
		res = rcvr_mmc();
	} while ((res & 0x80) && --n);

	return res;			/* Return with the response value */
}


/*-----------------------------------------------------------------------*/
/* Initialize IOs for SPI                                                */
/*-----------------------------------------------------------------------*/

static void init_spi (void)
{
	/**
	 * SD card:
	 * MISO: P2.6
	 * MOSI: P1.0
	 * SCK: P1.1
	 * CS: P1.5
	 */

	/* PC0, PC1 and PC5 bits off, P1.0, P1.1 and P1.5 IO */
	outw(P1CON, inw(P1CON) & 0x00DC);
	/* Turn PC2CON's PC6 bit off, making P2.6 IO */
	outw(P2CON, inw(P2CON) & 0x00BF);

	/* P1.0, P1.1, P1.5 => Output */
	outw(P1DIR, inw(P1DIR) & 0x00DC);
	/* P2.6 => Input */
	outw(P2DIR, inw(P2DIR) | 0x0040);

	CS_H();
	CK_L();
}


/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (void)
{
	uint8_t n, cmd, ty, buf[4];
	uint16_t tmr;

	INIT_PORT();
	CS_H();
	skip_mmc(10);			/* Dummy clocks */

	ty = 0;
	if (send_cmd(CMD0, 0) == 1) {			/* Enter Idle state */
		if (send_cmd(CMD8, 0x1AA) == 1) {	/* SDv2 */
			for (n = 0; n < 4; n++) buf[n] = rcvr_mmc();	/* Get trailing return value of R7 resp */
			if (buf[2] == 0x01 && buf[3] == 0xAA) {			/* The card can work at vdd range of 2.7-3.6V */
				for (tmr = 1000; tmr; tmr--) {				/* Wait for leaving idle state (ACMD41 with HCS bit) */
					if (send_cmd(ACMD41, 1UL << 30) == 0) break;
					DLY_US(1000);
				}
				if (tmr && send_cmd(CMD58, 0) == 0) {		/* Check CCS bit in the OCR */
					for (n = 0; n < 4; n++) buf[n] = rcvr_mmc();
					ty = (buf[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;	/* SDv2 (HC or SC) */
				}
			}
		} else {							/* SDv1 or MMCv3 */
			if (send_cmd(ACMD41, 0) <= 1) 	{
				ty = CT_SD1; cmd = ACMD41;	/* SDv1 */
			} else {
				ty = CT_MMC; cmd = CMD1;	/* MMCv3 */
			}
			for (tmr = 1000; tmr; tmr--) {			/* Wait for leaving idle state */
				if (send_cmd(cmd, 0) == 0) break;
				DLY_US(1000);
			}
			if (!tmr || send_cmd(CMD16, 512) != 0)			/* Set R/W block length to 512 */
				ty = 0;
		}
	}
	if (!(ty & CT_BLOCK))
	{
		BIT_SD_IS_SDV2_SET();
	}
	else
	{
		BIT_SD_IS_SDV2_RESET();
	}
	release_spi();

	return ty ? 0 : STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read partial sector                                                   */
/*-----------------------------------------------------------------------*/

uint16_t disk_readp_seg1 (
	uint16_t dseg,
	uint16_t daddr,
	uint32_t sector,	/* Sector number (LBA) */
	uint16_t offset,
	uint16_t count
)
{
	DRESULT res;
	uint8_t d;
	uint16_t bc, tmr;

	if (BIT_SD_IS_SDV2_TEST()) sector *= 512;	/* Convert to uint8_t address if needed */

	res = RES_ERROR;
	if (send_cmd(CMD17, sector) == 0) {		/* READ_SINGLE_BLOCK */

		tmr = 1000;
		do {							/* Wait for data packet in timeout of 100ms */
			DLY_US(100);
			d = rcvr_mmc();
		} while (d == 0xFF && --tmr);

		if (d == 0xFE) {				/* A data packet arrived */
			bc = 514 - offset - count;

			/* Skip leading BYTEs */
			if (offset) skip_mmc(offset);

			do
			{
				d = rcvr_mmc();
				writeb(dseg, daddr, d);
				daddr++;
				//inc_seg_addr_pair(dseg, daddr);
			} while (--count);

			/* Skip trailing CRC */
			skip_mmc(bc);

			res = RES_OK;
		}
	}

	release_spi();

	return res;
}

uint16_t disk_readp_seg (
	uint16_t dseg,
	uint16_t daddr,
	uint32_t sector	/* Sector number (LBA) */
)
{
	return disk_readp_seg1(dseg, daddr, sector, 0, 512);
}


DRESULT disk_readp (
	uint8_t *buff,		/* Pointer to the read buffer (NULL:Read BYTEs are forwarded to the stream) */
	uint32_t sector,	/* Sector number (LBA) */
	uint16_t offset,	/* uint8_t offset to read from (0..511) */
	uint16_t count		/* Number of BYTEs to read (ofs + cnt mus be <= 512) */
)
{
	return disk_readp_seg1(get_cs(), (uint16_t)buff, sector, offset, count);
}



/*-----------------------------------------------------------------------*/
/* Write partial sector                                                  */
/*-----------------------------------------------------------------------*/
#if _USE_WRITE

uint16_t disk_writep (
	const uint8_t *buff,	/* Pointer to the BYTEs to be written (NULL:Initiate/Finalize sector write) */
	uint32_t sc			/* Number of BYTEs to send, Sector number (LBA) or zero */
)
{
	uint16_t res;
	uint8_t bc, tmr;
	static uint8_t wc;


	res = RES_ERROR;

	if (buff) {		/* Send data BYTEs */
		bc = (uint8_t)sc;
		while (bc && wc) {		/* Send data BYTEs to the card */
			xmit_mmc(*buff++);
			wc--; bc--;
		}
		res = RES_OK;
	} else {
		if (sc) {	/* Initiate sector write transaction */
			if (BIT_SD_IS_SDV2_TEST()) sc *= 512;	/* Convert to uint8_t address if needed */
			if (send_cmd(CMD24, sc) == 0) {			/* WRITE_SINGLE_BLOCK */
				xmit_mmc(0xFF); xmit_mmc(0xFE);		/* Data block header */
				wc = 512;							/* Set uint8_t counter */
				res = RES_OK;
			}
		} else {	/* Finalize sector write transaction */
			bc = wc + 2;
			while (bc--) xmit_mmc(0);	/* Fill left BYTEs and CRC with zeros */
			if ((rcvr_mmc() & 0x1F) == 0x05) {	/* Receive data resp and wait for end of write process in timeout of 300ms */
				for (tmr = 10000; rcvr_mmc() != 0xFF && tmr; tmr--)	/* Wait for ready (max 1000ms) */
					DLY_US(100);
				if (tmr) res = RES_OK;
			}
			release_spi();
		}
	}

	return res;
}
#endif
