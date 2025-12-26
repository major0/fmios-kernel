/*
 * FMI/OS x86_64 Architecture Support Functions
 * Stage 1: Basic x86_64 support functions
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
#include "klibc.h"

void arch_early_init(void)
{
	/* Stage 1: Minimal early initialization */
	arch_disable_interrupts();
}

void arch_init(void)
{
	/* Stage 1: Basic architecture initialization */

	/* Debug: arch_init entry */
	__asm__ volatile("movb $'1', %%al; outb %%al, %0" ::"Nd"(0x3f8) : "al");

	klogf(KLOG_INFO, "x86_64 architecture initialized");

	/* Debug: arch_init complete */
	__asm__ volatile("movb $'2', %%al; outb %%al, %0" ::"Nd"(0x3f8) : "al");
}

void arch_halt(void)
{
	klogf(KLOG_INFO, "System halting...");

	/* Disable interrupts and halt */
	arch_disable_interrupts();
	while (1) {
		__asm__ volatile("hlt");
	}
}

void arch_disable_interrupts(void)
{
	__asm__ volatile("cli");
}

void arch_enable_interrupts(void)
{
	__asm__ volatile("sti");
}
