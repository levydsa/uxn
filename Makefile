CFLAGS = -std=c99 -Wall -Wextra -Wvla -pedantic -Os

SRC = uxn.c util.c zenova.c
OBJ = ${SRC:.c=.o}

all: options zenova extra

debug: CFLAGS += -DDEBUG
debug: all

extra:
	make -C extra

zenova: ${OBJ} ${DEPS}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

%.o: %.c uxn.h util.h
	${CC} -c ${CFLAGS} $<

options:
	@echo "CFLAGS  = ${CFLAGS}"
	@echo "LDFLAGS = ${LDFLAGS}"
	@echo "CC      = ${CC}"

clean:
	rm zenova ${OBJ} ${ROMS}
	${MAKE} -C extra clean

.PHONY: all options clean extra
