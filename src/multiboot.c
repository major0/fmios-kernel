/* multiboot.c - Multiboot utilities and initialization */
/* Copyright (C) 2012 Mark Ferrell
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ANY
 * DEVELOPER OR DISTRIBUTOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/**
 * Before initialization begins our configuration assumes the most basic of
 * settings.  Currently these settings reflect some x86 assumptions, in
 * particular the video address is fundementally x86 specific, though the
 * current code should make it eassy to abstract out alternate defaults.
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
#include <fmios/serial.h>
#include <fmios/video.h>

/* FIXME we need to put this somewhere useful */
#define NULL 0

struct multiboot_tag *mb_tag_find(unsigned long addr, uint16_t type)
{
	struct multiboot_tag *tag = (struct multiboot_tag *)(addr + 8);
	while (tag->type != MULTIBOOT_TAG_TYPE_END) {
		if (tag->type == type) {
			return tag;
		}
		tag = (struct multiboot_tag *)((uint8_t *)tag+((tag->size+7)&~7));
	}
	return NULL;
}

struct multiboot_tag *mb_tag_next(struct multiboot_tag *tag)
{
	tag = (struct multiboot_tag *)((uint8_t *)tag+((tag->size+7)&~7));
	if (tag->type == MULTIBOOT_TAG_TYPE_END) {
		return NULL;
	}
	return tag;
}

char *mb_cmdline_find(char *option, char *cmdline)
{
	int len;
	char *param;

	if (!option || !cmdline) {
		return NULL;
	}

	if (!strlen(option) || !strlen(cmdline)) {
		return NULL;
	}

	len = strlen(option);

	while (*cmdline != '\0') {

		if (*cmdline == ' ') {
			cmdline++;
			continue;
		}

		if (strncmp(option, cmdline, len) == 0) {
			cmdline += len;
			param = cmdline;

			if (*param == '=') {
				return ++param;
			}
		}

		while (*cmdline != '\0' && *cmdline != ' ') {
			cmdline++;
		}
	}

	return NULL;
}

static void mb_init_video(char *cmdline, struct video_config *config)
{
	char *param;

	param = mb_cmdline_find("video", cmdline);

	/* cmdline param will be in the format of:
	 * [addr|ega|cga|off][,<height>x<width>[,bpp]]]
	 */
	if (param) {
		if (strncmp("off", param, 3) == 0) {
			return;
		}

		if (strncmp("ega", param, 3) == 0
		 || strncmp("cga", param, 3) == 0) {
			config->type = MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT;
			config->addr = 0xb8000;
			param += 3;
		} else {
			config->addr = strtol(param, &param, 0);
		}

		if (*param == ',') {
			config->height = strtol(++param, &param, 0);
			config->width = strtol(++param, &param, 0);
			if (*param == ',') {
				config->depth = strtol(++param, &param, 0);
			}
		}
	}

	/* FIXME This should be filled in with other video types later */
	switch (config->type) {
	case MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT:
		ega_init(config->addr, config->width, config->height);
		break;
	}
}

static void mb_init_serial(char *cmdline)
{
	char *param;
	uint32_t iobase = 0;
	uint32_t baud = 0;
	uint16_t divisor = 0;
	uint8_t flags = 0;

	param = mb_cmdline_find("serial", cmdline);

	/* cmdline param will be in the format of:
	 * [iobase][,baud][,flags][,divisor]
	 */
	if (!param) {
		return;
	}

	if (strncmp("off", param, 3) == 0) {
		return;
	}

	iobase = strtol(param, &param, 0);

	if (*param == ',') {
		baud = strtol(++param, &param, 0);
	}

	if (*param == ',') {
		++param;
		switch (*param) {
		case '8':
			flags = SERIAL_8BIT;
			param++;
			break;
		case '7':
			flags = SERIAL_7BIT;
			param++;
			break;
		case '6':
			flags = SERIAL_6BIT;
			param++;
			break;
		case '5':
			flags = SERIAL_5BIT;
			param++;
			break;
		}

		if (flags) {
			switch (*param) {
			case 'e':
			case 'E':
				flags |= SERIAL_PARITY_EVEN;
				param++;
				break;
			case 'o':
			case 'O':
				flags |= SERIAL_PARITY_ODD;
				param++;
				break;
			case 'n':
			case 'N':
				flags |= SERIAL_PARITY_NONE;
				param++;
				break;
			}

			switch (*param) {
			case '2':
				flags |= SERIAL_STOP2;
				param++;
				break;
			case '1':
				flags |= SERIAL_STOP1;
				param++;
				break;
			}
		}
	}

	if (*param == ',') {
		divisor = strtol(param, &param, 0);
	}

	serial_init(iobase, baud, flags, divisor);
}


