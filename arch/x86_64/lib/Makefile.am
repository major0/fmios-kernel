# x86_64 Architecture-Specific Library

# Build architecture-specific library
noinst_LIBRARIES = libarch_support.a

# Architecture-specific support functions
libarch_support_a_SOURCES = \
	arch_support.c

# Architecture-specific compiler flags
libarch_support_a_CFLAGS = $(KERNEL_CFLAGS) -I$(top_srcdir)/include -I$(top_srcdir)/arch/x86_64/include
