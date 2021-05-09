#ifndef __SD_ELM_H
#define __SD_ELM_H

#include <stdint.h>

/* Card type flags (CardType) */
#define CT_MMC				0x01	/* MMC ver 3 */
#define CT_SD1				0x02	/* SD ver 1 */
#define CT_SD2				0x04	/* SD ver 2 */
#define CT_SDC				(CT_SD1|CT_SD2)	/* SD */
#define CT_BLOCK			0x08	/* Block addressing */

uint16_t disk_initialize (void);
uint16_t disk_readp (
	uint8_t *buff,		/* Pointer to the read buffer (NULL:Read bytes are forwarded to the stream) */
	uint32_t sector,	/* Sector number (LBA) */
	uint16_t offset,	/* Byte offset to read from (0..511) */
	uint16_t count		/* Number of bytes to read (ofs + cnt mus be <= 512) */
);

uint16_t disk_readp_seg (
	uint16_t dseg,
	uint16_t daddr,
	uint32_t sector	/* Sector number (LBA) */
);

#endif // __SD_ELM_H
