#include <fmios/fmios.h>
#include <fmios/malloc.h>
#include <fmios/page.h>
#include <fmios/io.h>

#include <string.h>

#include <multiboot.h>

extern const void __kernel_start;
extern const void _end;
extern unsigned long kernel_start;
extern unsigned long kernel_end;

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
	index = total - count;
	while ((index) >= 0) {
		if (entries[index].type) {
			memcpy(&entries[index+count], &entries[index],
				sizeof(struct pmap_entry));
		}
		index--;
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
struct pmap_table * init_malloc()
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
