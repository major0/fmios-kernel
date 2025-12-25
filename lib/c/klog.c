/*
 * FMI/OS Kernel Logging Implementation
 * Minimal logging for Stage 1 Hello World kernel
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

/* Log level names */
static const char *log_level_names[] = {"EMERG", "ALERT",  "CRIT", "ERR",
					"WARN",	 "NOTICE", "INFO", "DEBUG"};

int klogf(klog_level_t level, const char *format, ...)
{
	va_list args;
	int result;

	/* Print log level prefix */
	if (level < sizeof(log_level_names) / sizeof(log_level_names[0])) {
		kprintf("[%s] ", log_level_names[level]);
	} else {
		kprintf("[UNKNOWN] ");
	}

	/* Print the actual message */
	va_start(args, format);
	result = kvprintf(format, args);
	va_end(args);

	return result;
}

void kpanic(const char *format, ...)
{
	va_list args;

	kprintf("\n*** KERNEL PANIC ***\n");

	va_start(args, format);
	kprintf(format, args);
	va_end(args);

	kprintf("\nSystem halted.\n");

	/* Halt the system */
	while (1) {
		__asm__ volatile("hlt");
	}
}
