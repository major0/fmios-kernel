#include <types.h>
#include <string.h>

size_t strlen(const char *s)
{
	const char *start = s;

	while (*s) {
		s++;
	}

	return s - start;
}
