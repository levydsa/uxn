
UXNASM = ./uxnasm
ROMS = hello.rom

all: uxnasm ${ROMS}

%.rom: %.tal
	${UXNASM} $< $@

uxnasm:
	${CC} -o $@ uxnasm.c

clean:
	rm ${ROMS} ./uxnasm

.PHONY: all options clean
