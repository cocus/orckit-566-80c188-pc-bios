#include "serial.h"
#include "ints.h"

static void rtc_services(struct callregs *regs)
{
    switch(regs->ax.h)
    {
        default:
            puts("int 1a, unhandled "); serial_hexnum8(regs->ax.h); cout('\n');
            regs->flags |= CF;
    }
}
VECTOR(0x1a, rtc_services);