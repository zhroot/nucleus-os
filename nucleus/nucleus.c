/*
	NUCLEUSKL - OS Kernel based on XERXYS NUCLEUS 0.01 by Christian Lins
	(C)Copyright 2003 by Christian Lins <christian@netvader.de>
	
	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
	as published by the Free Software Foundation; 
	either version 2 of the License, or (at your option) any later version. 

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
	See the GNU General Public License for more details. 

	You should have received a copy of the GNU General Public License along with this program; 
	if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA. 
	
	Letzte Änderung: 25.07.2003
*/

// v0.1: ???
// v0.2: Doug Gale
//	- Added support for __attribute__((constructor/destructor))
//	- Fixed system halts, they now sleep in hlt loop, instead of busy loop
//	- Added calls to new code initialize paging, exceptions, TSS, etc.
//	- Various little test function calls

#include <datatypes.h>
#include <nucleus.h>
#include <support.h>
#include <stdio.h>	

// Roughly in order of initialization
#include <gdt.h>
#include <interrupts.h>
#include <tss.h>
#include <except.h>
#include <video/graphic.h>
#include <drivers/pic.h>	
#include <strings.h>
#include <multi.h>
#include <drivers/mem/mem.h>
#include <drivers/timer.h>
#include <drivers/input/keyboard.h>
#include <drivers/cpu/cpu.h>
#include <drivers/block.h>
#include <drivers/cmos.h>
#include <dynlink.h>

// Extras
#include <cdsl.h>
#include <drivers/fs/fat.h>

//char waste[1000000] = { 1, 2, 3 };

// interrupts_enable will panic if this is zero
int interrupts_ok;

// Constructor/Destructor function pointer
typedef void (*CtorDtor)(void);

static void call_constructors()
{
	CtorDtor *ppctor;

	// Provided by linker script
	extern char __ctors_st[];

	// Null terminated list
	ppctor = (CtorDtor*)__ctors_st;
	while (*ppctor)
		(*ppctor++)();
}

static void call_destructors()
{
	CtorDtor *ppdtor;

	// Provided by linker script
	extern char __dtors_st[];

	// Null terminated list
	ppdtor = (CtorDtor*)__dtors_st;
	while (*--ppdtor)
		(*ppdtor)();
}

// Versionskonstanten (--> nucleus.h für weitere Infos)
const int nucleus_version = 0x001; // Version: 0.01
const char nucleus_revision[] = "a2"; // Revision: alpha 2

// Systemstatus (--> nulcleus.h für weitere Infos)
char nucleuskl_status = NUC_STATUS_STARTING;

// Debuglevel (--> nucleus.h für weitere Infos)
char nucleuskl_debug = NUC_DEBUG_FULL;

void shutdown(void) // inline entfernt, da andere Lösung erforderlich
{
	// Original APM Code ist nicht im PM verfügbar
	printf(strings_get(STRINGS_SHUTDOWN));
	
	// Endlosschleife
	halt(0);
}

void terror_nuclei(const char *nachricht)
{
	// nucleus_showdebugscreen(); oder ähnlich
	//video_set_draw_color(CL_RED);
	printf("\n\nterror nuclei: %s\n", nachricht);
	shutdown();
}

void dopanic(char *pFile, int nLine)
{    
	printf("\n\nKernel panic! %s (%d)", pFile, nLine);
	// Raise debug exception (to show context and traceback)
//	__asm__ __volatile__ ( "int $0x03\n");
	halt(1);
}

static void test_thread(unsigned long param)
{
	// Do nothing, this tests thread self-destruct upon return
}

void trace_message(char *msg)
{
	char *in = msg;
	short *scrn = (void*)0xB8000;

	while (*in)
		*scrn++ = 0x0800 | *in++;

	delay(1000);
}

void /*__attribute__((constructor))*/ try_ide(void)
{
	unsigned sector[128];
	char buf[256];
	int n;

	// Wait for busy bit to clear
	while (inportb(0x1F7) & 0x80);

	// Setup to read C/H/S = 0/0/1 on master drive
	outportb(0x1F2, 1);			// Sector count
	outportb(0x1F3, 1);			// Sector number
	outportb(0x1F4, 0);			// Cylinder number (16-bit)
	outportb(0x1F5, 0);			// Cylinder number (16-bit)
	outportb(0x1F6, 0 | 0xA0);	// Head 0, master

	// Wait for the ready bit to set
	while (!(inportb(0x1F7) & 0x40));

	// Write command code
	outportb(0x1F7, 0x20);

	// Wait for busy bit to clear
	while (inportb(0x1F7) & 0x80);

	// Wait for command completion (DRQ)
	while (!(inportb(0x1F7) & 0x08));

	// Transfer sector
	for (n = 0; n < 128; n++)
		sector[n] = inportd(0x1F0);

	buf[0] = 0;
	for (n = 0; n < 32; n++)
		sprintf(buf + strlen(buf), " %02X", ((char*)sector)[n]);
	trace_message(buf);
	delay(15000);
}

