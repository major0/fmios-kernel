/*
 * FMI/OS x86_64 Boot Protocol Main Functions
 * Stage 1: Multiboot2 and UEFI boot protocol support
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
#include <string.h>

/* Global boot information structure */
#if defined(ENABLE_MULTIBOOT2) || defined(ENABLE_UEFI)
static boot_info_t global_boot_info;
#endif

#ifdef ENABLE_MULTIBOOT2
/* Multiboot2 main function - called from bootstrap.S */
void mb2_main(uint32_t magic, uint64_t multiboot_info_addr)
{
	/* Initialize boot info structure */
	memset(&global_boot_info, 0, sizeof(boot_info_t));

	/* Set protocol type */
	global_boot_info.protocol = BOOT_PROTOCOL_MULTIBOOT2;

	/* Store Multiboot2 specific data */
	global_boot_info.protocol_data.multiboot2.multiboot_magic = magic;
	global_boot_info.protocol_data.multiboot2.multiboot_info_addr = multiboot_info_addr;

	/* Parse Multiboot2 information */
	if (parse_multiboot2_info(multiboot_info_addr, &global_boot_info) != 0) {
		/* If parsing fails, set minimal defaults */
		global_boot_info.memory_map.num_regions = 0;
		global_boot_info.memory_map.total_memory = 0;
		global_boot_info.memory_map.usable_memory = 0;
		global_boot_info.cmdline.length = 0;
	}

	/* Call kernel main with boot information */
	kmain(&global_boot_info);
}
#endif /* ENABLE_MULTIBOOT2 */

#ifdef ENABLE_UEFI
/* UEFI main function - called from UEFI entry point */
void uefi_main(uint64_t image_handle, uint64_t system_table)
{
	/* Initialize boot info structure */
	memset(&global_boot_info, 0, sizeof(boot_info_t));

	/* Set protocol type */
	global_boot_info.protocol = BOOT_PROTOCOL_UEFI;

	/* Store UEFI specific data */
	global_boot_info.protocol_data.uefi.image_handle = image_handle;
	global_boot_info.protocol_data.uefi.system_table = system_table;

	/* Parse UEFI information */
	if (parse_uefi_info(image_handle, system_table, &global_boot_info) != 0) {
		/* If parsing fails, set minimal defaults */
		global_boot_info.memory_map.num_regions = 0;
		global_boot_info.memory_map.total_memory = 0;
		global_boot_info.memory_map.usable_memory = 0;
		global_boot_info.cmdline.length = 0;
	}

	/* Call kernel main with boot information */
	kmain(&global_boot_info);
}
#endif /* ENABLE_UEFI */

#ifdef ENABLE_MULTIBOOT2
/* Parse Multiboot2 information structure */
int parse_multiboot2_info(uint64_t multiboot_info_addr, boot_info_t *boot_info)
{
	/* For Stage 1, implement minimal parsing */
	/* This is a placeholder - in a real implementation, we would parse the Multiboot2 tags */
	(void)multiboot_info_addr; /* Suppress unused parameter warning */

	/* Set some default values for testing */
	boot_info->memory_map.num_regions = 1;
	boot_info->memory_map.max_regions = 128;
	boot_info->memory_map.regions[0].base_addr = 0x100000; /* 1MB */
	boot_info->memory_map.regions[0].length = 0x7F00000;   /* 127MB */
	boot_info->memory_map.regions[0].type = MEMORY_AVAILABLE;
	boot_info->memory_map.regions[0].attributes = 0;

	/* Calculate totals */
	calculate_memory_totals(&boot_info->memory_map);

	/* Set empty command line for now */
	boot_info->cmdline.length = 0;
	boot_info->cmdline.cmdline[0] = '\0';

	return 0; /* Success */
}
#endif /* ENABLE_MULTIBOOT2 */

#ifdef ENABLE_UEFI
/* Parse UEFI information structure */
int parse_uefi_info(uint64_t image_handle, uint64_t system_table, boot_info_t *boot_info)
{
	/* For Stage 1, implement minimal parsing */
	/* This is a placeholder - in a real implementation, we would use UEFI services */
	(void)image_handle; /* Suppress unused parameter warning */
	(void)system_table; /* Suppress unused parameter warning */

	/* Set some default values for testing */
	boot_info->memory_map.num_regions = 1;
	boot_info->memory_map.max_regions = 128;
	boot_info->memory_map.regions[0].base_addr = 0x100000; /* 1MB */
	boot_info->memory_map.regions[0].length = 0x7F00000;   /* 127MB */
	boot_info->memory_map.regions[0].type = MEMORY_AVAILABLE;
	boot_info->memory_map.regions[0].attributes = 0;

	/* Calculate totals */
	calculate_memory_totals(&boot_info->memory_map);

	/* Set empty command line for now */
	boot_info->cmdline.length = 0;
	boot_info->cmdline.cmdline[0] = '\0';

	return 0; /* Success */
}
#endif /* ENABLE_UEFI */

/* Check if a memory region is usable */
int memory_region_is_usable(const memory_region_t *region)
{
	return (region->type == MEMORY_AVAILABLE);
}

/* Calculate memory totals from memory map */
void calculate_memory_totals(memory_map_t *memory_map)
{
	uint64_t total = 0;
	uint64_t usable = 0;

	for (uint32_t i = 0; i < memory_map->num_regions; i++) {
		const memory_region_t *region = &memory_map->regions[i];
		total += region->length;

		if (memory_region_is_usable(region)) {
			usable += region->length;
		}
	}

	memory_map->total_memory = total;
	memory_map->usable_memory = usable;
}

/* Get command line parameter by name */
const char *cmdline_get_param(const cmdline_info_t *cmdline, const char *param)
{
	/* For Stage 1, return NULL - parameter parsing not implemented yet */
	(void)cmdline;
	(void)param;
	return NULL;
}
