
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;

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

_Noreturn void kmain(void)
{
#if OS86
	uint16_t __far * video_buffer = (uint16_t __far *)0xB8000000;
#elif OS286
	uint16_t __far * video_buffer = (uint16_t __far *)0x00180000;
#else
	uint16_t * video_buffer = (uint16_t *)0x000B8000;
#endif

#ifdef __ia16__
	/* Workaround, the ia16 optimizer has trouble copying from a near segment to a far segment */
	const char __far * const message = greeting;
#else
	const char * const message = greeting;
#endif

	for(int i = 0; message[i] != '\0'; i++)
	{
		video_buffer[i] = 0x1E00 | (uint8_t)message[i];
	}

	for(;;)
		;
}

