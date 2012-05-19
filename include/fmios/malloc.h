#ifndef __FMIOS_MEM_H
#define __FMIOS_MEM_H

#include <stdint.h>

/* This is /almost/ a 1:1 mapping from the Multiboot2 mmap_entry with the
 * exception that t tracks pages instead of bytes and the the reserved element
 * is replaced with 'flags' to be used as a bitfield to usage of available
 * memory */
struct pmap_entry {
	uint32_t	start;
	uint32_t	end;
	uint32_t	type;
	uint32_t	flags;
};

struct pmap_table {
	unsigned long		count;
	struct pmap_entry	entry[0];
};

#endif /* __FMIOS_MEM_H */
