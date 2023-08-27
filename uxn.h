
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

typedef struct Uxn Uxn;
typedef struct Stack Stack;
typedef struct Device Device;

struct Stack {
	u8 data[STACK_DATA_SIZE];
	u8 sp;   /* Stack Pointer */

	char const *name;
	u8 *ksp; /* Points to the stack pointer used in keep mode */
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

