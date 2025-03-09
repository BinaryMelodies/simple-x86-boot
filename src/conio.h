#ifndef _CONIO_H
#define _CONIO_H

#include "stdint.h"

static inline void outp(unsigned port, unsigned value)
{
	__asm__ volatile("outb %b0, %w1"
		:
		: "a"((uint8_t)value), "Nd"((uint16_t)port)
		: "memory");
}

static inline void outpw(unsigned port, unsigned value)
{
	__asm__ volatile("outw %b0, %w1"
		:
		: "a"((uint16_t)value), "Nd"((uint16_t)port)
		: "memory");
}

#ifndef __ia16__
static inline void outpl(unsigned port, unsigned value)
{
	__asm__ volatile("outl %b0, %w1"
		:
		: "a"((uint32_t)value), "Nd"((uint16_t)port)
		: "memory");
}
#endif

static inline unsigned inp(unsigned port)
{
	uint8_t ret;
	__asm__("inb %w1, %b0"
		: "=a"(ret)
		: "Nd"((uint16_t)port)
		: "memory");
	return ret;
}

static inline unsigned inpw(unsigned port)
{
	uint16_t ret;
	__asm__("inw %w1, %b0"
		: "=a"(ret)
		: "Nd"((uint16_t)port)
		: "memory");
	return ret;
}

#ifndef __ia16__
static inline unsigned inpl(unsigned port)
{
	uint32_t ret;
	__asm__("inl %w1, %b0"
		: "=a"(ret)
		: "Nd"((uint16_t)port)
		: "memory");
	return ret;
}
#endif

#endif // _CONIO_H
