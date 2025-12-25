# Include directory Makefile.am

# Generic headers (architecture-neutral)
nobase_include_HEADERS = \
	ktypes.h \
	klibc.h

# Architecture symlink will be created by top-level Makefile
# include/arch -> ../arch/x86_64/include

# Clean up symlinks
clean-local:
	rm -f arch

.PHONY: clean-local
