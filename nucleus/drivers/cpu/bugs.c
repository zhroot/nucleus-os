#include <stdio.h>
#include <drivers/cpu/cpu.h>
#include <drivers/cpu/bugs.h>

void check_popad(void)
{
	int res, inp = (int) &res;

	__asm__ __volatile__( 
		"movl $12345678,%%eax\n\r"
		"movl $0,%%edi\n\r"
		"pusha\n\r"
                "popa\n\r"
		"movl (%%edx,%%edi),%%ecx"
	  : "=&a" (res)
	  : "d" (inp)
	  : "ecx", "edi" );
	/* If this fails, it means that any user program may lock the CPU hard. Too bad. */
	if (res != 12345678)
		printf( "cpu: PopAd bug found.\n" );
}


void check_pentium_f00f(void)
{
	/* Pentium and Pentium MMX */
	cpu.f00f_bug = 0;
	if (cpu.family == 5 && cpu.manufacturer == idIntel)
	{
		printf("cpu: Intel Pentium with F0 0F bug.\n");
		cpu.f00f_bug = 1;
	}
}
