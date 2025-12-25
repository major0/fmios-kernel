/*
 * FMI/OS Kernel Linker Script
 * Stage 1: Multiboot2 format for GRUB bootloader
 * Multiboot2 starts in 32-bit mode, transitions to 64-bit
 */

ENTRY(_start)

SECTIONS
{
	/* Multiboot2 load address - GRUB loads at 1MB by default */
	. = 1M;

	/* Multiboot2 header must be within first 32KB and early in the file */
	.multiboot2 : ALIGN(8) {
		*(.multiboot2)
	}

	/* 32-bit bootstrap code (Multiboot2 entry point) immediately after header */
	.text : ALIGN(16) {
		__text_start = .;
		*(.text)
		*(.text.*)
		__text_end = .;
	}

	/* Page tables for long mode transition */
	. = ALIGN(4K);
	__page_tables_start = .;
	. += 0x3000;                   /* Reserve space for PML4, PDPT, PD */
	__page_tables_end = .;

	/* Initialization code - can be freed after boot */
	.init : ALIGN(4K) {
		__init_start = .;
		*kernel/main.o(.text)
		*(.init.text)
		__init_end = .;
	}

	/* Global Descriptor Table and other data */
	.data : ALIGN(4K) {
		__data_start = .;
		*(.data)
		*(.data.*)
		__data_end = .;
	}

	/* Read-only data */
	.rodata : ALIGN(4K) {
		__rodata_start = .;
		*(.rodata)
		*(.rodata.*)
		__rodata_end = .;
	}

	/* Uninitialized data */
	.bss : ALIGN(4K) {
		__bss_start = .;
		*(.bss)
		*(.bss.*)
		*(COMMON)
		__bss_end = .;
	}

	/* Kernel end marker */
	__kernel_end = .;

	/* Discard sections we don't need */
	/DISCARD/ : {
		*(.comment)
		*(.eh_frame)
		*(.eh_frame_hdr)
	}
}
