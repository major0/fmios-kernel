# Kernel Makefile.am for Stage 1

# Build the kernel binary
bin_PROGRAMS = fmi-kernel

# Kernel sources (minimal for Stage 1)
fmi_kernel_SOURCES = \
	main.c

# Kernel-specific compiler and linker flags
fmi_kernel_CFLAGS = $(KERNEL_CFLAGS) -I$(top_srcdir)/include -I$(top_srcdir)/arch/x86_64/include
fmi_kernel_LDFLAGS = $(KERNEL_LDFLAGS) -T$(srcdir)/kernel.ld

# Link against kernel libraries (order matters for static linking)
fmi_kernel_LDADD = \
	$(top_builddir)/arch/x86_64/libarch.a \
	$(top_builddir)/arch/x86_64/lib/libarch_support.a \
	$(top_builddir)/lib/c/libklibc.a

# Ensure libraries are built first
$(fmi_kernel_OBJECTS): $(fmi_kernel_LDADD)

# Extra files to distribute
EXTRA_DIST = kernel.ld
