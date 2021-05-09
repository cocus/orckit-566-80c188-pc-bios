#include "serial.h"
#include "ints.h"
#include "bda.h"

static void memory_size_services(struct callregs *regs)
{
    puts("mem = ");
    regs->ax.x = bda_read(mem_kbytes);
    serial_decimal(regs->ax.x); puts("k\n");
}
VECTOR(0x12, memory_size_services);