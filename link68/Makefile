include ../Makefile.cmn
include ../Makefile.sil

PROGRAMS = link68$(EXEEXT) relmod$(EXEEXT) reldump$(EXEEXT)
LIBS = ${LIBC}
CPPFLAGS += -DGEMDOS

all: $(PROGRAMS)

include SRCFILES

link68$(EXEEXT): $(LINK_OBJS) ${MAKEFILE}
	$(AM_V_LD)$(LINK68) $(LDFLAGS) $*.68k=$(CRT0),$(NOFLOAT),link68.o,preproc.o,lnkmess.o,$(LIBS)
	$(AM_V_at)$(RELMOD) $*.68k $@
	@$(RM) $*.68k

relmod$(EXEEXT): $(RELMOD_OBJS) ${MAKEFILE}
	$(AM_V_LD)$(LINK68) $(LDFLAGS) $*.68k=$(CRT0),$(NOFLOAT),$(RELMOD_OBJS),$(LIBS)
	$(AM_V_at)$(RELMOD) $*.68k $@
	@$(RM) $*.68k

reldump$(EXEEXT): $(RELDUMP_OBJS) ${MAKEFILE}
	$(AM_V_LD)$(LINK68) $(LDFLAGS) $*.68k=$(CRT0),$(NOFLOAT),$(RELDUMP_OBJS),$(LIBS)
	$(AM_V_at)$(RELMOD) $*.68k $@
	@$(RM) $*.68k

link68.rel::
	./link68$(EXEEXT) '[s]' $@=../libsrc/crt0.o,../libsrc/nofloat.o,link68.o,preproc.o,lnkmess.o,../libsrc/libc.a

install: all
	$(CP) $(PROGRAMS) $(BIN)

check::

clean:
	$(RM) *.o *.a *.s *.i *.1 *.2 *.rel *.mod *.68k $(PROGRAMS)
