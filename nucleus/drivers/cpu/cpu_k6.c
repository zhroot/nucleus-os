#include <stdio.h>
#include <support.h>
#include <drivers/cpu/msr.h>
#include <drivers/cpu/cpu_k6.h>

unsigned long mem_end = 1024UL;	// only 1MB, because I can't find the
				// procedure for memory detection

void AMD_K6_writeback(int family, int model, int stepping)
{
	/* mem_end == top of memory in bytes */
	int mem=(mem_end>>20)/4; /* turn into 4mb aligned pages */
	int c;
	struct regs amd_regs;

    if(family==5)
    {
        c=model;

        /* model 8 stepping 0-7 use old style, 8-F use new style */
        if(model==8)
        {
            if(stepping<8)
                c=7;
            else
                c=9;
        }

        switch(c)
        {
        /* old style write back */
        case 6:
        case 7:
            AMD_K6_read_msr(0xC0000082, &amd_regs);
            if(((amd_regs.eax >> 1) & 0x7F)==0)
				dprintf("AMD K6 : WriteBack currently disabled\n");
            else
		{
			dprintf("AMD K6 : WriteBack currently enabled (");
			dprintf("%ldMB)\n", ((amd_regs.eax >> 1) & 0x7F)*4);
		}

		dprintf("AMD K6 : Enabling WriteBack to ");
		dprintf("%ldMB\n", (unsigned long)mem*4);
		AMD_K6_write_msr(0xC0000082, ((mem << 1) & 0x7F), 0, &amd_regs);
            break;

        /* new style write back */
        case 9:
            AMD_K6_read_msr(0xC0000082, &amd_regs);
            if(((amd_regs.eax >> 22) & 0x3FF)==0)
				dprintf("AMD K6 : WriteBack Disabled\n");
            else
		{
			dprintf("AMD K6 : WriteBack Enabled (");
			dprintf("%ldMB\n", ((amd_regs.eax >> 22) & 0x3FF)*4);
		}

		dprintf("AMD K6 : Enabled WriteBack (");
		dprintf("%ldMB\n", (unsigned long)mem*4);
            AMD_K6_write_msr(0xC0000082, ((mem << 22) & 0x3FF), 0, &amd_regs);
            break;
        default:    /* dont set it on Unknowns + k5's */
            break;
        }
    }
}

void AMD_K6_write_msr(unsigned long msr, unsigned long v1, unsigned long v2, struct regs *amd_regs)
{
    asm (
        "pushfl\n"
        "cli\n"
        "wbinvd\n"
        "wrmsr\n"
        "popfl\n"
        : "=a" (amd_regs->eax),
          "=b" (amd_regs->ebx),
          "=c" (amd_regs->ecx),
          "=d" (amd_regs->edx)
        : "a" (v1),
          "d" (v2),
          "c" (msr)
        : "memory");
}

void AMD_K6_read_msr(unsigned long msr, struct regs *amd_regs)
{
    asm (
        "pushfl\n"
        "cli\n"
        "wbinvd\n"
        "xorl %%eax, %%eax\n"
        "xorl %%edx, %%edx\n"
        "rdmsr\n"
        "popfl\n"
        : "=a" (amd_regs->eax),
          "=b" (amd_regs->ebx),
          "=c" (amd_regs->ecx),
          "=d" (amd_regs->edx)
        : "c" (msr)
        : "memory");
}
