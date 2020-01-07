/*
 * CPU detection (CPUID, possible bugs etc.)	*
 * v0.1: TDS (20.04.2001)						*
 *	   - detection of all known processors		*
 *	   - L1+L2 Cache							*
 *	   - bug checking (PopAd, F0 0F)			*
 *	   - special cyrix detection				*
 * v0.2: TDS (30.03.2004)						*
 *	   - small bug fixes						*
 *	   - NucleusKL compatible					*
*/

#include <datatypes.h>
#include <interrupts.h>
#include <stdio.h>
#include <support.h>
#include <string.h>
#include <drivers/cpu/cpu.h>
#include <drivers/cpu/cpu_mhz.h>
#include <drivers/cpu/bugs.h>

struct cpu_struct cpu;

static char * manufacturers[] =
{
		"AuthenticAMD",
		"CentaurHauls",
		"CyrixInstead",
		"GenuineIntel",
		"NexGenDriven",
		"UMC UMC UMC ",
		"RiseRiseRise",
		"GenuineTMx86",
		"SiS SiS SiS ",
		"Geode by NSC"		
};

static char * display[] =
{
		"AMD",
		"Centaur/IDT",
		"Cyrix",
		"Intel",
		"NexGen",
		"UMC",
		"Rise",
		"Transmeta",
		"SiS",
		"National Semiconductor"
};

//AMD
static char *amd_0x04[16] =
{
	NULL, "80486 DX", NULL, "80486 DX2-WT", NULL, NULL, NULL,
	"80486 DX2, WB", "80486DX4-WT / 5x86-WT", "80486DX4-WB / 5x86-WB",
	"Elan SC400", NULL, NULL, NULL, "Am5x86 WT", "Am5x86 WB"
};
static char *amd_0x05[16]=
{
	"SSA5 (PR75, PR90, PR100)", "5k86 (PR120, PR133)",
	"5k86 (PR166)", "5k86 (PR200)", NULL, NULL,
	"K6 (0.30æm)", "K6 (0.25æm)", "K6-II", "K6-III",
	NULL, NULL, "K6-II+ (0.18æm)", "K6-III+ (0.18æm)", NULL, NULL
};
static char *amd_0x06[16]=
{
	NULL, "K7 (0.25æm)", "Athlon (0.18æm)", "Duron (Spitfire Core)",
	"Athlon (0.18æm, Thunderbird)", NULL, "Athlon (Palomino)",
	"Duron (Morgan)", "Athlon XP", NULL, "Athlon (Barton)",
	NULL, NULL, NULL, NULL, NULL
};
static char *amd_ext_0x00[16]=
{
	NULL, NULL, NULL, NULL, "Athlon 64 (0.13æm 754)", "Athlon 64 FX or Opteron (0.13æm)",
	NULL, "Athlon 64 (0.13æm 939)", "Athlon 64 (0.13æm 754)", NULL, NULL,
	"Athlon 64 (0.13æm 939)", "Athlon 64 (0.13æm 754)", NULL, NULL,
	"Athlon 64 (0.13æm 939)"	
};

static struct AMD_0x06_ext
{
	char model, stepping;
	char name[40];
} amd_0x06_ext[14] =
{
	{1, 1, "Rev C1"},
	{1, 2, "Rev C2"},
	{2, 1, "Rev A1"},
	{2, 2, "Rev A2"},
	{3, 0, "Rev A0"},
	{3, 1, "Rev A2"},
	{4, 0, "Rev A1"},
	{4, 1, "Rev A2"},
	{4, 2, "Rev A4-A8"},
	{4, 3, "Rev A9"},
	{6, 0, "Rev A0-A1"},
	{6, 1, "Rev A2"},
	{7, 0, "Rev A0"},
	{7, 1, "Rev A1"}
};

// Intel
static char *intel_0x03[16]=
{
	"i80386DX", NULL, "i80386SX", NULL, "i80386SL", NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};
