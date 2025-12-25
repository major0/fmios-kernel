/*
 * FMI/OS Kernel Types
 * Basic type definitions for the kernel
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

#ifndef _KTYPES_H
#define _KTYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Basic kernel types */
typedef int32_t pid_t;
typedef int32_t tid_t;
typedef int32_t pgid_t;
typedef long ssize_t;

/* Kernel object types */
enum kobj_type { KOBJ_INVALID = 0, KOBJ_SERVICE, KOBJ_PROCESS, KOBJ_THREAD, KOBJ_MEMORY, KOBJ_IPC };

/* Forward declarations for kernel objects */
struct kobj_s;
typedef struct kobj_s kobj_t;

#endif /* _KTYPES_H */
