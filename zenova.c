#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "uxn.h"
#include "util.h"

#define KEEP   KEEP_MASK
#define RETURN RETURN_MASK
#define SHORT  SHORT_MASK

#define PAGE_PROGRAM 0x0100

void
eval(Uxn *u, u16 pc)
{
	u->pc = pc;

	while (true) {
		Stack old_ws = u->ws;
		Stack old_rs = u->rs;
		if (!uxn_step(u)) break;
		eprintf(INSTR_FMT, INSTR_ARGS(u->instr));
		eprintf("\tws: ( ");
		print_stack(&old_ws);
		eprintf(" -- ");
		print_stack(&u->ws);
		eprintf(" )");

		eprintf(" rs: ( ");
		print_stack(&old_rs);
		eprintf(" -- ");
		print_stack(&u->rs);
		eprintf(" )\n");
	}
}

static void
console_deo(Device *d, u8 port)
{
	switch (port) {
	case 0x8:
		putchar(d->ports[port]);
		fflush(stdout);
		break;
	case 0x9:
		fputc(d->ports[port], stderr);
		fflush(stderr);
		break;
	case 0xa: break;
	case 0xb:
		{
			u16 addr = d->ports[port - 1] << 8 | d->ports[port];
			fputs((char *) d->u->ram + addr, stdout);
		}
		break;
	default:
		eprintf("dev: %s: #%01X Port Undefined\n", d->name, port);
	}
}

static void
system_deo(Device *d, u8 port)
{
	switch (port) {
	case 0xF:
		exit(d->ports[port]);
		break;
	default:
		eprintf("dev: %s: #%01X Port Undefined\n", d->name, port);
	}
}

int main(int argc, char *argv[]) {
	Uxn u = {
		.ws.name = "ws", .rs.name = "rs",
		.ram = ecalloc(RAM_SIZE, sizeof(u8)),
		.dev = {
			[0] = {
				.u = &u,
				.name = "system", .deo = system_deo
			},

			[1] = {
				.u = &u,
				.name = "console", .deo = console_deo
			}
		}
	};
	
	if (argc < 2) {
		eprintf("Usage: %s [FILE]\n", argv[0]);
		die("No input file");
	}

	FILE *fp = fopen(argv[1], "rb");
	if (!fp) die("fopen:");

	fread(u.ram + 0x0100, RAM_SIZE - 0x0100, sizeof(u8), fp);

	if (ferror(fp)) die("fread:");

	eval(&u, 0x0100);

	print_ram(u.ram);
	return 0;
}

