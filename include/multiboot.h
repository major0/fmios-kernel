/*  Copyright (C) 1999,2003,2007,2008,2009,2010  Free Software Foundation, Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ANY
 *  DEVELOPER OR DISTRIBUTOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 *  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _MULTIBOOT_H
#define _MULTIBOOT_H

/* How many bytes from the start of the file we search for the header.  */
#define MULTIBOOT1_SEARCH			8192
#define MULTIBOOT2_SEARCH			32768
#define MULTIBOOT1_HEADER_ALIGN			4
#define MULTIBOOT2_HEADER_ALIGN			8

/* The magic field should contain this.  */
#define MULTIBOOT1_HEADER_MAGIC			0x1BADB002
#define MULTIBOOT2_HEADER_MAGIC			0xe85250d6

/* This should be in %eax.  */
#define MULTIBOOT1_BOOTLOADER_MAGIC		0x2BADB002
#define MULTIBOOT2_BOOTLOADER_MAGIC		0x36d76289

/* Alignment of multiboot modules.  */
#define MULTIBOOT_MOD_ALIGN			0x00001000

/* Alignment of the multiboot info structure.  */
#define MULTIBOOT1_INFO_ALIGN			0x00000004
#define MULTIBOOT2_INFO_ALIGN			0x00000008

/* Flags set in the 'flags' member of the multiboot header.  */

/* Align all boot modules on i386 page (4KB) boundaries.  */
#define MULTIBOOT1_PAGE_ALIGN			0x00000001

/* Must pass memory information to OS.  */
#define MULTIBOOT1_MEMORY_INFO			0x00000002

/* Must pass video information to OS.  */
#define MULTIBOOT1_VIDEO_MODE			0x00000004

/* This flag indicates the use of the address fields in the header.  */
#define MULTIBOOT1_AOUT_KLUDGE			0x00010000

/* Flags to be set in the 'flags' member of the multiboot info structure.  */

/* is there basic lower/upper memory information? */
#define MULTIBOOT1_INFO_MEMORY			0x00000001
/* is there a boot device set? */
#define MULTIBOOT1_INFO_BOOTDEV			0x00000002
/* is the command-line defined? */
#define MULTIBOOT1_INFO_CMDLINE			0x00000004
/* are there modules to do something with? */
#define MULTIBOOT1_INFO_MODS			0x00000008

/* These next two are mutually exclusive */

/* is there a symbol table loaded? */
#define MULTIBOOT1_INFO_AOUT_SYMS		0x00000010
/* is there an ELF section header table? */
#define MULTIBOOT1_INFO_ELF_SHDR		0X00000020

/* is there a full memory map? */
#define MULTIBOOT1_INFO_MEM_MAP			0x00000040

/* Is there drive info?  */
#define MULTIBOOT1_INFO_DRIVE_INFO		0x00000080

/* Is there a config table?  */
#define MULTIBOOT1_INFO_CONFIG_TABLE		0x00000100

/* Is there a boot loader name?  */
#define MULTIBOOT1_INFO_BOOT_LOADER_NAME	0x00000200

/* Is there a APM table?  */
#define MULTIBOOT1_INFO_APM_TABLE		0x00000400

/* Is there video information?  */
#define MULTIBOOT1_INFO_VBE_INFO	        0x00000800
#define MULTIBOOT1_INFO_FRAMEBUFFER_INFO        0x00001000

