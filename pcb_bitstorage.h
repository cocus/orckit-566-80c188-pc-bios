#pragma once


#include <stdint.h>
#include "80c186eb.h"
#include "io.h"

#define BIT_LOCS(f) \
    f(LOC1, GCS0ST, 15) \
    f(LOC2, GCS0ST, 14) \
    f(LOC3, GCS0ST, 13) \
    f(LOC4, GCS0ST, 12) \
    f(LOC5, GCS0ST, 11) \
    f(LOC6, GCS0ST, 10) \
    f(LOC7, GCS0ST, 9) \
    f(LOC8, GCS0ST, 8) \
    f(LOC9, GCS0ST, 7) \
    f(LOC10, GCS0ST, 6) \
    f(LOC11, GCS0ST, 3) \
    f(LOC12, GCS0ST, 2) \
    f(LOC13, GCS0ST, 1) \
    f(LOC14, GCS0ST, 0)


#define BIT_LOCS_TEST(name, addr, bit) \
static inline uint8_t pcb_bs_ ## name ## _test(void) \
{ \
    return (inw(addr) & (bit << 1)); \
}

#define BIT_LOCS_SET(name, addr, bit) \
static inline void pcb_bs_ ## name ## _set(void) \
{ \
    outw(addr, inw(addr) | (bit << 1)); \
}

#define BIT_LOCS_RESET(name, addr, bit) \
static inline void pcb_bs_ ## name ## _reset(void) \
{ \
    outw(addr, inw(addr) & ~(bit << 1)); \
}

BIT_LOCS(BIT_LOCS_TEST);
BIT_LOCS(BIT_LOCS_SET);
BIT_LOCS(BIT_LOCS_RESET);

#define bit_test(loc) pcb_bs_## loc ##_test()
#define bit_set(loc) pcb_bs_## loc ##_set()
#define bit_reset(loc) pcb_bs_## loc ##_reset()