#ifndef _CPU_H_
#define _CPU_H_

#define idAMD		0
#define idIDT		1
#define idCyrix		2
#define idIntel		3
#define idNexGen	4
#define idUMC		5
#define idRise		6
#define idTransmeta	7
#define idSiS		7
#define idNSC		7

struct cache_struct
{
 	// standard values
	unsigned long CPU_L1_DTLB_ASSOC, CPU_L1_DTLB_ENTRIES, CPU_L1_ITLB_ASSOC, CPU_L1_ITLB_ENTRIES;
	unsigned long CPU_L1_DCACHE_SIZE, CPU_L1_DCACHE_ASSOC, CPU_L1_DCACHE_LINES, CPU_L1_DCACHE_LSIZE;
	unsigned long CPU_L1_ICACHE_SIZE, CPU_L1_ICACHE_ASSOC, CPU_L1_ICACHE_LINES, CPU_L1_ICACHE_LSIZE;
	unsigned long CPU_L2_CACHE_SIZE, CPU_L2_CACHE_ASSOC, CPU_L2_CACHE_LINES, CPU_L2_CACHE_LSIZE;
	// additional values (for AMD Athlon)
	unsigned long CPU_L1_EDTLB_ASSOC, CPU_L1_EDTLB_ENTRIES, CPU_L1_EITLB_ASSOC, CPU_L1_EITLB_ENTRIES;
	unsigned long CPU_L2_DTLB_ASSOC, CPU_L2_DTLB_ENTRIES, CPU_L2_UTLB_ASSOC, CPU_L2_UTLB_ENTRIES;
	unsigned long CPU_L2_EDTLB_ASSOC, CPU_L2_EDTLB_ENTRIES, CPU_L2_EUTLB_ASSOC, CPU_L2_EUTLB_ENTRIES;
};

struct cpu_struct
{
	unsigned char vendor[13];       // vendor name
	unsigned char name[64];            // detailed information
	unsigned char family, model, stepping, ext_family;
	unsigned long features, flags, levels;
	unsigned char main_family;		// 3=386, 4=486
	unsigned char manufacturer;     // idAMD, idIDT, etc.
	unsigned long clockrate;
	char f00f_bug;
	struct cache_struct cache;	// L1+L2 cache
};

extern struct cpu_struct cpu;

// features
#define HAS_FPU         0x0000001         // bit  0 = FPU
#define HAS_VME         0x0000002         // bit  1 = VME
#define HAS_DEBUG       0x0000004         // bit  2 = Debugger extensions
#define HAS_PSE         0x0000008         // bit  3 = Page Size Extensions
#define HAS_TSC         0x0000010         // bit  4 = Time Stamp Counter
#define HAS_MSR         0x0000020         // bit  5 = Model Specific Registers
#define HAS_PAE         0x0000040         // bit  6 = PAE
#define HAS_MCE         0x0000080         // bit  7 = Machine Check Extensions
#define HAS_CMPXCHG8    0x0000100         // bit  8 = CMPXCHG8 instruction
#define HAS_APIC        0x0000200         // bit  9 = APIC
#define HAS_BIT10       0x0000400
#define HAS_SYSENTER    0x0000800         // bit 11 = SYSENTER instruction
#define HAS_MTRR        0x0001000         // bit 12 = Memory Type Range Registers
#define HAS_GPE         0x0002000         // bit 13 = Global Paging Extensions
#define HAS_MCA         0x0004000         // bit 14 = Machine Check Architecture
#define HAS_CMOV        0x0008000         // bit 15 = CMOV instruction
#define HAS_PAT         0x0010000         // bit 16 = Page Attribue Table
#define HAS_PSE36       0x0020000         // bit 17 = PSE36 (Page Size Extensions)
#define HAS_MMX         0x0800000         // bit 23 = MMX
#define HAS_FXSAVE      0x1000000         // bit 24 = FXSAVE/FXRSTOR instruction
#define HAS_SSE         0x2000000         // bit 25 = SSE


#define cpu_cpuid	0x01
#define cpu_fpu		0x02
#define cpu_mmx		0x04
#define cpu_isse	0x08

#define CPUID_FAM			0x00000f00	// family mask
#define CPUID_XFAM			0x0ff00000	// extended family mask
#define CPUID_MOD			0x000000f0	// model mask
#define CPUID_STEP			0x0000000f	// stepping level mask 
#define CPUID_XFAM_MOD		0x0ff00ff0	// xtended fam, fam + model
#define ATHLON64_XFAM_MOD	0x00000f40	// xtended fam, fam + model
#define OPTERON_XFAM_MOD	0x00000f50	// xtended fam, fam + model

void get_cpuid(unsigned op, unsigned *eax, unsigned *ebx, unsigned *ecx, unsigned *edx);
void cpu_init(void);

#endif
