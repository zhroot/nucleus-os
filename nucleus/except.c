// except.c

// Exception handlers

// v0.1: Doug Gale
//    - initial revision
// v0.2: Doug Gale
//	- Added human-readable flags display in crash info display
//	- Added stack trace to print call stack
// v0.3: Doug Gale
//	- Separated traceback code so it can be used independently
//	- Fixed catastrophic failure upon return from exception
//	- Added support to dispatch exception different ways (V86, etc...)

#include <except.h>
#include <interrupts.h>
#include <support.h>
#include <multi.h>
#include <stdio.h>

// Prototypes for assembly stubs,
// DO NOT CALL! Only "called" by CPU exception!
extern void exception_stub_0x00();
extern void exception_stub_0x01();
extern void exception_stub_0x02();
extern void exception_stub_0x03();
extern void exception_stub_0x04();
extern void exception_stub_0x05();
extern void exception_stub_0x06();
extern void exception_stub_0x07();
extern void exception_stub_0x08();
extern void exception_stub_0x09();
extern void exception_stub_0x0a();
extern void exception_stub_0x0b();
extern void exception_stub_0x0c();
extern void exception_stub_0x0d();
extern void exception_stub_0x0e();
extern void exception_stub_0x0f();
extern void exception_stub_0x10();
extern void exception_stub_0x11();
extern void exception_stub_0x12();
extern void exception_stub_0x13();
extern void exception_stub_0x14();
extern void exception_stub_0x15();
extern void exception_stub_0x16();
extern void exception_stub_0x17();
extern void exception_stub_0x18();
extern void exception_stub_0x19();
extern void exception_stub_0x1a();
extern void exception_stub_0x1b();
extern void exception_stub_0x1c();
extern void exception_stub_0x1d();
extern void exception_stub_0x1e();
extern void exception_stub_0x1f();

typedef void (*exception_stub_func)();

// Array of pointers used to initialize exception vectors
static exception_stub_func exception_init_tbl[] = {
	exception_stub_0x00,
	exception_stub_0x01,
	exception_stub_0x02,
	exception_stub_0x03,
	exception_stub_0x04,
	exception_stub_0x05,
	exception_stub_0x06,
	exception_stub_0x07,
	exception_stub_0x08,
	exception_stub_0x09,
	exception_stub_0x0a,
	exception_stub_0x0b,
	exception_stub_0x0c,
	exception_stub_0x0d,
	exception_stub_0x0e,
	exception_stub_0x0f,
	exception_stub_0x10,
	exception_stub_0x11,
	exception_stub_0x12,
	exception_stub_0x13,
	exception_stub_0x14,
	exception_stub_0x15,
	exception_stub_0x16,
	exception_stub_0x17,
	exception_stub_0x18,
	exception_stub_0x19,
	exception_stub_0x1a,
	exception_stub_0x1b,
	exception_stub_0x1c,
	exception_stub_0x1d,
	exception_stub_0x1e,
	exception_stub_0x1f
};

// Array of pointers to english exception names
static char *exception_english[] = {
	"Divide error",
	"Debug",
	"NMI",
	"Breakpoint",
	"Overflow",
	"Bound range exceeded",
	"Invalid opcode",
	"Coprocessor not available",
	"Double fault",
	"Coprocessor segment overrun",
	"Invalid TSS",
	"Segment not present",
	"Stack fault",
	"General protection fault",
	"Page fault",
	"(Intel Reserved)",
	"FPU error",
	"Alignment fault",
	"Machine check",
	"SSE/SSE2 fault",
	"(Intel Reserved)",
	"(Intel Reserved)",
	"(Intel Reserved)",
	"(Intel Reserved)",
	"(Intel Reserved)",
	"(Intel Reserved)",
	"(Intel Reserved)",
	"(Intel Reserved)",
	"(Intel Reserved)",
	"(Intel Reserved)",
	"(Intel Reserved)",
	"(Intel Reserved)"
};

// ...other language tables?...

static char **ppCurrentLanguage = exception_english;

void exception_init(void)
{
	int i;
	
	for(i = 0; i < 32; i++)
		interrupt_install(INT_EXCEPTION, i, exception_init_tbl[i]);
}

