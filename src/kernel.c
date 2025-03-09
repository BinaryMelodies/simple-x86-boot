
#include "stdbool.h"
#include "stdint.h"
#include "stdnoreturn.h"
#include "i86.h"

#if OS86
const char greeting[] = "Greetings! OS/86 running in real mode (8086)";
#elif OS286
const char greeting[] = "Greetings! OS/286 running in 16-bit protected mode (80286)";
#elif OS386
const char greeting[] = "Greetings! OS/386 running in 32-bit protected mode (80386)";
#elif OS64
const char greeting[] = "Greetings! OS/64 running in 64-bit long mode";
#else
# error Unknown target
#endif

#if OS86
uint16_t far * const video_buffer = (uint16_t far *)MK_FP(0xB800, 0);
#elif OS286
uint16_t far * const video_buffer = (uint16_t far *)MK_FP(0x0018, 0);
#else
uint16_t * const video_buffer = (uint16_t *)0x000B8000;
#endif

static inline void video_putchar(int offset, uint16_t value)
{
	video_buffer[offset] = value;
}

static inline void video_putstr(int offset, const char far * text)
{
	for(int i = 0; text[i] != '\0'; i++)
	{
		video_putchar(offset + i, 0x1E00 | (uint8_t)text[i]);
	}
}

noreturn void kmain(void)
{
	video_putstr(0, greeting);

	for(;;)
		;
}

