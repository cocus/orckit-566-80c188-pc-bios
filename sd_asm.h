#pragma once

#include <stdint.h>

void xmit_mmc (
	uint8_t d			/* Data to be sent */
);

uint8_t rcvr_mmc (void);

void skip_mmc (
	uint16_t n		/* Number of BYTEs to skip */
);