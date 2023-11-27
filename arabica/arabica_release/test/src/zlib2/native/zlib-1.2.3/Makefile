CC=gcc

CFLAGS=-O3 -DHAVE_UNISTD_H

AR=ar rc
RANLIB=ranlib

OBJ=compress.o  deflate.o  gzio.o  infback.o  inffast.o  inflate.o  inftrees.o  trees.o  uncompr.o  zadler32.o  zcrc32.o  zutil.o 

libz.a: $(OBJ)
	$(AR) $@ $(OBJ)
	-@ ($(RANLIB) $@ || true) >/dev/null 2>&1

%.o : %.c
	$(CC) $(CFLAGS) –c %.c
