#include "serial.h"
#include "io.h"
#include "80c186eb.h"

void serial_init(void)
{
    outw(B0CMP, 0x8033); // 38400 @ 16MHz
    outw(S0CON, 0x0021); // mode 1 asynchronous 10-bit
}

void cout(char c)
{
    if (c == '\n')
        cout('\r');
    while((inb(S0STS) & 8) == 0);
    outb(T0BUF, c);
}

void puts(char * str)
{
    while(*str)
    {
        cout(*str);
        str++;
    }
}

void serial_hexnum4(uint8_t n)
{
    if (n > 9)
    {
        n = 'A' + (n - 10);
    }
    else
    {
        n = '0' + n;
    }
    cout(n);
}

void serial_hexnum8(uint8_t n)
{
    serial_hexnum4(n >> 4);
    serial_hexnum4(n & 0xf);
}

void serial_hexnum16(uint16_t n)
{
    serial_hexnum8(n >> 8);
    serial_hexnum8(n & 0xff);
}

void serial_hexnum32(uint32_t n)
{
    serial_hexnum16(n >> 16);
    serial_hexnum16(n & 0xffff);
}

/**
 * From https://github.com/davidknoll/80c188-bios/blob/master/bios/console.c
 */
/* Convert a number to (4-digit) BCD */
int bintobcd(int i)
{
    int r = 0;
    r += (i / 1000) * 0x1000;
    i %= 1000;
    r += (i / 100) * 0x100;
    i %= 100;
    r += (i / 10) * 0x10;
    i %= 10;
    r += i;
    return r;
}

/**
 * From https://github.com/davidknoll/80c188-bios/blob/master/c/iofunc.c
 */
/* Output a decimal number */
void serial_decimal(int32_t i)
{
    if (i < 0) {
        cout('-');
        i = -i;
    }

    if (i < 10) {
        serial_hexnum4(i);
    } else if (i < 100) {
        serial_hexnum8(bintobcd(i));
    } else if (i < 1000) {
        serial_hexnum4(bintobcd(i) >> 8);
        serial_hexnum8(bintobcd(i));
    } else if (i < 10000) {
        serial_hexnum16(bintobcd(i));
    } else {
        serial_hexnum4(bintobcd(i / 10000));
        serial_hexnum16(bintobcd(i));
    }
}

void serial_hexdump(uint16_t dseg, uint16_t daddr, uint16_t cnt)
{
    uint16_t i = 0;
    uint8_t chr_idx = 0;
    uint8_t b;
    uint8_t dsp[17] = { 0 };

    while (cnt--)
    {
        if (i % 16 == 0)
        {
            if (i != 0)
            {
                puts(" |"); puts(dsp); puts("|");
                for (chr_idx = 0; chr_idx < 16; chr_idx++)
                {
                    dsp[chr_idx] = '\0';
                }
            }
            cout('\n');
            serial_hexnum16(dseg); cout(':'); serial_hexnum16(daddr); cout(':'); cout(' ');
        }
        b = readb(dseg, daddr);
        serial_hexnum8(b); cout(' ');
        if ((b >= 0x20) &&
            (b <= 0x7e))
        {
            dsp[i%16] = b;
        }
        else
        {
            dsp[i%16] = '.';
        }
        inc_seg_addr_pair(dseg, daddr);
        i++;
    }
    puts(" |"); puts(dsp); puts("|");

    cout('\n');
}
