top_srcdir=..
subdir=cgen_z8k

#
# cross compiling
#
include ../GNUmakefile.cmn
include ../Makefile.sil

PROGRAMS = c1z8k$(EXEEXT)

LIBS =

CPPFLAGS = -I../common
CFLAGS = $(OPTS) $(WARN)
LDFLAGS += -s

all: $(PROGRAMS)

include SRCFILES

c1z8k$(EXEEXT): $(C1Z8K_OBJS)
	$(AM_V_LD)$(CC) ${CFLAGS} $(C1Z8K_OBJS) ${LIBS} $(LDFLAGS) $(GLIBC_SO) -o $@

$(C1Z8K_OBJS): cgen.h cskel.h icode.h

install: all
	$(CP) $(PROGRAMS) $(BIN)

check::

dist::
	$(CP) -a $(SRCS) $(EXTRA_DIST1) $(top_srcdir)/$(DISTDIR2)/$(subdir)
	$(CP) -a $(EXTRA_DIST2) $(top_srcdir)/$(DISTDIR2)/$(subdir)

clean:
	$(RM) *.o *.a $(PROGRAMS)
