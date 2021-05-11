#include "serial.h"
#include "ints.h"

#include "pcb_bitstorage.h"
#include "pcb_storage.h"

#include "leds.h"

// Reverse scancode table, so we have a scancode to return with the ASCII
static const unsigned char scancodes[] = {
	// NUL SOH   STX   ETX   EOT   ENQ   ACK   BEL    BS    HT    LF    VT    FF    CR    SO    SI
	0x03, 0x1E, 0x30, 0x2E, 0x20, 0x12, 0x21, 0x22, 0x0E, 0x0F, 0x24, 0x25, 0x26, 0x1C, 0x31, 0x18,
	// DLE DC1   DC2   DC3   DC4   NAK   SYN   ETB   CAN    EM   SUB   ESC    FS    GS    RS    US
	0x19, 0x10, 0x13, 0x1F, 0x14, 0x16, 0x2F, 0x11, 0x2D, 0x15, 0x2C, 0x01, 0x2B, 0x1B, 0x07, 0x0C,
	// SP    !     "     #     $     %     &     '     (     )     *     +     ,     -     .     /
	0x39, 0x02, 0x28, 0x04, 0x05, 0x06, 0x08, 0x28, 0x0A, 0x0B, 0x09, 0x0D, 0x33, 0x0C, 0x34, 0x35,
	// 0     1     2     3     4     5     6     7     8     9     :     ;     <     =     >     ?
	0x0B, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x27, 0x27, 0x33, 0x0D, 0x34, 0x35,
	// @     A     B     C     D     E     F     G     H     I     J     K     L     M     N     O
	0x03, 0x1E, 0x30, 0x2E, 0x20, 0x12, 0x21, 0x22, 0x23, 0x17, 0x24, 0x25, 0x26, 0x32, 0x31, 0x18,
	// P     Q     R     S     T     U     V     W     X     Y     Z     [     \     ]     ^     _
	0x19, 0x10, 0x13, 0x1F, 0x14, 0x16, 0x2F, 0x11, 0x2D, 0x15, 0x2C, 0x1A, 0x2B, 0x1B, 0x07, 0x0C,
	// `     a     b     c     d     e     f     g     h     i     j     k     l     m     n     o
	0x29, 0x1E, 0x30, 0x2E, 0x20, 0x12, 0x21, 0x22, 0x23, 0x17, 0x24, 0x25, 0x26, 0x32, 0x31, 0x18,
	// p     q     r     s     t     u     v     w     x     y     z     {     |     }     ~   DEL
	0x19, 0x10, 0x13, 0x1F, 0x14, 0x16, 0x2F, 0x11, 0x2D, 0x15, 0x2C, 0x1A, 0x2B, 0x1B, 0x29, 0x53
};

static void keyboard_wait_and_read(struct callregs *regs)
{
    while(!cin_kbhit());

    regs->ax.x = cin_buf();

    if (regs->ax.l < 0x80) regs->ax.h = scancodes[regs->ax.l];
}

static void keyboard_status(struct callregs *regs)
{
    unsigned short c;

    if (cin_kbhit())
    {
        regs->flags &= ~ZF;
        regs->ax.x = cin_buf();
        if (regs->ax.l < 0x80) regs->ax.h = scancodes[regs->ax.l];
    }
    else
        regs->flags |= ZF;
}

static void keyboard_shift_status(struct callregs *regs)
{
    regs->ax.h = 0;
    regs->ax.l = 0x20; // faked numlock
}

static void keyboard_services(struct callregs *regs)
{
    switch (regs->ax.h)
    {
        // Wait for keystroke and read
        case 0x0:
        // Wait for keystroke and read  (AT,PS/2 enhanced keyboards)
        case 0x10:
        {
            keyboard_wait_and_read(regs);
            break;
        }
        // Get keystroke status
        case 0x1:
        // Get keystroke status  (AT,PS/2 enhanced keyboards)
        case 0x11:
        {
            keyboard_status(regs);
            break;
        }
        // Get shift status
        case 0x2: keyboard_shift_status(regs); break;
        default: break;
    }
}
VECTOR(0x16, keyboard_services);