static char *intel_0x04[16]=
{
	"i80486DX-25/33", "i80486DX-50", "i80486SX", "i80486DX2",
	"i80486SL", "i80486SX2", NULL,  "i80486DX2 WB",
	"i80486DX4", "i80486DX4 WB", NULL, NULL, NULL, NULL, NULL, NULL
};
static char *intel_0x05[16]=
{
	"P5 60/66 A-step", "P5 60/66", "P54C 75-200", "P24T Overdrive PODP5V83",
	"P55C MMX", NULL, NULL, "P54C Mobile Pentium 75-200",
	"P55C (0.25æm) Mobile Pentium MMX", NULL, NULL, NULL, NULL, NULL, NULL, NULL
};
static char *intel_0x06[16]=
{
	"P6 PentiumPro A-Step", "P6 PentiumPro", NULL, "PII (0.28æm) Klamath", NULL,
	"PII (0.25æm) Deschutes", "PII (on-die L2 Cache) Mobile Pentium",
	"PIII (0.25æm) Katmai", "PIII (0.18æm) (256KB on-die L2 cache) Coppermine",
	"PIII (0.13æm) (512KB or 1MB on-die L2 cache) Coppermine",
	"PIII (0.18æm) (1MB or 2MB on-die L2 cache) Cascades",
	"PIII (0.13æm) (256KB or 512KB on-die L2 cache)", NULL, NULL, NULL, NULL
};
static char *intel_ext_0x00[16]=
{
	"P4 (0.18æm)", "P4 (0.18æm)", "P4 (0.13æm)", "P4 (0.09æm)",
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};
//Cyrix
static char *cyrix_name[128]=
{
		/***********************/
		/**  486 DLC and SLC  **/
		/***********************/
		"Cx486_SLC", "Cx486_DLC", "Cx486_SLC2", "Cx486_DLC2",
		"Cx486_SRx (Retail Upgrade Cx486SLC)",
		"Cx486_DRx (Retail Upgrade Cx486DLC)",
		"Cx486_SRx2 (Retail Upgrade 2x Cx486SLC)",
		"Cx486_DRx2 (Retail Upgrade 2x Cx486DLC)",
		"", "", "", "", "", "", "",
		/***********************/
		/* 486 S, DX, DX2, DX4 */
		/***********************/
		"Cx486S B step", "Cx486S2 B step", "Cx486Se B step",
		"Cx486S2e B step", "", "", "", "", "", "",
		"Cx486DX", "Cx486DX2", "", "", "", "Cx486DX4",
		/***********************/
		/***	   5x86	  ***/
		/***********************/
		"", "", "", "", "", "", "", "",
		"5x86_1xs (5x86 1x Core/Bus Clock)",
		"5x86_2xs (5x86 2x Core/Bus Clock)",
		"5x86_1xp (5x86 1x Core/Bus Clock)",
		"5x86_2xp (5x86 2x Core/Bus Clock)",
		"5x86_4xs (5x86 4x Core/Bus Clock)",
		"5x86_3xs (5x86 3x Core/Bus Clock)",
		"5x86_4xp (5x86 4x Core/Bus Clock)",
		"5x86_3xp (5x86 3x Core/Bus Clock)",
		/***********************/
		/***	   6x86	  ***/
		/***********************/
		"6x86_1xs (6x86 1x Core/Bus Clock)",
		"6x86_2xs (6x86 2x Core/Bus Clock)",
		"6x86_1xp (6x86 1x Core/Bus Clock)",
		"6x86_2xp (6x86 2x Core/Bus Clock)",
		"6x86_4xs (6x86 4x Core/Bus Clock)",
		"6x86_3xs (6x86 3x Core/Bus Clock)",
		"6x86_4xp (6x86 4x Core/Bus Clock)",
		"6x86_3xp (6x86 3x Core/Bus Clock)",
		"", "", "", "", "", "", "", "",
		/***********************/
		/***	   Gx86	  ***/
		/***********************/
		"", "", "", "",
		"Gx86_4xs (Gx86 4x Core/Bus Clock)",
		"Gx86_3xs (Gx86 3x Core/Bus Clock)",
		"Gx86_4xp (Gx86 4x Core/Bus Clock)",
		"Gx86_3xp (Gx86 3x Core/Bus Clock)",
		"", "", "", "", "", "", "", "", "",
		/***********************/
		/***	   m2		***/  // Now called 6x86MX
		/***********************/
		// m2s_base	 0x50	 // m2 s base 1x
		"m2_1xs (m2 1x   Core/Bus Clock)",
		"m2_2xs (m2 2x Core/Bus Clock)",
		"m2_2p5xs (m2 2.5x Core/Bus Clock)",
		"m2_3xs (m2 3x Core/Bus Clock)",
		"m2_3p5xs (m2 3.5x Core/Bus Clock)",
		"m2_4xs (m2 4x Core/Bus Clock)",
		"m2_4p5xs (m2 4.5x Core/Bus Clock)",
		"m2_5xs (m2 5x Core/Bus Clock)",
		// m2p_base	 0x58	 // m2 p base 1x
		"m2_1xp (m2 1x Core/Bus Clock)",
		"m2_2xp (m2 2x Core/Bus Clock)",
		"m2_2p5xp (m2 2.5x Core/Bus Clock)",
		"m2_3xp (m2 3x Core/Bus Clock)",
		"m2_3p5xp (m2 3.5x Core/Bus Clock)",
		"m2_4xp (m2 4x Core/Bus Clock)",
		"m2_4p5xp (m2 4.5x Core/Bus Clock)",
		"m2_5xp (m2 5x Core/Bus Clock)"
};

