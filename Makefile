TOPDIR	:= $(shell pwd -P)

K_OBJS	= \
  arch/$(ARCH)/boot.o arch/$(ARCH)/console.o \
  lib/itoa.o lib/printk.o lib/strlen.o \
  src/kmain.o

LIBGCC	:= $(shell gcc -print-libgcc-file-name)
CPPFLAGS:= -I$(TOPDIR)/arch/$(ARCH)/include -I$(TOPDIR)/include
CFLAGS	:= -O0 -ggdb -Wall -nostdinc -fno-zero-initialized-in-bss -fstrict-aliasing -ffreestanding $(CPPFLAGS)
LIBS	:= $(LIBGCC)
LDFLAGS	:= -nostdlib -L$(TOPDIR)/arch/$(ARCH)/lib -L$(TOPDIR)/lib $(LIBS)

ifdef CHOST
CROSS_COMPILE := $(CHOST)-
endif

CC	:= $(CROSS_COMPILE)gcc
AS	:= $(CROSS_COMPILE)as
AR	:= $(CROSS_COMPILE)ar
LD	:= $(CROSS_COMPILE)ld
STRIP	:= $(CROSS_COMPILE)strip
RANLIB	:= $(CROSS_COMPILE)ranlib

all: fmios-kernel

fmios-kernel: $(K_OBJS)
	$(LD) $^ -o $@

clean:
	rm -f $(K_OBJS)

distclean: clean
	rm -f fmios-kernel

install:
	install -D -m 0755 fmios-kernel $(DESTDIR)/boot/fmios-kernel