/* This is /almost/ a 1:1 mapping from the Multiboot2 mmap_entry with the
 * exception that the reserved element is replaced with 'flags' to be used as a
 * bitfield to track memory that is 'in use' */
struct mmap_entry {
	uint64_t	addr;
	uint64_t	len;
	uint32_t	type;
	uint32_t	flags;
};
#define MEMORY_MAP_USED 1

static void * mb2_map_mem(unsigned long addr, struct mmap_entry *mmap)
{
	struct multiboot_tag *tag;
	multiboot_memory_map_t *mb_mmap;

	tag = mb_tag_find(addr, MULTIBOOT_TAG_TYPE_MMAP);

	if (!tag) {
		struct multiboot_tag_basic_meminfo *meminfo;

		/* Construct a mapping from oldschool upper/lower memory
		 * information */
		tag = mb_tag_find(addr, MULTIBOOT_TAG_TYPE_BASIC_MEMINFO);

		if (!tag) {
			return NULL;
		}

		meminfo = (struct multiboot_tag_basic_meminfo *)tag;

		mmap[0].addr = 0;
		mmap[0].len = meminfo->mem_lower * 1024;
		mmap[0].type = MULTIBOOT_MEMORY_AVAILABLE;
		mmap[0].flags = 0;

		mmap[1].addr = 1024 * 1024;
		mmap[1].len = meminfo->mem_upper * 1024;
		mmap[1].type = MULTIBOOT_MEMORY_AVAILABLE;
		mmap[1].flags = 0;
	} else {
		int index = 0;

		/* Convert the multiboot2 memory map to generic format */
		for (mb_mmap = ((struct multiboot_tag_mmap *)tag)->entries;
		     (uint8_t *)mb_mmap < (uint8_t *)tag + tag->size;
		     mb_mmap = (multiboot_memory_map_t *)((uint32_t)mb_mmap
		     + ((struct multiboot_tag_mmap *)tag)->entry_size)) {

			mmap[index].addr = mb_mmap->addr;
			mmap[index].len = mb_mmap->len;
			mmap[index].type = mb_mmap->type;
			mmap[index].flags = 0;

			index++;
		}
	}

	return mmap;
}

static void * mb2_map_used(unsigned long addr, struct mmap_entry *used)
{
	struct multiboot_tag *tag;

	/* multiboot2 lays all the multiboot data out in a single large
	 * run of data */
	/* FIXME this does not include structures MB2 points at.  Technically
	 * the memory is contiguous, but we have no idea where it actually ends */
	used[0].addr = addr;
	used[0].len = *(uint32_t *)addr;
	used[0].type = MULTIBOOT_MEMORY_AVAILABLE;
	used[0].flags = MEMORY_MAP_USED;

	/* Find any module data */
	tag = mb_tag_find(addr, MULTIBOOT_TAG_TYPE_MODULE);
	for (;tag; tag = mb_tag_next(tag)) {
		struct multiboot_tag_module *tag_module = (struct multiboot_tag_module *)tag;

		/* FIXME this can potentially flag regions of memory as 'used'
		 * even though there may exist unused holes inbetween the
		 * modules */
		if (used[1].flags == 0) {
			used[1].addr = tag_module->mod_start;
			used[1].len = tag_module->mod_end - used[1].addr;
			used[1].type = MULTIBOOT_MEMORY_AVAILABLE;
			used[1].flags = MEMORY_MAP_USED;
		} else {
			if (used[1].addr > tag_module->mod_start) {
				used[1].addr = tag_module->mod_start;
			} else if (tag_module->mod_end > (used[1].addr + used[1].len)) {
				used[1].len = tag_module->mod_end - used[1].addr;
			}
		}
	}

	return used;
}

