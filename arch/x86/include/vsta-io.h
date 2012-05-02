#ifndef __ARCH_X86_IO_H__
#define __ARCH_X86_IO_H__

/*
 * Inlined i386 processor specific functions
 *
 * Note that the "\n" and "\t" terminators at the end of the assembly
 * opcodes are simply there to prettify the assembly output from the
 * compiler - if you compile using gcc and use the "-S" switch you'll
 * see what I mean :-)
 */
#include "arch/setjmp.h"
#include "arch/registers.h"
#include "arch/vm.h"
#include "vsta/percpu.h"

extern void refresh_segregs(void);
extern int cpfail(void);
extern unsigned char stack[];

#define local_save_flags(x)	\
	__asm__ __volatile__ ("pushfl ; popl %0" : "=g" (x) : : "memory")
#define local_irq_save(x)	\
	__asm__ __volatile__ ("pushfl ; popl %0 ; cli" : "=g" (x) : : "memory")
#define local_irq_restore(x)	\
	__asm__ __volatile__ ("pushl %0 ; popfl" : : "g" (x) : "memory", "cc")
#define local_irq_set(x)	\
	__asm__ __volatile__ ("pushl %0 ; popfl ; sti" : : "g" (x) : "memory", "cc")
#define local_irq_disable()	\
	__asm__ __volatile__ ("cli" : : : "memory")
#define local_irq_enable()	\
	__asm__ __volatile__ ("sti" : : : "memory")

/*
 * set_cr0()
 *	Set the value of the processor config register cr0
 */
static inline void
set_cr0(ulong addr)
{
	__asm__ __volatile__(
		"movl %0, %%cr0\n\t"
		: /* No output */
		: "r" (addr));
}

/*
 * set_cr3()
 *	Set the value of the processor config register cr3 - the L1 page
 *	table pointer
 */
static inline void
set_cr3(ulong addr)
{
	__asm__ __volatile__(
		"movl %0, %%cr3\n\t"
		: /* No output */
		: "r" (addr)); 
}

/*
 * get_cr3()
 *	Return the value of the processor config register cr0
 */
static inline ulong
get_cr0(void)
{
	register ulong res;
	
	__asm__ __volatile__(
		"movl %%cr0, %0\n\t"
		: "=r" (res)
		: /* No input */); 
	return(res);
}

/*
 * get_cr3()
 *	Get the value of the processor config register cr2 - the fault
 *	address register
 */
static inline ulong
get_cr2(void)
{
	register ulong res;
	
	__asm__ __volatile__(
		"movl %%cr2, %0\n\t"
		: "=r" (res)
		: /* No input */); 
	return(res);
}

/*
 * get_cr3()
 *	Get the value of the processor config register cr3 - the L1 page
 *	table pointer
 */
static inline ulong
get_cr3(void)
{
	register ulong res;
	
	__asm__ __volatile__(
		"movl %%cr3, %0\n\t"
		: "=r" (res)
		: /* No input */); 
	return(res);
}

/*
 * flush_tlb()
 *	Flush the processor page table "translation lookaside buffer"
 *
 * Shoot the whole thing on the i386; invalidate individual entries
 * on the i486 and later.
 */
static inline void
flush_tlb(void *va)
{
	__asm__ __volatile__ (
		"movl %%cr3, %%eax\n\t"
		"movl %%eax, %%cr3\n\t"
		: /* No output */
		: /* No input */
		: "ax");
}

/* Load the global descriptor table register */
#define load_GDT(x)	\
	__asm__("lgdt (%0)": :"r" (x));

/* Load the interrupt descriptor table register */
#define load_IDT(x)	\
	__asm__("lidt (%0)": :"r" (x));

/* Load the task register */
#define load_TR(n) __asm__ __volatile__("ltr %%ax"::"a" (__TSS(n)<<3))

/*
 * inportb()
 *	Get a byte from an I/O port
 */
static inline uchar
inportb(int port)
{
	register uchar res;
	
	__asm__ __volatile__(
		"inb %%dx,%%al\n\t"
		: "=a" (res)
		: "d" (port));
	return(res);
}

/*
 * outportb()
 *	Write a byte to an I/O port
 */
static inline void
outportb(int port, uchar data)
{
	__asm__ __volatile__(
		"outb %%al,%%dx\n\t"
		: /* No output */
		: "a" (data), "d" (port));
}

/*
 * idle_stack()
 *	Switch to using the idle stack
 */
#define idle_stack() __asm__ __volatile__ \
		( "movl $idle_stack-0x40,%%esp\n\t" \
		  "movl $idle_stack,%%ebp\n\t" \
		  : : )

/*
 * on_idle_stack()
 *	Tell if we're running on the idle stack
 */
static inline int
on_idle_stack(void)
{
	int res;

	__asm__ __volatile__ (
		"	subl %0,%0\n"
		"	cmpl $idle_top,%%esp\n"
		"	ja 1f\n"
		"	incl %0\n"
		"1:"
		: "=r" (res)
		: /* No input */);
	return(res);
}

/*
 * idle()
 *	Run idle - do nothing except wait for something to happen :-)
 *
 * We watch for num_run to go non-zero; we use sti/halt to atomically
 * enable interrupts and halt the CPU--this saves a fair amount of power
 * and heat.
 */
static inline void
idle(void)
{
	__asm__ __volatile__ (
		"movl $num_run,%%eax\n\t"
		"movl $0,%%edx\n"
		"1:\t"
		"cmpl %%edx,(%%eax)\n\t"
		"jne 2f\n\t"
		"hlt\n\t"
		"jmp 1b\n\t"
		".align 2,0x90\n"
		"2:\n\t"
		: /* No output */
		: /* No input */
		: "ax", "dx");
}

/*
 * setjmp()
 *	Save context, returning 0
 *
 * We don't bother to save registers whose value we know at the end of
 * a matching longjmp (eax and edi)
 */
static inline int
setjmp(jmp_buf regs)
{
	register int retcode;

	__asm__ __volatile__ (
		"movl $1f,(%%edi)\n\t"
		"movl %%esi,8(%%edi)\n\t"
		"movl %%esp,%%eax\n\t"
		"movl %%ebp,12(%%edi)\n\t"
		"subl $4,%%eax\n\t"
		"movl %%eax,16(%%edi)\n\t"
		"xorl %%eax,%%eax\n\t"
		"movl %%ebx,20(%%edi)\n\t"
		"movl %%edx,24(%%edi)\n\t"
		"movl %%ecx,28(%%edi)\n"
		"1:\n\t"
		: "=a" (retcode)
		: "D" (regs));
	return(retcode);
}

/*
 * longjmp()
 *	Restore context, returning a specified result
 */
static inline void
longjmp(jmp_buf env, int val)
{
	__asm__ volatile (
		"movl 28(%%edi),%%ecx\n\t"
		"movl 20(%%edi),%%ebx\n\t"
		"movl 16(%%edi),%%esp\n\t"
		"movl 12(%%edi),%%ebp\n\t"
		"movl  8(%%edi),%%esi\n\t"
		"movl   (%%edi),%%edx\n\t"
		"movl   %%edx,(%%esp)\n\t"
		"movl 24(%%edi),%%edx\n\t"
		"ret\n\t"::"D" (env), "a" (val));
}

#endif /* __ARCH_I386_PROCESSOR_H__ */

/* arch-tag: 3d72325f-ff39-4e38-9174-d61c60c7cc6c */

