#include <stdint.h>
#include "io.h"
#include "serial.h"
#include "80c186eb.h"

#include "pff.h"
#include "sd_elm.h"

#include "bda.h"

#include "pcb_bitstorage.h"

#include "ints.h"

#include "gcs.h"
#include "leds.h"

#include "services_disk.h"

#define __unused __attribute__((unused))

FATFS fatfs = { 0 }; /* File system object */


static void basic_services(struct callregs __unused *regs)
{
    serial_hexdump(regs->es, regs->bx.x, 512);
}
VECTOR(0x18, basic_services);

static void set_vector(int vector, void *handler)
{
    writew(0, vector * 4, (unsigned short)handler);
    writew(0, vector * 4 + 2, get_cs());
}

static unsigned char vpt[] = {80, 25, 8, 4000 & 0xff, 4000 >> 8};

extern void unused_int(void);

void do_unused_int(struct callregs *r)
{
    r->flags |= CF;
}

static void install_vectors(void)
{
    struct vector {
        unsigned short num;
        void *handler;
    };

    extern struct vector vectors_start;
    extern struct vector vectors_end;

    struct vector *v;
    int i;

    for (i = 0; i < 256; ++i)
    {
        set_vector(i, unused_int);
    }
    for (v = &vectors_start; v < &vectors_end; ++v)
    {
        puts("adding vector for 0x"); serial_hexnum8(v->num); puts(", at 0x"); serial_hexnum16((uint16_t)v->handler); cout('\n');
        set_vector(v->num, v->handler);
    }

    set_vector(0x1d, (void*)0);
    extern struct disk_base_table ddpt1440;
    set_vector(0x1e, (void*)&ddpt1440);
}

static void bda_init(void)
{
    int i;

    for (i = 0; i < 256; ++i)
        writeb(0x40, i, 0);
    
    bda_write(equipment_list_flags, 0x0041);
    bda_write(mem_kbytes, 96);

    bda_write(video_mode, 0x03);
    bda_write(num_screen_cols, 80);
    bda_write(last_screen_row, 25 - 1);
    bda_write(crt_controller_base, 0x3d4);
    bda_write(video_display_data_area, 1);
    bda_write(dcc_index, 0x0a);

    bda_write(kb_mode_type, (1 << 4)); // 101/102 enhanced kb

    bda_write(soft_reset_flag, 0x1234);


    // No com ports
    bda_write(com1_port_address, 0x0000);
    bda_write(com2_port_address, 0x0000);
    bda_write(com3_port_address, 0x0000);
    bda_write(com4_port_address, 0x0000);
    // No lpt ports
    bda_write(lpt1_port_address, 0x0000);
    bda_write(lpt2_port_address, 0x0000);
    bda_write(lpt3_port_address, 0x0000);
    bda_write(lpt4_port_address, 0x0000);
}

/*
    crlf();

    puts("CS   SS   DS   ES\n");
    asm ("mov %%cs, %0" : "=r"(seg_reg)); serial_hexnum16(seg_reg); cout(' ');
    asm ("mov %%ss, %0" : "=r"(seg_reg)); serial_hexnum16(seg_reg); cout(' ');
    asm ("mov %%ds, %0" : "=r"(seg_reg)); serial_hexnum16(seg_reg); cout(' ');
    asm ("mov %%es, %0" : "=r"(seg_reg)); serial_hexnum16(seg_reg); crlf();


    

    puts("This is a string boi!\n");

    puts("Global @0 = 0x"); serial_hexnum16(global); crlf();
    global ^= 0xdead;
    puts("Global @1 = 0x"); serial_hexnum16(global); crlf();

    serial_hexdump((uint8_t*)0x3000, 1*1024);
*/


void die (/* Stop with dying message */
	FRESULT rc	/* FatFs return value */
)
{
	puts("Failed with rc="); serial_hexnum16(rc); puts(".\r\n");
	//for (;;) ;
}


void sd_test(void)
{
	FATFS fatfs;			/* File system object */
	DIR dir;				/* Directory object */
	FILINFO fno;			/* File information object */
	UINT bw, br, i;
	BYTE buff[64];
    FRESULT rc;

	puts("\r\nMount a volume.\r\n");
	rc = pf_mount(&fatfs);
	if (rc) { die(rc); return; }

#if PF_USE_DIR
	puts("\r\nOpen root directory.\r\n");
	rc = pf_opendir(&dir, "");
	if (rc) { die(rc); return; }

	puts("\r\nDirectory listing...\r\n");
	for (;;) {
		rc = pf_readdir(&dir, &fno);	/* Read a directory item */
		if (rc || !fno.fname[0]) break;	/* Error or end of dir */
		if (fno.fattrib & AM_DIR)
		{
            puts("   <dir>  "); puts(fno.fname); puts("\r\n");
        }
		else
		{
            serial_hexnum32(fno.fsize); puts("  "); puts(fno.fname); puts("\r\n");
        }
	}
	if (rc) { die(rc); return; }
#endif

	puts("\r\nOpen a test file (message.txt).\r\n");
	rc = pf_open("MESSAGE.TXT");
	if (rc) { die(rc); return; }

	puts("\r\nType the file content.\r\n");
	for (;;) {
		rc = pf_read(get_cs(), (uint16_t)buff, sizeof(buff), &br);	/* Read a chunk of file */
		if (rc || !br) break;			/* Error or end of file */
		for (i = 0; i < br; i++)		/* Type the data */
			cout(buff[i]);
	}
	if (rc) { die(rc); return; }

#if PF_USE_WRITE
	printf("\r\nOpen a file to write (write.txt).\r\n");
	rc = pf_open("WRITE.TXT");
	if (rc) { die(rc); return; }

	printf("\r\nWrite a text data. (Hello world!)\r\n");
	for (;;) {
		rc = pf_write("Hello world!\r\r\n", 14, &bw);
		if (rc || !bw) break;
	}
	if (rc) { die(rc); return; }

	printf("\r\nTerminate the file write process.\r\n");
	rc = pf_write(0, 0, &bw);
	if (rc) { die(rc); return; }
#endif



	puts("\r\nTest completed.\r\n");
	//for (;;) ;
}


