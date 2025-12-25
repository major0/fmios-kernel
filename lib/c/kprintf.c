/*
 * FMI/OS Kernel Printf Implementation
 * Minimal printf for Stage 1 Hello World kernel
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
#include <stdarg.h>
#include <string.h>

/* Simple serial output for x86_64 (COM1) */
static void serial_putchar(char c)
{
	/* COM1 port 0x3F8 - minimal implementation for Stage 1 */
	__asm__ volatile("outb %0, $0x3F8" : : "a"(c));
}

static void serial_puts(const char *str)
{
	while (*str) {
		serial_putchar(*str++);
	}
}

/* Minimal printf implementation for Stage 1 */
int kprintf(const char *format, ...)
{
	va_list args;
	int result;

	va_start(args, format);
	result = kvprintf(format, args);
	va_end(args);

	return result;
}

/* Printf with va_list for Stage 1 */
int kvprintf(const char *format, va_list args)
{
	char buffer[1024];
	int result;

	result = kvsnprintf(buffer, sizeof(buffer), format, args);
	serial_puts(buffer);
	return result;
}

/* Minimal snprintf implementation */
int ksnprintf(char *str, size_t size, const char *format, ...)
{
	va_list args;
	int result;

	va_start(args, format);
	result = kvsnprintf(str, size, format, args);
	va_end(args);

	return result;
}

/* Very basic vsnprintf for Stage 1 - just handles %s and %d */
int kvsnprintf(char *str, size_t size, const char *format, va_list ap)
{
	size_t pos = 0;
	const char *p = format;

	if (size == 0)
		return 0;

	while (*p && pos < size - 1) {
		if (*p == '%' && *(p + 1)) {
			p++;
			switch (*p) {
				case 's': {
					const char *s = va_arg(ap, const char *);
					if (s) {
						while (*s && pos < size - 1) {
							str[pos++] = *s++;
						}
					}
					break;
				}
				case 'd': {
					int val = va_arg(ap, int);
					char num_buf[32];
					int len = 0;

					if (val == 0) {
						if (pos < size - 1)
							str[pos++] = '0';
					} else {
						bool negative = val < 0;
						if (negative)
							val = -val;

						while (val > 0 && len < 31) {
							num_buf[len++] = '0' + (val % 10);
							val /= 10;
						}

						if (negative && pos < size - 1) {
							str[pos++] = '-';
						}

						while (len > 0 && pos < size - 1) {
							str[pos++] = num_buf[--len];
						}
					}
					break;
				}
				case '%':
					if (pos < size - 1)
						str[pos++] = '%';
					break;
				default:
					if (pos < size - 1)
						str[pos++] = '%';
					if (pos < size - 1)
						str[pos++] = *p;
					break;
			}
		} else {
			str[pos++] = *p;
		}
		p++;
	}

	str[pos] = '\0';
	return pos;
}
