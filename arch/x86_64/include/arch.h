/*
 * FMI/OS x86_64 Architecture Definitions
 * Stage 1: Basic x86_64 support for Hello World kernel
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

#ifndef _ARCH_X86_64_H
#define _ARCH_X86_64_H

#include <stdint.h>

/* x86_64 specific types */
typedef uint64_t paddr_t; /* Physical address */
typedef uint64_t vaddr_t; /* Virtual address */

/* Page size for x86_64 */
#define PAGE_SIZE 4096
#define PAGE_SHIFT 12

/* Architecture initialization */
void arch_early_init(void);
void arch_init(void);

/* Architecture-specific functions */
void arch_halt(void) __attribute__((noreturn));
void arch_disable_interrupts(void);
void arch_enable_interrupts(void);

#endif /* _ARCH_X86_64_H */
