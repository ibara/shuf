# This Makefile was written by Brian Callahan <bcallah@openbsd.org>
# and released into the Public Domain.

CFLAGS ?=	-O2 -pipe
PREFIX ?=	/usr/local

all:
	${CC} ${CFLAGS} ${LDFLAGS} -o shuf shuf.c

nonbsd:
	${CC} ${CFLAGS} -DLIBBSD ${LDFLAGS} -o shuf shuf.c -lbsd

install:
	install -c -S -s -m 755 shuf ${DESTDIR}${PREFIX}/bin
	install -c -S -m 644 shuf.1 ${DESTDIR}${PREFIX}/man/man1

clean:
	rm -f shuf shuf.core
