/*
 * FMI/OS Kernel Memory Allocation
 * Minimal memory allocator for Stage 1 Hello World kernel
 *
 * Copyright (C) 2024 FMI/OS Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "klibc.h"
#include <string.h>

/* Very simple memory allocator for Stage 1 - just a stub */
static char memory_pool[64 * 1024]; /* 64KB pool for Stage 1 */
static size_t pool_offset = 0;

void *kmalloc(size_t size)
{
	/* Simple bump allocator for Stage 1 */
	if (size == 0)
		return NULL;

	/* Align to 8-byte boundary */
	size = (size + 7) & ~7;

	if (pool_offset + size > sizeof(memory_pool)) {
		return NULL; /* Out of memory */
	}

	void *ptr = &memory_pool[pool_offset];
	pool_offset += size;

	/* Clear allocated memory */
	memset(ptr, 0, size);

	return ptr;
}

void *kcalloc(size_t nmemb, size_t size)
{
	if (nmemb == 0 || size == 0)
		return NULL;

	/* Check for overflow */
	if (nmemb > SIZE_MAX / size)
		return NULL;

	size_t total_size = nmemb * size;
	void *ptr = kmalloc(total_size);

	/* kmalloc already clears memory, so just return */
	return ptr;
}

void *krealloc(void *ptr, size_t size)
{
	/* Very simple realloc for Stage 1 - just allocate new */
	if (ptr == NULL) {
		return kmalloc(size);
	}

	if (size == 0) {
		kfree(ptr);
		return NULL;
	}

	/* For Stage 1, just allocate new memory and copy */
	void *new_ptr = kmalloc(size);
	if (new_ptr == NULL) {
		return NULL;
	}

	/* We don't track allocation sizes in Stage 1, so this is unsafe */
	/* In a real implementation, we'd track sizes */
	/* For now, just return the new allocation */
	return new_ptr;
}

void kfree(void *ptr)
{
	/* Stage 1 bump allocator doesn't support free */
	/* In a real implementation, this would return memory to pool */
	(void)ptr; /* Suppress unused parameter warning */
}
