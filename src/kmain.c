/* Copyright (C) 1999  Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/* Before initialization begins our configuration assumes the most basic of
 * settings.  Currently these settings reflect some x86 assumptions, in
 * particular the video address is fundementally x86 specific.
 * serial=off
 * video=0xb8000
 *
 * Within limts the configuration options can be modified before fully
 * initialization of data comming from the Multiboot. Currently the only
 * setting that can be altered are video and serial.  For most situations only
 * the serial would need to be modified unless explicitly disabling a given
 * output. All configuration variables can be disabled by setting them to
 * 'off'.  Other settings are as * follows:
 *
 * video=[off|<addr>][,<height>,<width>,<bpp>]
 * serial=[off|<addr>][.<baud><bits><stopbit>[,<divisor>]
 */
#include <multiboot.h>

static void multiboot1_init(uint32_t addr)
{
	multiboot1_info_t *mbi = (multiboot1_info_t *)addr;

	printk("MULTIBOOT1\n");

	if (mbi->flags & MULTIBOOT1_INFO_CMDLINE
	    && strlen((char *)mbi->cmdline)) {
		printk("KERNEL: cmdline=%s\n", (char *)mbi->cmdline);
	}

	/* Are mods_* valid? Be careful as some platforms flag an empty module
	 * list (aka QEMU) */
	if (mbi->flags & MULTIBOOT1_INFO_MODS && mbi->mods_count != 0) {
		multiboot1_module_t *mod;
		int i;

		printk("MODULES: count=%d, addr=0x%x\n",
				(int) mbi->mods_count,
				(int) mbi->mods_addr);

		for (i = 0, mod = (multiboot1_module_t *) mbi->mods_addr;
				i < mbi->mods_count; i++, mod++) {
			printk ("  start=0x%x, end=0x%x, cmdline=%s\n",
					(unsigned) mod->mod_start,
					(unsigned) mod->mod_end,
					(char *) mod->cmdline);
		}
	}

	if (mbi->flags & MULTIBOOT1_INFO_AOUT_SYMS) {
		printk("AOUT: not supported.\n");
		return;
	}

	/* Are mmap_* valid? */
	if (mbi->flags & MULTIBOOT1_INFO_MEM_MAP) {
		multiboot1_memory_map_t *mmap;

		/* FIXME This is a mess */
		for (mmap = (multiboot1_memory_map_t *) mbi->mmap_addr;
		     (uint32_t) mmap < mbi->mmap_addr + mbi->mmap_length;
		     mmap = (multiboot1_memory_map_t *) ((uint32_t) mmap
		     + mmap->size + sizeof (mmap->size))) {
			printk ("MMAP: addr=0x%x%x, length=0x%x%x, type=0x%x\n",
					(uint32_t) (mmap->addr >> 32),
					(uint32_t) (mmap->addr),
					(uint32_t) (mmap->len >> 32),
					(uint32_t) mmap->len,
					mmap->type);
		}
	}

#if 0
	if (mbi->flags & MULTIBOOT1_INFO_BOOT_LOADER_NAME) {
		printk("LOADER: name=%s\n", (char *)mbi->boot_loader_name);
	}

	/* Is the section header table of ELF valid? */
	if (mbi->flags & MULTIBOOT1_INFO_ELF_SHDR) {
		multiboot_section_header_table_t *multiboot_sec = &(mbi->u.sec);

		printk ("ELF: num=%u, size=0x%x,"
			" addr=0x%x, shndx=0x%x\n",
				(unsigned)multiboot_sec->num,
				(unsigned)multiboot_sec->size,
				(unsigned)multiboot_sec->addr,
				(unsigned)multiboot_sec->shndx);
	}

	if (mbi->flags & MULTIBOOT1_INFO_MEMORY) {
		printk("MEM: lower=%uKB, upper=%uKB\n",
				(unsigned)mbi->mem_lower,
				(unsigned)mbi->mem_upper);
	}

	if (mbi->flags & MULTIBOOT1_INFO_BOOTDEV) {
		uint8_t bootdev = (uint8_t)(mbi->boot_device >> 24) & 0xff;
		uint8_t slice = (uint8_t)(mbi->boot_device >> 16) & 0xff;
		uint8_t part = (uint8_t)(mbi->boot_device >> 8) & 0xff;
		printk("BOOTDEV: bios=0x%x, slice=%d, part=%d\n", bootdev,
			(slice == 0xff) ? -1 : slice,
			(part == 0xff) ? -1 : part);
	}
#endif

	if (mbi->flags & MULTIBOOT1_INFO_FRAMEBUFFER_INFO) {
		printk("FB: addr=0x%x, width=%u, height=%u, depth=%u,",
				(uint32_t)mbi->framebuffer_addr,
				(uint32_t)mbi->framebuffer_width,
				(uint32_t)mbi->framebuffer_height,
				(uint32_t)mbi->framebuffer_bpp);
		switch ((uint32_t)mbi->framebuffer_type) {
		case MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED:
			printk(" type=indexed\n");
			break;
		case MULTIBOOT_FRAMEBUFFER_TYPE_RGB:
			printk(" type=rgb\n");
			break;
		case MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT:
			printk(" type=ega\n");
			break;
		default:
			printk(" type=unknown\n");
			break;
		}
	}
#if 0
	/* FIXME Can we use this information? */
	if (mbi->flags & MULTIBOOT1_INFO_DRIVE_INFO) {
		printk("DRIVES: addr=0x%x <unused>\n",
				(uint32_t)mbi->drives_addr);
	}
	if (mbi->flags & MULTIBOOT1_INFO_CONFIG_TABLE) {
		printk("ROM CONFIG: addr=0x%x <unused>\n",
				(uint32_t)mbi->config_table);
	}
	if (mbi->flags & MULTIBOOT1_INFO_APM_TABLE) {
		printk("APM: addr=0x%x <unused>\n",
				(uint32_t)mbi->apm_table);
	}
	if (mbi->flags & MULTIBOOT1_INFO_VBE_INFO) {
		printk("VBE: addr=0x%x <unused>\n",
				(uint32_t)mbi->vbe_control_info);
	}
#endif
}

