#include <fmios/fmios.h>
#include <fmios/page.h>
#include <fmios/serial.h>
#include <fmios/video.h>
#include <fmios/io.h>
#include <multiboot.h>
#include <string.h>

struct pmap_entry {
	uint32_t	start;
	uint32_t	end;
	uint32_t	type;
	uint32_t	flags;
};

/* This is /almost/ a 1:1 mapping from the Multiboot2 mmap_entry with the
 * exception that the reserved element is replaced with 'flags' to be used as a
 * bitfield to track memory that is 'in use' */
struct pmap_table {
	unsigned long		count;
	struct pmap_entry	entry[0];
};

/* Start looking for free pages at the end of the kernel */
extern const void __kernel_start;
extern const void _end;
unsigned long kernel_start;
unsigned long kernel_end;

extern char * cmdline_get_opt(char *cmdline, char *option);

static void init_video(char *cmdline, struct video_config *config)
{
	char *param;

	param = cmdline_get_opt(cmdline, "video");

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

static void init_serial(char *cmdline)
{
	uint32_t iobase = 0;
	uint32_t baud = 0;
	uint16_t divisor = 0;
	uint8_t flags = 0;
	char *param;

	param = cmdline_get_opt(cmdline, "serial");

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

/* These routines are platform specific and must be defined to boot */
static void __init_memory(struct pmap_table *pmap)
{
	struct pmap_entry *entry = pmap->entry;
	int index;

	printk("Page Mapping:\n");

	/* Display the memory map we currently have to aid in board bringups */
	for (index = 0; index < pmap->count; index++) {
		if (entry[index].type != MULTIBOOT_MEMORY_AVAILABLE) {
			continue;
		}

		switch (entry[index].flags) {
		case MEMORY_PMAP_KERNEL:
			printk("  kernel: ");
			break;
		case MEMORY_PMAP_LOADER:
			printk("  loader: ");
			break;
		case MEMORY_PMAP_MODULE:
			printk("  module: ");
			break;
		default:
			printk("  unused: ");
			break;
		}

		printk("start=0x%x, end=0x%x\n", entry[index].start,
				entry[index].end);
	}

	printk("No platform initialization defined\n");
}
weak_symbol(__init_memory, init_memory);

/* Find free pages for the kernel config */
static void * find_free_pages(size_t count)
{
	unsigned long pstart = PAGE_NUM(kernel_end) + 1;
	unsigned long pend = pstart + count;
	int mod_count = mb_mod_count();
	int module;

	/* Check the MBI location */
	if (PAGE_NUM(mb_mbi_start()) < pend &&
	    PAGE_NUM(mb_mbi_start()) > pstart) {
		pstart = (PAGE_NUM(mb_mbi_end()) + 1);
		pend = pstart + count;
	}

	for (module = 0; module < mod_count; module++) {
		unsigned long start;
		unsigned long end;

		start = mb_mod_start(module);
		end = mb_mod_end(module);

		if (!start || !end) {
			continue;
		}

		if (PAGE_NUM(start) <= pend && PAGE_NUM(start) >= pstart) {
			pstart = PAGE_NUM(end + 1);
			pend = (pstart + (count - 1));
		}
	}

	/* FIXME is this address in valid memory? */
	return (void *)(pstart * PAGE_SIZE);
}

static int pmap_shift(struct pmap_entry *entries, int count)
{
	int total = 0;
	int index;

#ifdef CONFIG_ENABLE_DEBUG
	printk("pmap_shift: shifting by %d\n", count);
#endif

	/* Figure out how many entries we have left */
	while (entries[total+1].flags != MEMORY_PMAP_END) {
		total++;
		if (total > 42) {
			printk("pmap_shift: huge pmap?\n");
		}
	}

	if (!total) {
		printk("error: failed to shift page map\n");
		return 0;
	}

	/* This is something of a reverse memcpy as we are copying overlapping
	 * regions */
	while (count--) {
		index = total - 1;
		while ((index) >= 0) {
			if (entries[index].type) {
				memcpy(&entries[index+1], &entries[index],
					sizeof(struct pmap_entry));
			}
			index--;
		}
	}

	return 1;
}

/* Insert the given pmap entry into the existing pmap table.  We need to
 * properly split existing regions and reset the start/len elements with each
 * insert. */
static int pmap_add(struct pmap_entry *entries, struct pmap_entry *entry, int index)
{
	/* Sanity checking */
	if (entries[index].type != MULTIBOOT_MEMORY_AVAILABLE) {
		return 0;
	}
	if (entry->start > entries[index].end) {
		return 0;
	}

#ifdef CONFIG_ENABLE_DEBUG
	printk("pmap_add:\n");
	printk("  cur: start=0x%x, end=0x%x, type=0x%x, flags=0x%x\n",
			entries[index].start, entries[index].end,
			entries[index].type, entries[index].flags);
	printk("  new: start=0x%x, end=0x%x, type=0x%x, flags=0x%x\n",
			entry->start, entry->end, entry->type, entry->flags);
#endif

	/* Add the entry to the list in a sorted order.  We have 3 types of
	 * insertions we can do.
	 * 1) Insert this entry at the start of the current entry
	 * 2) Insert this entry at the tail of the current entry
	 * 3) Split the current entry in two and do the insertion.  */

	/* Consumes the start of this entry */
	if (entry->start == entries[index].start &&
	    entry->end >= entries[index].start) {

		/* Extend the next current with the new entry */
		if (entries[index].flags == entry->flags &&
		    entries[index].end <= entry->end) {
			entries[index].end = entry->end;
			if (entries[index].start < entry->end) {
				entries[index+1].start = entry->end + 1;
			}
			return 0;
		}

		if (!pmap_shift(&entries[index], 1)) {
			return -1;
		}
		entries[index].start = entry->start;
		entries[index].end = entry->end;
		entries[index].type = entry->type;
		entries[index].flags = entry->flags;

		entries[index+1].start = entry->end + 1;
		return 1;
	}

	/* Consume the tail of the current entry */
	if (entry->start <= entries[index].end &&
	    entry->end >= entries[index].end) {

		/* Merge with the current entry if the flags are the same */
		/* FIXME do we need to merge the follow entry as well? */
		if (entries[index].flags == entry->flags) {
			entries[index].end = entry->end;
			if (entries[index+1].type == MULTIBOOT_MEMORY_AVAILABLE
			 && entries[index+1].start <= entry->end) {
				entries[index+1].start = entry->end + 1;
			}
			return 0;
		}

		/* Merge the next entry with the new entry */
		if (entries[index+1].flags == entry->flags &&
		    entries[index+1].start >= entry->end) {
			entries[index+1].start = entry->start;
			entries[index].end = entry->start - 1;
			return 0;
		}

		/* make space to add a new entry */
		if (!pmap_shift(&entries[index], 1)) {
			return -1;
		}

		entries[index].end = entry->start - 1;
		entries[index+1].start = entry->start;
		entries[index+1].end = entry->end;
		entries[index+1].type = entry->type;
		entries[index+1].flags = entry->flags;

		return 1;
	}

	/* Split the current entry in 2 and insert the new entry after
	 * the current */
	if (entry->start < entries[index].end &&
	    entry->end > entries[index].start) {

		/* This really shouldn't be possible */
		if (entry->flags == entries[index].flags) {
			printk("error: pmap_add() overlapping entries\n");
			return -1;
		}

		if (!pmap_shift(&entries[index], 2)) {
			return -1;
		}

		entries[index+2].start = entry->end + 1;
		entries[index+2].end = entries[index].end;
		entries[index+2].type = entries[index].type;
		entries[index+2].flags = entries[index].flags;

		entries[index+1].start = entry->start;
		entries[index+1].end = entry->end;
		entries[index+1].type = entry->type;
		entries[index+1].flags = entry->flags;

		entries[index].end = entry->start - 1;

		return 2;
	}

	printk("error: pmap_add bottomed out\n");
	return -1;
}

static int __pmap_init(struct pmap_table *pmap)
{
	struct pmap_entry *entries = pmap->entry;
	struct pmap_entry new;
	int index, entry_max;
	int mod, mod_max;
	int ret;

	/* This loop makes 2 assumptions.
	 * 1) That the memory map returned by multiboot is already ordered from
	 *    start of memory (low address) to the end of memory (high address).
	 * 2) That no memory mappings returned by multiboot contain overlapping
	 *    regions.
	 */
	entry_max = mb_mmap_count();
#ifdef CONFIG_ENABLE_DEBUG
	printk("Initializing page map with %d entries\n", entry_max);
#endif
	for (index = 0; index < entry_max; index++) {
		if (!entries[index].type) {
			entries[index].start = PAGE_NUM(mb_mmap_start(index));
			entries[index].end = PAGE_NUM(mb_mmap_end(index)) - 1;
			entries[index].type = mb_mmap_type(index);
			entries[index].flags = MEMORY_PMAP_UNUSED;

#ifdef CONFIG_ENABLE_DEBUG
			printk("pmap_init: start=0x%x, end=0x%x, type=0x%x, flags=0x%x\n",
					entries[index].start, entries[index].end,
					entries[index].type, entries[index].flags);
#endif
		}

		if (entries[index].type != MULTIBOOT_MEMORY_AVAILABLE) {
			continue;
		}

		if (PAGE_NUM(kernel_start) <= entries[index].end &&
		    PAGE_NUM(kernel_start) >= entries[index].start) {
			new.start = PAGE_NUM(kernel_start);
			new.end = PAGE_NUM(kernel_end);
			new.type = MULTIBOOT_MEMORY_AVAILABLE;
			new.flags = MEMORY_PMAP_KERNEL;

			ret = pmap_add(pmap->entry, &new, index);
			if (ret < 0) {
				printk("error: failed to map the kernel\n");
				return 0;
			}
		}

		if (PAGE_NUM(mb_mbi_start()) <= entries[index].end &&
		    PAGE_NUM(mb_mbi_end()) >= entries[index].start) {
			new.start = PAGE_NUM(mb_mbi_start());
			new.end = PAGE_NUM(mb_mbi_end());
			new.type = MULTIBOOT_MEMORY_AVAILABLE;
			new.flags = MEMORY_PMAP_LOADER;

			ret = pmap_add(pmap->entry, &new, index);
			if (ret < 0) {
				printk("error: failed to map multiboot information\n");
				return 0;
			}
		}

		/* Find any module data */
		mod_max = mb_mod_count();
		for (mod = 0; index < mod_max; mod++) {
			if (PAGE_NUM(mb_mod_start(mod)) <= entries[index].end &&
			    PAGE_NUM(mb_mod_end(mod)) >= entries[index].start) {
				new.start = PAGE_NUM(mb_mod_start(mod));
				new.end = PAGE_NUM(mb_mod_end(mod));
				new.type = MULTIBOOT_MEMORY_AVAILABLE;
				new.flags = MEMORY_PMAP_MODULE;

				ret = pmap_add(pmap->entry, &new, index);
				if (ret < 0) {
					printk("error: failed to map module\n");
					return 0;
				}
			}
		}
	}

	index = 0;
	while (entries[index].type) {
		index++;
	}
	pmap->count = index;

	return index;
}

/**
 * @return pointer to the kernel configuration structure
 *
 * This routine is responsible for examining the available memory as advertised
 * by the multiboot information and figuring out which portions of that memory
 * are in-use by existing data such as the kernel, the multiboot information,
 * and the boot modules. Fundementally this comes down to some non-trivial work
 * to locate a free page in memory in which to copy all the useful data we
 * need.
 */
static struct pmap_table * pmap_init()
{
	/* The number of pmap_entry structures we need, technically every entry
	 * added to an existing map entry has the potential to split entries in
	 * half.  So in a worst-case condition we need double the count. */
	size_t count = 2;
	size_t size = sizeof(struct pmap_table);
	struct pmap_table *pmap;

	count += mb_mmap_count();
	count += mb_mod_count();

	size += (count * sizeof(struct pmap_entry));

	pmap = (struct pmap_table *)find_free_pages(PAGE_NUM(size) + 1);

	if (pmap) {
		memset(pmap, 0, sizeof(struct pmap_table));

		memset(pmap->entry, 0, (sizeof(struct pmap_entry) * count));
		pmap->count = count;
		pmap->entry[count].flags = MEMORY_PMAP_END;

		if (__pmap_init(pmap)) {
			struct pmap_entry entry;
			int ret;

			entry.start = PAGE_NUM((unsigned long)pmap);
			entry.end = PAGE_NUM((unsigned long)pmap + size);
			entry.type = MULTIBOOT_MEMORY_AVAILABLE;
			entry.flags = MEMORY_PMAP_KERNEL;

			ret = pmap_add(&pmap->entry[0], &entry, pmap->count);
			if (ret >= 0) {
				return pmap;
			}
		}
	}

	return NULL;
}

/** Start of OS independant initialization
 * @magic Multiboot magic number
 * @addr Address of Multiboot Information Structure
 * Parse the Multiboot Information and initialize the system */
int fmios_init(unsigned long magic, unsigned long addr)
{
	struct pmap_table *pmap;
	char *cmdline = "";

	kernel_start = (unsigned long)&__kernel_start;
	kernel_end = (unsigned long)&_end;

	if (!mb_init(addr, magic)) {
		return 1;
	}

	cmdline = mb_mbi_cmdline();

	init_serial(cmdline);

	/* Grab the curent video configuration from within multiboot. */
	if (mb_fb_type()) {
		struct video_config fb;

		memset(&fb, 0, sizeof(struct video_config));

		fb.addr = mb_fb_addr();
		fb.pitch = mb_fb_pitch();
		fb.width = mb_fb_width();
		fb.height = mb_fb_height();
		fb.depth = mb_fb_depth();
		fb.type = mb_fb_type();

		if (fb.type) {
			init_video(cmdline, &fb);
		}
	}

	printk("FMIOS\n"); /* FIXME add version information */

	/* Initialize a temporary page mapping to be used by the platform
	 * specific init_memory() routine.  This map lists what memory is
	 * available, and what memory is currently in-use by the kernel, mbi,
	 * and modules. */
	pmap = pmap_init();
	if (!pmap) {
		printk("error mapping memory\n");
		return 1;
	}
	init_memory(pmap);

	/* At this point we return to to boot.S/entry.S to clear the stack and
	 * to allow any extra platform specific code to be fired off.  From
	 * there the platform specific code needs to enter the scheduler */
	return 0;
}
