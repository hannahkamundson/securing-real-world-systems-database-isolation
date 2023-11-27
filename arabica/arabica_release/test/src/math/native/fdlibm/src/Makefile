
ifneq ($(CIL),yes)
  CC=gcc
  AR=ar rc
else
  CC=cilly --merge
  AR=cilly --merge --mode=AR rc
endif

CFLAGS=-O3 -I../include -D_LITTLE_ENDIAN 

# the original J2SE CFLAGS in the source code
# CFLAGS=-g -I../include -fno-strict-aliasing -fPIC -W -Wall  -Wno-unused -Wno-parentheses -fno-omit-frame-pointer   -Di586 -DARCH='"i586"' -DLINUX -DRELEASE='"1.5.0-internal"' -DFULL_VERSION='"1.5.0-internal-gtan_04_jul_2006_18_00"' -D_LARGEFILE64_SOURCE -D_GNU_SOURCE -D_REENTRANT -D_LITTLE_ENDIAN 

#CFLAGS=-g -DDEBUG
#CFLAGS=-O3 -Wall -Wwrite-strings -Wpointer-arith -Wconversion \
#           -Wstrict-prototypes -Wmissing-prototypes

# todo: goal: compile all c and link it into a library.

LDSHARED=gcc
CPP=gcc -E

RANLIB=ranlib
TAR=tar
SHELL=/bin/sh

# get all obj files
OBJS = $(patsubst %.c,%.o,$(wildcard *.c))

all: libm.a

libm.a: ${OBJS}
	${AR} $@ ${OBJS}

${OBJS}: %.o: %.c
	${CC} -c $(CFLAGS) $< -o $@

# declare clean to be a phony target
.PHONY: clean
clean:
	rm -f *.o libm.a