int is_cpuid_supported(void)
{
	int result;

	asm(
		"pushfl\n\r"				/* get extended flags */
		"popl	 %%eax\n\r"
		"movl	 %%eax, %%ebx\n\r"	/* save current flags */
		"xorl	 $0x200000, %%eax\n\r"	/* toggle bit 21 */
		"pushl	%%eax\n\r"			/* put new flags on stack */
		"popfl\n\r"					/* flags updated now in flags */
		"pushfl\n\r"				/* get extended flags */
		"popl	 %%eax\n\r"
		"xorl	 %%ebx, %%eax\n\r"	/* if bit 21 r/w then supports cpuid */
		"mov $0,%%eax\n\r"
		"jz	  0f\n\r"
		"incl %%eax\n\r"
		"0:\n\r"
		: "=a" (result)
		:
		: "ebx"
	);
	
	return result;
}

#define get_cpuid(in, a, b, c, d)	asm("cpuid": "=a" (a), "=b" (b), "=c" (c), "=d" (d) : "a" (in));

void get_serial_number(unsigned long *a, unsigned long *b, unsigned long *c)
{
	unsigned long tmp;

 	get_cpuid(1, a, tmp, tmp, tmp);
 	get_cpuid(3, tmp, tmp, b, c);
}

unsigned long moreinfo[4];

char cyrix_type(void);

void fillinfo(void)
{
	get_cpuid(2, moreinfo[0], moreinfo[4], moreinfo[8], moreinfo[12]);
}

