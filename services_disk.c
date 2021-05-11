#include "sd_elm.h"
#include "serial.h"
#include "ints.h"

#include "bda.h"

#include "pcb_bitstorage.h"
#include "pcb_storage.h"

#include "leds.h"

#include "pff.h"

#include "services_disk.h"

enum disks
{
    DISK_FLOPPY = 0x00,
    DISK_HDD1 = 0x80
};

#define MOTOR_WAIT 37
#define RATE_500 0x00
#define RATE_300 0x40
#define RATE_250 0x80
#define RATE_RSV 0xC0	// Reserved

// 3 1/2" 1.44MB, 80 cyl * 2 hd * 18 spt * 512 bps
struct disk_base_table ddpt1440 = {
    0xAF,		// 1st specify byte- head unload, step rate
    0x02,		// 2nd specify byte- head load, DMA mode
    MOTOR_WAIT,	// Motor-off wait (ticks)
    0x02,		// (128 << this) bytes per sector
    18,			// Sectors per track
    0x1B,		// Gap length
    0xFF,		// Data length
    0x6C,		// Gap length for format
    0xF6,		// Fill byte for format
    15,			// Head settle time (ms)
    8,			// Motor start time (1/8s)
    79,			// Max track number
    RATE_500	// Data transfer rate
};

struct
{
    enum disks disk_number;
    uint16_t num_cylinders;
    uint16_t num_heads;
    uint16_t sectors_per_track;
    uint8_t cmos_disk_type;
    struct disk_base_table *dbt;
} static _disks_parameters[] = 
{
    // placeholder (modified upon .img size)
    { DISK_FLOPPY, 80, 2, 18, 4, &ddpt1440 },
    // placeholder
    { DISK_HDD1, 80, 2, 36, 0xff, (void*)0 }
};

#define SECTORS_PER_TRACK 18LU
#define NUM_HEADS 2LU
#define NUM_CYLINDERS 80LU


#define SECTOR_SIZE 512LU

enum int13_status
{
    INT13_STATUS_NO_ERROR = 0,
    INT13_STATUS_BAD_COMMAND = 1,
    INT13_STATUS_ADDRESS_MARK_NOT_FOUND_OR_BAD_SECTOR = 2,
    INT13_STATUS_DISKETTE_WRITE_PROTECT = 3,
    INT13_STATUS_SECTOR_NOT_FOUND = 4,
    INT13_STATUS_FIXED_DISK_RESET_FAILED = 5,
    INT13_STATUS_DISKETTE_CHANGED_OR_REMOVED = 6,
    INT13_STATUS_BAD_FIXED_DISK_PARAMETER_TABLE = 7,
    INT13_STATUS_DMA_OVERRUN = 8,
    INT13_STATUS_DMA_ACCESS_ACROSS_64K_BOUNDARY = 9,
    INT13_STATUS_BAD_FIXED_DISK_SECTOR_FLAG = 10,
    INT13_STATUS_BAD_FIXED_DISK_CYLINDER = 11,
    INT13_STATUS_UNSUPPORTED_TRACK_INVALID_MEDIA = 12,
    INT13_STATUS_INVALID_NUMBER_OF_SECTORS_ON_FIXED_DISK_FORMAT = 13,
    INT13_STATUS_FIXED_DISK_CONTROLLED_DATA_ADDRESS_MARK_DETECTED = 14,
    INT13_STATUS_FIXED_DISK_DMA_ARBITRATION_LEVEL_OUT_OF_RANGE = 15,
    INT13_STATUS_ECC_CRC_ERROR_ON_DISK_READ = 16,
    INT13_STATUS_RECOVERABLE_FIXED_DISK_DATA_ERROR_FIXED_BY_ECC = 17,
    INT13_STATUS_CONTROLLER_ERROR = 0x20,
    INT13_STATUS_SEEK_FAILURE = 0x40,
    INT13_STATUS_TIMEOUT = 0x80,
    INT13_STATUS_FIXED_DISK_DRIVE_NOT_READY = 0xaa,
    INT13_STATUS_FIXED_DISK_UNDEFINED_ERROR = 0xbb,
    INT13_STATUS_FIXED_DISK_WRITE_FAULT = 0xcc,
    INT13_STATUS_FIXED_DISK_STATUS_ERROR = 0xe0,
    INT13_STATUS_SENSE_OPERATION_FAILED = 0xff
};


