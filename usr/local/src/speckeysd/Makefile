# speckeysd - bind keys 
# See COPYRIGHT file for copyright and license details.

include config.mk

SRC = speckeysd.c
OBJ = ${SRC:.c=.o}

all: options speckeysd

options:
	@echo speckeysd build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.mk

speckeysd: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f speckeysd ${OBJ} speckeysd-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p speckeysd-${VERSION}
	@cp -R COPYRIGHT Makefile README config.mk \
		${SRC} speckeysd-${VERSION}
	@tar -cf speckeysd-${VERSION}.tar speckeysd-${VERSION}
	@gzip speckeysd-${VERSION}.tar
	@rm -rf speckeysd-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f speckeysd ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/speckeysd

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/speckeysd

.PHONY: all options clean dist install uninstall