int sd_boot(void)
{
    /*if (disk_initialize())
    {
        puts("failed!\n");
        return -1;
    }
    
    puts("\nsd_boot: read sector 0 ");
    if (disk_readp_seg(0, 0x7c00, 0))
    {
        puts("failed\n");
        return -1;
    }
    */

    sd_test();

	DIR dir;				/* Directory object */
	FILINFO fno;			/* File information object */
	UINT bw, br, i;
	BYTE buff[64];
    FRESULT rc;

    puts("sd_boot: sd init");
	serial_hexdump(get_cs(), (uint16_t)&fatfs, sizeof(fatfs));
    rc = pf_mount(&fatfs);
	if (rc)
    {
        puts(" failed with error 0x"); serial_hexnum8(rc); cout('\n');
        return -1;
    }
    puts(" ok\n");

    puts("sd_boot: booting from virtual drive A");
    rc = pf_open("FLOPPY-A.IMG");
    if (rc != FR_OK)
    {
        puts(" no floppy-a.img with error 0x"); serial_hexnum8(rc); cout('\n');
        return -1;
    }
    puts(", reading first sector");
    for (int i = 0; i < 512/sizeof(buff); i++)
    {
        rc = pf_read(get_cs(), (uint16_t)buff, sizeof(buff), &br);	/* Read a chunk of file */
        if (rc != FR_OK)
        {
            puts(" error reading file!\n");
            return -1;
        }
        memcpy_seg(0, (void*)0x7c00+(i*sizeof(buff)), get_cs(), buff, sizeof(buff));
    }

    serial_hexdump(0, 0x7c00, 512);

    puts("\nsd_boot: Booting from SD card...\n");
    asm volatile(
        "xor %sp, %sp\n"
        "mov %sp, %ds\n"
        "mov %sp, %ss\n"
        "mov $0x7bff, %sp\n"
        "mov $0x00, %dl\n"
        "jmp $0x0000, $0x7c00");
}

int builtin_boot(void)
{
    extern unsigned char boot_bin[];
    extern unsigned int boot_bin_len;

    puts("BootBin @ 0x"); serial_hexnum16((uint16_t)boot_bin); puts(", size = 0x"); serial_hexnum16(boot_bin_len); cout('\n');
    memcpy_seg(0x0000, (void*)0x7c00, get_cs(), boot_bin, boot_bin_len);
    //serial_hexdump(0x0000, 0x7c00, boot_bin_len);
    asm volatile(
        "mov $0x80, %dl\n"
        "jmp $0x0000, $0x7c00");
}

int int13_boot(void)
{
    asm volatile(
        "xor %ax, %ax\n"
        "mov %ax, %es\n"
        "mov $0x7c00, %bx\n"
        "mov $0x02, %ah\n"
        "mov $0x01, %al\n"
        "mov $0x00, %ch\n"
        "mov $0x01, %cl\n"
        "mov $0x00, %dh\n"
        "mov $0x00, %dl\n"
        "int $0x13\n"
        "mov $0x00, %dl\n"
        "jmp $0x0000, $0x7c00");
}

//volatile int global = 0xc0c0;
void root(void)
{
    serial_init();
    init_gcs(6, GCS6_IO_START, GCS6_IO_END);
    led_init();

    led_set_single(LED_1);

    puts("My Bios! " BUILD_TIMESTAMP "\n");
    puts("RAM 00000-"); serial_hexnum16(inw(LCSSP) & 0xffc0); puts("0, ROM ");
    serial_hexnum16(inw(UCSST) & 0xffc0); puts("0-FFFFF\n");

    uint16_t seg_reg;
    puts("CS   SS   DS   ES\n");
    asm ("mov %%cs, %0" : "=r"(seg_reg)); serial_hexnum16(seg_reg); cout(' ');
    asm ("mov %%ss, %0" : "=r"(seg_reg)); serial_hexnum16(seg_reg); cout(' ');
    asm ("mov %%ds, %0" : "=r"(seg_reg)); serial_hexnum16(seg_reg); cout(' ');
    asm ("mov %%es, %0" : "=r"(seg_reg)); serial_hexnum16(seg_reg); cout('\n');

    install_vectors();
    bda_init();

    led_set_single(LED_2);

    if (sd_boot())
    {
        puts("Error booting :( falling back to internal\n");
        builtin_boot();
        //int13_boot();
    }

    puts("\nEntering while(1)\n");

    while(1)
    {
        
    }
}
