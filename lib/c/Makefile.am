# Kernel C Library Makefile.am

# Build kernel klibc as a static library
noinst_LIBRARIES = libklibc.a

# Kernel klibc sources (freestanding environment)
libklibc_a_SOURCES = \
	kprintf.c \
	kmalloc.c \
	klog.c

# Kernel-specific compiler flags
libklibc_a_CFLAGS = $(KERNEL_CFLAGS) -I$(top_srcdir)/include
libklibc_a_CPPFLAGS = -I$(top_srcdir)/include

# Headers for klibc (already in include/)
# klibc.h is in top-level include directory
