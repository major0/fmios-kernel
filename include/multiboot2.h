/*  multiboot2.h - Multiboot 2 header file.  */
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
#ifndef _MULTIBOOT2_H
#define _MULTIBOOT2_H

/* How many bytes from the start of the file we search for the header.  */
#define MULTIBOOT2_SEARCH			32768
#define MULTIBOOT2_HEADER_ALIGN			8

/* The magic field should contain this.  */
#define MULTIBOOT2_HEADER_MAGIC			0xe85250d6

/* This should be in %eax.  */
#define MULTIBOOT2_BOOTLOADER_MAGIC		0x36d76289

/* Alignment of multiboot modules.  */
#define MULTIBOOT2_MOD_ALIGN			0x00001000

/* Alignment of the multiboot info structure.  */
#define MULTIBOOT2_INFO_ALIGN			0x00000008

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

struct multiboot2_color
{
	uint8_t red;
	uint8_t green;
	uint8_t blue;
};

struct multiboot2_mmap_entry
{
	uint64_t addr;
	uint64_t len;
#define MULTIBOOT2_MEMORY_AVAILABLE		1
#define MULTIBOOT2_MEMORY_RESERVED		2
#define MULTIBOOT2_MEMORY_ACPI_RECLAIMABLE       3
#define MULTIBOOT2_MEMORY_NVS                    4
#define MULTIBOOT2_MEMORY_BADRAM                 5
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
			struct multiboot2_color framebuffer_palette[0];
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

#endif /* MULTIBOOT2_H */