/* Flags set in the 'flags' member of the multiboot header.  */
#define MULTIBOOT2_TAG_ALIGN                  8
#define MULTIBOOT2_TAG_TYPE_END               0
#define MULTIBOOT2_TAG_TYPE_CMDLINE           1
#define MULTIBOOT2_TAG_TYPE_BOOT_LOADER_NAME  2
#define MULTIBOOT2_TAG_TYPE_MODULE            3
#define MULTIBOOT2_TAG_TYPE_BASIC_MEMINFO     4
#define MULTIBOOT2_TAG_TYPE_BOOTDEV           5
#define MULTIBOOT2_TAG_TYPE_MMAP              6
#define MULTIBOOT2_TAG_TYPE_VBE               7
#define MULTIBOOT2_TAG_TYPE_FRAMEBUFFER       8
#define MULTIBOOT2_TAG_TYPE_ELF_SECTIONS      9
#define MULTIBOOT2_TAG_TYPE_APM               10
#define MULTIBOOT2_TAG_TYPE_EFI32             11
#define MULTIBOOT2_TAG_TYPE_EFI64             12
#define MULTIBOOT2_TAG_TYPE_SMBIOS            13
#define MULTIBOOT2_TAG_TYPE_ACPI_OLD          14
#define MULTIBOOT2_TAG_TYPE_ACPI_NEW          15
#define MULTIBOOT2_TAG_TYPE_NETWORK           16

#define MULTIBOOT2_HEADER_TAG_END  0
#define MULTIBOOT2_HEADER_TAG_INFORMATION_REQUEST  1
#define MULTIBOOT2_HEADER_TAG_ADDRESS  2
#define MULTIBOOT2_HEADER_TAG_ENTRY_ADDRESS  3
#define MULTIBOOT2_HEADER_TAG_CONSOLE_FLAGS  4
#define MULTIBOOT2_HEADER_TAG_FRAMEBUFFER  5
#define MULTIBOOT2_HEADER_TAG_MODULE_ALIGN  6

#define MULTIBOOT2_ARCHITECTURE_I386  0
#define MULTIBOOT2_ARCHITECTURE_MIPS32  4
#define MULTIBOOT2_HEADER_TAG_OPTIONAL 1

#define MULTIBOOT2_CONSOLE_FLAGS_CONSOLE_REQUIRED 1
#define MULTIBOOT2_CONSOLE_FLAGS_EGA_TEXT_SUPPORTED 2

#ifndef __ASSEMBLY__

#include <stdint.h>

struct multiboot1_header
{
	/* Must be MULTIBOOT1_MAGIC - see above.  */
	uint32_t magic;

	/* Feature flags.  */
	uint32_t flags;

	/* The above fields plus this one must equal 0 mod 2^32. */
	uint32_t checksum;

	/* These are only valid if MULTIBOOT1_AOUT_KLUDGE is set.  */
	uint32_t header_addr;
	uint32_t load_addr;
	uint32_t load_end_addr;
	uint32_t bss_end_addr;
	uint32_t entry_addr;

	/* These are only valid if MULTIBOOT1_VIDEO_MODE is set.  */
	uint32_t mode_type;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
};

struct multiboot2_header
{
	/* Must be MULTIBOOT2_MAGIC - see above.  */
	uint32_t magic;

	/* ISA */
	uint32_t architecture;

	/* Total header length.  */
	uint32_t header_length;

	/* The above fields plus this one must equal 0 mod 2^32. */
	uint32_t checksum;
};


/* The symbol table for a.out.  */
struct multiboot1_aout_symbol_table
{
	uint32_t tabsize;
	uint32_t strsize;
	uint32_t addr;
	uint32_t reserved;
};
typedef struct multiboot1_aout_symbol_table multiboot1_aout_symbol_table_t;

/* The section header table for ELF.  */
struct multiboot1_section_header_table
{
	uint32_t num;
	uint32_t size;
	uint32_t addr;
	uint32_t shndx;
};
typedef struct multiboot1_section_header_table multiboot1_section_header_table_t;

struct multiboot1_info
{
	/* Multiboot info version number */
	uint32_t flags;

	/* Available memory from BIOS */
	uint32_t mem_lower;
	uint32_t mem_upper;

	/* "root" partition */
	uint32_t boot_device;

	/* Kernel command line */
	uint32_t cmdline;

	/* Boot-Module list */
	uint32_t mods_count;
	uint32_t mods_addr;

	union {
		multiboot1_aout_symbol_table_t sym;
		multiboot1_section_header_table_t sec;
	} u;

	/* Memory Mapping buffer */
	uint32_t mmap_length;
	uint32_t mmap_addr;

	/* Drive Info buffer */
	uint32_t drives_length;
	uint32_t drives_addr;

