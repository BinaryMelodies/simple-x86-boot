#ifndef _I86_H
#define _I86_H

#if __ia16__
# define far __far
# define MK_FP(__s, __o) ((void far *)(((uint32_t)(uint16_t)(__s) << 16) | (uint16_t)(__o)))
# define FP_OFF(__p) ((void *)(uint16_t)(uint32_t)(void far *)(__p))
# define FP_SEG(__p) ((uint16_t)((uint32_t)(void far *)(__s) >> 16))
#else
# define far
#endif

#endif // _I86_H
