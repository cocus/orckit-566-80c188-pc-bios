#include "serial.h"
#include "ints.h"

static void printer_services(struct callregs *regs)
{
    switch(regs->ax.h)
    {
        case 0x1:
            regs->ax.h = 0;
            break;
        default:
            puts("int 17, unhandled "); serial_hexnum8(regs->ax.h); cout('\n');
            regs->ax.x = 0x8; // busy
    }
}
VECTOR(0x17, printer_services);