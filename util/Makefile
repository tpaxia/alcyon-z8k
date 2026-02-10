include ../Makefile.cmn
include ../Makefile.sil

inst_PROGRAMS = size68$(EXEEXT) nm68$(EXEEXT) ar68$(EXEEXT)
PROGRAMS = $(inst_PROGRAMS) analyze$(EXEEXT)
LIBS = ${LIBC}

all: $(PROGRAMS)

include SRCFILES

size68$(EXEEXT): $(SIZE_OBJS)
	$(AM_V_LD)$(LINK68) $(LDFLAGS) $*.68k=$(CRT0) $(NOFLOAT) $(SIZE_OBJS) $(LIBS)
	$(AM_V_at)$(RELMOD) $*.68k $@
	@$(RM) $*.68k

nm68$(EXEEXT): $(NM_OBJS)
	$(AM_V_LD)$(LINK68) $(LDFLAGS) $*.68k=$(CRT0) $(NOFLOAT) $(NM_OBJS) $(LIBS)
	$(AM_V_at)$(RELMOD) $*.68k $@
	@$(RM) $*.68k

ar68$(EXEEXT): $(AR_OBJS)
	$(AM_V_LD)$(LINK68) $(LDFLAGS) $*.68k=$(CRT0) $(NOFLOAT) $(AR_OBJS) $(LIBS) $(LIBS)
	$(AM_V_at)$(RELMOD) $*.68k $@
	@$(RM) $*.68k

analyze$(EXEEXT): $(ANALYZE_OBJS)
	$(AM_V_LD)$(LINK68) $(LDFLAGS) $*.68k=$(CRT0) $(NOFLOAT) $(ANALYZE_OBJS) $(LIBS)
	$(AM_V_at)$(RELMOD) $*.68k $@
	@$(RM) $*.68k

install: all
	$(CP) $(inst_PROGRAMS) $(BIN)

check::

clean:
	$(RM) *.o *.a *.s *.i *.1 *.2 *.68k $(PROGRAMS)
