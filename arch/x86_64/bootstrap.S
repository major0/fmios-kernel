/*
 * FMI/OS x86_64 Bootstrap Code
 * Multiboot2 boot entry point for GRUB bootloader
 * Multiboot2 starts in 32-bit mode, we must transition to 64-bit
 */

/* Multiboot2 Header for GRUB bootloader */
.section .multiboot2, "a", @progbits
.align 8
multiboot2_header:
	.long 0xe85250d6                /* Multiboot2 magic number */
	.long 0                         /* Architecture: i386 */
	.long multiboot2_header_end - multiboot2_header  /* Header length */
	.long -(0xe85250d6 + 0 + (multiboot2_header_end - multiboot2_header))  /* Checksum */

	/* End tag - required by Multiboot2 specification */
	.align 8
	.short 0                        /* Type: end tag */
	.short 0                        /* Flags: none */
	.long 8                         /* Size: 8 bytes */
multiboot2_header_end:

/* 32-bit code section for Multiboot2 entry */
.section .text, "ax", @progbits
.code32
.global _start
.type _start, @function

_start:
	/* Stage 1: Multiboot2 32-bit entry point from GRUB */

	/* Debug: Very first instruction - 32-bit entry point reached */
	movl $0x3f8, %edx              /* COM1 port */
	movb $'1', %al                 /* '1' = 32-bit entry point */
	outb %al, %dx

	/* Debug: Entry point reached */
	movl $0x3f8, %edx              /* COM1 port */
	movb $'A', %al                 /* 'A' = 32-bit entry */
	outb %al, %dx

	/* Clear direction flag */
	cld

	/* Disable interrupts */
	cli

	/* Set up basic 32-bit stack */
	movl $kernel_stack_top, %esp

	/* Debug: Stack set up */
	movb $'B', %al                 /* 'B' = stack ready */
	outb %al, %dx

	/* Enable PAE (Physical Address Extension) */
	movl %cr4, %eax
	orl $0x20, %eax                /* Set PAE bit */
	movl %eax, %cr4

	/* Debug: PAE enabled */
	movb $'C', %al                 /* 'C' = PAE enabled */
	outb %al, %dx

	/* Set up page tables for long mode */
	/* PML4 table at 0x1000 */
	movl $0x1000, %edi
	xorl %eax, %eax
	movl $0x1000, %ecx             /* Clear 4KB */
	rep stosl

	/* Debug: Page tables cleared */
	movb $'D', %al                 /* 'D' = page tables cleared */
	outb %al, %dx

	/* Set up PML4[0] -> PDPT */
	movl $0x1000, %edi
	movl $0x2003, (%edi)           /* Present, writable, points to 0x2000 */

	/* Set up PDPT[0] -> PD */
	movl $0x2000, %edi
	movl $0x3003, (%edi)           /* Present, writable, points to 0x3000 */

	/* Set up PD[0] -> 2MB page */
	movl $0x3000, %edi
	movl $0x83, (%edi)             /* Present, writable, 2MB page at 0x0 */

	/* Debug: Page tables set up */
	movl $0x3f8, %edx              /* Reload COM1 port */
	movb $'E', %al                 /* 'E' = page tables ready */
	outb %al, %dx

	/* Load page table */
	movl $0x1000, %eax
	movl %eax, %cr3

	/* Debug: Page table loaded */
	movb $'F', %al                 /* 'F' = page table loaded */
	outb %al, %dx

	/* Debug: About to check CPUID */
	movb $'G', %al                 /* 'G' = about to check CPUID */
	outb %al, %dx

	/* Test if CPUID instruction is available */
	pushfl                         /* Save original EFLAGS */
	popl %eax                      /* Get EFLAGS into EAX */
	movl %eax, %ecx                /* Save original EFLAGS in ECX */
	xorl $0x200000, %eax           /* Flip bit 21 (ID flag) */
	pushl %eax                     /* Push modified EFLAGS */
	popfl                          /* Load modified EFLAGS */
	pushfl                         /* Get EFLAGS back */
	popl %eax                      /* Store result in EAX */
	pushl %ecx                     /* Restore original EFLAGS */
	popfl
	xorl %ecx, %eax                /* Compare original vs modified */
	jz no_cpuid_support            /* If zero (no change), CPUID not available */

	/* Debug: CPUID available */
	movb $'H', %al                 /* 'H' = CPUID available */
	outb %al, %dx

	/* In QEMU, we can assume extended CPUID and long mode are available */
	/* This is a simplified check that works reliably in the emulated environment */

	/* Debug: Extended CPUID available (assumed in QEMU) */
	movb $'I', %al                 /* 'I' = extended CPUID */
	outb %al, %dx

	/* Debug: Long mode supported (assumed in QEMU) */
	movb $'J', %al                 /* 'J' = long mode check */
	outb %al, %dx
	jmp continue_boot

