#ifndef _FMIOS_FMIOS_H
#define _FMIOS_FMIOS_H

#include <fmios/config.h>
#include <asm/config.h>
#include <fmios/types.h>

#define MEMORY_PMAP_UNUSED	0x00
#define MEMORY_PMAP_KERNEL	0x01
#define MEMORY_PMAP_LOADER	0x02
#define MEMORY_PMAP_MODULE	0x04
#define MEMORY_PMAP_END		~(0)

#ifndef __ASSEMBLY__

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

struct kconfig {
	unsigned long		mb_addr;
	unsigned long		mb_magic;
	char			*cmdline;

	/* Fix alignment issues? */
	uint32_t		reserved;

	/* Try to keep this at the bottom of the structure */
	struct pmap_table	pmap;
};

#define weak_symbol(symbol, name) _weak_alias (symbol, name)
#define _weak_alias(symbol, name) \
	extern __typeof (symbol) name __attribute__ ((weak, alias (#symbol)));

#endif /* __ASSEMBLY__ */

#endif /* _FMIOS_FMIOS_H */
