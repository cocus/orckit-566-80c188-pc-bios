#include "serial.h"
#include "ints.h"
#include "io.h"

#include "bda.h"

/**
 * From https://github.com/davidknoll/80c188-bios/blob/master/bios/int10h.c
 */
/* Map BIOS attribute colour codes to SGR ones */
static const unsigned char atttosgrcol[] = {
	0,	// black
	4,	// blue
	2,	// green
	6,	// cyan
	1,	// red
	5,	// magenta
	3,	// yellow
	7	// white
};

/* Output an SGR attribute sequence */
static void doescatt(unsigned char att)
{
/*	static unsigned char lastatt;
	if (att == lastatt) return;			// Don't repeat unnecessarily
	lastatt = att;
*/
	puts("\x1B[m");					// Reset SGR
	if (att & 0x80) puts("\x1B[5m");	// Foreground blink
	puts("\x1B[4");					// Background colour
	cout('0' + atttosgrcol[(att & 0x70) >> 4]);
	cout('m');
	if (att & 0x08) puts("\x1B[1m");	// Foreground bright
	puts("\x1B[3");					// Foreground colour
	cout('0' + atttosgrcol[att & 0x07]);
	cout('m');
}

/* Set the cursor position */
static void doescpos(int row, int col)
{
	puts("\x1B[");
	serial_decimal(row + 1);
	cout(';');
	serial_decimal(col + 1);
	cout('H');
}

/* Resize the terminal window */
static void doescrsz(int rows, int cols)
{
	puts("\x1B[8;");
	serial_decimal(rows);
	cout(';');
	serial_decimal(cols);
	cout('t');
}

/* Set scroll region */
static void doescrgn(int top, int bottom)
{
	puts("\x1B[");
	serial_decimal(top + 1);
	cout(';');
	serial_decimal(bottom + 1);
	cout('r');
}

static void set_video_mode(struct callregs *regs)
{
    switch (regs->ax.l & 0x7F)
    {
        case 0x00:  // 40x25 grey text
        case 0x01:  // 40x25 colour text
        {
            doescrsz(25, 40);
            bda_write(num_screen_cols, 40); // Columns
            break;
        }
        case 0x02:  // 80x25 grey text
        case 0x03:  // 80x25 colour text
        case 0x07:  // 80x25 mono text
        {
            doescrsz(25, 80);
            bda_write(num_screen_cols, 80); // Columns
            break;
        }
        default:
            return;
    }

    if (!(regs->ax.l & 0x80))
    {
        puts("\x1B[m\f\x1B[H\x1B[2J");  // Reset SGR, clear screen
    }

    // Current mode
    bda_write(video_mode, regs->ax.l);
    // We don't support background intensity
    bda_write(crt_mode_control, bda_read(crt_mode_control) | 0x20);
    bda_write(last_screen_row, 24);

    // Bit 7 taken from the video mode
    bda_write(video_mode_options, bda_read(video_mode_options) & ~0x80);
    bda_write(video_mode_options, bda_read(video_mode_options) | (regs->ax.l % 0x80));
}

static void set_text_mode_cursor_shape(struct callregs *regs)
{
    // Cursor shape can't be changed, update BDA anyway
    bda_write(cursor_end, regs->cx.l & 0x1F);
    bda_write(cursor_start, regs->cx.h & 0x1F);
}

static void set_cursor_position(struct callregs *regs)
{
    // Page number is ignored
    doescpos(regs->dx.h, regs->dx.l);
    bda_write(cursor_offsets[regs->bx.h & 0x07], regs->dx.x);
}

static void read_cursor_position(struct callregs *regs)
{
    // As standard this function just returns values from the BDA
    // Not implemented for real, although there is an escape code ESC[6n
    regs->cx.l = bda_read(cursor_end);
    regs->cx.h = bda_read(cursor_start);
    regs->dx.x = bda_read(cursor_offsets[regs->bx.h & 0x07]);
}

