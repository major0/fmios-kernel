/*
 * FMI/OS Kernel C Library Interface
 * Single source of truth for kernel C library functions
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

#ifndef _KLIBC_H
#define _KLIBC_H

#include "ktypes.h"
#include <stdarg.h>
#include <stddef.h>

/* Kernel I/O functions (different semantics than standard library) */
int kprintf(const char *format, ...);
int ksnprintf(char *str, size_t size, const char *format, ...);
int kvsnprintf(char *str, size_t size, const char *format, va_list ap);

/* Kernel memory management (different semantics than standard library) */
void *kmalloc(size_t size);
void *kcalloc(size_t nmemb, size_t size);
void *krealloc(void *ptr, size_t size);
void kfree(void *ptr);

/* Kernel logging and debugging */
typedef enum {
	KLOG_EMERG = 0,
	KLOG_ALERT,
	KLOG_CRIT,
	KLOG_ERR,
	KLOG_WARNING,
	KLOG_NOTICE,
	KLOG_INFO,
	KLOG_DEBUG
} klog_level_t;

int klogf(klog_level_t level, const char *format, ...);
void kpanic(const char *format, ...) __attribute__((noreturn));

/* Kernel file operations (different semantics than standard library) */
int kopen(const char *pathname, int flags);
ssize_t kread(int fd, void *buf, size_t count);
ssize_t kwrite(int fd, const void *buf, size_t count);
int kclose(int fd);

#endif /* _KLIBC_H */
