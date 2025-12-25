/*
 * FMI/OS x86_64 Boot Definitions
 * Stage 1: PVH ELF boot support only
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

#ifndef _BOOT_X86_64_H
#define _BOOT_X86_64_H

#include <stdint.h>

/* PVH boot information structure (minimal for Stage 1) */
struct pvh_boot_info {
	uint32_t magic;
	uint32_t version;
	uint64_t rsdp_paddr;
	uint64_t memmap_paddr;
	uint32_t memmap_entries;
	uint32_t reserved;
};

/* Boot entry point */
extern void _start(void);

/* Kernel main entry point */
extern void kmain(void);

#endif /* _BOOT_X86_64_H */