static void _set_hdd_status(struct callregs *regs, unsigned char err)
{
    bda_write(hard_disk_status, err);
}



static uint8_t _set_diskette_status(unsigned char err)
{
    bda_write(diskette_status, err);
    return err;
}

static uint8_t _diskette_reset(struct callregs *regs)
{
    // technically nothing to do here?
    // TODO: remount FATFS + re-open file?
    return _set_diskette_status(INT13_STATUS_NO_ERROR);
}

static uint8_t _diskette_status(struct callregs *regs)
{
    // AL = status
    regs->ax.l = bda_read(diskette_status);
    return INT13_STATUS_NO_ERROR;
}

static uint8_t _diskette_read(struct callregs *regs)
{
    unsigned long cylinder =
        regs->cx.h | (((unsigned short)regs->cx.l & 0xc0) << 2);
    unsigned long head = regs->dx.h;
    unsigned long sector = regs->cx.l & 0x3f;
    unsigned long lba =
        (sector - 1) + 
        _disks_parameters[DISK_FLOPPY].sectors_per_track * (head + (cylinder * _disks_parameters[DISK_FLOPPY].num_heads));
    unsigned short dst = regs->bx.x;
    unsigned short count = regs->ax.l;
    unsigned short i;
    unsigned int read_bytes;

    if ((sector == 0) || (count == 0))
    {
        // Invalid command / parameter
        return _set_diskette_status(INT13_STATUS_BAD_COMMAND);
    }

    lba *= SECTOR_SIZE;

    if (pf_lseek(lba) != FR_OK)
    {
        // seeking to the specified offset failed!
        puts("_diskette_read: failed to seek to offset 0x"); serial_hexnum32(lba); cout('\n');

        return _set_diskette_status(INT13_STATUS_SEEK_FAILURE);
    }

    //puts("diskette_read lba 0x"); serial_hexnum32(lba); puts(", place into ES:BX "); serial_hexnum16(regs->es); cout(':'); serial_hexnum16(dst); puts(" (0x"); serial_hexnum32(((uint32_t)(regs->es) << 4) + dst); cout(')');
    regs->ax.l = 0;
    for (i = 0; i < count; ++i) {
        if (pf_read(regs->es, dst, 512, &read_bytes) != FR_OK) break;
        //if (read_bytes != 512) break;
        //++lba;
        dst += SECTOR_SIZE;
        ++regs->ax.l;
    }
    
    //puts(", requested 0x"); serial_hexnum16(count); puts(" sectors, read 0x"); serial_hexnum16(regs->ax.l); cout('\n');
    return _set_diskette_status(INT13_STATUS_NO_ERROR);
}


static uint8_t _diskette_parameters(struct callregs *regs)
{
    //puts("disk param, disk = 0x"); serial_hexnum8(regs->dx.l); cout('\n');

    // DL = number of drives attached
    regs->dx.l = 1;
    // DH = number of sides (0 based)
    regs->dx.h = _disks_parameters[DISK_FLOPPY].num_heads - 1;
    // CH = cylinders (0-1023 dec.)
	// CL = sectors per track
    regs->cx.x = _disks_parameters[DISK_FLOPPY].sectors_per_track | (_disks_parameters[DISK_FLOPPY].num_cylinders << 8);
    // BL = CMOS drive type
    regs->bx.l = _disks_parameters[DISK_FLOPPY].cmos_disk_type;
    // ES:DI = pointer to DDPT
    regs->es = get_cs();
    regs->di.x = (uint16_t)_disks_parameters[DISK_FLOPPY].dbt;
    
    return _set_diskette_status(INT13_STATUS_NO_ERROR);
}