static char *checkname(char *result)
{
	unsigned char x;
	unsigned long reg_eax, reg_ebx, reg_ecx, reg_edx;

	if (cpu.manufacturer == idAMD)
	{
		// enable write-back mode
		switch (cpu.family)
		{
			case 0x04: strcpy(result, amd_0x04[cpu.model]); break;
			case 0x05:	if (cpu.stepping > 7) 
						result = "K6-II (CXT core)"; 
					else
						strcpy(result, amd_0x05[cpu.model]); break;
			case 0x06:	if (cpu.model == 0x06 && cpu.stepping == 0x02)
					{
						if (cpu.cache.CPU_L2_CACHE_SIZE < 256)
						{
							strcpy(result, "Mobile Duron");
						}
						else
						{
							get_cpuid (0x80000001, reg_eax, reg_ebx, reg_ecx, reg_edx);
							if ((reg_edx & (1<<19)) == 0)
							{
								strcpy(result, "Athlon XP");
							}
							else
							{
								strcpy(result, "Athlon MP");
							}
						}
					}
					else
					if (cpu.model == 0x08)
					{
						if (cpu.cache.CPU_L2_CACHE_SIZE < 256)
						{
							strcpy(result, "Duron");
						}
						else
						{
							get_cpuid (0x80000001, reg_eax, reg_ebx, reg_ecx, reg_edx);
							if ((reg_edx & (1<<19)) == 0)
							{
								strcpy(result, "Athlon XP (Thoroughbred)");
							}
							else
							{
								strcpy(result, "Athlon MP (Thoroughbred)");
							}
						}		
					}
					else
						strcpy(result, amd_0x06[cpu.model]);
					break;
			case 0x0F: switch(cpu.ext_family)
						{
							case 0x00: strcpy(result, amd_ext_0x00[cpu.model]); break;
							default: result = "K8";
						} break;
			default: result = "Unknown";
		}
	}
	if (cpu.manufacturer == idIntel)
	{
		if (moreinfo[0] == 0x01)
		{
			for (x=1;x<16;x++)
			{
				switch (moreinfo[x])
				{
					case 0x40: result = "P2 Celeron"; break;
					case 0x41: result = "P2 Celeron A"; break;
					case 0x42: result = "PPro"; break;
					case 0x43: result = "P2"; break;
					case 0x44: result = "P2 Xeon (1MB L2 cache)"; break;
					case 0x45: result = "P2 Xeon (2MB L2 cache XEON, Xena?)"; break;
				}
			}
				if (cpu.flags & cpu_isse)
				{
				if (strcmp(result, "P2"))	result = "P3";
				if (strcmp(result, "P2 Celeron A"))	result = "P3 Celeron A";
				if (strcmp(result, "Xeon"))	result = "P3 Xeon";
				}
		}
		
		switch (cpu.family)
		{
			case 0x03: strcpy(result, intel_0x03[cpu.model]); break;
			case 0x04: strcpy(result, intel_0x04[cpu.model]); break;
			case 0x05: strcpy(result, intel_0x05[cpu.model]); break;
			case 0x06:
				if (cpu.model == 0x05)
					{
						if( cpu.cache.CPU_L2_CACHE_SIZE == 512) strcpy(result, intel_0x06[cpu.model]);
						if( cpu.cache.CPU_L2_CACHE_SIZE <  512) strcpy(result, "Celeron");
						if( cpu.cache.CPU_L2_CACHE_SIZE >  512) strcpy(result, "Pentium II Xeon");
					}
					else
					strcpy(result, intel_0x06[cpu.model]);
				
				break;
			case 0x07: result = "Itanium (IA-64)";
			case 0x0F:
				switch(cpu.ext_family)
						{
							case 0x00: strcpy(result, intel_ext_0x00[cpu.model]); break;
							case 0x01: result = "Itanium 2"; break;
							default: result = "K8";
						} break;
			default: result = "Unknown";
		}
	}
	if (cpu.manufacturer == idIDT)
	{
		switch (cpu.family)
		{
			case 0x05:
			{
				switch (cpu.model)
				{
					case 0x4: result = "Winchip C6"; break;
					case 0x8:	switch (cpu.stepping)
							{
								default: result = "Winchip C2";	break;
								case 7:	case 8:	case 9: result = "Winchip 2A"; break;
								case 10: case 11: case 12: case 13:
								case 14: case 15: result = "Winchip 2B"; break;
							}
							break;
					case 0x9: result = "Winchip C3"; break;
				} break;
			}
		/* Family 6 is when VIA bought out Cyrix & IDT
		 * This is the CyrixIII family. */
			case 0x06:
			{
				switch (cpu.model)
				{
					case 0x06: result = "VIA Cyrix III"; break;
					case 0x07: 	result = "VIA C3";
							if (cpu.stepping > 0x07)
								result = "\"Ezra\"";
							break;
				}
			}
			break;
			default: result = "Unknown";
 		}
	}
	if (cpu.manufacturer == idCyrix)
	{
		switch (cpu.family)
		{
			case 0x4:
			{
				switch (cpu.model)
				{
					case 0x4: result = "GX"; break;
					case 0x5: result = "MediaGX"; break;
					case 0x9: result = "5x86"; break;
				} break;
			}
			case 0x5:
			{
				switch (cpu.model)
				{
					case 0x2: result = "6x86MX"; break;
					case 0x4: result = "GXm"; break;
				} break;
			}
			case 0x6:
			{
				switch (cpu.model)
				{
					case 0x0: result = "6x86/MX"; break;
					case 0x2: result = "MII"; break;
				} break;
			}
			default: x = cyrix_type(); strcpy(result, cyrix_name[x]);
		}
	}
	if (cpu.manufacturer == idNexGen)
	{
		switch (cpu.family)
		{
			case 0x5:
			{
				switch (cpu.model)
				{
					case 0x0: result = "Nx586 / Nx586 FPU"; break;
				}
			} break;
			default: result = "Unknown";
		}
	}
	if (cpu.manufacturer == idUMC)
	{
		switch (cpu.family)
		{
			case 0x4:
			{
				switch (cpu.model)
				{
					case 0x1: result = "U5D"; break;
					case 0x2: result = "U5S"; break;
				} break;
			}
			default: result = "Unknown";
		}
	}
	if (cpu.manufacturer == idRise)
	{
		switch (cpu.family)
		{
			case 0x5:
			{
				switch (cpu.model)
				{
					case 0x0: result = "mP6 iDragon (0.25æm)"; break;
					case 0x2: result = "mP6 iDragon (0.18æm)"; break;
					case 0x8: result = "mP6 iDragon II (0.25æm)"; break;
					case 0x9: result = "mP6 iDragon II (0.18æm)"; break;
				} break;
			}
			default: result = "Unknown";
		}
	}
	if (cpu.manufacturer == idSiS)
	{
		switch (cpu.family)
		{
			case 0x5:
			{
				switch (cpu.model)
				{
					case 0x0: result = "mP6 iDragon (0.25æm)"; break;
					case 0x2: result = "mP6 iDragon (0.18æm)"; break;
					case 0x8: result = "mP6 iDragon II (0.25æm)"; break;
					case 0x9: result = "mP6 iDragon II (0.18æm)"; break;
				} break;
			}
			default: result = "Unknown";
		}
	}
	if (cpu.manufacturer == idNSC)
	{
		switch (cpu.family)
		{
			case 0x5:
			{
				switch (cpu.model)
				{
					case 0x0: result = "mP6 iDragon (0.25æm)"; break;
					case 0x2: result = "mP6 iDragon (0.18æm)"; break;
					case 0x8: result = "mP6 iDragon II (0.25æm)"; break;
					case 0x9: result = "mP6 iDragon II (0.18æm)"; break;
				} break;
			}
			default: result = "Unknown";
		}
	}
	return result;
}

