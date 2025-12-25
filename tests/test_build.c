/*
 * FMI/OS Build Test
 * Stage 1: Basic build system validation
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Test that basic build system works */
int main(void)
{
	printf("FMI/OS Stage 1 Build Test\n");
	printf("Testing basic build system functionality...\n");

	/* Basic functionality test */
	const char *test_string = "Hello, Build System!";
	size_t len = strlen(test_string);

	if (len > 0) {
		printf("String length test: PASS (%zu characters)\n", len);
	} else {
		printf("String length test: FAIL\n");
		return 1;
	}

	printf("Build test completed successfully\n");
	return 0;
}
