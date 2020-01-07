// tss.c

// Task state segment

// v0.1 2004.09.02 Doug Gale
//  - Initial revision.
//	- Minimal initialization of TSS. Still needs testing.

// The TSS is only used to provide a stack pointer for priviledge transitions

#include <tss.h>
#include <gdt.h>

typedef struct {
	unsigned tss_prevtask:16;	// Previous
	unsigned :16;

	unsigned tss_esp0;			// Stack pointer for PL0
	unsigned tss_ss0:16;
	unsigned :16;

	unsigned tss_esp1;			// Stack pointer for PL1
	unsigned tss_ss1:16;
	unsigned :16;

	unsigned tss_esp2;			// Stack pointer for PL2
	unsigned tss_ss2:16;
	unsigned :16;

	unsigned tss_cr3;
	unsigned tss_eip;
	unsigned tss_eflags;
	unsigned tss_eax;
	unsigned tss_ecx;
	unsigned tss_edx;
	unsigned tss_ebx;
	unsigned tss_esp;
	unsigned tss_ebp;
	unsigned tss_esi;
	unsigned tss_edi;

	unsigned tss_es:16;
	unsigned :16;

	unsigned tss_cs:16;
	unsigned :16;

	unsigned tss_ss:16;
	unsigned :16;

	unsigned tss_ds:16;
	unsigned :16;

	unsigned tss_fs:16;
	unsigned :16;

	unsigned tss_gs:16;
	unsigned :16;

	unsigned tss_ldtseg:16;
	unsigned :16;

	unsigned tss_trap:1;
	unsigned :15;

	unsigned tss_iomapbase:16;
} TaskStateSegment;

TaskStateSegment tss;

void tss_init(void)
{
	tss.tss_iomapbase = sizeof(tss);
	gdt_set_tss(&tss, sizeof(tss));
}
