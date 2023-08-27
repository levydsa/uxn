
typedef unsigned char  u8;
typedef unsigned short u16;
typedef signed char    i8;
typedef signed short   i16;

#define STACK_DATA_SIZE (0x100 - 1)
#define RAM_SIZE  0x10000
#define MAX_DEVS  0x10
#define MAX_PORTS 0x10

#define KEEP_MASK   (1 << 7)
#define RETURN_MASK (1 << 6)
#define SHORT_MASK  (1 << 5)

#define OPCODE_MASK 0x1F

/* BRK is handled especially, since it has the same opcode as LIT */
#define BRK 0x00
enum {
	LIT, INC, POP, NIP, SWP, ROT, DUP, OVR, /* Stack */
	EQU, NEQ, GTH, LTH, JMP, JCN, JSR, STH, /* Logic */
	LDZ, STZ, LDR, STR, LDA, STA, DEI, DEO, /* Memory */
	ADD, SUB, MUL, DIV, AND, ORA, EOR, SFT, /* Arithmetic */
	NUM_OPCODES
};

typedef struct Uxn    Uxn;
typedef struct Stack  Stack;
typedef struct Device Device;

struct Stack {
	u8 data[STACK_DATA_SIZE];
	u8 sp;   /* Stack Pointer */
	u8 *ksp; /* Points to the stack pointer used in keep mode */

	char const *name;
};

struct Device {
	Uxn *u;
	u8   ports[MAX_PORTS];
	u8   (*dei)(Device *, u8);
	void (*deo)(Device *, u8);

	char const *name;
};

struct Uxn {
	u8 *ram;
	Stack ws, rs; /* Work Stack, Return Stack */
	Device dev[MAX_DEVS];
	
	u8  instr;
	u16 pc;
};

void uxn_eval(Uxn *, u16);
bool uxn_step(Uxn *);

#ifndef UXN_IMPL
#define UXN_IMPL

/*
 * The macros assume 'u' to be assigned to a 'Uxn' for accessing the program
 * counter and the stacks, it is a bit mystifying to have implicit variables in
 * macros, but in this case the code becomes more readable and terse. Be
 * careful!
 */

#define KEEP   (u->instr & KEEP_MASK)
#define RETURN (u->instr & RETURN_MASK)
#define SHORT  (u->instr & SHORT_MASK)

#define PC u->pc

#define TS RETURN ? &u->rs : &u->ws
#define DS RETURN ? &u->ws : &u->rs

#define POP8()  pop8(TS)
#define POP16() POP8() | (u16) POP8() << 8
#define POP()   SHORT ? POP16() : POP8()

#define DEVR8(a)  devr8(u, a)
#define DEVR16(a) DEVR8((a) + 1) | (u16) DEVR8(a) << 8
#define DEVR(a)   SHORT ? DEVR16(a) : DEVR8(a)

#define PEEK8(a)  u->ram[a]
#define PEEK16(a) PEEK8((a) + 1) | (u16) PEEK8(a) << 8
#define PEEK(a)   SHORT ? PEEK16(a) : PEEK8(a)

#define PUSH8(a)  push8(TS, a)
#define PUSH16(a) PUSH8((a) >> 8), PUSH8(a)
#define PUSH(a)   if (SHORT) PUSH16(a); else PUSH8(a)

#define STASH8(a)  push8(DS, a)
#define STASH16(a) STASH8((a) >> 8), STASH8(a)
#define STASH(a)   if (SHORT) STASH16(a); else STASH8(a)

#define DEVW8(a, b)  devw8(u, (u8)(a), b)
#define DEVW16(a, b) DEVW8(a, (b) >> 8), DEVW8((a) + 1, b)
#define DEVW(a, b)   if (SHORT) DEVW16(a, b); else DEVW8(a, b)

#define POKE8(a, b)  u->ram[(u16)(a)] = b
#define POKE16(a, b) POKE8(a, (b) >> 8), POKE8((a) + 1, b)
#define POKE(a, b)   if (SHORT) POKE16(a, b); else POKE8(a, b)

#define WARP(a)     SHORT ? a : PC + (i8) a
#define SHIFT(a, b) b >> (a & 0xF) << (a >> 4)

