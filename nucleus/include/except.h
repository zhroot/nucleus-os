#ifndef EXCEPT_H
#define EXCEPT_H

// v0.1: Doug Gale
//    - initial revision

// WARNING: excepta.asm depends on this structure
typedef struct {
	unsigned r_eax;
	unsigned r_ebx;
	unsigned r_ecx;
	unsigned r_edx;
	unsigned r_esi;
	unsigned r_edi;
	unsigned r_ebp;
	unsigned r_esp;		// NOT restored
	unsigned r_ss;			// NOT restored
	unsigned r_ds;
	unsigned r_es;
	unsigned r_fs;
	unsigned r_gs;
	unsigned r_cr2;		// NOT restored. Page fault linear address
	unsigned r_dr6;		// NOT restored. Debug register
	unsigned r_type;		// The exception code (0 to 31)
	unsigned r_error_code;	// Exception error code, if present, zero otherwise
	unsigned r_eip;
	unsigned r_cs;
	unsigned r_eflags;
} ExceptionContext;

void exception_init(void);
void exception_handler(ExceptionContext *pContext);
void install_exceptions(void);
void exception_print_traceback(unsigned ebp);

// Exception constants
#define EXCEPTION_DIVIDE			0x00
#define EXCEPTION_DEBUG				0x01
#define EXCEPTION_NMI				0x02
#define EXCEPTION_BREAKPOINT		0x03
#define EXCEPTION_OVERFLOW			0x04
#define EXCEPTION_BOUND				0x05
#define EXCEPTION_INVALIDOPCODE		0x06
#define EXCEPTION_FPUNOTAVAILABLE	0x07
#define EXCEPTION_DOUBLEFAULT		0x08
#define EXCEPTION_FPUSEGOVERRUN		0x09
#define EXCEPTION_INVALIDTSS		0x0A
#define EXCEPTION_SEGMENTNOTPRESENT	0x0B
#define EXCEPTION_STACKFAULT		0x0C
#define EXCEPTION_GPF				0x0D
#define EXCEPTION_PAGEFAULT			0x0E
#define EXCEPTION_FPUERROR			0x10
#define EXCEPTION_ALIGNMENTFAULT	0x11
#define EXCEPTION_MACHINECHECK		0x12
#define EXCEPTION_SSEFAULT			0x13

#endif // EXCEPT_H