static int is_486(void)
{
	int result;

		asm (
	"pushf ;"			/* save EFLAGS */
	"popl	%%eax ;"		/* get EFLAGS */
	"movl	%%eax, %%ecx ;"		/* temp storage EFLAGS */
	"xorl	$0x40000, %%eax ;"	/* change AC bit in EFLAGS */
	"pushl	%%eax ;"		/* put new EFLAGS value on stack */
	"popf ;"			/* replace current EFLAGS value */
	"pushf ;"			/* get EFLAGS */
	"popl	%%eax ;"		/* save new EFLAGS in EAX */
	"cmpl	%%ecx, %%eax ;"		/* compare temp and new EFLAGS */
	"movl   $0,%%eax ;"
	"jz	0f ;"
	"incl   %%eax ;"		/* 80486 present */
	"0:"
	"pushl	%%ecx ;"		/* get original EFLAGS */
	"popf ;"			/* restore EFLAGS */
	: "=a" (result)
	:
	: "ecx" );

	return result;
}

static int is_386DX(void) /* return TRUE for 386DX, and FALSE for 386SX */
{
	int result;

		asm (
	"movl	%%cr0,%%edx ;"		/* Get CR0 */
	"pushl	%%edx ;"		/* save CR0 */
		"andb	$0xEF, %%dl ;"		/* clear bit4 */
	"movl	%%edx, %%cr0 ;" 	/* set CR0 */
	"movl	%%cr0, %%edx ;"		/* and read CR0 */
	"andb	$0x10, %%dl ;"		/* bit4 forced high? */
	"popl	%%edx ;"		/* restore reg w/ CR0 */
	"movl	%%edx, %%cr0 ;"		/* restore CR0 */
	"movl	$1, %%eax ;"		/* TRUE, 386DX */
	"jz	0f ;"
	"movl	$0, %%eax ;"		/* FALSE, 386SX */
	"0:"
	: "=a" (result)
	:
		: "%edx", "memory"
	);

	return result;
}

static int is_486DX(void) /* return TRUE for 486DX, and FALSE for 486SX */
{
	dword memory_location = 0x00;

	__asm__ (
		"fninit ;"
		"fnstsw %0 ;"
		: "=g" (memory_location)
	);
	
	return (memory_location == 0x37F) ? 1 : 0;
}

static int is_fpu(void)
{
	int result;

		asm (
	"fninit ;"
	"movl	$0x5a5a, %%eax ;"
	"fnstsw  %%eax ;"
	"cmpl	$0, %%eax ;"
	"jne	 0f ;"
	"movl	$1, %%eax ;"
	"jmp	 1f ;"

	"0:"
	"movl	$0, %%eax ;"

	"1:"
	: "=a" (result)
	:
	: "memory" );

	return result;
}

static int is_cyrix(void)
{
	int result;

	asm (
		"xorw	 %%ax, %%ax ;"		/* clear eax */
		"sahf ;"			/* clear flags, bit 1 is always 1 in flags */
		"movw	 $5, %%ax ;"
		"movw	 $2, %%bx ;"
		"div	  %%bl ;"		/* do an operation that does not change flags */
		"lahf ;"			/* get flags */
		"cmpb	 $2, %%ah ;"		/* check for change in flags */
		"movl	 $0, %%eax ;"		/* FALSE NON-Cyrix CPU */
		"jne	  0f ;"			/* flags changed not Cyrix */
		"incl	 %%eax ;"		/* TRUE Cyrix CPU */
		"0:"
		: "=a" (result)
		:
		: "ebx"
	);

	return result;
}

