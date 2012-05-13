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
#include <fmios/serial.h>
#include <fmios/video.h>
#include <fmios/io.h>
#include <multiboot.h>
#include <string.h>

/* FIXME these need to go somewhere else */
#define PAGE_SIZE	0x1000
#define PAGE_NUM(addr)	(((unsigned long)addr) / PAGE_SIZE)
/* FIXME this should just mask out the lower bits to find the page address */
#define PAGE_OF(addr)	((((unsigned long)addr) / PAGE_SIZE) * PAGE_SIZE)

/* Start looking for free pages at the end of the kernel */
extern const void __kernel_start;
extern const void _end;
static unsigned long kernel_start;
static unsigned long kernel_end;
struct kconfig *kconfig = NULL;

struct multiboot_tag * mb_tag_find(unsigned long addr, uint16_t type)
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

struct multiboot_tag * mb_tag_next(struct multiboot_tag *tag)
{
	tag = (struct multiboot_tag *)((uint8_t *)tag+((tag->size+7)&~7));
	if (tag->type == MULTIBOOT_TAG_TYPE_END) {
		return NULL;
	}
	return tag;
}

char * mb_cmdline(unsigned long addr, unsigned long magic)
{
#ifdef CONFIG_ENABLE_MULTIBOOT1
	if (magic == MULTIBOOT1_BOOTLOADER_MAGIC) {
		multiboot1_info_t *mbi = (multiboot1_info_t *)addr;

		if (mbi->flags & MULTIBOOT1_INFO_CMDLINE) {
			return (char *)mbi->cmdline;
		}
	} else
#endif
	{
		struct multiboot_tag *tag;

		tag = mb_tag_find(addr, MULTIBOOT_TAG_TYPE_CMDLINE);
		if (tag) {
			struct multiboot_tag_string *tag_string
				= (struct multiboot_tag_string *)tag;
			return tag_string->string;
		}
	}

	return NULL;
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

size_t mb_mbi_len(unsigned long addr, unsigned long magic)
{
	size_t size = 0;

#ifdef CONFIG_ENABLE_MULTIBOOT1
	if (magic == MULTIBOOT1_BOOTLOADER_MAGIC) {
		/* FIXME Multiboot1 references a number of external structures
		 * which may exist at the end of mbi structure.  We need to
		 * check every possible flag to see if structures are appended
		 * to the end of the mbi region */
		size = sizeof(multiboot1_info_t);
	} else
#endif
	{
		/* Multiboot2 specifies the length of data in memory as its
		 * first element */
		size = *(uint32_t *)addr;
	}

	return size;
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
	uint32_t iobase = 0;
	uint32_t baud = 0;
	uint16_t divisor = 0;
	uint8_t flags = 0;
	char *param;

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

static size_t mb_mmap_count(unsigned long addr, unsigned long magic)
{
	size_t count = 0;

#ifdef CONFIG_ENABLE_MULTIBOOT1
	if (magic == MULTIBOOT1_BOOTLOADER_MAGIC) {
		multiboot1_info_t *mbi = (multiboot1_info_t *)addr;

		/* If we have basic mem info set count to 2 and replace it
		 * later if we have more detailed data. */
		if (mbi->flags & MULTIBOOT1_INFO_MEMORY) {
			count = 2;
		}

		if (mbi->flags & MULTIBOOT1_INFO_MEM_MAP) {
			multiboot1_memory_map_t *mb_mmap;

			/* Convert the multiboot memory map to generic format */
			count = 0;
			for (mb_mmap = (multiboot1_memory_map_t *) mbi->mmap_addr;
			     (uint32_t) mb_mmap < mbi->mmap_addr + mbi->mmap_length;
			     mb_mmap = (multiboot1_memory_map_t *) ((uint32_t) mb_mmap
			     + mb_mmap->size + sizeof (mb_mmap->size))) {
				count++;
			}
		}

		if (mbi->flags & MULTIBOOT1_INFO_MODS) {
			/* If the modules are not contiguous in memory then
			 * they will split available memory in half, so we need
			 * to entries per used entry inserted.  At this stage
			 * we can't tell if an entry is or isn't contiguous in
			 * memory so just over compensate.  We can always move
			 * our heap pointer back down as we merge entries. */
			count += (mbi->mods_count * 2);
		}

		/* FIXME we should technically traverse the MB1 information
		 * structure validating that all data structures it references
		 * are within the same page */
	} else
#endif
	{
		struct multiboot_tag *tag;


		/* If we have basic memory info then start the counter at 2 and
		 * replace it with any detailed count we can find later. */
		tag = mb_tag_find(addr, MULTIBOOT_TAG_TYPE_BASIC_MEMINFO);
		if (tag) {
			count = 2;
		}

		tag = mb_tag_find(addr, MULTIBOOT_TAG_TYPE_MMAP);
		if (tag) {
			multiboot_memory_map_t *mb_mmap;

			/* Replace the basic memory count with the real-deal */
			count = 0;

			for (mb_mmap = ((struct multiboot_tag_mmap *)tag)->entries;
			     (uint8_t *)mb_mmap < (uint8_t *)tag + tag->size;
			     mb_mmap = (multiboot_memory_map_t *)((uint32_t)mb_mmap
			     + ((struct multiboot_tag_mmap *)tag)->entry_size)) {

				count++;
			}
		}

		tag = mb_tag_find(addr, MULTIBOOT_TAG_TYPE_MODULE);
		for (;tag; tag = mb_tag_next(tag)) {
			count++;
		}
	}

	/* Add 1 segment for the kernel and 1 segment for the mbi */
	return (count +2);
}

/* Find free pages for the kernel config */
static struct kconfig *mb_alloc_pages(unsigned long addr,
		unsigned long magic, size_t count)
{
	unsigned long pstart = PAGE_NUM(kernel_end) + 1;
	unsigned long pend = pstart + count;

	/* Check the MBI location */
	if (PAGE_NUM(addr) < pend && PAGE_NUM(addr) > pstart) {
		printk("page collision with mbi\n");
		pstart = (PAGE_NUM(addr + mb_mbi_len(addr, magic)) + 1);
		pend = pstart + count;
	}

#ifdef CONFIG_ENABLE_MULTIBOOT1
	if (magic == MULTIBOOT1_BOOTLOADER_MAGIC) {
		multiboot1_info_t *mbi = (multiboot1_info_t *)addr;

		if (mbi->flags & MULTIBOOT1_INFO_MODS
		    && mbi->mods_count != 0) {
			multiboot1_module_t *mod;
			int i;

			for (i = 0, mod = (multiboot1_module_t *) mbi->mods_addr;
			     i < mbi->mods_count; i++, mod++) {

				if (PAGE_NUM(mod->mod_start) < pend &&
				    PAGE_NUM(mod->mod_start) > pstart) {
					printk("page collision with module\n");
					pstart = (PAGE_NUM(mod->mod_end)+1);
					pend = pstart + count;
				}
			}
		}
	} else
#endif
	{
		struct multiboot_tag *tag;

		/* Find any module data */
		tag = mb_tag_find(addr, MULTIBOOT_TAG_TYPE_MODULE);
		for (;tag; tag = mb_tag_next(tag)) {
			struct multiboot_tag_module *tag_module
				= (struct multiboot_tag_module *)tag;

			if (PAGE_NUM(tag_module->mod_start) < pend &&
			    PAGE_NUM(tag_module->mod_start) > pstart) {
				printk("page collision with module\n");
				pstart = (PAGE_NUM(tag_module->mod_end)+1);
				pend = pstart + count;
			}
		}
	}

	/* FIXME is this address in valid memory? */
	return (struct kconfig *)(pstart * PAGE_SIZE);
}

static int mb_mmap_shift(struct mmap_entry *entries, int count)
{
	int index = 0;

	/* Figure out how many entries we have left */
	while (entries[index+1].flags != MEMORY_MAP_END) {
		index++;
	}

	if (!index) {
		return 0;
	}

	while ((index-count) >= 0) {
		if (entries[index-count].type) {
			memcpy(&entries[index], &entries[index-count],
				sizeof(struct mmap_entry));
		}
		index--;
	}

	return 1;
}

/* Insert the given mmap entry into the existing mmap table.  We need to
 * properly split existing regions and reset the start/len elements with each
 * insert. */
static int mb_mmap_add(struct mmap_table *mmap, struct mmap_entry *entry)
{
	struct mmap_entry *entries = &mmap->entry[0];
	int index;

#ifdef CONFIG_ENABLE_DEBUG
	printk("new mmap: addr=0x%x%x, len=0x%x%x, type=0x%x, flags=0x%x\n",
			(uint32_t)(entry->addr >> 32),
			(uint32_t)(entry->addr),
			(uint32_t)(entry->len >> 32),
			(uint32_t)(entry->len),
			entry->type, entry->flags);
#endif

	for (index = 0; index < mmap->count; index++) {
		if (entry->addr >= (entries[index].addr + entries[index].len)
		    || (entry->addr + entry->len) <= entries[index].addr) {
#ifdef CONFIG_ENABLE_DEBUG
			printk("skip map: addr=0x%x%x, len=0x%x%x, type=0x%x, flags=0x%x\n",
				(uint32_t)(entries[index].addr >> 32),
				(uint32_t)(entries[index].addr),
				(uint32_t)(entries[index].len >> 32),
				(uint32_t)(entries[index].len),
				entries[index].type, entries[index].flags);
#endif
			continue;
		}


		if (!entries[index].type == MULTIBOOT_MEMORY_AVAILABLE) {
			printk("error: used address in unusable memory\n");
			return 0;
		}

		/* Move all the entries after the current entry up by 2
		 * positions */
		if (!mb_mmap_shift(&entries[index+1], 2)) {
			printk("error: failed to shift memory map\n");
			return 0;
		}

#ifdef CONFIG_ENABLE_DEBUG
		printk("add into: addr=0x%x%x, len=0x%x%x, type=0x%x, flags=0x%x\n",
			(uint32_t)(entries[index].addr >> 32),
			(uint32_t)(entries[index].addr),
			(uint32_t)(entries[index].len >> 32),
			(uint32_t)(entries[index].len),
			entries[index].type, entries[index].flags);
#endif

		/* Copy the new entry in splitting the old entry into 2
		 * entries */
		entries[index+1].addr = entry->addr;
		entries[index+1].len = entry->len;
		entries[index+1].type = entry->type;
		entries[index+1].flags = entry->flags;

		/* Trailing segment of original memory */
		entries[index+2].addr = (entry->addr + entry->len);
		entries[index+2].len = (entries[index].addr + entries[index].len)
		      -	(entries[index+2].addr);
		entries[index+2].type = entries[index].type;
		entries[index+2].flags = entries[index].flags;

		entries[index].len = entries[index+1].addr - entries[index].addr;

		index++;
	}
	return index;
}

static int mb_mmap_init(struct kconfig *kconfig)
{
	struct mmap_entry *entry = kconfig->mmap.entry;
	struct mmap_entry current;

#ifdef CONFIG_ENABLE_MULTIBOOT1
	if (kconfig->mb_magic == MULTIBOOT1_BOOTLOADER_MAGIC) {
		printk("fix multiboot1 init\n");
	} else
#endif
	{
		struct multiboot_tag *tag;

		tag = mb_tag_find(kconfig->mb_addr, MULTIBOOT_TAG_TYPE_MMAP);
		if (!tag) {
			struct multiboot_tag_basic_meminfo *meminfo;
			int index = 0;

			tag = mb_tag_find(kconfig->mb_addr,
					  MULTIBOOT_TAG_TYPE_BASIC_MEMINFO);

			if (!tag) {
				return 0;
			}

			printk("no mapping available\n");
			printk("using basic memory information\n");

			meminfo = (struct multiboot_tag_basic_meminfo *)tag;

			entry[index].addr = 0;
			entry[index].len = meminfo->mem_lower * 1024;
			entry[index].type = MULTIBOOT_MEMORY_AVAILABLE;
			entry[index].flags = MEMORY_MAP_UNUSED;
			index++;

			entry[index].addr = 1024 * 1024;
			entry[index].len = meminfo->mem_upper * 1024;
			entry[index].type = MULTIBOOT_MEMORY_AVAILABLE;
			entry[index].flags = MEMORY_MAP_UNUSED;
			index++;
		} else {
			multiboot_memory_map_t *mb_mmap;
			int index = 0;

			/* We assume these memory mappings are already sane, so
			 * just insert them */
			for (mb_mmap = ((struct multiboot_tag_mmap
							*)tag)->entries;
			     (uint8_t *)mb_mmap < (uint8_t *)tag + tag->size;
			     mb_mmap = (multiboot_memory_map_t *)((uint32_t)mb_mmap
			     + ((struct multiboot_tag_mmap *)tag)->entry_size)) {

				entry[index].addr = mb_mmap->addr;
				entry[index].len = mb_mmap->len;
				entry[index].type = mb_mmap->type;
				entry[index].flags = MEMORY_MAP_UNUSED;
				index++;
			}
		}

		current.addr = kernel_start;
		current.len = (kernel_end - kernel_start);
		current.type = MULTIBOOT_MEMORY_AVAILABLE;
		current.flags = MEMORY_MAP_KERNEL;
		if (!mb_mmap_add(&kconfig->mmap, &current)) {
			printk("error: failed to map the kernel\n");
			return 0;
		}

		/* Insert a mapping for the multiboot data */
		current.addr = kconfig->mb_addr;
		current.len = mb_mbi_len(kconfig->mb_addr, kconfig->mb_magic);
		current.type = MULTIBOOT_MEMORY_AVAILABLE;
		current.flags = MEMORY_MAP_LOADER;
		if (!mb_mmap_add(&kconfig->mmap, &current)) {
			printk("error: failed to map multiboot information\n");
			return 0;
		}

		/* Find any module data */
		tag = mb_tag_find(kconfig->mb_addr, MULTIBOOT_TAG_TYPE_MODULE);
		for (;tag; tag = mb_tag_next(tag)) {
			struct multiboot_tag_module *tag_module
				= (struct multiboot_tag_module *)tag;

			current.addr = tag_module->mod_start;
			current.len = (tag_module->mod_end - tag_module->mod_start);
			current.type = MULTIBOOT_MEMORY_AVAILABLE;
			current.flags = MEMORY_MAP_MODULE;

			if (!mb_mmap_add(&kconfig->mmap, &current)) {
				printk("error: failed to map modules\n");
				return 0;
			}
		}
	}

	return kconfig->mmap.count;
}

/**
 * @addr address of multiboot information structure
 * @magic multiboot magic identifying mbi type
 * @return pointer to the kernel configuration structure
 *
 * This routine is responsible for examining the available memory as advertised
 * by the multiboot information and figuring out which portions of that memory
 * are in-use by existing data such as the kernel, the multiboot information,
 * and the boot modules. Fundementally this comes down to some non-trivial work
 * to locate a free page in memory in which to copy all the useful data we
 * need.
 */
static struct kconfig * kconfig_alloc(unsigned long addr, unsigned long magic)
{
	/* The kconfig tracks various kernel config options, a pointer to the
	 * kernel cmdline, the number of CPUs, and tracks the number of
	 * mmap_entries follow the kconfig structure */
	size_t size = sizeof(struct kconfig);
	/* The number of mmap_entry structures we need, technically every entry
	 * added to an existing map entry has the potential to split entries in
	 * half.  So in a worst-case condition we need double the count. */
	size_t count = (mb_mmap_count(addr, magic) * 2);

	size += (count * sizeof(struct mmap_entry));

	kconfig = mb_alloc_pages(addr, magic, PAGE_NUM(size) + 1);

	if (kconfig) {
		printk("kernel: start=0x%x,  end=0x%x\n",
			kernel_start, kernel_end);
		printk("config: start=0x%x, len=0x%x\n", kconfig, size);

		memset(kconfig, 0, sizeof(struct kconfig));

		kconfig->cmdline = mb_cmdline(addr, magic);
		kconfig->mb_magic = magic;
		kconfig->mb_addr = addr;

		memset(&kconfig->mmap.entry[0], 0,
		       (sizeof(struct mmap_entry) * count));
		kconfig->mmap.count = count;
		kconfig->mmap.entry[count].flags = MEMORY_MAP_END;

		if (mb_mmap_init(kconfig)) {
			struct mmap_entry entry;

			entry.addr = (unsigned long)kconfig;
			entry.len = size;
			entry.type = MULTIBOOT_MEMORY_AVAILABLE;
			entry.flags = MEMORY_MAP_KERNEL;

			if (mb_mmap_add(&kconfig->mmap, &entry)) {
				return kconfig;
			}
		}
	}

	return NULL;
}

/* These routines are platform specific and must be defined to boot */
static void __init_memory(struct kconfig *kconfig)
{
	int index;
	struct mmap_entry *entry = kconfig->mmap.entry;

	printk("Available memory:\n");

	/* Display the memory map we currently have to aid in board bringups */
	for (index = 0; index < kconfig->mmap.count; index++) {
		if (entry[index].type != MULTIBOOT_MEMORY_AVAILABLE) {
			continue;
		}

		switch (entry[index].flags) {
		case MEMORY_MAP_KERNEL:
			printk("kernel: ");
			break;
		case MEMORY_MAP_LOADER:
			printk("loader: ");
			break;
		case MEMORY_MAP_MODULE:
			printk("module: ");
			break;
		default:
			printk("unused: ");
			break;
		}

		printk("addr=0x%x%x, end=0x%x%x\n",
		       (uint32_t)(entry[index].addr >> 32),
		       (uint32_t)(entry[index].addr),
		       (uint32_t)((entry[index].addr + entry[index].len) >> 32),
		       (uint32_t)(entry[index].addr + entry[index].len));
	}

	printk("No platform initialization defined\n");
}
weak_symbol(__init_memory, init_memory);

/** Start of OS independant initialization
 * @magic Multiboot magic number
 * @addr Address of Multiboot Information Structure
 * Parse the Multiboot Information and initialize the system */
int mb_init(unsigned long magic, unsigned long addr)
{
	char *cmdline = "";

	if (addr & 7) {
		printk("Unaligned mbi: 0x%x\n", addr);
		return 1;
	}

	kernel_start = (unsigned long)&__kernel_start;
	kernel_end = (unsigned long)&_end;

#ifdef CONFIG_ENABLE_MULTIBOOT1
	if (magic == MULTIBOOT1_BOOTLOADER_MAGIC) {
		multiboot1_info_t *mbi = (multiboot1_info_t *)addr;

		printk("Multiboot1 at 0x%x\n", addr);

		if (mbi->flags & MULTIBOOT1_INFO_CMDLINE) {
			cmdline = (char *)mbi->cmdline;
		}
		mb_init_serial(cmdline);

		if (mbi->flags & MULTIBOOT1_INFO_FRAMEBUFFER_INFO) {
			struct video_config fb;

			memset(&fb, 0, sizeof(struct video_config));

			fb.addr = mbi->framebuffer_addr;
			fb.height = mbi->framebuffer_height;
			fb.width = mbi->framebuffer_width;
			fb.type = mbi->framebuffer_type;
			fb.depth = mbi->framebuffer_bpp;

			mb_init_video(cmdline, &fb);
		}
	} else
#endif
	if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
		printk("Invalid magic number: 0x%x\n", (unsigned)magic);
		return 1;
	} else {
		struct multiboot_tag *tag;

		printk("Multiboot2 at 0x%x\n", addr);

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
	}

	/* Dealing with the memory map is complicated enough to justify
	 * stuffing it behind a unified routine for MB1 and MB2 */
	kconfig = kconfig_alloc(addr, magic);

	if (!kconfig) {
		printk("error initializing memory\n");
		return 1;
	}

	init_memory(kconfig);

	/* At this point we return to to boot.S/entry.S to clear the stack and
	 * to allow any extra platform specific code to be fired off.  From
	 * there the platform specific code needs to enter the scheduler */
	return 0;
}
