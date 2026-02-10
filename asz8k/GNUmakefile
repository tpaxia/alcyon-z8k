CC = cc
CFLAGS = -O -DPDDIR='"."' -fcommon \
	-Wno-implicit-int -Wno-implicit-function-declaration \
	-Wno-return-type -Wno-incompatible-pointer-types \
	-Wno-deprecated-non-prototype -Wno-int-conversion \
	-Wno-parentheses -Wno-dangling-else

SRCS = aascom.c adirec.c aglocom.c ainit.c ainput.c alook.c amisc.c \
       aoutput.c aparse.c atoken.c avm.c axref.c \
       asz8k.c asz8kglo.c asz8kgram.c portlib.c

OBJS = $(SRCS:.c=.o)

all: asz8k

asz8k: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

clean:
	rm -f *.o asz8k

.PHONY: all clean