static void cx_w(char index, char value)
{
	int ints_were_enabled = interrupts_disable();

	outportb(0x22, index);		/* tell CPU which config. register */
	outportb(0x23, value);		/* write to CPU config. register */

	if (ints_were_enabled)
		interrupts_enable();
}

static char cx_r(char index)
{
	char value;
	int ints_were_enabled = interrupts_disable();

	outportb(0x22, index);		/* tell CPU which config. register */
	value = inportb(0x23);		/* read CPU config, register */

	if (ints_were_enabled)
		interrupts_enable();

	return value;
}

#define UNKNOWN   0xff
#define Cx486_pr  0xfd  /* ID Register not supported, software created */
#define Cx486S_a  0xfe  /* ID Register not supported, software created */
#define CR2_MASK  0x4   /* LockNW */
#define CR3_MASK  0x80  /* Resereved bit 7 */

static char cyrix_type(void)
{
	char orgc2, newc2, orgc3, newc3;
	int cr2_rw=0, cr3_rw=0, type;
	int ints_were_enabled = interrupts_disable();

	type = UNKNOWN;

	/* Test Cyrix c2 register read/writable */
	orgc2 = cx_r (0xc2);		/* get current c2 value */
	newc2 = orgc2 ^ CR2_MASK;	/* toggle test bit */
	cx_w (0xc2, newc2);		/* write test value to c2 */
	cx_r (0xc0);			/* dummy read to change bus */
	if (cx_r (0xc2) != orgc2)	/* did test bit toggle */
		cr2_rw = 1;		/* yes bit changed */
	cx_w (0xc2, orgc2);		/* return c2 to original value */
	/* end c2 read writeable test */
	/* Test Cyrix c3 register read/writable */
	orgc3 = cx_r (0xc3);		/* get current c3 value */
	newc3 = orgc3 ^ CR3_MASK;	/* toggle test bit */
	cx_w (0xc3, newc3);		/* write test value to c3 */
	cx_r (0xc0);			/* dummy read to change bus */
	if (cx_r (0xc3) != orgc3)	/* did test bit change */
		cr3_rw = 1;		/* yes it did */
		cx_w (0xc3, orgc3);		/* return c3 to original value */
	/* end c3 read writeable test */
	if ((cr2_rw && cr3_rw) || (!cr2_rw && cr3_rw)) /*DEV ID register ok */
	{
		/* <<<<<<< READ DEVICE ID Reg >>>>>>>> */
		type = cx_r (0xfe);	/* lower byte gets IDIR0 */
	}
	else if (cr2_rw && !cr3_rw)	/* Cx486S A step */
	{
		type = Cx486S_a;	/* lower byte */
	}
	else if (!cr2_rw && !cr3_rw)	/* Pre ID Regs. Cx486SLC or DLC */
	{
		type = Cx486_pr;	/* lower byte */
	}
	/* This could be broken down more, but is it needed? */
	if (type < 0x30 || type > 0xfc)
	{
		cpu.family = 4;		/* 486 class-including 5x86 */
		cpu.model = 0xFF;	/* Unknown */
	}
	else if (type < 0x50)
	{
		cpu.family = 5;		/* Pentium class-6x86 and Media GX */
		cpu.model = 0xFF;	/* Unknown */
	}
	else	
	{
		cpu.family = 6;		/* Pentium || class- 6x86MX */
		cpu.model = 0xFF;	/* Unknown */
		cpu.flags |= cpu_mmx;
	}

	if (ints_were_enabled)
		interrupts_enable();

	return type;
}

