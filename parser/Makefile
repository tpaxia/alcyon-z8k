include ../Makefile.cmn
include ../Makefile.sil

PROGRAMS = c068$(EXEEXT)
LIBS = ${LIBC} ${LIBM}
CPPFLAGS += -DGEMDOS

all: $(PROGRAMS)

include SRCFILES

c068$(EXEEXT): $(C068_OBJS) $(LIB1OBJS) ${MAKEFILE}
	$(AM_V_LD)$(LINK68) $(LDFLAGS) $*.68k=$(CRT0) $(NOFLOAT) $(C068_OBJS) $(LIBS) ${LIBM}
	$(AM_V_at)$(RELMOD) $*.68k $@
	@$(RM) $*.68k

install: all
	$(CP) $(PROGRAMS) $(BIN)

check::
	cmp c068$(EXEEXT) ${BIN}c068$(EXEEXT)

clean:
	$(RM) *.o *.a *.s *.i *.1 *.2 *.rel *.mod *.68k $(PROGRAMS)