static void scroll_window(struct callregs *regs)
{
    // Left/right boundaries given are ignored, scroll will be full-width
    doescrgn(regs->cx.h, regs->dx.h);
    doescatt(regs->bx.h);
    // Scroll up/down, AL=00h to clear the entire window
    puts("\x1B[");
    serial_decimal(regs->ax.l ? regs->ax.l : (regs->dx.h - regs->cx.h));
    cout((regs->ax.x & 0x0100) ? 'T' : 'S');
    puts("\x1B[r");	// Reset scroll region now we're done
}

static void write_character_and_attr_at_cursor(struct callregs *regs)
{
    doescatt(regs->bx.l);   // Page number is ignored
}

static void write_char_at_cursor(struct callregs *regs)
{
    uint16_t i;
    // Output character specified number of times
    puts("\x1B[s");    // Do not update cursor (save it)
    for (i = 0; i < regs->cx.x; i++) cout(regs->ax.l);
    puts("\x1B[u");    // Do not update cursor (restore it)
}

static void write_char_in_teletype_mode(struct callregs *regs)
{
    // Ignoring page number and graphics foreground pixel colour
    cout(regs->ax.l);
}

static void get_current_video_mode(struct callregs *regs)
{
    regs->ax.h = bda_read(num_screen_cols);
    regs->ax.l = bda_read(video_mode);
    regs->bx.h = bda_read(active_page);
}

static void write_character_string(struct callregs *regs)
{
    // Page number is ignored
    uint16_t sz = regs->cx.x;
    uint16_t dseg = regs->es;
    uint16_t daddr = regs->bp.x;
    doescpos(regs->dx.h, regs->dx.l);
    if (!(regs->ax.l & 0x02)) doescatt(regs->bx.l);
    if (!(regs->ax.l & 0x01)) puts("\x1B[s");	// Do not update cursor (save it)

    while(sz--) {
        if (regs->ax.l & 0x02) {	// Use attributes in string
            doescatt(readb(dseg, daddr + 1));
            cout(readb(dseg, daddr));
            inc_seg_addr_pair(dseg, daddr);
        } else {			// Use attributes in BL
            cout(readb(dseg, daddr));
        }
        inc_seg_addr_pair(dseg, daddr);
    }
    if (!(regs->ax.l & 0x01)) puts("\x1B[u");	// Do not update cursor (restore it)
}

static void video_services(struct callregs *regs)
{
    switch (regs->ax.h) {
        // Set video mode
        case 0x00: set_video_mode(regs); break;
        // Set text mode cursor shape
        case 0x01: set_text_mode_cursor_shape(regs); break;
        // Set cursor position
        case 0x02: set_cursor_position(regs); break;
        // Read cursor position and size
        case 0x03: read_cursor_position(regs); break;

        // 0x04 is for a light pen, which we don't have

        // Select page
        // Just update the BDA, nothing else
        case 0x05: bda_write(active_page, regs->ax.l); break;

        // Scroll window up
        case 0x06:
        // Scroll window down
        case 0x07:
        {
            scroll_window(regs);
            break;
        }
        // 0x08 read character and attribute at cursor, dummy character & attribute
        case 0x08: regs->ax.x = 0x07 | 0x00; break;
        // Write character and attribute at cursor
        case 0x09: write_character_and_attr_at_cursor(regs); // fallthru
        // Write character at cursor
        case 0x0a: write_char_at_cursor(regs); break;
        // 0x0B set background/border/palette, nothing to do on serial
        // 0x0C & 0x0D only valid in graphics modes
        // Write character in teletype mode
        case 0x0e: write_char_in_teletype_mode(regs); break;

        // Get current video mode
        case 0x0F: get_current_video_mode(regs); break;

        // 0x10-0x12 not much to do on serial

        // Write character string
        case 0x13: write_character_string(regs); break;

        default:
        {
            puts("services_video: unhandled service 0x"); serial_hexnum8(regs->ax.h); cout('\n');
        }
    }
}
VECTOR(0x10, video_services);