void check_cache(void)
{
	static unsigned proc_cache_l1[4] = { 0, 0, 0, 0 };
	static unsigned proc_cache_l2[4] = { 0, 0, 0, 0 };

		// query L1 cache information
	get_cpuid(0x80000005, proc_cache_l1[0], proc_cache_l1[1],
			proc_cache_l1[2], proc_cache_l1[3]);
		// query L2 cache information
	get_cpuid(0x80000006, proc_cache_l2[0], proc_cache_l2[1],
			proc_cache_l2[2], proc_cache_l2[3]);

	if (cpu.manufacturer == idAMD)
		{
			// K5/K6 supports a restricted range
			cpu.cache.CPU_L1_DTLB_ASSOC   = (proc_cache_l1[1] >> 24) & 0xff;
			cpu.cache.CPU_L1_DTLB_ENTRIES = (proc_cache_l1[1] >> 16) & 0xff;
			cpu.cache.CPU_L1_ITLB_ASSOC   = (proc_cache_l1[1] >>  8) & 0xff;
			cpu.cache.CPU_L1_ITLB_ENTRIES = (proc_cache_l1[1] >>  0) & 0xff;

			cpu.cache.CPU_L1_DCACHE_SIZE  = (proc_cache_l1[2] >> 24) & 0xff;
			cpu.cache.CPU_L1_DCACHE_ASSOC = (proc_cache_l1[2] >> 16) & 0xff;
			cpu.cache.CPU_L1_DCACHE_LINES = (proc_cache_l1[2] >>  8) & 0xff;
			cpu.cache.CPU_L1_DCACHE_LSIZE = (proc_cache_l1[2] >>  0) & 0xff;

			cpu.cache.CPU_L1_ICACHE_SIZE  = (proc_cache_l1[3] >> 24) & 0xff;
			cpu.cache.CPU_L1_ICACHE_ASSOC = (proc_cache_l1[3] >> 16) & 0xff;
			cpu.cache.CPU_L1_ICACHE_LINES = (proc_cache_l1[3] >>  8) & 0xff;
			cpu.cache.CPU_L1_ICACHE_LSIZE = (proc_cache_l1[3] >>  0) & 0xff;

			cpu.cache.CPU_L2_CACHE_SIZE   = (proc_cache_l2[2] >> 16) & 0xffff;
			cpu.cache.CPU_L2_CACHE_ASSOC  = (proc_cache_l2[2] >> 12) & 0x0f;
			cpu.cache.CPU_L2_CACHE_LINES  = (proc_cache_l2[2] >>  8) & 0x0f;
			cpu.cache.CPU_L2_CACHE_LSIZE  = (proc_cache_l2[2] >>  0) & 0xff;

			if (cpu.family == 0x06) // AMD ATHLON
			{
				// Athlon supports these additional parameters
				cpu.cache.CPU_L1_EDTLB_ASSOC   = (proc_cache_l1[0] >> 24) & 0xff;
				cpu.cache.CPU_L1_EDTLB_ENTRIES = (proc_cache_l1[0] >> 16) & 0xff;
				cpu.cache.CPU_L1_EITLB_ASSOC   = (proc_cache_l1[0] >>  8) & 0xff;
				cpu.cache.CPU_L1_EITLB_ENTRIES = (proc_cache_l1[0] >>  0) & 0xff;

				cpu.cache.CPU_L2_DTLB_ASSOC	= (proc_cache_l2[0] >> 28) & 0x0f;
				cpu.cache.CPU_L2_DTLB_ENTRIES  = (proc_cache_l2[0] >> 16) & 0xfff;
				cpu.cache.CPU_L2_UTLB_ASSOC	= (proc_cache_l2[0] >> 12) & 0x0f;
				cpu.cache.CPU_L2_UTLB_ENTRIES  = (proc_cache_l2[0] >>  0) & 0xfff;

				cpu.cache.CPU_L2_EDTLB_ASSOC   = (proc_cache_l2[1] >> 28) & 0x0f;
				cpu.cache.CPU_L2_EDTLB_ENTRIES = (proc_cache_l2[1] >> 16) & 0xfff;
				cpu.cache.CPU_L2_EUTLB_ASSOC   = (proc_cache_l2[1] >> 12) & 0x0f;
				cpu.cache.CPU_L2_EUTLB_ENTRIES = (proc_cache_l2[1] >>  0) & 0xfff;
			}
	}
}


