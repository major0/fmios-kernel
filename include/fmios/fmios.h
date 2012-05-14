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

#define weak_symbol(symbol, name) _weak_alias (symbol, name)
#define _weak_alias(symbol, name) \
	extern __typeof (symbol) name __attribute__ ((weak, alias (#symbol)));

#endif /* __ASSEMBLY__ */

#endif /* _FMIOS_FMIOS_H */
