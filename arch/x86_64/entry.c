/*
 * FMI/OS x86_64 Entry Point
 * Stage 1: Basic kernel entry and initialization
 *
 * Copyright (C) 2024 FMI/OS Project
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "arch.h"
#include "boot.h"
#include "klibc.h"

/* Kernel main entry point called from bootstrap.S */
void kmain(void)
{
	/* Stage 1: Hello World kernel */

	/* Debug: kmain entry */
	__asm__ volatile("movb $'T', %%al; outb %%al, $0x3f8" ::: "al");

	/* Initialize architecture */
	arch_init();

	/* Debug: arch_init complete */
	__asm__ volatile("movb $'U', %%al; outb %%al, $0x3f8" ::: "al");

	/* Debug: About to print hello world */
	__asm__ volatile("movb $'W', %%al; outb %%al, $0x3f8" ::: "al");

	/* Print hello world message */
	kprintf("FMI/OS Stage 1 - Hello World Kernel\n");

	/* Debug: First kprintf complete */
	__asm__ volatile("movb $'X', %%al; outb %%al, $0x3f8" ::: "al");

	kprintf("Architecture: x86_64\n");
	kprintf("Boot Protocol: PVH ELF\n");

	/* Debug: Basic kprintf complete */
	__asm__ volatile("movb $'Y', %%al; outb %%al, $0x3f8" ::: "al");

	klogf(KLOG_INFO, "Kernel initialization complete");

	/* Debug: First klogf complete */
	__asm__ volatile("movb $'Z', %%al; outb %%al, $0x3f8" ::: "al");

	klogf(KLOG_INFO, "Stage 1: Basic x86_64 kernel running");

	/* For Stage 1, just halt after printing messages */
	kprintf("Stage 1 complete - halting system\n");

	/* Debug: About to halt */
	__asm__ volatile("movb $'V', %%al; outb %%al, $0x3f8" ::: "al");

	arch_halt();
}
