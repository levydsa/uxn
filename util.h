
#define LENGTH(X) (sizeof(X) / sizeof(X[0]))
#define MAX(A, B)               ((A) > (B) ? (A) : (B))
#define MIN(A, B)               ((A) < (B) ? (A) : (B))
#define BETWEEN(X, A, B)        ((A) <= (X) && (X) <= (B))
#define eprintf(...) fprintf(stderr, __VA_ARGS__)

#define ESC "\x1B"
#define CSI ESC "["

#define EL(n)     CSI #n "K"
#define ED(n)     CSI #n "J"
#define CHA(n)    CSI #n "G"
#define CPL(n)    CSI #n "F"
#define CUP(n, m) CSI #n ";" #m "H"
#define SGR(n)    CSI #n "m"

#define INSTR_FMT SGR(1) "%s%s%s%s\t" SGR(0) "#%02X"
#define INSTR_ARGS(instr)                                             \
	instr != 0x00 ? opnames[instr & OPCODE_MASK] : "BRK",         \
	instr & KEEP_MASK && (instr & OPCODE_MASK) != LIT ? "k" : "", \
	instr & RETURN_MASK ? "r" : "",                               \
	instr & SHORT_MASK  ? "2" : "",                               \
	instr

extern char const *opnames[NUM_OPCODES];

void die(const char *fmt, ...);
void *ecalloc(size_t nmemb, size_t size);
void print_stack(Stack *);
void print_ram(u8 *);

