#include "delay.h"

void dly_us(uint16_t us)
{
	us >>= 3;
	while(us--)
	{
		asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	}
}

void dly_ms(uint16_t ms)
{
	while(ms--)
	{
		dly_us(1000);
	}	
}