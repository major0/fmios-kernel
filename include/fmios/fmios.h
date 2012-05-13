#ifndef _FMIOS_FMIOS_H
#define _FMIOS_FMIOS_H

#include <fmios/config.h>
#include <asm/config.h>
#include <fmios/types.h>

#define MEMORY_MAP_UNUSED	0x00
#define MEMORY_MAP_KERNEL	0x01
#define MEMORY_MAP_LOADER	0x02
#define MEMORY_MAP_MODULE	0x04
#define MEMORY_MAP_END		~(0)

#ifndef __ASSEMBLY__

struct mmap_entry {
	uint64_t	addr;
	uint64_t	len;
	uint32_t	type;
	uint32_t	flags;
};

/* This is /almost/ a 1:1 mapping from the Multiboot2 mmap_entry with the
 * exception that the reserved element is replaced with 'flags' to be used as a
 * bitfield to track memory that is 'in use' */
struct mmap_table {
	unsigned long		count;
	struct mmap_entry	entry[0];
};

struct kconfig {
	unsigned long		mb_addr;
	unsigned long		mb_magic;
	char			*cmdline;

	/* Fix alignment issues? */
	uint32_t		reserved;

	/* Try to keep this at the bottom of the structure */
	struct mmap_table	mmap;
};

#define weak_symbol(symbol, name) _weak_alias (symbol, name)
#define _weak_alias(symbol, name) \
	extern __typeof (symbol) name __attribute__ ((weak, alias (#symbol)));

#endif /* __ASSEMBLY__ */

#endif /* _FMIOS_FMIOS_H */
