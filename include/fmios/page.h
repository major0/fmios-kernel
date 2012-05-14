#ifndef _FMIOS_PAGE_H
#define _FMIOS_PAGE_H

#ifndef __ASSEMBLY__

#define PAGE_NUM(addr)	(((unsigned long)addr) / PAGE_SIZE)
/* FIXME this should just mask out the lower bits to find the page address */
#define PAGE_OF(addr)	((((unsigned long)addr) / PAGE_SIZE) * PAGE_SIZE)

#endif /* __ASSEMBLY__ */

#endif
