/* boot.S - bootstrap the kernel */
/* Copyright (C) 2012 Mark Ferrell
 * Copyright (C) 1999, 2001  Free Software Foundation, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#define __ASSEMBLY__

#include <fmios/fmios.h>
#include <fmios/page.h>
#include <multiboot.h>

#ifdef HAVE_ASM_USCORE
# define EXT_C(sym)	_ ## sym
#else
# define EXT_C(sym)	sym
#endif

/* Entered through 32-bit task gate constructed in 16-bit mode
 *
 * Our parameters are passed on the stack, which is located
 * in high memory.  Before any significant memory use occurs,
 * we must switch it down to a proper stack.
 */
.text
	.globl	start, _start
start:
_start:
	jmp     multiboot_entry

#ifdef CONFIG_ENABLE_MULTIBOOT1
.align  MULTIBOOT1_HEADER_ALIGN
#ifdef CONFIG_WITH_FRAMEBUFFER
#define MULTIBOOT1_HEADER_OPTIONS (MULTIBOOT1_PAGE_ALIGN|MULTIBOOT1_MEMORY_INFO|MULTIBOOT1_VIDEO_MODE)
#else
#define MULTIBOOT1_HEADER_OPTIONS (MULTIBOOT1_PAGE_ALIGN|MULTIBOOT1_MEMORY_INFO)
#endif
multiboot1_header:
        .long   MULTIBOOT1_HEADER_MAGIC
        .long   MULTIBOOT1_HEADER_OPTIONS
        /* checksum */
        .long   -(MULTIBOOT1_HEADER_MAGIC + MULTIBOOT1_HEADER_OPTIONS)
#ifdef CONFIG_WITH_FRAMEBUFFER
	.long	0	/* what mode should we use? */
        .long	CONFIG_FRAMEBUFFER_WIDTH
	.long	CONFIG_FRAMEBUFFER_HEIGHT
	.long	CONFIG_FRAMEBUFFER_DEPTH
#endif /* CONFIG_WITH_FRAMEBUFFER */
#endif /* CONFIG_ENABLE_MULTIBOOT1 */
.align  MULTIBOOT_HEADER_ALIGN
multiboot2_header:
	.long	MULTIBOOT2_HEADER_MAGIC
	.long	MULTIBOOT_ARCHITECTURE_I386
	.long	multiboot2_header_end - multiboot2_header
	/* checksum */
	.long	-(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386 + (multiboot2_header_end - multiboot2_header))
#ifdef CONFIG_WITH_FRAMEBUFFER
framebuffer_tag_start:
	.short MULTIBOOT_HEADER_TAG_FRAMEBUFFER
	.short MULTIBOOT_HEADER_TAG_OPTIONAL
	.long framebuffer_tag_end - framebuffer_tag_start
	.long CONFIG_FRAMEBUFFER_WIDTH
	.long CONFIG_FRAMEBUFFER_HEIGHT
	.long CONFIG_FRAMEBUFFER_DEPTH
framebuffer_tag_end:
#endif /* CONFIG_WITH_FRAMEBUFFER */
	.short MULTIBOOT_HEADER_TAG_END
	.short 0
	.long 8
multiboot2_header_end:

#define IDLE_STACK_SIZE 0x40
multiboot_entry:
	/* Set our stack pointer */
	movl	$(stack + STACK_SIZE), %esp

	/* Reset EFLAGS */
	pushl	$0
	popf

	/* Push pointer to the mbi */
	pushl	%ebx
	pushl	%eax

        /* Enter mb_init() */
        call    EXT_C(fmios_init)

        /* Halt. */
        pushl   $halt_message
        call    EXT_C(printk)

	.global halt
halt:
	hlt
        jmp     halt
.data
	.align 8
halt_message:
        .asciz  "System Halted."
	/* Our global stack */
	.align 8
	.comm stack, STACK_SIZE