	/* ROM configuration table */
	uint32_t config_table;

	/* Boot Loader Name */
	uint32_t boot_loader_name;

	/* APM table */
	uint32_t apm_table;

	/* Video */
	uint32_t vbe_control_info;
	uint32_t vbe_mode_info;
	uint16_t vbe_mode;
	uint16_t vbe_interface_seg;
	uint16_t vbe_interface_off;
	uint16_t vbe_interface_len;

	uint64_t framebuffer_addr;
	uint32_t framebuffer_pitch;
	uint32_t framebuffer_width;
	uint32_t framebuffer_height;
	uint8_t framebuffer_bpp;
#define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED 0
#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB     1
#define MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT	2
	uint8_t framebuffer_type;
	union {
		struct {
			uint32_t framebuffer_palette_addr;
			uint16_t framebuffer_palette_num_colors;
		};
		struct {
		uint8_t framebuffer_red_field_position;
		uint8_t framebuffer_red_mask_size;
		uint8_t framebuffer_green_field_position;
		uint8_t framebuffer_green_mask_size;
		uint8_t framebuffer_blue_field_position;
		uint8_t framebuffer_blue_mask_size;
		};
	};
};
typedef struct multiboot1_info multiboot1_info_t;

struct multiboot2_header_tag
{
	uint16_t type;
	uint16_t flags;
	uint32_t size;
};

struct multiboot2_header_tag_information_request
{
	uint16_t type;
	uint16_t flags;
	uint32_t size;
	uint32_t requests[0];
};

struct multiboot2_header_tag_address
{
	uint16_t type;
	uint16_t flags;
	uint32_t size;
	uint32_t header_addr;
	uint32_t load_addr;
	uint32_t load_end_addr;
	uint32_t bss_end_addr;
};

struct multiboot2_header_tag_entry_address
{
	uint16_t type;
	uint16_t flags;
	uint32_t size;
	uint32_t entry_addr;
};

struct multiboot2_header_tag_console_flags
{
	uint16_t type;
	uint16_t flags;
	uint32_t size;
	uint32_t console_flags;
};

struct multiboot2_header_tag_framebuffer
{
	uint16_t type;
	uint16_t flags;
	uint32_t size;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
};

struct multiboot2_header_tag_module_align
{
	uint16_t type;
	uint16_t flags;
	uint32_t size;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
};

struct multiboot_color
{
	uint8_t red;
	uint8_t green;
	uint8_t blue;
};

#define MULTIBOOT_MEMORY_AVAILABLE		1
#define MULTIBOOT_MEMORY_RESERVED		2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE       3
#define MULTIBOOT_MEMORY_NVS                    4
#define MULTIBOOT_MEMORY_BADRAM                 5
struct multiboot1_mmap_entry
{
	uint32_t size;
	uint64_t addr;
	uint64_t len;
	uint32_t type;
} __attribute__((packed));
typedef struct multiboot1_mmap_entry multiboot1_memory_map_t;

struct multiboot1_mod_list
{
	/* the memory used goes from bytes 'mod_start' to 'mod_end-1' inclusive */
	uint32_t mod_start;
	uint32_t mod_end;

	/* Module command line */
	uint32_t cmdline;

	/* padding to take it to 16 bytes (must be zero) */
	uint32_t pad;
};
typedef struct multiboot1_mod_list multiboot1_module_t;

/* APM BIOS info.  */
struct multiboot1_apm_info
{
	uint16_t version;
	uint16_t cseg;
	uint32_t offset;
	uint16_t cseg_16;
	uint16_t dseg;
	uint16_t flags;
	uint16_t cseg_len;
	uint16_t cseg_16_len;
	uint16_t dseg_len;
};


struct multiboot2_mmap_entry
{
	uint64_t addr;
	uint64_t len;
	uint32_t type;
	uint32_t zero;
} __attribute__((packed));
typedef struct multiboot2_mmap_entry multiboot2_memory_map_t;

struct multiboot2_tag
{
	uint32_t type;
	uint32_t size;
};

