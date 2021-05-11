#include "serial.h"
#include "ints.h"
#include "bda.h"

static void bios_eq_flags_services(struct callregs *regs)
{
    //puts("eq_flags = 0x");
    regs->ax.x = bda_read(equipment_list_flags);
    //serial_hexnum16(regs->ax.x); cout('\n');
}
VECTOR(0x11, bios_eq_flags_services);