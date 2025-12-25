/*
 * FMI/OS x86_64 Bootstrap Code
 * Stage 1: PVH ELF boot entry point
 */

.section .text
.global _start
.type _start, @function

_start:
	/* Stage 1: Minimal PVH ELF boot entry point */

	/* Clear direction flag */
	cld

	/* Set up a basic stack */
	movq $kernel_stack_top, %rsp

	/* Clear frame pointer */
	xorq %rbp, %rbp

	/* Call architecture early initialization */
	call arch_early_init

	/* Call kernel main */
	call kmain

	/* If kmain returns, halt the system */
	call arch_halt

/* Basic kernel stack (16KB for Stage 1) */
.section .bss
.align 16
kernel_stack_bottom:
	.skip 16384
kernel_stack_top:

.section .note.gnu.property,"a",@note
.align 8
.long 4
.long 16
.long 5
.asciz "GNU"
.long 0xc0000002
.long 4
.long 3
.align 8
