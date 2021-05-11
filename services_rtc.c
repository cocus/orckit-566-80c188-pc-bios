#include "serial.h"
#include "ints.h"

static void rtc_services(struct callregs *regs)
{
    switch(regs->ax.h)
    {
        // Get system time
        case 0x00:
        {
            regs->cx.x = 0;  // stay on 0
            regs->dx.x = 0;
            regs->ax.l = 0;   // no day wrap
            break;
        }
        // Read time from RTC
        case 0x02:
        {
            regs->cx.h = 0x12;  // hours (in BCD)
            regs->cx.l = 0x34;  // minutes (in BCD)
            regs->dx.h = 0x56;  // seconds (in BCD)
            regs->dx.l = 0;     // no daylight savings time
            break;
        }        
        // Read date from RTC
        case 0x04:
        {
            regs->cx.h = 0x20;  // century (in BCD)
            regs->cx.l = 0x21;  // year (in BCD)
            regs->dx.h = 0x05;  // month (in BCD)
            regs->dx.l = 0x11;  // day (in BCD)
            break;
        }
        
        case 0x01:
        case 0x05:
        {
            break;
        }
        default:
            puts("int 1a, unhandled "); serial_hexnum8(regs->ax.h); cout('\n');
    }
}
VECTOR(0x1a, rtc_services);