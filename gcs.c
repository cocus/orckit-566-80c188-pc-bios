#include "gcs.h"
#include "io.h"

void init_gcs(uint8_t nbr, uint16_t start, uint16_t end)
{
    outw(0xff80 | (nbr << 2), start | 0xf);
    outw((0xff80 | (nbr << 2)) + 2, end | 0x8);
}