static void multiboot2_init_mmap(struct multiboot2_tag *tag)
{
	multiboot2_memory_map_t *mmap;

	for (mmap = ((struct multiboot2_tag_mmap *)tag)->entries;
	     (uint8_t *)mmap < (uint8_t *)tag + tag->size;
	     mmap = (multiboot2_memory_map_t *)((uint32_t)mmap
	     + ((struct multiboot2_tag_mmap *)tag)->entry_size)) {
		printk("MMAP: addr=0x%x%x, length=0x%x%x, type=0x%x\n",
			(unsigned)(mmap->addr >> 32),
			(unsigned)(mmap->addr & 0xffffffff),
			(unsigned)(mmap->len >> 32),
			(unsigned)(mmap->len & 0xffffffff),
			(unsigned)(mmap->type));
	}
}

static void multiboot2_init_fb(struct multiboot2_tag *tag)
{
	struct multiboot2_tag_framebuffer *tagfb
		= (struct multiboot2_tag_framebuffer *)tag;
	void *fb = (void *)(unsigned long)tagfb->common.framebuffer_addr;

	printk("FB: addr=0x%x, width=%u, height=%u, depth=%u,",
		fb,
		tagfb->common.framebuffer_width,
		tagfb->common.framebuffer_height,
		tagfb->common.framebuffer_bpp);

	switch (tagfb->common.framebuffer_type) {
	case MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED:
		printk(" type=indexed\n");
		break;

	case MULTIBOOT_FRAMEBUFFER_TYPE_RGB:
		printk(" type=rgb\n");
		break;

	case MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT:
		printk(" type=ega\n");
		break;

	default:
		printk(" type=unknown\n");
		break;
	}
}