/* The multiboot specification gives us information about available memory, but
 * does not supply information as to which regions of memory are currently
 * used. According to the current maintainers this is a feature.  This means
 * that OS developers must go through the tedious work of figuring out what
 * regions are in use by examining all the known data in memory and building a
 * new map. Perhaps more annoying is that Multiboot2 fixes the mmap alignment
 * issues by removing the 32bit element from the start of the entry structure
 * found in multiboot1 (making it incompatible with multiboot1) yet it retains
 * the multiboot1 behavior of using numeric types in the type element as
 * opposed to using a bitfield to indicate a ranges behavior. The later would
 * have allowed a range to be flagged as usable ram while also flagging it to
 * be in-use.  The current implemention in Multiboot2 unfortunately can not
 * flag memory ranges this way without breaking compatibility with previous
 * Multiboot2 specifications. */
static struct mmap_entry * mb2_mmap(unsigned long magic,
		unsigned long addr)
{
	struct mmap_entry used[8];
	struct mmap_entry mmap[8];
	int index;

	memset(&used, 0, sizeof(struct mmap_entry) * 8);
	memset(&mmap, 0, sizeof(struct mmap_entry) * 8);

	if (!mb2_map_used(addr, used)) {
		return NULL;
	}

	if (!mb2_map_mem(addr, mmap)) {
		return NULL;
	}

	/* TODO Add the kernel mapping to used memory and merge the regions */
	/* FIXME Multiboot only supplies the kernel's start/end if the format
	 * is AOUT, we must rely on a linker script to locate the used regions
	 * for ELF, luckily this information stays constant regardless of the
	 * multiboot format */

	return NULL;
}

#ifdef CONFIG_ENABLE_MULTIBOOT1
static void * mb1_map_mem(unsigned long addr, struct mmap_entry *mmap)
{
	multiboot1_info_t *mbi = (multiboot1_info_t *)addr;
	multiboot1_memory_map_t *mb_mmap;

	if (!(mbi->flags & MULTIBOOT1_INFO_MEM_MAP)) {
		/* Construct a mapping from oldschool upper/lower memory
		 * information */
		if (!(mbi->flags & MULTIBOOT1_INFO_MEMORY)) {
			return NULL;
		}

		mmap[0].addr = 0;
		mmap[0].len = mbi->mem_lower * 1024;
		mmap[0].type = MULTIBOOT_MEMORY_AVAILABLE;
		mmap[0].flags = 0;

		mmap[1].addr = 1024 * 1024;
		mmap[1].len = mbi->mem_upper * 1024;
		mmap[1].type = MULTIBOOT_MEMORY_AVAILABLE;
		mmap[1].flags = 0;
	} else {
		/* Convert the multiboot memory map to generic format */
		int index = 0;
		for (mb_mmap = (multiboot1_memory_map_t *) mbi->mmap_addr;
		     (uint32_t) mb_mmap < mbi->mmap_addr + mbi->mmap_length;
		     mb_mmap = (multiboot1_memory_map_t *) ((uint32_t) mb_mmap
		     + mb_mmap->size + sizeof (mb_mmap->size))) {
			mmap[index].addr = mb_mmap->addr;
			mmap[index].len = mb_mmap->len;
			mmap[index].type = mb_mmap->type;
			mmap[index].flags = 0;
			index++;
		}
	}
	return mmap;
}

static void * mb1_map_used(unsigned long addr, struct mmap_entry *used)
{
	multiboot1_info_t *mbi = (multiboot1_info_t *)addr;
	multiboot1_memory_map_t *mmap;

	/* Flag the Multiboot header region as in use */
	/* FIXME this does not include structures in memory that
	 * multiboot points at, such as the the module structure, the
	 * framebuffer structures, etc.. */
	used[0].addr = addr;
	used[0].len = sizeof(struct multiboot1_header);
	used[0].type = MULTIBOOT_MEMORY_AVAILABLE;
	used[0].flags = MEMORY_MAP_USED;

	/* Construct a mapping of used memory from the module list */
	/* FIXME validate the modules are contiguous */
	if (mbi->flags & MULTIBOOT1_INFO_MODS && mbi->mods_count != 0) {
		multiboot1_module_t *mod;

		mod = (multiboot1_module_t *) mbi->mods_addr;
		used[1].addr = mod->mod_start;
		mod = (multiboot1_module_t *) mbi->mods_addr + (mbi->mods_count - 1);
		used[1].len = mod->mod_end - used[1].addr;
		used[1].type = MULTIBOOT_MEMORY_AVAILABLE;
		used[1].flags = MEMORY_MAP_USED;
	}

	return used;
}