// Print the traceback starting from the passed frame pointer
void exception_print_traceback(unsigned r_ebp)
{
	unsigned *walk, retaddr;

	walk = (unsigned *)r_ebp;
	do {
		// Get return address
		retaddr = walk[1];

		printf("Called from %08x\n", retaddr);

		// Get previous stack frame
		walk = (unsigned*)walk[0];
	} while ((unsigned)walk != 0xFFFFFFFF);
}

// Crash with register contents and traceback
static void exception_crash(ExceptionContext *context)
{
	static struct FlagTbl {
		unsigned flag;
		char *name;
	} flag_table[] = {
		{ EFL_CF,		"CF" },
		{ EFL_PF,		"PF" },
		{ EFL_AF,		"AF" },
		{ EFL_ZF,		"ZF" },
		{ EFL_SF,		"SF" },
		{ EFL_TF,		"TF" },
		{ EFL_IF,		"IF" },
		{ EFL_DF,		"DF" },
		{ EFL_OF,		"OF" },
		// IOPL0 is zero, so I don't detect it
		{ EFL_IOPL1,	"IOPL1" },
		{ EFL_IOPL2,	"IOPL2" },
		{ EFL_IOPL3,	"IOPL3" },
		{ EFL_NT,		"NT" },
		{ EFL_RF,		"RF" },
		{ EFL_VM,		"VM" },
		{ EFL_AC,		"AC" },
		{ EFL_VIF,		"VIF" },
		{ EFL_VIP,		"VIP" },
		{ EFL_ID,		"ID" },
		{ 0,			0 }
	};
	struct FlagTbl *ft;

	printf("\n\n");
	printf("---------------------------------------"
			"----------------------------------------\n");
	printf("\n\nException %02lX (%s)\n", 
			context->r_type, ppCurrentLanguage[context->r_type]);
	printf("EAX=%08lX EBX=%08lX ECX=%08lX EDX=%08lX\n",
			context->r_eax, context->r_ebx, context->r_ecx, context->r_edx);
	printf("ESI=%08lX EDI=%08lX EBP=%08lX\n",
			context->r_esi, context->r_edi, context->r_ebp);
	printf("SS:ESP=%04lX:%08lX CS:EIP=%04lX:%08lX\n",
			context->r_ss, context->r_esp, context->r_cs, context->r_eip);
	printf("DS=%04lX ES=%04lX FS=%04lX GS=%04lX\n",
			context->r_ds, context->r_es, context->r_fs, context->r_gs);
	printf("EFLAGS=%08lX ( ", context->r_eflags);

	// Print human readable flags
	for (ft = flag_table; ft->flag; ft++) {
		if (context->r_eflags & ft->flag) {
			printf("%s ", ft->name);
		}
	}
	printf(")\n");

	printf("CR2=%08lX DR6=%08lX\n",
			context->r_cr2, context->r_dr6);
	printf("---------------------------------------"
			"----------------------------------------\n");

	exception_print_traceback(context->r_ebp);

	halt(0);
}

void exception_handler(ExceptionContext *context)
{
	switch (context->r_type) {
	case EXCEPTION_DIVIDE:
	case EXCEPTION_NMI:
	case EXCEPTION_BOUND:
	case EXCEPTION_INVALIDOPCODE:
	case EXCEPTION_FPUNOTAVAILABLE:
	case EXCEPTION_DOUBLEFAULT:
	case EXCEPTION_FPUSEGOVERRUN:
	case EXCEPTION_INVALIDTSS:
	case EXCEPTION_SEGMENTNOTPRESENT:
	case EXCEPTION_STACKFAULT:
	case EXCEPTION_GPF:
	case EXCEPTION_PAGEFAULT:
	case EXCEPTION_FPUERROR:
	case EXCEPTION_MACHINECHECK:
	case EXCEPTION_SSEFAULT:
		exception_crash(context);
		break;

	case EXCEPTION_DEBUG:
	case EXCEPTION_BREAKPOINT:
	case EXCEPTION_OVERFLOW:
	case EXCEPTION_ALIGNMENTFAULT:
		printf("Exception! (%08x) (%s)...ignoring\n",
				context->r_type, ppCurrentLanguage[context->r_type]);
		break;
	}
}

