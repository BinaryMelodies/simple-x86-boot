
#include "stdbool.h"
#include "stddef.h"
#include "stdint.h"
#include "stdnoreturn.h"
#include "string.h"
#include "i86.h"
#include "conio.h"

#if !OS86
typedef union descriptor_t
{
	struct
	{
		uint16_t limit0;
		uint32_t base0 : 24;
		uint8_t access;
#if OS286
		uint16_t reserved;
#else
		uint8_t limit1 : 4;
		uint8_t flags : 4;
		uint8_t base1;
#endif
	} __attribute__((packed)) segment;
} descriptor_t;

typedef struct descriptor_table_t
{
	uint16_t limit;
	unsigned long base;
} __attribute__((packed)) descriptor_table_t;

enum
{
	DESCRIPTOR_ACCESS_DATA = 0x0092,
	DESCRIPTOR_ACCESS_CODE = 0x009A,
	DESCRIPTOR_ACCESS_CPL0 = 0x0000,
	DESCRIPTOR_ACCESS_CPL3 = 0x0060,
	DESCRIPTOR_FLAGS_16BIT = 0x00,
	DESCRIPTOR_FLAGS_32BIT = 0x40,
	DESCRIPTOR_FLAGS_64BIT = 0x20,
	DESCRIPTOR_FLAGS_G = 0x80,
};

#if OS286
# define descriptor_set_segment(__descriptor, __base, __limit, __attributes, __flags) descriptor_set_segment(__descriptor, __base, __limit, __attributes)
#endif

static inline void descriptor_set_segment(descriptor_t * descriptor, uint32_t base, unsigned int limit, uint16_t access, uint8_t flags)
{
#if !OS286
	if(limit > 0xFFF)
	{
		flags |= DESCRIPTOR_FLAGS_G;
		limit >>= 12;
	}
	else
	{
		flags &= ~DESCRIPTOR_FLAGS_G;
	}
#endif
	descriptor->segment.limit0 = limit;
	descriptor->segment.base0 = base;
	descriptor->segment.access = access;
#if OS286
	descriptor->segment.reserved = 0;
#else
	descriptor->segment.limit1 = limit >> 16;
	descriptor->segment.flags = flags >> 4;
	descriptor->segment.base1 = base >> 24;
#endif
}

enum
{
	SEL_KERNEL_CS = 0x08,
	SEL_KERNEL_SS = 0x10,
#if OS286
	SEL_KERNEL_ES = 0x18,
	SEL_USER_CS = 0x20,
	SEL_USER_SS = 0x28,
	SEL_MAX = 0x30,
#else
	SEL_USER_CS = 0x18,
	SEL_USER_SS = 0x20,
	SEL_MAX = 0x28,
#endif
};

descriptor_t gdt[SEL_MAX / 8];

static inline void load_gdt(descriptor_t * table, uint16_t size)
{
	descriptor_table_t gdtr;
	gdtr.limit = size - 1;
	gdtr.base = (size_t)table;

	/* after calling LGDT, a long return is used to load CS:[E/R]IP, and then the remaining registers are filled with SEL_KERNEL_SS */

#if OS286
	/* The 80286 only has the SS, DS and ES registers */
	asm volatile(
		"pushw\t%1\n\t"
		"pushw\t$1f\n\t"
		"lgdt\t(%0)\n\t"
		"lretw\n"
		"1:\n\t"
		"movw\t%w2, %%ss\n\t"
		"movw\t%w2, %%ds\n\t"
		"movw\t%w2, %%es"
		:
		: "B"(&gdtr), "ri"(SEL_KERNEL_CS), "r"(SEL_KERNEL_SS)
		: "memory");
#elif OS386
	asm volatile(
		"pushl\t%1\n\t"
		"pushl\t$1f\n\t"
		"lgdt\t(%0)\n\t"
		"lretl\n"
		"1:\n\t"
		"movw\t%w2, %%ss\n\t"
		"movw\t%w2, %%ds\n\t"
		"movw\t%w2, %%es\n\t"
		"movw\t%w2, %%fs\n\t"
		"movw\t%w2, %%gs\n\t"
		:
		: "r"(&gdtr), "ri"((uint32_t)SEL_KERNEL_CS), "r"((uint16_t)SEL_KERNEL_SS)
		: "memory");
#elif OS64
	asm volatile(
		"pushq\t%1\n\t"
		"pushq\t$1f\n\t"
		"lgdt\t(%0)\n\t"
		"lretq\n"
		"1:\n\t"
		"movw\t%w2, %%ss\n\t"
		"movw\t%w2, %%ds\n\t"
		"movw\t%w2, %%es\n\t"
		"movw\t%w2, %%fs\n\t"
		"movw\t%w2, %%gs\n\t"
		:
		: "r"(&gdtr), "ri"((uint64_t)SEL_KERNEL_CS), "r"((uint16_t)SEL_KERNEL_SS)
		: "memory");
#endif
}
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