static struct mmap_entry * mb1_mmap(unsigned long magic,
		unsigned long addr)
{
	struct mmap_entry used[8];
	struct mmap_entry mmap[8];
	int index;

	memset(&used, 0, sizeof(struct mmap_entry) * 8);
	memset(&mmap, 0, sizeof(struct mmap_entry) * 8);

	if (!mb1_map_used(addr, used)) {
			return NULL;
	}

	if (!mb1_map_mem(addr, mmap)) {
		return;
	}

	/* TODO Add the kernel mapping to used memory and merge the regions */
	/* FIXME Multiboot only supplies the kernel's start/end if the format
	 * is AOUT, we must rely on a linker script to locate the used regions
	 * for ELF, luckily this information stays constant regardless of the
	 * multiboot format */

	return mmap;
}
#endif

/** OS independant initialization
 * @magic Multiboot magic number
 * @addr Address of Multiboot Information Structure
 * Parse the Multiboot Information and initialize the system */
void mb_init(unsigned long magic, unsigned long addr)
{
	struct mmap_entry *mmap;
	char *cmdline = "";
	int index = 0;

	if (addr & 7) {
		printk("Unaligned mbi: 0x%x\n", addr);
		return;
	}

#ifdef CONFIG_ENABLE_MULTIBOOT1
	if (magic == MULTIBOOT1_BOOTLOADER_MAGIC) {
		if (mbi->flags & MULTIBOOT1_INFO_CMDLINE) {
			cmdline = mbi->cmdline;
		}
		mb1_init_serial(cmdline);

		if (mbi->flags & MULTIBOOT1_INFO_FRAMEBUFFER_INFO) {
			struct video_config fb;

			memset(&fb, 0, sizeif(struct video_config));

			fb.addr = mbi->framebuffer_addr;
			fb.height = mbi->framebuffer_height;
			fb.width = mbi->framebuffer_width;
			fb.type = mbi->framebuffer_type;
			fb.depth = mbi->framebuffer_bpp;

			mb_video_init(cmdline, &fb);
		}

		mmap = mb2_mmap(magic, addr);
	} else
#endif
	if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
		printk("Invalid magic number: 0x%x\n", (unsigned)magic);
		return;
	} else {
		struct multiboot_tag *tag;

		tag = mb_tag_find(addr, MULTIBOOT_TAG_TYPE_CMDLINE);
		if (tag) {
			struct multiboot_tag_string *tag_string
				= (struct multiboot_tag_string *)tag;
			mb_init_serial(tag_string->string);
		}

		tag = mb_tag_find(addr, MULTIBOOT_TAG_TYPE_FRAMEBUFFER);
		if (tag) {
			struct video_config fb;
			struct multiboot_tag_framebuffer *tagfb
				= (struct multiboot_tag_framebuffer *)tag;

			memset(&fb, 0, sizeof(struct video_config));
			fb.type = tagfb->common.framebuffer_type;
			fb.addr = tagfb->common.framebuffer_addr;
			fb.height = tagfb->common.framebuffer_height;
			fb.width = tagfb->common.framebuffer_width;
			fb.depth = tagfb->common.framebuffer_bpp;

			mb_init_video(cmdline, &fb);
		}

		mmap = mb2_mmap(magic, addr);
	}

	for (index = 0; index < 8; index++) {
		if (!mmap[index].type) {
			continue;
		}
		printk ("MMAP[%d]: addr=0x%x%x, length=0x%x%x, type=0x%x, flags=0x%x\n",
				index,
				(uint32_t) (mmap[index].addr >> 32),
				(uint32_t) (mmap[index].addr),
				(uint32_t) (mmap[index].len >> 32),
				(uint32_t) (mmap[index].len),
				mmap[index].type,
				mmap[index].flags);
	}

	/* TODO init_mem(mmap); */
}
