#ifndef _FMIOS_TYPES_H
#define _FMIOS_TYPES_H

#ifndef __ASSEMBLY__

#ifndef NULL
#define NULL (void *)0
#endif

#if defined(__GNUC__) && defined(__SIZE_TYPE__)
#define SIZE_TYPE		__SIZE_TYPE__
#define WCHAR_TYPE		__WCHAR_TYPE__
#define PTRDIFF_TYPE		__PTRDIFF_TYPE__
#define WINT_TYPE		__WINT_TYPE__
#define INTMAX_TYPE		__INTMAX_TYPE__
#define UINTMAX_TYPE		__UINTMAX_TYPE__
#else
#define SIZE_TYPE	unsigned long
#define PTRDIFF_TYPE	long
#define WCHAR_TYPE	unsigned int
#endif

typedef SIZE_TYPE		size_t;
typedef PTRDIFF_TYPE		ptrdiff_t;
typedef PTRDIFF_TYPE		ssize_t;

/* Mostly just to stop warnings from newlib's headers */
typedef WCHAR_TYPE		wchar_t;
typedef WINT_TYPE		wint_t;

#include <stdint.h>
#include <stdlib.h>

#endif /* __ASSEMBLY__ */

#endif /* _FMIOS_TYPES_H */
