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
#include <fmios/fmios.h>
#include <fmios/types.h>
#include <fmios/page.h>
#include <fmios/serial.h>
#include <fmios/video.h>
#include <fmios/io.h>
#include <multiboot.h>
#include <string.h>

static unsigned long multiboot_magic = 0;
static unsigned long multiboot_addr = 0;

static int mb_valid(void)
{
	if (multiboot_addr) {
		return 1;
	}
	printk("error: multiboot not initialized\n");
	return 0;
}

static struct multiboot_tag * mb_tag_find(uint16_t type)
{
	struct multiboot_tag *tag = (struct multiboot_tag *)(multiboot_addr + 8);

	if (!mb_valid()) {
		return NULL;
	}

	while (tag->type != MULTIBOOT_TAG_TYPE_END) {
		if (tag->type == type) {
			return tag;
		}
		tag = (struct multiboot_tag *)((uint8_t *)tag+((tag->size+7)&~7));
	}
	return NULL;
}

static struct multiboot_tag * mb_tag_next(struct multiboot_tag *tag)
{
	if (!mb_valid()) {
		return NULL;
	}

	tag = (struct multiboot_tag *)((uint8_t *)tag+((tag->size+7)&~7));
	if (tag->type == MULTIBOOT_TAG_TYPE_END) {
		return NULL;
	}
	return tag;
}

char * mb_mbi_cmdline(void)
{
	struct multiboot_tag *tag;

	if (!mb_valid()) {
		return NULL;
	}

#ifdef CONFIG_ENABLE_MULTIBOOT1
	if (multiboot_magic == MULTIBOOT1_BOOTLOADER_MAGIC) {
		multiboot1_info_t *mbi = (multiboot1_info_t *)multiboot_addr;

		if (mbi->flags & MULTIBOOT1_INFO_CMDLINE) {
			return (char *)mbi->cmdline;
		}
		return NULL;
	}
#endif
	tag = mb_tag_find(MULTIBOOT_TAG_TYPE_CMDLINE);
	if (tag) {
		struct multiboot_tag_string *tag_string
			= (struct multiboot_tag_string *)tag;
		return tag_string->string;
	}

	return NULL;
}

unsigned long mb_mbi_start(void)
{
	return multiboot_addr;
}

unsigned long mb_mbi_len(void)
{
	if (!mb_valid()) {
		return 0;
	}

#ifdef CONFIG_ENABLE_MULTIBOOT1
	if (multiboot_magic == MULTIBOOT1_BOOTLOADER_MAGIC) {
		/* FIXME Multiboot1 references a number of external structures
		 * which may exist at the end of mbi structure.  We need to
		 * check every possible flag to see if structures are appended
		 * to the end of the mbi region */
		return sizeof(multiboot1_info_t);
	}
#endif
	/* Multiboot2 specifies the length of data in memory as its
	 * first element */
	return *(uint32_t *)multiboot_addr;
}

unsigned long mb_mbi_end(void)
{
	if (!mb_valid()) {
		return 0;
	}

	return (multiboot_addr + mb_mbi_len());
}

int mb_mod_count(void)
{
	struct multiboot_tag *tag;
	int count;

	if (!mb_valid()) {
		return 0;
	}

#ifdef CONFIG_ENABLE_MULTIBOOT1
	if (multiboot_magic == MULTIBOOT1_BOOTLOADER_MAGIC) {
		multiboot1_info_t *mbi = (multiboot1_info_t *)multiboot_addr;

		if (mbi->flags & MULTIBOOT1_INFO_MODS && mbi->mods_count) {
			return  mbi->mods_count;
		}
		return 0;
	}
#endif
	/* Find any module data */
	tag = mb_tag_find(MULTIBOOT_TAG_TYPE_MODULE);
	for (count = 0;tag; tag = mb_tag_next(tag)) {
		count++;
	}

	return count;
}

#ifdef CONFIG_ENABLE_MULTIBOOT1
static multiboot1_module_t * mb1_mod_find(int module)
{
	multiboot1_info_t *mbi = (multiboot1_info_t *)multiboot_addr;

	if (!mb_valid()) {
		return NULL;
	}

	if (multiboot_magic != MULTIBOOT1_BOOTLOADER_MAGIC) {
		printk("error: mb1_mod_find() invalid magic\n");
		return NULL;
	}

	if (mbi->flags & MULTIBOOT1_INFO_MODS && mbi->mods_count != 0) {
		multiboot1_module_t *mb1_mod;
		int index;

		mb1_mod = (multiboot1_module_t *)mbi->mods_addr;

		for (index = 0; index < mbi->mods_count; index++, mb1_mod++) {
			if (index == module) {
				return mb1_mod;
			}
		}
	}

	return NULL;
}
#endif