int main(void)
{
	int fp;

	struct datetime datetime1;

//	trace_message("In main\n");

	trace_message("Trying IDE");
	try_ide();
	trace_message("Trying IDE done");

	// Call constructors (functions using __attribute__((constructor)))
	call_constructors();

	// Initialize GDT
//	trace_message("Init GDT\n");
	gdt_init();

	// Initialize IDT
//	trace_message("Init interrupts\n");
	interrupts_init();

	// Initialize TSS
//	trace_message("Init TSS\n");
	tss_init();

	// Install exception handlers as soon as possible
//	trace_message("Init exceptions\n");
	exception_init();

	// init graphics - needed by all screen output functions
	trace_message("Init graphic\n");
	graphic_init();	

	// Initialize IRQ controllers (hardware)
	trace_message("Init IRQ controllers\n");
	irq_init(); 

	// debuglevel and language are being load from nucload
	nucleuskl_debug = *(char *)NUCLOAD_DEBUGLEVEL;
	strings_setlang(*(char *)NUCLOAD_LANGUAGE);

	// adjust output if minor version is only 1 digit long
	if(NUC_MINOR_VERSION < 10)
		dprintf("%s%d.0%d%s%s\n\n", strings_get(STRINGS_NUCLOAD1), 
				NUC_MAJOR_VERSION, NUC_MINOR_VERSION, NUC_REVISION, strings_get(STRINGS_NUCLOAD2));
	else
		dprintf("%s%d.%d%s%s\n\n", strings_get(STRINGS_NUCLOAD1), 
				NUC_MAJOR_VERSION, NUC_MINOR_VERSION, NUC_REVISION, strings_get(STRINGS_NUCLOAD2));
	
	trace_message("Init threading\n");

	// Initialize multithreading before any interrupts occur
	multi_init();

	// output debuglevel (if debuging is enabled ;-)
	dprintf("%s\n", strings_get(STRINGS_DEBUG + nucleuskl_debug));

	// Initialize memory manager
	trace_message("Init memory manager\n");
	mm_init();

	// Test exception return path
	trace_message("Test breakpoint exception\n");
	__asm__ ( "int $0x03\n" );

	// Test page fault handler
//	*(char*)0xD0000000 = 0;		// Cause page fault
	
	// Initialize timer
	trace_message("Init timer\n");
	timer_init();

	// Initialize keyboard
	trace_message("Init keyboard\n");
	keyboard_init();

	// Set global enable for interrupts
	trace_message("Enabling interrupts\n");
	interrupts_ok = 1;

	// the two PICs are being initialized
	dprintf(strings_get(STRINGS_INTINIT));
	interrupts_enable();
	dprintf(strings_get(STRINGS_OK));

	// cpuid Funktion untersucht die CPU
	trace_message("Init cpu\n");
	dprintf("%s", strings_get(STRINGS_CPUSEARCH));
	cpu_init();
	dprintf("%s%s%d%s", cpu.name, strings_get(STRINGS_CPUFOUND1), cpu.clockrate, strings_get(STRINGS_CPUFOUND2));
	dprintf("Speicher: %ld KB\n", (dword)(detect_memory()/1024));

	// Initialize block device drivers
	block_init();

	// Show graphics information
	graphic_info();	
	
	// Test FAT code
	fp = _fat_fassign("NUCLEUS.BIN", 0xFF);
	if (fp > 0)
	{
//		while ((res =_fat_fread(fp, (char *)&buf, 1)) > 0)
		{
		}
		_fat_fclose(fp);
	}

	nucleuskl_status = NUC_STATUS_RUNNING;
	
	// Initialize CMOS
	datetime1 = cmos_get_time();

	// Quick test
	dyn_test();

	dprintf("Creating threads...\n");

	thread_create(128, 20, test_thread, 0);
	thread_create(128, 20, cdsl, 0);

	// This just releases its timeslice every time it is scheduled
	while (1 == 1) {
		(*(char*)(0xb8000 + 159))++;
		multi_yield();
	}

	terror_nuclei(strings_get(STRINGS_KERNELEXIT));

	call_destructors();

	// Return to where?
	halt(0);

	return 0;
}
