/*
 * FMI/OS Kernel Linker Script
 * Stage 1: PVH ELF format for x86_64
 */

ENTRY(_start)

SECTIONS
{
	/* Load address for PVH ELF kernel */
	. = 0x200000;

	/* Initialization code - can be freed after boot */
	.init : ALIGN(4K) {
		__init_start = .;
		*arch/x86_64/bootstrap.o(.text)
		*arch/x86_64/entry.o(.text)
		*kernel/main.o(.text)
		*(.init.text)
		__init_end = .;
	}

	/* Runtime kernel code */
	.text : ALIGN(4K) {
		__text_start = .;
		*(.text)
		*(.text.*)
		__text_end = .;
	}

	/* Read-only data */
	.rodata : ALIGN(4K) {
		__rodata_start = .;
		*(.rodata)
		*(.rodata.*)
		__rodata_end = .;
	}

	/* Initialized data */
	.data : ALIGN(4K) {
		__data_start = .;
		*(.data)
		*(.data.*)
		__data_end = .;
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
		*(.note.*)
		*(.eh_frame)
		*(.eh_frame_hdr)
	}
}