static struct multiboot_tag_module * mb2_mod_find(int module)
{
	struct multiboot_tag *tag;
	int index;

	if (!mb_valid()) {
		return NULL;
	}

	if (multiboot_magic != MULTIBOOT1_BOOTLOADER_MAGIC) {
		printk("error: mb2_mod_find() invalid magic\n");
		return NULL;
	}

	/* Find any module data */
	tag = mb_tag_find(MULTIBOOT_TAG_TYPE_MODULE);
	for (index = 0;tag; tag = mb_tag_next(tag)) {
		struct multiboot_tag_module *tag_module
			= (struct multiboot_tag_module *)tag;

		if (index == module) {
			return tag_module;
		}
	}

	return NULL;
}

char * mb_mod_cmdline(int module)
{
	struct multiboot_tag_module *mb2_mod;

	if (!mb_valid()) {
		return NULL;
	}

#ifdef CONFIG_ENABLE_MULTIBOOT1
	if (multiboot_magic == MULTIBOOT1_BOOTLOADER_MAGIC) {
		multiboot1_module_t *mb1_mod;
		mb1_mod = mb1_mod_find(module);
		if (mb1_mod) {
			return (char *)mb1_mod->cmdline;
		}
		return 0;
	}
#endif

	mb2_mod = mb2_mod_find(module);
	if (mb2_mod) {
		return mb2_mod->cmdline;
	}

	return 0;
}

unsigned long mb_mod_start(int module)
{
	struct multiboot_tag_module *mb2_mod;

	if (!mb_valid()) {
		return 0;
	}

#ifdef CONFIG_ENABLE_MULTIBOOT1
	if (multiboot_magic == MULTIBOOT1_BOOTLOADER_MAGIC) {
		multiboot1_module_t *mb1_mod;
		mb1_mod = mb1_mod_find(module);
		if (mb1_mod) {
			return mb1_mod->mod_start;
		}
		return 0;
	}
#endif

	mb2_mod = mb2_mod_find(module);
	if (mb2_mod) {
		return mb2_mod->mod_start;
	}

	return 0;
}

unsigned long mb_mod_end(int module)
{
	struct multiboot_tag_module *mb2_mod;

	if (!mb_valid()) {
		return 0;
	}

#ifdef CONFIG_ENABLE_MULTIBOOT1
	if (multiboot_magic == MULTIBOOT1_BOOTLOADER_MAGIC) {
		multiboot1_module_t *mb1_mod;
		mb1_mod = mb1_mod_find(module);
		if (mb1_mod) {
			return mb1_mod->mod_end;
		}
		return 0;
	}
#endif
	mb2_mod = mb2_mod_find(module);
	if (module) {
		return mb2_mod->mod_end;
	}

	return 0;
}

unsigned long mb_mod_len(int module)
{
	struct multiboot_tag_module *mb2_mod;

	if (!mb_valid()) {
		return 0;
	}

#ifdef CONFIG_ENABLE_MULTIBOOT1
	if (multiboot_magic == MULTIBOOT1_BOOTLOADER_MAGIC) {
		multiboot1_module_t *mb1_mod;
		mb1_mod = mb1_mod_find(module);
		if (mb1_mod) {
			return (mb1_mod->mod_end - mb1_mod->mod_start);
		}
		return 0;
	}
#endif
	mb2_mod = mb2_mod_find(module);
	if (mb2_mod) {
		return (mb2_mod->mod_end - mb2_mod->mod_start);
	}

	return 0;
}

#ifdef CONFIG_ENABLE_MULTIBOOT1
static multiboot1_memory_map_t * mb1_mmap_find(int mmap)
{
	multiboot1_info_t *mbi = (multiboot1_info_t *)multiboot_addr;
	multiboot1_memory_map_t *mb_mmap;
	int index;

	if (!mb_valid()) {
		return NULL;
	}

	if (multiboot_magic != MULTIBOOT1_BOOTLOADER_MAGIC) {
		printk("error: mb1_mmap_find() invalid magic\n");
		return NULL;
	}

	/* This should never be called if a mmap is not available */
	if (!(mbi->flags & MULTIBOOT1_INFO_MEM_MAP)) {
		printk("mb1_mmap_find(): no mmap available\n");
		return NULL;
	}

	index = 0;
	for (mb_mmap = (multiboot1_memory_map_t *) mbi->mmap_addr;
			(uint32_t) mb_mmap < mbi->mmap_addr+mbi->mmap_length;
			mb_mmap = (multiboot1_memory_map_t *) ((uint32_t) mb_mmap
			+ mb_mmap->size + sizeof (mb_mmap->size))) {
		if (mmap == index++) {
			return mb_mmap;
		}
	}

	return NULL;
}
#endif

