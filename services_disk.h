#pragma once

/* Diskette Drive Parameter Table */
struct disk_base_table {
    unsigned char stepunld;		/* Bits 7-4: step rate, bits 3-0: head unload time */
    unsigned char lddma;		/* Bits 7-1: head load time, bit 0: non-DMA mode */
    unsigned char mtroff;		/* Motor off time in clock ticks */
    unsigned char bps;			/* (128 << this) = bytes per sector */
    unsigned char spt;			/* Sectors per track */
    unsigned char gaplen;		/* Gap between sectors */
    unsigned char datalen;		/* Data length, ignored if bytes per sector field nonzero */
    unsigned char fmtgaplen;	/* Gap length when formatting */
    unsigned char fmtfill;		/* Format filler byte */
    unsigned char hdsetl;		/* Head settle time in ms */
    unsigned char mtrstart;		/* Motor start time in 1/8s */
    /* IBM SurePath BIOS */
    unsigned char maxcyl;		/* Maximum cylinder number */
    unsigned char datarate;		/* Data transfer rate */
    unsigned char cmostype;		/* Drive type in CMOS */
};