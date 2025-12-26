/*
 * FMI/OS x86_64 Boot Definitions
 * Stage 1: Multiboot2 and UEFI boot support
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

/* Boot protocol types */
typedef enum {
	BOOT_PROTOCOL_UNKNOWN = 0,
	BOOT_PROTOCOL_MULTIBOOT2 = 1,
	BOOT_PROTOCOL_UEFI = 2
} boot_protocol_t;

/* Memory region types */
typedef enum {
	MEMORY_AVAILABLE = 1,
	MEMORY_RESERVED = 2,
	MEMORY_ACPI_RECLAIMABLE = 3,
	MEMORY_ACPI_NVS = 4,
	MEMORY_BAD = 5,
	MEMORY_HOLE = 6
} memory_region_type_t;

/* Memory region structure */
struct memory_region_s {
	uint64_t base_addr;
	uint64_t length;
	memory_region_type_t type;
	uint32_t attributes;
};
typedef struct memory_region_s memory_region_t;

/* Memory map structure */
struct memory_map_s {
	uint32_t num_regions;
	uint32_t max_regions;
	memory_region_t regions[128]; /* Support up to 128 memory regions */
	uint64_t total_memory;
	uint64_t usable_memory;
};
typedef struct memory_map_s memory_map_t;

/* Command line information */
struct cmdline_info_s {
	char cmdline[4096]; /* Command line parameters */
	uint32_t length;
};
typedef struct cmdline_info_s cmdline_info_t;

/* CPU information */
struct cpu_info_s {
	uint32_t vendor_id[4];	    /* CPU vendor string */
	uint32_t brand_string[12];  /* CPU brand string */
	uint32_t features;	    /* CPU feature flags */
	uint32_t extended_features; /* Extended CPU features */
};
typedef struct cpu_info_s cpu_info_t;

/* Standardized boot information structure */
struct boot_info_s {
	boot_protocol_t protocol;
	memory_map_t memory_map;
	cmdline_info_t cmdline;
	cpu_info_t cpu_info;

	/* Protocol-specific data */
	union {
		struct {
			uint64_t multiboot_info_addr;
			uint32_t multiboot_magic;
		} multiboot2;

		struct {
			uint64_t system_table;
			uint64_t image_handle;
			uint64_t loaded_image_protocol;
		} uefi;
	} protocol_data;
};
typedef struct boot_info_s boot_info_t;

/* UEFI-specific definitions */
#define EFI_SUCCESS 0
#define EFI_LOAD_ERROR 1
#define EFI_INVALID_PARAMETER 2
#define EFI_UNSUPPORTED 3
#define EFI_BAD_BUFFER_SIZE 4
#define EFI_BUFFER_TOO_SMALL 5
#define EFI_NOT_READY 6
#define EFI_DEVICE_ERROR 7
#define EFI_WRITE_PROTECTED 8
#define EFI_OUT_OF_RESOURCES 9
#define EFI_VOLUME_CORRUPTED 10
#define EFI_VOLUME_FULL 11
#define EFI_NO_MEDIA 12
#define EFI_MEDIA_CHANGED 13
#define EFI_NOT_FOUND 14
#define EFI_ACCESS_DENIED 15
#define EFI_NO_RESPONSE 16
#define EFI_NO_MAPPING 17
#define EFI_TIMEOUT 18
#define EFI_NOT_STARTED 19
#define EFI_ALREADY_STARTED 20
#define EFI_ABORTED 21
#define EFI_ICMP_ERROR 22
#define EFI_TFTP_ERROR 23
#define EFI_PROTOCOL_ERROR 24

/* UEFI memory types */
#define EfiReservedMemoryType 0
#define EfiLoaderCode 1
#define EfiLoaderData 2
#define EfiBootServicesCode 3
#define EfiBootServicesData 4
#define EfiRuntimeServicesCode 5
#define EfiRuntimeServicesData 6
#define EfiConventionalMemory 7
#define EfiUnusableMemory 8
#define EfiACPIReclaimMemory 9
#define EfiACPIMemoryNVS 10
#define EfiMemoryMappedIO 11
#define EfiMemoryMappedIOPortSpace 12
#define EfiPalCode 13
#define EfiPersistentMemory 14
#define EfiMaxMemoryType 15

/* UEFI memory descriptor */
struct efi_memory_descriptor_s {
	uint32_t type;
	uint64_t physical_start;
	uint64_t virtual_start;
	uint64_t number_of_pages;
	uint64_t attribute;
};
typedef struct efi_memory_descriptor_s efi_memory_descriptor_t;

/* Boot entry points */
extern void _start(void);
#ifdef ENABLE_UEFI
extern void _start_uefi(void);
#endif

/* Boot-mode specific main functions */
#ifdef ENABLE_MULTIBOOT2
extern void mb2_main(uint32_t magic, uint64_t multiboot_info_addr);
#endif
#ifdef ENABLE_UEFI
extern void uefi_main(uint64_t image_handle, uint64_t system_table);
#endif

/* Kernel main entry point */
extern void kmain(boot_info_t *boot_info);

/* Boot information processing functions */
#ifdef ENABLE_MULTIBOOT2
extern int parse_multiboot2_info(uint64_t multiboot_info_addr, boot_info_t *boot_info);
#endif
#ifdef ENABLE_UEFI
extern int parse_uefi_info(uint64_t image_handle, uint64_t system_table, boot_info_t *boot_info);
#endif

/* Memory map utility functions */
extern int memory_region_is_usable(const memory_region_t *region);
extern void calculate_memory_totals(memory_map_t *memory_map);

/* Command line utility functions */
extern const char *cmdline_get_param(const cmdline_info_t *cmdline, const char *param);

#endif /* _BOOT_X86_64_H */
