include ../Makefile.cmn
include ../Makefile.sil

PROGRAMS = c168$(EXEEXT)
LIBS = ${LIBC}
CPPFLAGS += -DGEMDOS

all: $(PROGRAMS)

include SRCFILES

c168$(EXEEXT): $(C168_OBJS) $(LIB1OBJS) ${MAKEFILE}
	$(AM_V_LD)$(LINK68) $(LDFLAGS) $*.68k=$(CRT0) $(NOFLOAT) $(C168_OBJS) $(LIBS)
	$(AM_V_at)$(RELMOD) $*.68k $@
	@$(RM) $*.68k

install: all
	$(CP) $(PROGRAMS) $(BIN)

check::
	cmp c168$(EXEEXT) ${BIN}c168$(EXEEXT)

clean:
	$(RM) *.o *.a *.s *.i *.1 *.2 *.rel *.mod *.68k $(PROGRAMS)