struct multiboot2_tag_string
{
	uint32_t type;
	uint32_t size;
	char string[0];
};

struct multiboot2_tag_module
{
	uint32_t type;
	uint32_t size;
	uint32_t mod_start;
	uint32_t mod_end;
	char cmdline[0];
};

struct multiboot2_tag_basic_meminfo
{
	uint32_t type;
	uint32_t size;
	uint32_t mem_lower;
	uint32_t mem_upper;
};

struct multiboot2_tag_bootdev
{
	uint32_t type;
	uint32_t size;
	uint32_t biosdev;
	uint32_t slice;
	uint32_t part;
};

struct multiboot2_tag_mmap
{
	uint32_t type;
	uint32_t size;
	uint32_t entry_size;
	uint32_t entry_version;
	struct multiboot2_mmap_entry entries[0];
};

struct multiboot2_vbe_info_block
{
	uint8_t external_specification[512];
};

struct multiboot2_vbe_mode_info_block
{
	uint8_t external_specification[256];
};

struct multiboot2_tag_vbe
{
	uint32_t type;
	uint32_t size;

	uint16_t vbe_mode;
	uint16_t vbe_interface_seg;
	uint16_t vbe_interface_off;
	uint16_t vbe_interface_len;

	struct multiboot2_vbe_info_block vbe_control_info;
	struct multiboot2_vbe_mode_info_block vbe_mode_info;
};

struct multiboot2_tag_framebuffer_common
{
	uint32_t type;
	uint32_t size;

	uint64_t framebuffer_addr;
	uint32_t framebuffer_pitch;
	uint32_t framebuffer_width;
	uint32_t framebuffer_height;
	uint8_t framebuffer_bpp;
#define MULTIBOOT2_FRAMEBUFFER_TYPE_INDEXED 0
#define MULTIBOOT2_FRAMEBUFFER_TYPE_RGB     1
#define MULTIBOOT2_FRAMEBUFFER_TYPE_EGA_TEXT	2
	uint8_t framebuffer_type;
	uint16_t reserved;
};

struct multiboot2_tag_framebuffer
{
	struct multiboot2_tag_framebuffer_common common;

	union {
		struct {
			uint16_t framebuffer_palette_num_colors;
			struct multiboot_color framebuffer_palette[0];
		};
		struct {
			uint8_t framebuffer_red_field_position;
			uint8_t framebuffer_red_mask_size;
			uint8_t framebuffer_green_field_position;
			uint8_t framebuffer_green_mask_size;
			uint8_t framebuffer_blue_field_position;
			uint8_t framebuffer_blue_mask_size;
		};
	};
};

struct multiboot2_tag_elf_sections
{
	uint32_t type;
	uint32_t size;
	uint32_t num;
	uint32_t entsize;
	uint32_t shndx;
	char sections[0];
};

struct multiboot2_tag_apm
{
	uint32_t type;
	uint32_t size;
	uint16_t version;
	uint16_t cseg;
	uint32_t offset;
	uint16_t cseg_16;
	uint16_t dseg;
	uint16_t flags;
	uint16_t cseg_len;
	uint16_t cseg_16_len;
	uint16_t dseg_len;
};

struct multiboot2_tag_efi32
{
	uint32_t type;
	uint32_t size;
	uint32_t pointer;
};

struct multiboot2_tag_efi64
{
	uint32_t type;
	uint32_t size;
	uint64_t pointer;
};

struct multiboot2_tag_smbios
{
	uint32_t type;
	uint32_t size;
	uint8_t major;
	uint8_t minor;
	uint8_t reserved[6];
	uint8_t tables[0];
};

struct multiboot2_tag_old_acpi
{
	uint32_t type;
	uint32_t size;
	uint8_t rsdp[0];
};

struct multiboot2_tag_new_acpi
{
	uint32_t type;
	uint32_t size;
	uint8_t rsdp[0];
};

struct multiboot2_tag_network
{
	uint32_t type;
	uint32_t size;
	uint8_t dhcpack[0];
};

#endif /* __ASSEMBLY__ */

#endif /* MULTIBOOT_H */
