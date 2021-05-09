#pragma once

#include <stdint.h>

#define GCS6_IO_START 0x2000
#define GCS6_IO_END 0x2040

void init_gcs(uint8_t nbr, uint16_t start, uint16_t end);