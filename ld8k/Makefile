# Z8000 cross-linker tools for macOS
#
# Ported from CP/M-8000 link utilities.
# Produces big-endian x.out format executables for Z8000.

CC = cc
CFLAGS = -O2 -Wall -Wno-parentheses -Wno-dangling-else -Wno-implicit-fallthrough

ALL = ld8k xcon ar8k xdump

all: $(ALL)

ld8k: ld8k.c xout.h
	$(CC) $(CFLAGS) -o $@ ld8k.c

xcon: xcon.c xout.h obj.h
	$(CC) $(CFLAGS) -o $@ xcon.c

ar8k: ar8k.c xout.h
	$(CC) $(CFLAGS) -o $@ ar8k.c

xdump: xdump.c xout.h
	$(CC) $(CFLAGS) -o $@ xdump.c

clean:
	rm -f $(ALL)

.PHONY: all clean