static uint8_t _diskette_get_dasd_type(struct callregs *regs)
{
    // AH = 00 drive not present
    //    = 01 diskette, no change detection present
    //    = 02 diskette, change detection present
    //    = 03 fixed disk present
    regs->ax.h = 1;
    // CX:DX = number of fixed disk sectors; if 3 is returned in AH
    //regs->cx.x = regs->dx.x = 0xffff;

    return _set_diskette_status(INT13_STATUS_NO_ERROR);
}

/*
static void disk_read_hdd(struct callregs *regs)
{
    regs->ax.l = 0;
    regs->flags &= ~CF;

    for (i = 0; i < count; ++i) {
        if (disk_readp_seg(regs->es, dst, lba)) break;
        ++lba;
        dst += SECTOR_SIZE;
        ++regs->ax.l;
    }

    unsigned long absolute = regs->es;
    absolute <<= 4;
    absolute += dst;
    puts("disk_read: disk 0x"); serial_hexnum8(regs->dx.l); puts(" ES:BX "); serial_hexnum16(regs->es); puts(":"); serial_hexnum16(dst); puts(" (0x"); serial_hexnum32(absolute); puts("), count=0x"); serial_hexnum8(count); puts(", read=0x"); serial_hexnum16(regs->ax.l); puts(", lba=0x"); serial_hexnum32(lba); cout('\n');

    set_disk_status(regs, regs->ax.l != count ? 0xff : 0x00);
}

static void disk_read(struct callregs *regs)
{
    unsigned long cylinder =
        regs->cx.h | (((unsigned short)regs->cx.l & 0xc0) << 2);
    unsigned long head = regs->dx.h;
    unsigned long sector = regs->cx.l & 0x3f;
    unsigned long lba =
        (cylinder * NUM_HEADS + head) * SECTORS_PER_TRACK + (sector - 1);
    unsigned short i;
    unsigned short dst = regs->bx.x;
    unsigned short count = regs->ax.l;

    switch (regs->dx.l)
    {
        case 0x00:
        {
            disk_read_diskette(regs);
            break;
        }
        case 0x80:
        {
            disk_read_hdd(regs);
            break;
        }

    }
}

*/
static void disk_services(struct callregs *regs)
{
    led_set_single(LED_5);
    // CF = 0 if successful
	//    = 1 if error
    regs->flags &= ~CF;

    uint8_t ret = 0;

    switch (regs->dx.l)
    {
        case 0x00:
        {
            switch (regs->ax.h)
            {
                case 0x00: ret = _diskette_reset(regs); break;
                case 0x01: ret = _diskette_status(regs); break;
                case 0x02: ret = _diskette_read(regs); break;
                //case 0x03: disk_write(regs); break;
                case 0x08: ret = _diskette_parameters(regs); break;
                case 0x15: ret = _diskette_get_dasd_type(regs); break;
                default:
                {
                    puts("disk_services: unhandled diskette service 0x"); serial_hexnum8(regs->ax.h); cout('\n');
                    break;
                }
            }
            break;
        }

        default:
        {
            ret = INT13_STATUS_BAD_COMMAND;
        }
    }

    if (ret != INT13_STATUS_NO_ERROR)
    {
        puts("disk_services: error on service 0x"); serial_hexnum8(regs->ax.h); puts(", for disk 0x"); serial_hexnum8(regs->dx.l); puts(", err = 0x"); serial_hexnum8(ret); cout('\n');
        regs->ax.h = ret;   // Pass the error code
        regs->flags |= CF;  // Carry set on error
    }
    led_unset_single(LED_5);
}
VECTOR(0x13, disk_services);