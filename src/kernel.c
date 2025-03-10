
#include "stdbool.h"
#include "stddef.h"
#include "stdint.h"
#include "stdnoreturn.h"
#include "string.h"
#include "i86.h"
#include "conio.h"

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

enum
{
	SCREEN_WIDTH = 80,
	SCREEN_HEIGHT = 25
};

static uint8_t screen_x = 0, screen_y = 0, screen_attribute = 0x07;

#if OS86
uint16_t far * const screen_buffer = (uint16_t far *)MK_FP(0xB800, 0);
#elif OS286
uint16_t far * const screen_buffer = (uint16_t far *)MK_FP(0x0018, 0);
#else
uint16_t * const screen_buffer = (uint16_t *)0x000B8000;
#endif

static inline void screen_set_word(int offset, uint16_t value)
{
	screen_buffer[offset] = value;
}

static inline uint16_t screen_get_word(int offset)
{
	return screen_buffer[offset];
}

static inline void screen_scroll_lines(int count)
{
	if(count > SCREEN_HEIGHT)
	{
		count = SCREEN_HEIGHT;
	}
	for(int i = 0; i < SCREEN_WIDTH * (SCREEN_HEIGHT - count); i++)
	{
		screen_buffer[i] = screen_buffer[i + SCREEN_WIDTH * count];
	}
	for(int i = 0; i < SCREEN_WIDTH * count; i++)
	{
		screen_buffer[i + SCREEN_WIDTH * (SCREEN_HEIGHT - count)] = (screen_attribute << 8) | ' ';
	}
}

static inline void screen_move_cursor(void)
{
	uint16_t location = screen_y * SCREEN_WIDTH + screen_x;
	outp(0x3D4, 0x0E);
	outp(0x3D5, location >> 8);
	outp(0x3D4, 0x0F);
	outp(0x3D5, location);
}

static inline void screen_putchar(int c)
{
	switch(c)
	{
	case '\b':
		if(screen_x > 0)
		{
			screen_x--;
		}
		break;
	case '\t':
		screen_x = (screen_x + 8) & ~7;
		break;
	case '\n':
		screen_x = 0;
		screen_y++;
		break;
	default:
		if(' ' <= c && c <= '~')
		{
			screen_set_word(screen_y * SCREEN_WIDTH + screen_x, (screen_attribute << 8) + (uint8_t)c);
			screen_x++;
		}
		break;
	}
	if(screen_x >= SCREEN_WIDTH)
	{
		screen_y += screen_x / SCREEN_WIDTH;
		screen_x %= SCREEN_WIDTH;
	}
	if(screen_y >= SCREEN_HEIGHT)
	{
		screen_scroll_lines(screen_y + 1 - SCREEN_HEIGHT);
		screen_y = SCREEN_HEIGHT - 1;
	}
	screen_move_cursor();
}

static inline void screen_putstr(const char far * text)
{
	for(int i = 0; text[i] != '\0'; i++)
	{
		screen_putchar((uint8_t)text[i]);
	}
}

noreturn void kmain(void)
{
	screen_attribute = 0x1E;
	screen_putstr(greeting);

	for(int i = 0; i < SCREEN_HEIGHT; i++)
	{
		screen_putstr("scroll test\n");
	}

	for(;;)
		;
}

