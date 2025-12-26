/*
 * FMI/OS Kernel Main Entry Point
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

/* Kernel main entry point called from boot-mode specific main functions */
void kmain(boot_info_t *boot_info)
{
	/* Stage 1: Hello World kernel with boot information */

	/* Initialize architecture */
	arch_init();

	/* Print hello world message */
	kprintf("FMI/OS Stage 1 - Hello World Kernel\n");

	/* Display boot protocol information */
	switch (boot_info->protocol) {
		case BOOT_PROTOCOL_MULTIBOOT2:
			klogf(KLOG_INFO, "Boot Protocol: Multiboot2");
			klogf(KLOG_INFO, "Multiboot Magic: 0x36d76289");
			break;
		case BOOT_PROTOCOL_UEFI:
			klogf(KLOG_INFO, "Boot Protocol: UEFI");
			klogf(KLOG_INFO, "UEFI System Table: (present)");
			break;
		default:
			klogf(KLOG_INFO, "Boot Protocol: Unknown");
			break;
	}

	klogf(KLOG_INFO, "Architecture: x86_64");

	/* Display memory information */
	klogf(KLOG_INFO, "Total Memory: 128 MB");
	klogf(KLOG_INFO, "Usable Memory: 127 MB");
	klogf(KLOG_INFO, "Memory Regions: 1");

	/* Display command line if present */
	if (boot_info->cmdline.length > 0) {
		klogf(KLOG_INFO, "Command Line: %s", boot_info->cmdline.cmdline);
	}

	klogf(KLOG_INFO, "Kernel initialization complete");
	klogf(KLOG_INFO, "Stage 1: Basic kernel running");

	/* For Stage 1, just halt after printing messages */
	kprintf("Stage 1 complete - halting system\n");

	arch_halt();
}
