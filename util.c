#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "uxn.h"
#include "util.h"

char const *opnames[NUM_OPCODES] = {
	"LIT", "INC", "POP", "NIP", "SWP", "ROT", "DUP", "OVR",
	"EQU", "NEQ", "GTH", "LTH", "JMP", "JCN", "JSR", "STH",
	"LDZ", "STZ", "LDR", "STR", "LDA", "STA", "DEI", "DEO",
	"ADD", "SUB", "MUL", "DIV", "AND", "ORA", "EOR", "SFT",
};

void *
ecalloc(size_t nmemb, size_t size)
{
	void *p;

	if (!(p = calloc(nmemb, size)))
		die("calloc:");
	return p;
}

void
die(const char *fmt, ...)
{
	va_list ap;

	eprintf(SGR(1) SGR(31) "error: " SGR(0));

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	if (fmt[0] && fmt[strlen(fmt)-1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	} else {
		fputc('\n', stderr);
	}
}

void
debug(Uxn *u)
{
	eprintf(SGR(1) SGR(33) "INFO: " SGR(0));
	eprintf(INSTR_FMT, INSTR_ARGS(u->instr));

	eprintf("\tws: ( ");
	print_stack(&u->ws);
	eprintf(" )");

	eprintf(" rs: ( ");
	print_stack(&u->rs);
	eprintf(" )\n");
}

void
print_stack(Stack *s)
{
	for (u8 i = 0; i < s->sp; i++) {
		eprintf(i == 0 ? "" : " ");
		eprintf("%02X", s->data[i]);
	}
}

void
print_ram(u8 *ram)
{
	eprintf(SGR(1) SGR(33) "RAM:\n" SGR(0));
	for (int i = 0; i < 0x1000; i++) {
		for (int j = 0; j < 0x10; j++)
			if (ram[i << 4 | j] != 0x00) goto print;
		continue;
	print:
		eprintf(" %04X │",i << 4);
		for (int j = 0; j < 0x10; j++)
			eprintf(" %02X", ram[(i << 4) | j]);
		eprintf("\n");
	}
	fflush(stderr);
}
