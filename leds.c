#include "leds.h"
#include "80c186eb.h"
#include "io.h"
#include "gcs.h"
#include "pcb_bitstorage.h"
#include "pcb_storage.h"

#define LED_BASE 0x2002

void led_init(void)
{
    BYTE_LEDS_SET(0);
	outw(LED_BASE, 0xff);
}

void led_set_single(enum Led led)
{
    uint8_t curr = BYTE_LEDS_GET();

    curr |= led;

    BYTE_LEDS_SET(curr);
    outb(LED_BASE, ~curr);
}

void led_unset_single(enum Led led)
{
    uint8_t curr = BYTE_LEDS_GET();

    curr &= ~led;

    BYTE_LEDS_SET(curr);
    outb(LED_BASE, ~curr);
}