static u8
pop8(Stack *s)
{
	if (s->sp == 0x00) die("%s: Underflow", s->name);
	return s->data[--(*s->ksp)];
}

static void
push8(Stack *s, u8 x)
{
	if (s->sp == 0xFF) die("%s: Overflow", s->name);
	s->data[s->sp++] = x;
}

static u8
devr8(Uxn *u, u8 a)
{
	Device *dev = &u->dev[a >> 4];
	if (!dev->dei) die("u->dev[%01X].dei is NULL", a >> 4);
	return dev->dei(dev, a & 0xF);
}

static void
devw8(Uxn *u, u8 a, u8 b)
{
	Device *dev = &u->dev[a >> 4];
	if (!dev->deo) die("u->dev[%01X].deo is NULL", a >> 4);
	dev->ports[a & 0xF] = b;
	dev->deo(dev, a & 0xF);
}

bool
uxn_step(Uxn *u)
{
	u16 a, b, c;

	if (!u) return false;

	u->instr = u->ram[u->pc++];
	if (u->instr == BRK) return false;
	
	u8 wsp_copy = u->ws.sp;
	u8 rsp_copy = u->rs.sp;

	u->ws.ksp = KEEP ? &wsp_copy : &u->ws.sp;
	u->rs.ksp = KEEP ? &rsp_copy : &u->rs.sp;

	u8 op = u->instr & OPCODE_MASK;
	if (op == LIT && !KEEP) goto impl;

	switch (op) {
	case LIT: PUSH(PEEK(PC)); PC += SHORT ? 2 : 1;    break;
	case INC: a = POP();            PUSH(a + 1);      break;
	case POP:                       POP();            break;
	case NIP: a = POP(); b = POP(); PUSH(a);          break;
	case SWP: a = POP(); b = POP(); PUSH(a); PUSH(b); break;
	case DUP: a = POP();            PUSH(a); PUSH(a); break;
	case OVR: a = POP(); b = POP(); PUSH(b);          break;
	case ROT: a = POP(); b = POP(); c = POP();
	          PUSH(c); PUSH(a); PUSH(b); break;

	case JCN: a = POP(); b = POP8(); if (b) PC = WARP(a);       break;
	case JMP: a = POP();             PC = WARP(a);              break;
	case JSR: a = POP();             STASH16(PC); PC = WARP(a); break;
	case STH: a = POP();             STASH(a);                  break;
	case EQU: a = POP(); b = POP();  PUSH8(a == b);             break;
	case NEQ: a = POP(); b = POP();  PUSH8(a != b);             break;
	case GTH: a = POP(); b = POP();  PUSH8(a < b);              break;
	case LTH: a = POP(); b = POP();  PUSH8(a > b);              break;

	case STA: a = POP16(); b = POP(); POKE(a, b);              break;
	case LDA: a = POP16();            PUSH(PEEK(a));           break;
	case STZ: a = POP8();  b = POP(); POKE(a, b);              break;
	case LDZ: a = POP8();             PUSH(PEEK(a));           break;
	case STR: a = POP8();  b = POP(); POKE(PC + (i8) a, b);    break;
	case LDR: a = POP8();             PUSH(PEEK(PC + (i8) a)); break;
	case DEO: a = POP8();  b = POP(); DEVW(a, b);              break;
	case DEI: a = POP8();             PUSH(DEVR(a));           break;

	case SFT: a = POP8(); b = POP(); PUSH(SHIFT(a, b)); break;
	case ORA: a = POP();  b = POP(); PUSH(b | a);       break;
	case EOR: a = POP();  b = POP(); PUSH(b ^ a);       break;
	case ADD: a = POP();  b = POP(); PUSH(b + a);       break;
	case SUB: a = POP();  b = POP(); PUSH(b - a);       break;
	case MUL: a = POP();  b = POP(); PUSH(b * a);       break;
	case DIV: a = POP();  b = POP(); if (a == 0) goto zero; PUSH(b / a); break;
	}
	
	u->ws.ksp = &u->ws.sp;
	return true;

	zero: die("Division by zero");
	impl: die("Instruction not implemented: #%02X", u->instr);
	return false;
}

#endif /* UXN_IMPL */

