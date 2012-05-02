#ifndef __ARCH_X86_IO_H__
#define __ARCH_X86_IO_H__
#include <types.h>

extern void putchar(int c);

/*
 * inb()
 *	Get a byte from an I/O port
 */
static inline uint8_t inb(int port)
{
	register uint8_t res;

	__asm__ __volatile__(
		"inb %%dx,%%al\n\t"
		: "=a" (res)
		: "d" (port));
	return(res);
}

/*
 * outb()
 *	Write a byte to an I/O port
 */
static inline void outb(int port, uint8_t data)
{
	__asm__ __volatile__(
		"outb %%al,%%dx\n\t"
		: /* No output */
		: "a" (data), "d" (port));
}

#endif /* __ARCH_X86_IO__ */
