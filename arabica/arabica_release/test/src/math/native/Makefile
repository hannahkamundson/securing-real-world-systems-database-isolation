javaprefix =/usr/java/jdk

projdir=/home/gtan/project
sharedir=${projdir}/demo/share

ifneq ($(CIL),yes)
  CC=gcc
  AR=ar rc
  CFLAGS=-O3 -I${javaprefix}/include -I${javaprefix}/include/linux -Ifdlibm/include
else
  CC=cilly --merge
  AR=cilly --merge --mode=AR rc
#  CFLAGS= --save-temps --emitgraph --stats --debug=yes -O3 -I${projdir}/include -I${javaprefix}/include/linux -Ifdlibm/include

# flags for performance numbers
  CFLAGS= --save-temps --jspc-no-comments --jspc-runnable-mode --stats -O3 -I${projdir}/include -I${javaprefix}/include/linux -Ifdlibm/include
endif

#CFLAGS=-g -DDEBUG
#CFLAGS=-O3 -Wall -Wwrite-strings -Wpointer-arith -Wconversion \
#           -Wstrict-prototypes -Wmissing-prototypes

LDSHARED=gcc
CPP=gcc -E

RANLIB=ranlib
TAR=tar
SHELL=/bin/sh

OBJS = StrictMath.o

all: ${OBJS} fdlibm
	${CC} ${CFLAGS} ${OBJS} fdlibm/src/libm.a -o libStrictMath.so -shared
	mv -f libStrictMath.so ..

${OBJS}: %.o: %.c
	${CC} -c $(CFLAGS) $< -o $@

.PHONY: fdlibm
fdlibm: 
	cd fdlibm/src && $(MAKE)

# declare clean to be a phony target
.PHONY: clean
clean:
	rm -f *.o
	cd fdlibm/src && $(MAKE) clean
