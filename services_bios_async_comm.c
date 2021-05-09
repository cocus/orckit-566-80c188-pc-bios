#include "serial.h"
#include "ints.h"

static void bios_async_comm_services(struct callregs *regs)
{
    switch(regs->ax.h)
    {
        case 0x00:
            regs->ax.x = 0;
            break;
        default:
            puts("int 14, unhandled "); serial_hexnum8(regs->ax.h); cout('\n');
            regs->ax.x = 0;
            break;
    }
}
VECTOR(0x14, bios_async_comm_services);