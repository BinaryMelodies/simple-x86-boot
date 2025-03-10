
#include "stdbool.h"
#include "stdint.h"
#include "stdnoreturn.h"
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
	VIDEO_WIDTH = 80,
	VIDEO_HEIGHT = 25
};

static uint8_t video_x = 0, video_y = 0, video_attribute = 0x07;

#if OS86
uint16_t far * const video_buffer = (uint16_t far *)MK_FP(0xB800, 0);
#elif OS286
uint16_t far * const video_buffer = (uint16_t far *)MK_FP(0x0018, 0);
#else
uint16_t * const video_buffer = (uint16_t *)0x000B8000;
#endif

static inline void video_set_word(int offset, uint16_t value)
{
	video_buffer[offset] = value;
}

static inline uint16_t video_get_word(int offset)
{
	return video_buffer[offset];
}

static inline void video_scroll_lines(int count)
{
	if(count > VIDEO_HEIGHT)
	{
		count = VIDEO_HEIGHT;
	}
	for(int i = 0; i < VIDEO_WIDTH * (VIDEO_HEIGHT - count); i++)
	{
		video_buffer[i] = video_buffer[i + VIDEO_WIDTH * count];
	}
	for(int i = 0; i < VIDEO_WIDTH * count; i++)
	{
		video_buffer[i + VIDEO_WIDTH * (VIDEO_HEIGHT - count)] = (video_attribute << 8) | ' ';
	}
}

static inline void video_move_cursor(void)
{
	uint16_t location = video_y * VIDEO_WIDTH + video_x;
	outp(0x3D4, 0x0E);
	outp(0x3D5, location >> 8);
	outp(0x3D4, 0x0F);
	outp(0x3D5, location);
}

static inline void video_putchar(int c)
{
	switch(c)
	{
	case '\b':
		if(video_x > 0)
		{
			video_x--;
		}
		break;
	case '\t':
		video_x = (video_x + 8) & ~7;
		break;
	case '\n':
		video_x = 0;
		video_y++;
		break;
	default:
		if(' ' <= c && c <= '~')
		{
			video_set_word(video_y * VIDEO_WIDTH + video_x, (video_attribute << 8) + (uint8_t)c);
			video_x++;
		}
		break;
	}
	if(video_x >= VIDEO_WIDTH)
	{
		video_y += video_x / VIDEO_WIDTH;
		video_x %= VIDEO_WIDTH;
	}
	if(video_y >= VIDEO_HEIGHT)
	{
		video_scroll_lines(video_y + 1 - VIDEO_HEIGHT);
		video_y = VIDEO_HEIGHT - 1;
	}
	video_move_cursor();
}

static inline void video_putstr(const char far * text)
{
	for(int i = 0; text[i] != '\0'; i++)
	{
		video_putchar((uint8_t)text[i]);
	}
}

noreturn void kmain(void)
{
	video_attribute = 0x1E;
	video_putstr(greeting);

	for(int i = 0; i < VIDEO_HEIGHT; i++)
	{
		video_putstr("scroll test\n");
	}

	for(;;)
		;
}

