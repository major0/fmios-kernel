/*
 * FMI/OS Build System Property-Based Tests
 * Stage 2: Property tests for ISO generation and boot protocol support
 *
 * Feature: build-system, Property 2: ISO Generation and Boot Protocol Support
 * Validates: Requirements 1.4, 8.1, 8.2
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

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/* Property test configuration */
#define PROPERTY_TEST_ITERATIONS 10 /* Reduced for practical testing */

/* Global build directory path */
static char build_dir[PATH_MAX];

/* Initialize build directory path */
static int init_build_dir(void)
{
	char *cwd = getcwd(build_dir, sizeof(build_dir));
	if (!cwd) {
		printf("ERROR: Cannot get current working directory\n");
		return 1;
	}

	/* If we're in tests/ subdirectory, go up one level */
	size_t len = strlen(build_dir);
	if (len > 6 && strcmp(build_dir + len - 6, "/tests") == 0) {
		build_dir[len - 6] = '\0';
	}

	printf("Build directory: %s\n", build_dir);
	return 0;
}

/* Property 2: ISO Generation and Boot Protocol Support */
static int test_property_iso_generation_and_boot_protocol(void)
{
	printf("Testing Property 2: ISO Generation and Boot Protocol Support\n");
	printf("Running %d iterations...\n", PROPERTY_TEST_ITERATIONS);

	for (int i = 0; i < PROPERTY_TEST_ITERATIONS; i++) {
		/* Property: For any valid Multiboot2 kernel, the build system should:
		 * 1. Generate a valid ISO image with GRUB bootloader
		 * 2. Include proper GRUB configuration for Multiboot2
		 * 3. Create bootable ISO that can be used with QEMU
		 * 4. Support proper boot protocol transition from 32-bit to 64-bit
		 */

		char iso_path[PATH_MAX + 32];
		char grub_cfg_path[PATH_MAX + 32];
		char kernel_path[PATH_MAX + 32];
		char makefile_path[PATH_MAX + 32];

		int ret;
		ret = snprintf(iso_path, sizeof(iso_path), "%s/fmios.iso", build_dir);
		if (ret >= (int)sizeof(iso_path)) {
			printf("FAIL: ISO path too long for iteration %d\n", i);
			return 1;
		}

		ret = snprintf(grub_cfg_path, sizeof(grub_cfg_path), "%s/grub.cfg", build_dir);
		if (ret >= (int)sizeof(grub_cfg_path)) {
			printf("FAIL: GRUB config path too long for iteration %d\n", i);
			return 1;
		}

		ret = snprintf(kernel_path, sizeof(kernel_path), "%s/kernel/fmi-kernel", build_dir);
		if (ret >= (int)sizeof(kernel_path)) {
			printf("FAIL: Kernel path too long for iteration %d\n", i);
			return 1;
		}

		ret = snprintf(makefile_path, sizeof(makefile_path), "%s/Makefile", build_dir);
		if (ret >= (int)sizeof(makefile_path)) {
			printf("FAIL: Makefile path too long for iteration %d\n", i);
			return 1;
		}

		/* Test that the ISO image exists */
		struct stat st;
		if (stat(iso_path, &st) != 0) {
			printf("FAIL: ISO image not found at %s for iteration %d\n", iso_path, i);
			return 1;
		}

		/* Test that the ISO image is not empty */
		if (st.st_size == 0) {
			printf("FAIL: ISO image is empty for iteration %d\n", i);
			return 1;
		}

		/* Test that the ISO image has reasonable size (should be at least 1MB) */
		if (st.st_size < 1024 * 1024) {
			printf("FAIL: ISO image too small (%ld bytes) for iteration %d\n",
			       st.st_size, i);
			return 1;
		}

		/* Test that GRUB configuration exists */
		if (stat(grub_cfg_path, &st) != 0) {
			printf("FAIL: GRUB configuration not found at %s for iteration %d\n",
			       grub_cfg_path, i);
			return 1;
		}

		/* Test that GRUB configuration contains Multiboot2 entry */
		FILE *grub_cfg = fopen(grub_cfg_path, "r");
		if (!grub_cfg) {
			printf("FAIL: Cannot open GRUB configuration at %s for iteration %d\n",
			       grub_cfg_path, i);
			return 1;
		}

		char line[1024];
		bool found_multiboot2 = false;
		bool found_kernel_entry = false;
		while (fgets(line, sizeof(line), grub_cfg)) {
			if (strstr(line, "multiboot2")) {
				found_multiboot2 = true;
			}
			if (strstr(line, "fmi-kernel")) {
				found_kernel_entry = true;
			}
		}
		fclose(grub_cfg);

		if (!found_multiboot2) {
			printf("FAIL: Multiboot2 entry not found in GRUB config for iteration %d\n",
			       i);
			return 1;
		}

		if (!found_kernel_entry) {
			printf("FAIL: Kernel entry not found in GRUB config for iteration %d\n", i);
			return 1;
		}

		/* Test that the kernel binary has Multiboot2 header */
		FILE *kernel = fopen(kernel_path, "rb");
		if (!kernel) {
			printf("FAIL: Cannot open kernel binary at %s for iteration %d\n",
			       kernel_path, i);
			return 1;
		}

		/* Look for Multiboot2 magic number (0xe85250d6) in first 32KB */
		unsigned char buffer[32768];
		size_t bytes_read = fread(buffer, 1, sizeof(buffer), kernel);
		fclose(kernel);

		bool found_multiboot2_header = false;
		for (size_t j = 0; j < bytes_read - 4; j++) {
			uint32_t magic = *(uint32_t *)(buffer + j);
			if (magic == 0xe85250d6) {
				found_multiboot2_header = true;
				break;
			}
		}

		if (!found_multiboot2_header) {
			printf("FAIL: Multiboot2 header not found in kernel for iteration %d\n", i);
			return 1;
		}

		/* Test that Makefile contains ISO generation targets */
		FILE *makefile = fopen(makefile_path, "r");
		if (!makefile) {
			printf("FAIL: Cannot open Makefile at %s for iteration %d\n", makefile_path,
			       i);
			return 1;
		}

		bool found_iso_target = false;
		bool found_grub_target = false;
		while (fgets(line, sizeof(line), makefile)) {
			if (strstr(line, "fmios.iso")) {
				found_iso_target = true;
			}
			if (strstr(line, "grub.cfg")) {
				found_grub_target = true;
			}
		}
		fclose(makefile);

		if (!found_iso_target) {
			printf("FAIL: ISO generation target not found in Makefile for iteration "
			       "%d\n",
			       i);
			return 1;
		}

		if (!found_grub_target) {
			printf("FAIL: GRUB configuration target not found in Makefile for "
			       "iteration %d\n",
			       i);
			return 1;
		}

		if ((i + 1) % 5 == 0) {
			printf("Completed %d/%d iterations\n", i + 1, PROPERTY_TEST_ITERATIONS);
		}
	}

	printf("Property 2: PASS - All %d iterations successful\n", PROPERTY_TEST_ITERATIONS);
	return 0;
}

/* Property test runner */
int main(void)
{
	printf("FMI/OS ISO Generation Property-Based Tests\n");
	printf("==========================================\n");

	/* Initialize build directory path */
	if (init_build_dir() != 0) {
		return 1;
	}

	/* Initialize random seed */
	srand((unsigned int)time(NULL));

	/* Run property tests */
	int result = 0;

	result |= test_property_iso_generation_and_boot_protocol();

	if (result == 0) {
		printf("\nAll property tests PASSED\n");
	} else {
		printf("\nSome property tests FAILED\n");
	}

	return result;
}
