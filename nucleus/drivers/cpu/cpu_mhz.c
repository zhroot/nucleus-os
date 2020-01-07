/********************************************************
* v0.1 ???												*
* v0.2 Doug Gale										*
*		- Fixed disabling of interrupts					*
********************************************************/

#include <support.h>
#include <interrupts.h>
#include <drivers/cpu/cpu_mhz.h>
#include <drivers/cpu/msr.h>


/*
 *  $Id: MHz.c,v 1.8 2001/12/09 16:35:51 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 * Estimate CPU MHz routine by Andrea Arcangeli <andrea@suse.de>
 * Small changes by David Sterba <sterd9am@ss1000.ms.mff.cuni.cz>
 *
 */

#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>

unsigned long calibrate_tsc(void)
{
	int ints_were_enabled = interrupts_disable();

	unsigned long startlow, starthigh;
	unsigned long endlow, endhigh;
	unsigned long count;
	
	/* Set the Gate high, disable speaker */
	outportb(0x61, (inportb(0x61) & ~0x02) | 0x01);

	/*
	 * Now let's take care of CTC channel 2
	 *
	 * Set the Gate high, program CTC channel 2 for mode 0,
	 * (interrupt on terminal count mode), binary count,
	 * load 5 * LATCH count, (LSB and MSB) to begin countdown.
	 */
	outportb(0x43, 0xB0);			/* binary, mode 0, LSB/MSB, Ch 2 */
	outportb(0x42, CALIBRATE_LATCH & 0xff);	/* LSB of count */
	outportb(0x42, CALIBRATE_LATCH >> 8);	/* MSB of count */

	rdtsc(startlow,starthigh);
	count = 0;
	do {
		count++;
	} while ((inportb(0x61) & 0x20) == 0);
	rdtsc(endlow,endhigh);

	if (ints_were_enabled)
		interrupts_enable();
	
	/* Error: ECTCNEVERSET */
	if (count <= 1)
		goto bad_ctc;
	
	/* 64-bit subtract - gcc just messes up with long longs */
	__asm__("subl %2,%0\n\t"
			"sbbl %3,%1"
			:"=a" (endlow), "=d" (endhigh)
			:"g" (startlow), "g" (starthigh),
			"0" (endlow), "1" (endhigh));
	
	/* Error: ECPUTOOFAST */
	if (endhigh)
		goto bad_ctc;
	
	/* Error: ECPUTOOSLOW */
	if (endlow <= CALIBRATE_TIME)
		goto bad_ctc;
	
	__asm__("divl %2"
			:"=a" (endlow), "=d" (endhigh)
			:"r" (endlow), "0" (0), "1" (CALIBRATE_TIME));
	
	return endlow;
	
	/*
	* The CTC wasn't reliable: we got a hit on the very first read,
	* or the CPU was so fast/slow that the quotient wouldn't fit in
	* 32 bits..
	*/
bad_ctc:
	return 0;
}

unsigned long get_clockrate(char tsc)
{
	unsigned long eax=0, edx=1000, cpu_khz = 0x01;
    unsigned long tsc_quotient;

	if (tsc)
	{
		tsc_quotient = calibrate_tsc();

		if (tsc_quotient)
		{
			__asm__(
				"divl %2"
				:"=a" (cpu_khz), "=d" (edx)
				:"r" (tsc_quotient),
		         "0" (eax), "1" (edx)
			);
		}
    }
	return (unsigned long)(cpu_khz / 1000);
}