static void multiboot2_init(uint32_t addr)
{
	struct multiboot2_tag *tag;
	uint32_t size = *(uint32_t *)addr;

	printk("MULTIBOOT2\n");

	for (tag = (struct multiboot2_tag *) (addr + 8);
	     tag->type != MULTIBOOT2_TAG_TYPE_END;
	     tag = (struct multiboot2_tag *)((uint8_t *)tag + ((tag->size + 7) & ~7))) {
		struct multiboot2_tag_string *tag_string = (struct multiboot2_tag_string *)tag;
		struct multiboot2_tag_module *tag_module = (struct multiboot2_tag_module *)tag;
		struct multiboot2_tag_basic_meminfo *tag_meminfo = (struct multiboot2_tag_basic_meminfo *)tag;
		struct multiboot2_tag_bootdev *tag_bootdev = (struct multiboot2_tag_bootdev *)tag;
		struct multiboot2_tag_elf_sections *tag_elf = (struct multiboot2_tag_elf_sections *)tag;

		switch (tag->type) {
		case MULTIBOOT2_TAG_TYPE_CMDLINE:
			if (strlen(tag_string->string)) {
				printk("KERNEL: cmdline=%s\n", tag_string->string);
			}
			break;
		case MULTIBOOT2_TAG_TYPE_FRAMEBUFFER:
			multiboot2_init_fb(tag);
			break;
		case MULTIBOOT2_TAG_TYPE_MMAP:
			multiboot2_init_mmap(tag);
			break;
		case MULTIBOOT2_TAG_TYPE_MODULE:
			printk("MODULE: start=0x%x, end=0x%x\n, cmdline=%s\n",
				tag_module->mod_start, tag_module->mod_end,
				tag_module->cmdline);
			break;
#if 0
		case MULTIBOOT2_TAG_TYPE_BOOT_LOADER_NAME:
			printk("LOADER: name=%s\n", tag_string->string);
			break;
		case MULTIBOOT2_TAG_TYPE_BOOTDEV:
			printk("BOOTDEV: bios=0x%x, slice=%d, part=%d\n",
				tag_bootdev->biosdev, tag_bootdev->slice,
				tag_bootdev->part);
			break;
		case MULTIBOOT2_TAG_TYPE_BASIC_MEMINFO:
			printk("MEM: lower=%uKB, upper=%uKB\n",
				tag_meminfo->mem_lower,
				tag_meminfo->mem_upper);
			break;
		case MULTIBOOT2_TAG_TYPE_ELF_SECTIONS:
			printk("ELF: num=%u, size=0x%x, addr=0x%x, shndx=0x%x\n",
				tag_elf->num, tag_elf->size,
				tag_elf->sections, tag_elf->shndx);
			break;
		case MULTIBOOT2_TAG_TYPE_VBE:
			printk("VBE: <unused>\n");
			break;
		case MULTIBOOT2_TAG_TYPE_APM:
			printk("APM: <unused>\n");
			break;
		case MULTIBOOT2_TAG_TYPE_EFI32:
			printk("EFI32: <unused>\n");
			break;
		case MULTIBOOT2_TAG_TYPE_EFI64:
			printk("EFI64: <unused>\n");
			break;
		case MULTIBOOT2_TAG_TYPE_SMBIOS:
			printk("SMBIOS: <unused>\n");
			break;
		case MULTIBOOT2_TAG_TYPE_ACPI_OLD:
			printk("ACPI (old): <unused>\n");
			break;
		case MULTIBOOT2_TAG_TYPE_ACPI_NEW:
			printk("ACPI: <unused>\n");
			break;
#endif
		}
	}

	tag = (struct multiboot2_tag *)((uint8_t *)tag + ((tag->size + 7) & ~7));
	if (size != ((unsigned)tag - addr)) {
		printk("ERROR: size mismatch 0x%x != 0x%x\n",
				size, (unsigned)tag - addr);
	}
}

/** OS independant initialization
 * @magic Multiboot magic number
 * @addr Address of Multiboot Information Structure
 * Parse the Multiboot Information and initialize the system */
void kmain (unsigned long magic, unsigned long addr)
{
	if (magic != MULTIBOOT1_BOOTLOADER_MAGIC &&
	    magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
		printk("Invalid magic number: 0x%x\n", (unsigned)magic);
		return;
	}

	if (addr & 7) {
		printk("Unaligned mbi: 0x%x\n", addr);
		return;
	}

	if (magic == MULTIBOOT1_BOOTLOADER_MAGIC) {
		multiboot1_init(addr);
		return;
	}

	if (magic == MULTIBOOT2_BOOTLOADER_MAGIC) {
		multiboot2_init(addr);
		return;
	}
}