static struct multiboot_mmap_entry * mb2_mmap_find(int mmap)
{
	struct multiboot_tag *tag;
	struct multiboot_mmap_entry *mb_mmap;
	int index;

	if (!mb_valid()) {
		return NULL;
	}

	if (multiboot_magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
		printk("error: mb2_mmap_find() invalid magic\n");
		return NULL;
	}

	tag = mb_tag_find(MULTIBOOT_TAG_TYPE_MMAP);

	if (!tag) {
		printk("mb2_mmap_find(): no mmap available\n");
		return NULL;
	}

	index = 0;
	for (mb_mmap = ((struct multiboot_tag_mmap *)tag)->entries;
		     (uint8_t *)mb_mmap < (uint8_t *)tag + tag->size;
		     mb_mmap = (struct multiboot_mmap_entry *)((uint32_t)mb_mmap
		     + ((struct multiboot_tag_mmap *)tag)->entry_size)) {

		if (index == mmap) {
			return mb_mmap;
		}
		index++;
	}

	return NULL;
}

int mb_mmap_count(void)
{
	struct multiboot_tag *tag;
	struct multiboot_mmap_entry *mb2_mmap;
	int count;

	if (!mb_valid()) {
		return 0;
	}

#ifdef CONFIG_ENABLE_MULTIBOOT1
	if (multiboot_magic == MULTIBOOT1_BOOTLOADER_MAGIC) {
		multiboot1_info_t *mbi = (multiboot1_info_t *)multiboot_addr;
		multiboot1_memory_map_t *mb1_mmap;

		if (!(mbi->flags & MULTIBOOT1_INFO_MEM_MAP)) {
			if (!(mbi->flags & MULTIBOOT1_INFO_MEMORY)) {
				return 0;
			}
			return 2;
		}

		count = 0;
		do {
			mb1_mmap = mb1_mmap_find(count);
			if (mb1_mmap) {
				count++;
			}
		} while (mb1_mmap);

		return count;
	}
#endif
	tag = mb_tag_find(MULTIBOOT_TAG_TYPE_MMAP);

	if (!tag) {
		tag = mb_tag_find(MULTIBOOT_TAG_TYPE_BASIC_MEMINFO);
		if (!tag) {
			return 0;
		}
		return 2;
	}

	count = 0;
	do {
		mb2_mmap = mb2_mmap_find(count);
		if (mb2_mmap) {
			count++;
		}
	} while (mb2_mmap);

	return count;
}

unsigned long mb_mmap_start(int mmap)
{
	struct multiboot_tag *tag;
	struct multiboot_mmap_entry *mb2_mmap;

	if (!mb_valid()) {
		return 0;
	}

#ifdef CONFIG_ENABLE_MULTIBOOT1
	if (multiboot_magic == MULTIBOOT1_BOOTLOADER_MAGIC) {
		multiboot1_info_t *mbi = (multiboot1_info_t *)multiboot_addr;
		multiboot1_memory_map_t *mb1_mmap;

		if (mbi->flags & MULTIBOOT1_INFO_MEM_MAP) {
			mb1_mmap = mb1_mmap_find(mmap);
			if (mb1_mmap) {
				return mb1_mmap->addr;
			}
			return 0;
		}

		/* Emulate mmeory mappings */
		if (mbi->flags & MULTIBOOT1_INFO_MEMORY) {
			if (mmap == 1) {
				return 1024 * 1024;
			}
		}
		return 0;
	}
#endif
	tag = mb_tag_find(MULTIBOOT_TAG_TYPE_MMAP);

	if (tag) {
		mb2_mmap = mb2_mmap_find(mmap);
		if (mb2_mmap) {
			return mb2_mmap->addr;
		}
	} else {
		tag = mb_tag_find(MULTIBOOT_TAG_TYPE_BASIC_MEMINFO);

		if (tag) {
			if (mmap == 1) {
				return 1024 * 1024;
			}
		}
	}

	return 0;
}

