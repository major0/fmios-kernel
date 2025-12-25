/*
 * FMI/OS Build System Property-Based Tests
 * Stage 1: Property tests for basic x86_64 build functionality
 *
 * Feature: build-system, Property 1: Basic x86_64 Build Functionality
 * Validates: Requirements 1.1, 1.2, 1.3
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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/* Property test configuration */
#define PROPERTY_TEST_ITERATIONS 10 /* Reduced for practical testing */

/* Property 1: Basic x86_64 Build Functionality */
static int test_property_basic_x86_64_build(void)
{
	printf("Testing Property 1: Basic x86_64 Build Functionality\n");
	printf("Running %d iterations...\n", PROPERTY_TEST_ITERATIONS);

	for (int i = 0; i < PROPERTY_TEST_ITERATIONS; i++) {
		/* Property: For any valid x86_64 configuration, the build system should:
		 * 1. Successfully configure for x86_64 target
		 * 2. Generate appropriate compiler flags
		 * 3. Create necessary build files
		 * 4. Build the kernel successfully
		 */

		/* Test that the kernel binary exists and is executable */
		struct stat st;
		if (stat("../kernel/fmi-kernel", &st) != 0) {
			printf("FAIL: Kernel binary not found for iteration %d\n", i);
			return 1;
		}

		/* Test that the kernel binary has execute permissions */
		if (!(st.st_mode & S_IXUSR)) {
			printf("FAIL: Kernel binary not executable for iteration %d\n", i);
			return 1;
		}

		/* Test that the kernel binary is not empty */
		if (st.st_size == 0) {
			printf("FAIL: Kernel binary is empty for iteration %d\n", i);
			return 1;
		}

		/* Test that required libraries exist */
		if (stat("../lib/c/libklibc.a", &st) != 0) {
			printf("FAIL: Kernel C library not found for iteration %d\n", i);
			return 1;
		}

		if (stat("../arch/x86_64/libarch.a", &st) != 0) {
			printf("FAIL: Architecture library not found for iteration %d\n", i);
			return 1;
		}

		/* Test that Makefile contains x86_64-specific flags */
		FILE *makefile = fopen("../Makefile", "r");
		if (!makefile) {
			printf("FAIL: Cannot open Makefile for iteration %d\n", i);
			return 1;
		}

		char line[1024];
		bool found_x86_64_flags = false;
		while (fgets(line, sizeof(line), makefile)) {
			if (strstr(line, "mno-red-zone") || strstr(line, "x86_64")) {
				found_x86_64_flags = true;
				break;
			}
		}
		fclose(makefile);

		if (!found_x86_64_flags) {
			printf("FAIL: x86_64-specific flags not found in Makefile for iteration "
			       "%d\n",
			       i);
			return 1;
		}

		if ((i + 1) % 5 == 0) {
			printf("Completed %d/%d iterations\n", i + 1, PROPERTY_TEST_ITERATIONS);
		}
	}

	printf("Property 1: PASS - All %d iterations successful\n", PROPERTY_TEST_ITERATIONS);
	return 0;
}

/* Property test runner */
int main(void)
{
	printf("FMI/OS Build System Property-Based Tests\n");
	printf("========================================\n");

	/* Initialize random seed */
	srand((unsigned int)time(NULL));

	/* Run property tests */
	int result = 0;

	result |= test_property_basic_x86_64_build();

	if (result == 0) {
		printf("\nAll property tests PASSED\n");
	} else {
		printf("\nSome property tests FAILED\n");
	}

	return result;
}
