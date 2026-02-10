include ../Makefile.cmn
include ../Makefile.sil

PROGRAMS = as68$(EXEEXT)
LIBS = ${LIBC}
CPPFLAGS += -DGEMDOS

all: $(PROGRAMS)

include SRCFILES

as68$(EXEEXT): $(AS68_OBJS) $(LIB1OBJS) ${MAKEFILE}
	$(AM_V_LD)$(LINK68) $(LDFLAGS) $*.68k=$(CRT0) $(NOFLOAT) $(AS68_OBJS) $(LIBS) $(LIBS)
	$(AM_V_at)$(RELMOD) $*.68k $@
	@$(RM) $*.68k

install: all
	$(CP) $(PROGRAMS) $(BIN)

check::

clean:
	$(RM) *.o *.a *.i *.1 *.2 *.rel *.mod *.68k $(PROGRAMS)