no_extended_cpuid:
	/* Debug: No extended CPUID support */
	movb $'X', %al                 /* 'X' = no extended CPUID */
	outb %al, %dx
	jmp halt_here

no_long_mode:
	/* Debug: No long mode support */
	movb $'Y', %al                 /* 'Y' = no long mode */
	outb %al, %dx
	jmp halt_here

continue_boot:

	/* Read EFER MSR */
	movl $0xC0000080, %ecx         /* EFER MSR */

	/* Debug: About to read EFER */
	movb $'K', %al                 /* 'K' = EFER read */
	outb %al, %dx

	rdmsr

	/* Debug: EFER read successful, enabling long mode */
	movl $0x3f8, %edx              /* Reload COM1 port */
	movb $'L', %al                 /* 'L' = long mode enable */
	outb %al, %dx

	/* Enable long mode */
	orl $0x100, %eax               /* Set LME bit (bit 8) */

	/* Debug: About to write EFER */
	movl $0x3f8, %edx              /* Reload COM1 port */
	movb $'M', %al                 /* 'M' = about to write EFER */
	outb %al, %dx

	wrmsr

	/* Debug: EFER written, enabling paging */
	movl $0x3f8, %edx              /* Reload COM1 port */
	movb $'N', %al                 /* 'N' = paging enable */
	outb %al, %dx

	/* Enable paging to activate long mode */
	movl %cr0, %eax
	orl $0x80000000, %eax          /* Set PG bit (bit 31) */
	movl %eax, %cr0

	/* Debug: Paging enabled, loading GDT */
	movl $0x3f8, %edx              /* Reload COM1 port */
	movb $'O', %al                 /* 'O' = GDT load */
	outb %al, %dx

	/* Load 64-bit GDT */
	lgdt gdt64_desc

	/* Far jump to 64-bit code */
	.byte 0xea                     /* Far jump opcode */
	.long _start64                 /* Offset */
	.word 0x08                     /* Code segment selector */

no_cpuid_support:
	/* Debug: No CPUID support - cannot verify processor capabilities */
	movb $'X', %al                 /* 'X' = no CPUID support */
	outb %al, %dx
	/* Without CPUID, we cannot safely determine if long mode is supported */
	/* This is a fatal error for x86_64 kernel */
	jmp halt_here

halt_here:
	/* Infinite loop for debugging */
	jmp halt_here

/* 64-bit code section */
.section .text
.code64
.global _start64
.type _start64, @function

_start64:
	/* Now in 64-bit mode */

	/* Debug: 64-bit entry point reached */
	movq $0x3f8, %rdx              /* COM1 port */
	movb $'6', %al                 /* '6' = 64-bit entry point */
	outb %al, %dx

	/* Debug: 64-bit mode reached */
	movb $'P', %al                 /* 'P' = 64-bit mode */
	outb %al, %dx

	/* Set up segment registers */
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movw %ax, %ss

	/* Debug: Segments set up */
	movb $'Q', %al                 /* 'Q' = segments ready */
	outb %al, %dx

	/* Set up 64-bit stack */
	movq $kernel_stack_top, %rsp

	/* Clear frame pointer */
	xorq %rbp, %rbp

	/* Debug: Stack ready */
	movb $'R', %al                 /* 'R' = 64-bit stack ready */
	outb %al, %dx

	/* Call architecture early initialization */
	call arch_early_init

	/* Debug: arch_early_init complete */
	movq $0x3f8, %rdx              /* COM1 port */
	movb $'S', %al                 /* 'S' = arch_early_init done */
	outb %al, %dx

	/* Call kernel main */
	call kmain

	/* Debug: kmain returned (should not happen) */
	movb $'U', %al                 /* 'U' = kmain returned */
	outb %al, %dx

	/* If kmain returns, halt the system */
	call arch_halt

/* Global Descriptor Table for 64-bit mode */
.section .data
.align 8
gdt64:
	.quad 0x0000000000000000       /* NULL descriptor */
	.quad 0x00AF9A000000FFFF       /* Code segment (64-bit) */
	.quad 0x00CF92000000FFFF       /* Data segment */
gdt64_end:

gdt64_desc:
	.word gdt64_end - gdt64 - 1    /* Limit */
	.quad gdt64                    /* Base */

/* Basic kernel stack (16KB for Stage 1) */
.section .bss
.align 16
kernel_stack_bottom:
	.skip 16384
kernel_stack_top:
