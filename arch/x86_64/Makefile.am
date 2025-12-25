# x86_64 Architecture-Specific Makefile.am

# Subdirectories
SUBDIRS = include lib

# Architecture-specific sources for Stage 1
arch_SOURCES = \
	bootstrap.S \
	entry.c

# Build libarch.a for x86_64
noinst_LIBRARIES = libarch.a
libarch_a_SOURCES = $(arch_SOURCES)

# Architecture-specific compiler flags
libarch_a_CFLAGS = $(KERNEL_CFLAGS) -I$(top_srcdir)/include -I$(srcdir)/include
libarch_a_CCASFLAGS = $(KERNEL_CFLAGS) -I$(top_srcdir)/include -I$(srcdir)/include