void check_cpu(void)
{
	unsigned cpuid_levels, reg_eax, reg_ebx, reg_ecx, reg_edx, tmp;
	unsigned char b;

	if (is_cpuid_supported())
	{
		cpu.flags |= cpu_cpuid;
		get_cpuid(0, cpuid_levels, reg_ebx, reg_ecx, reg_edx);
		cpu.levels = cpuid_levels;
		cpu.vendor[ 0] = LOBYTE(LOWORD(reg_ebx));
		cpu.vendor[ 1] = HIBYTE(LOWORD(reg_ebx));
		cpu.vendor[ 2] = LOBYTE(HIWORD(reg_ebx));
		cpu.vendor[ 3] = HIBYTE(HIWORD(reg_ebx));
		cpu.vendor[ 4] = LOBYTE(LOWORD(reg_edx));
		cpu.vendor[ 5] = HIBYTE(LOWORD(reg_edx));
		cpu.vendor[ 6] = LOBYTE(HIWORD(reg_edx));
		cpu.vendor[ 7] = HIBYTE(HIWORD(reg_edx));
		cpu.vendor[ 8] = LOBYTE(LOWORD(reg_ecx));
		cpu.vendor[ 9] = HIBYTE(LOWORD(reg_ecx));
		cpu.vendor[10] = LOBYTE(HIWORD(reg_ecx));
		cpu.vendor[11] = HIBYTE(HIWORD(reg_ecx));
		cpu.vendor[12] = '\0';
		cpu.manufacturer = 0xFF;
		for (tmp=0; tmp<9; tmp++)
		{
			if (strcmp(cpu.vendor, manufacturers[tmp]) == 0)
			{
				cpu.manufacturer = tmp;
				break;
			}
		}
		if (cpuid_levels > 0)
		{
			get_cpuid (1, reg_eax, tmp, tmp, reg_edx);
			cpu.family = (reg_eax & CPUID_FAM) >> 8;
			cpu.model = (reg_eax & CPUID_MOD) >> 4;
			cpu.stepping = (reg_eax & CPUID_STEP);
			cpu.ext_family = (reg_eax & CPUID_XFAM);
			cpu.features = reg_edx;
			checkname(cpu.name);
			// fill extended info structure for intel's
			fillinfo();
			check_cache();
			if (cpu.features & HAS_FPU) cpu.flags |= cpu_fpu;
			if (cpu.features & HAS_MMX) cpu.flags |= cpu_mmx;
			if (cpu.features & HAS_SSE) cpu.flags |= cpu_isse;
		}
	}
	else
	{
		cpu.vendor[0] = 'U';
		cpu.vendor[1] = 'n';
		cpu.vendor[2] = 'k';
		cpu.vendor[3] = 'n';
		cpu.vendor[4] = 'o';
		cpu.vendor[5] = 'w';
		cpu.vendor[6] = 'n';
		cpu.vendor[7] = '\0';
		if (is_fpu())
			cpu.flags |= cpu_fpu;
		if (!is_486())
		{
			if (is_386DX())
			{
				cpu.main_family = 3;	// 386
				cpu.model = 0;			// DX
			}
			else						// It is a 386SX
			{
				cpu.main_family = 3;	// 386
				cpu.model = 1;			// SX
			}
		}
		else
		{
				if (is_cyrix())
				{
				cpu.manufacturer = idCyrix;
				b = cyrix_type();
				strcpy(cpu.name, cyrix_name[b]);
				}
				else
				if (is_486DX())
				{
					cpu.main_family = 4;	// 486
					cpu.model = 0;			// DX
				}
				else
				{
					cpu.main_family = 4;	// 486
					cpu.model = 1;			// SX
				}
		}
	}
}

void cpu_init(void)
{
	dprintf("\ncpu: Checking CPU...");
	strcpy(cpu.name, "Unknown Processor");
	check_cpu();
	dprintf("OK\n");
	if (cpu.flags & cpu_cpuid)
	{
	   	dprintf("cpu: CPUID supported (");
		dprintf(cpu.vendor);
		if (cpu.levels)
			dprintf(", %ld levels", cpu.levels);
		dprintf("), %02X/%02X/%02X\n", cpu.family, cpu.model, cpu.stepping);
		dprintf("cpu: Identified as %s %s\n", display[cpu.manufacturer], cpu.name);
		dprintf("cpu: L1 D Cache: %ldKB, L1 I Cache: %ldKB", cpu.cache.CPU_L1_DCACHE_SIZE,
			cpu.cache.CPU_L1_ICACHE_SIZE);
		if (cpu.cache.CPU_L2_CACHE_SIZE)
			dprintf(", L2 Cache: %ldKB", cpu.cache.CPU_L2_CACHE_SIZE);
		cpu.clockrate = get_clockrate(cpu.features & HAS_TSC);
		dprintf("\ncpu: Speed CPU: %ldMHz\n", cpu.clockrate);
		check_popad();
		check_pentium_f00f();
	}
	else
	{
		dprintf("cpu: %d86 ", cpu.main_family);
		if (cpu.model != 0xFF)
			dprintf("%s", (cpu.model == 0x00) ? "DX " : "SX ");
		dprintf("detected\n");
	}
}
