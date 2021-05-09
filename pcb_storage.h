#pragma once

#define BIT_SD_IS_SDV2_SET() bit_set(LOC1)
#define BIT_SD_IS_SDV2_RESET() bit_reset(LOC1)
#define BIT_SD_IS_SDV2_TEST() bit_test(LOC1)

#define BYTE_LEDS_SET(x) outb(RFBASE, x)
#define BYTE_LEDS_GET() inb(RFBASE)

#define BYTE_DISK_STATUS_SET(x) outb(RFTIME, x)
#define BYTE_DISK_STATUS_GET() inb(RFTIME)