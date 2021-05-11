#include "serial.h"
#include "ints.h"
#include "io.h"

// Table returned by AH=C0h (Get Configuration)
static const unsigned char cfgtbl[] = {
    0x08, 0x00,						// Number of bytes following
    0xFC, 0x00,						// Model (here 1986 XT), submodel
    0x00,							// BIOS revision
    0x20, 0x04, 0x00, 0x48, 0x01	// Features
};


static void system_bios_services(struct callregs *regs)
{
    switch(regs->ax.h)
    {
        // Return System Configuration Parameters (PS/2 only)
        case 0xc0:
            regs->flags &= ~CF;
            regs->es = get_cs();
            regs->bx.x = (uint16_t)&cfgtbl;
            regs->ax.h = 0x00;
            //puts("bios_services: parameters, ret ES:BX "); serial_hexnum16(regs->es); cout(':'); serial_hexnum16(regs->bx.x); cout ('\n');
            break;
        default:
            puts("int 15, unhandled "); serial_hexnum8(regs->ax.h); cout('\n');
            regs->flags |= CF;
    }
}
VECTOR(0x15, system_bios_services);