unsigned long mb_mmap_len(int mmap)
{
	struct multiboot_tag *tag;
	struct multiboot_mmap_entry *mb2_mmap;

	if (!mb_valid()) {
		return 0;
	}

#ifdef CONFIG_ENABLE_MULTIBOOT1
	if (multiboot_magic == MULTIBOOT1_BOOTLOADER_MAGIC) {
		multiboot1_info_t *mbi = (multiboot1_info_t *)multiboot_addr;
		multiboot1_memory_map_t *mb1_mmap;

		if (mbi->flags & MULTIBOOT1_INFO_MEM_MAP) {
			mb1_mmap = mb1_mmap_find(mmap);
			if (mb1_mmap) {
				return mb1_mmap->len;
			}
			return 0;
		}

		/* Emulate mmeory mappings */
		if (mbi->flags & MULTIBOOT1_INFO_MEMORY) {
			switch (mmap) {
			case 0:		return mbi->mem_lower * 1024;
			case 1:		return mbi->mem_upper * 1024;
			}
			return 0;
		}
		return 0;
	}
#endif
	tag = mb_tag_find(MULTIBOOT_TAG_TYPE_MMAP);

	if (tag) {
		mb2_mmap = mb2_mmap_find(mmap);
		if (mb2_mmap) {
			return mb2_mmap->len;
		}
	} else {
		struct multiboot_tag_basic_meminfo *meminfo;

		tag = mb_tag_find(MULTIBOOT_TAG_TYPE_BASIC_MEMINFO);
		meminfo = (struct multiboot_tag_basic_meminfo *)tag;

		switch (mmap) {
		case 0:	return meminfo->mem_lower * 1024;
		case 1:	return meminfo->mem_upper * 1024;
		}

	}

	return 0;
}

unsigned long mb_mmap_end(int mmap)
{
	if (!mb_valid()) {
		return 0;
	}

	return (mb_mmap_start(mmap) + mb_mmap_len(mmap));
}

uint32_t mb_mmap_type(int mmap)
{
	struct multiboot_tag *tag;
	struct multiboot_mmap_entry *mb2_mmap;

#ifdef CONFIG_ENABLE_MULTIBOOT1
	if (multiboot_magic == MULTIBOOT1_BOOTLOADER_MAGIC) {
		multiboot1_info_t *mbi = (multiboot1_info_t *)multiboot_addr;
		multiboot1_memory_map_t *mb1_mmap;

		if (!(mbi->flags & MULTIBOOT1_INFO_MEM_MAP)) {
			if (!(mbi->flags & MULTIBOOT1_INFO_MEMORY)) {
				return 0;
			}

			return MULTIBOOT_MEMORY_AVAILABLE;
		}

		mb1_mmap = mb1_mmap_find(mmap);
		if (mb1_mmap) {
			return mb1_mmap->type;
		}
	}
#endif
	tag = mb_tag_find(MULTIBOOT_TAG_TYPE_MMAP);

	if (!tag) {
		if (mmap > 1) {
			return 0;
		}

		tag = mb_tag_find(MULTIBOOT_TAG_TYPE_BASIC_MEMINFO);
		if (!tag) {
			return 0;
		}
		return MULTIBOOT_MEMORY_AVAILABLE;
	}

	mb2_mmap = mb2_mmap_find(mmap);
	return mb2_mmap->type;
}

uint64_t mb_fb_addr(void)
{
	struct multiboot_tag *tag;

#ifdef CONFIG_ENABLE_MULTIBOOT1
	if (multiboot_magic == MULTIBOOT1_BOOTLOADER_MAGIC) {
		multiboot1_info_t *mbi = (multiboot1_info_t *)multiboot_addr;

		if (mbi->flags & MULTIBOOT1_INFO_FRAMEBUFFER_INFO) {
			return mbi->framebuffer_addr;
		}
		return 0;
	}
#endif

	tag = mb_tag_find(MULTIBOOT_TAG_TYPE_FRAMEBUFFER);
	if (tag) {
		struct multiboot_tag_framebuffer *fb
			= (struct multiboot_tag_framebuffer *)tag;
		return fb->common.framebuffer_addr;
	}

	return 0;
}

