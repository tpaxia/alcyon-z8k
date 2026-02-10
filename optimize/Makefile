include ../Makefile.cmn
include ../Makefile.sil

PROGRAMS = optimize$(EXEEXT)
LIBS = ${LIBC}
CPPFLAGS += -DGEMDOS

all: $(PROGRAMS)

include SRCFILES

# symbols etoa and ftoa are unresolved
optimize$(EXEEXT): $(OPTIMIZE_OBJS) ${MAKEFILE}
	$(AM_V_LD)$(LINK68) $(LDFLAGS) $*.68k=$(CRT0) $(NOFLOAT) $(OPTIMIZE_OBJS) $(LIBS)
	$(AM_V_at)$(RELMOD) $*.68k $@
	@$(RM) $*.68k

install: all
	$(CP) $(PROGRAMS) $(BIN)

check::
	cmp optimize$(EXEEXT) ${BIN}optimize$(EXEEXT)

clean:
	$(RM) *.o *.a *.i *.1 *.2 *.rel *.mod *.68k $(PROGRAMS)