static inline void screen_puthex(size_t value)
{
	char buffer[sizeof(value) * 2 + 1];
	int ptr = sizeof(buffer);
	buffer[--ptr] = '\0';
	do
	{
		int d = value & 0xF;
		buffer[--ptr] = d < 10 ? '0' + d : 'A' + d - 10;
		value >>= 4;
	} while(value != 0);
	screen_putstr(&buffer[ptr]);
}

static inline void screen_putdec(ssize_t value)
{
	// this approximates the number of digits for around 10 bytes
	char buffer[(sizeof(value) * 5 + 1) / 2];
	int ptr = sizeof(buffer);
	if(value < 0)
	{
		screen_putchar('-');
		value = -value;
	}
	buffer[--ptr] = '\0';
	do
	{
		buffer[--ptr] = '0' + (value % 10);
		value /= 10;
	} while(value != 0);
	screen_putstr(&buffer[ptr]);
}

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

noreturn void kmain(void)
{
#if OS286
	descriptor_set_segment(&gdt[SEL_KERNEL_CS / 8], 0, 0xFFFF, DESCRIPTOR_ACCESS_CODE | DESCRIPTOR_ACCESS_CPL0, DESCRIPTOR_FLAGS_16BIT);
	descriptor_set_segment(&gdt[SEL_KERNEL_SS / 8], 0, 0xFFFF, DESCRIPTOR_ACCESS_DATA | DESCRIPTOR_ACCESS_CPL0, DESCRIPTOR_FLAGS_16BIT);
	descriptor_set_segment(&gdt[SEL_KERNEL_ES / 8], 0x0B8000, 0xFFFF, DESCRIPTOR_ACCESS_DATA | DESCRIPTOR_ACCESS_CPL0, DESCRIPTOR_FLAGS_16BIT);
	descriptor_set_segment(&gdt[SEL_USER_CS / 8], 0, 0xFFFF, DESCRIPTOR_ACCESS_CODE | DESCRIPTOR_ACCESS_CPL3, DESCRIPTOR_FLAGS_16BIT);
	descriptor_set_segment(&gdt[SEL_USER_SS / 8], 0, 0xFFFF, DESCRIPTOR_ACCESS_DATA | DESCRIPTOR_ACCESS_CPL3, DESCRIPTOR_FLAGS_16BIT);
#elif OS386
	descriptor_set_segment(&gdt[SEL_KERNEL_CS / 8], 0, 0xFFFFFFFF, DESCRIPTOR_ACCESS_CODE | DESCRIPTOR_ACCESS_CPL0, DESCRIPTOR_FLAGS_32BIT);
	descriptor_set_segment(&gdt[SEL_KERNEL_SS / 8], 0, 0xFFFFFFFF, DESCRIPTOR_ACCESS_DATA | DESCRIPTOR_ACCESS_CPL0, DESCRIPTOR_FLAGS_32BIT);
	descriptor_set_segment(&gdt[SEL_USER_CS / 8], 0, 0xFFFFFFFF, DESCRIPTOR_ACCESS_CODE | DESCRIPTOR_ACCESS_CPL3, DESCRIPTOR_FLAGS_32BIT);
	descriptor_set_segment(&gdt[SEL_USER_SS / 8], 0, 0xFFFFFFFF, DESCRIPTOR_ACCESS_DATA | DESCRIPTOR_ACCESS_CPL3, DESCRIPTOR_FLAGS_32BIT);
#elif OS64
	descriptor_set_segment(&gdt[SEL_KERNEL_CS / 8], 0, 0, DESCRIPTOR_ACCESS_CODE | DESCRIPTOR_ACCESS_CPL0, DESCRIPTOR_FLAGS_64BIT);
	descriptor_set_segment(&gdt[SEL_KERNEL_SS / 8], 0, 0, DESCRIPTOR_ACCESS_DATA | DESCRIPTOR_ACCESS_CPL0, 0);
	descriptor_set_segment(&gdt[SEL_USER_CS / 8], 0, 0, DESCRIPTOR_ACCESS_CODE | DESCRIPTOR_ACCESS_CPL3, DESCRIPTOR_FLAGS_64BIT);
	descriptor_set_segment(&gdt[SEL_USER_SS / 8], 0, 0, DESCRIPTOR_ACCESS_DATA | DESCRIPTOR_ACCESS_CPL3, 0);
#endif

#if !OS86
	load_gdt(gdt, sizeof gdt);
#endif

	screen_attribute = 0x1E;
	screen_putstr(greeting);

	for(int i = 0; i < SCREEN_HEIGHT; i++)
	{
		screen_putstr("scroll test\n");
	}

	screen_puthex((size_t)0x1A2B3C4D);
	screen_putdec(-12345);
#if !OS86
	screen_putdec(sizeof(descriptor_t));
#endif

	for(;;)
		;
}