uint32_t mb_fb_pitch(void)
{
	struct multiboot_tag *tag;

#ifdef CONFIG_ENABLE_MULTIBOOT1
	if (multiboot_magic == MULTIBOOT1_BOOTLOADER_MAGIC) {
		multiboot1_info_t *mbi = (multiboot1_info_t *)multiboot_addr;

		if (mbi->flags & MULTIBOOT1_INFO_FRAMEBUFFER_INFO) {
			return mbi->framebuffer_pitch;
		}
		return 0;
	}
#endif

	tag = mb_tag_find(MULTIBOOT_TAG_TYPE_FRAMEBUFFER);
	if (tag) {
		struct multiboot_tag_framebuffer *fb
			= (struct multiboot_tag_framebuffer *)tag;
		return fb->common.framebuffer_pitch;
	}

	return 0;
}

uint32_t mb_fb_width(void)
{
	struct multiboot_tag *tag;

#ifdef CONFIG_ENABLE_MULTIBOOT1
	if (multiboot_magic == MULTIBOOT1_BOOTLOADER_MAGIC) {
		multiboot1_info_t *mbi = (multiboot1_info_t *)multiboot_addr;

		if (mbi->flags & MULTIBOOT1_INFO_FRAMEBUFFER_INFO) {
			return mbi->framebuffer_width;
		}
		return 0;
	}
#endif

	tag = mb_tag_find(MULTIBOOT_TAG_TYPE_FRAMEBUFFER);
	if (tag) {
		struct multiboot_tag_framebuffer *fb
			= (struct multiboot_tag_framebuffer *)tag;
		return fb->common.framebuffer_width;
	}

	return 0;
}

uint32_t mb_fb_height(void)
{
	struct multiboot_tag *tag;

#ifdef CONFIG_ENABLE_MULTIBOOT1
	if (multiboot_magic == MULTIBOOT1_BOOTLOADER_MAGIC) {
		multiboot1_info_t *mbi = (multiboot1_info_t *)multiboot_addr;

		if (mbi->flags & MULTIBOOT1_INFO_FRAMEBUFFER_INFO) {
			return mbi->framebuffer_height;
		}
		return 0;
	}
#endif

	tag = mb_tag_find(MULTIBOOT_TAG_TYPE_FRAMEBUFFER);
	if (tag) {
		struct multiboot_tag_framebuffer *fb
			= (struct multiboot_tag_framebuffer *)tag;
		return fb->common.framebuffer_height;
	}

	return 0;
}

uint8_t mb_fb_depth(void)
{
	struct multiboot_tag *tag;

#ifdef CONFIG_ENABLE_MULTIBOOT1
	if (multiboot_magic == MULTIBOOT1_BOOTLOADER_MAGIC) {
		multiboot1_info_t *mbi = (multiboot1_info_t *)multiboot_addr;

		if (mbi->flags & MULTIBOOT1_INFO_FRAMEBUFFER_INFO) {
			return mbi->framebuffer_bpp;
		}
		return 0;
	}
#endif

	tag = mb_tag_find(MULTIBOOT_TAG_TYPE_FRAMEBUFFER);
	if (tag) {
		struct multiboot_tag_framebuffer *fb
			= (struct multiboot_tag_framebuffer *)tag;
		return fb->common.framebuffer_bpp;
	}

	return 0;
}

uint8_t mb_fb_type(void)
{
	struct multiboot_tag *tag;

#ifdef CONFIG_ENABLE_MULTIBOOT1
	if (multiboot_magic == MULTIBOOT1_BOOTLOADER_MAGIC) {
		multiboot1_info_t *mbi = (multiboot1_info_t *)multiboot_addr;

		if (mbi->flags & MULTIBOOT1_INFO_FRAMEBUFFER_INFO) {
			return mbi->framebuffer_type;
		}
		return 0;
	}
#endif

	tag = mb_tag_find(MULTIBOOT_TAG_TYPE_FRAMEBUFFER);
	if (tag) {
		struct multiboot_tag_framebuffer *fb
			= (struct multiboot_tag_framebuffer *)tag;
		return fb->common.framebuffer_type;
	}

	return 0;
}

int mb_init(unsigned long addr, unsigned long magic)
{
	multiboot_magic = magic;
	multiboot_addr = addr;

	if (addr & 7) {
		printk("Unaligned mbi: 0x%x\n", addr);
		return 1;
	}

#ifdef CONFIG_ENABLE_MULTIBOOT1
	if (magic == MULTIBOOT1_BOOTLOADER_MAGIC) {
		printk("Found multiboot1 header at 0x%x\n", addr);
	} else
#endif
	if (magic == MULTIBOOT2_BOOTLOADER_MAGIC) {
		printk("Found multiboot2 header at 0x%x\n", addr);
	} else {
		printk("Invalid magic number: 0x%x\n", (unsigned)magic);
		return 0;
	}

	return 1;
}
