include ../Makefile.cmn
include ../Makefile.sil
include SRCFILES

PROGRAMS = cp68$(EXEEXT)

LIBS = $(LIBC)

all: $(PROGRAMS)

cp68$(EXEEXT): ${OBJS}
	$(AM_V_LD)$(LINK68) ${LDFLAGS} $*.68k=$(CRT0) $(NOFLOAT) ${OBJS} ${LIBS}
	$(AM_V_at)$(RELMOD) $*.68k $@
	@$(RM) $*.68k

install: all
	$(CP) $(PROGRAMS) $(BIN)

check::
	cmp cp68$(EXEEXT) ${BIN}cp68$(EXEEXT)

clean::
	$(RM) *.o *.68k $(PROGRAMS)
