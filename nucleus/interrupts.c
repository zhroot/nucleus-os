#include <interrupts.h>
#include <irqa.h>
#include <drivers/pic.h>
#include <support.h>

IdtEntry idt[256];

// v0.1: ???
// v0.2: Doug Gale
//	- Cleaned up initialization
//	- Installs dummy interrupt handlers on unhandled IRQs
//	- Added sanity check for interrupt number when installing interrupt vector

// This sets up the software side of the interrupts
// This sets the IDT vectors, and enables the IRQs
void interrupts_init(void)
{
	int i;
	
	for (i = 0; i < 256; i++)
	{
		idt[i].zero_byte = 0x00;
		idt[i].flags = 0x8E;
		idt[i].func_ptr_00_15 = (unsigned)isr_null & 0x0000FFFF;
		idt[i].func_ptr_16_31 = (unsigned)isr_null >> 16;
		idt[i].selector = 0x0008;
	}

	// Install dummy IRQ handlers
	for (i = 0; i < 8; i++) {
		interrupt_install(INT_HARDWARE, i, isr_dummy_0_7);
		interrupt_install(INT_HARDWARE, i + 8, isr_dummy_8_15);
	}

	// Setup the CPU to use the IDT
	load_idt((unsigned)idt, 0x7FF);
}

void interrupt_install(int type, int number, void (*func_ptr)())
{
	if (number < 0 || number >= 256)
		panic();

	switch (type) {
	case INT_EXCEPTION:
		idt[number].func_ptr_00_15 = (unsigned)func_ptr & 0x0000FFFF;
		idt[number].func_ptr_16_31 = (unsigned)func_ptr >> 16;
		break;
	case  INT_HARDWARE:
		idt[number + 32].func_ptr_00_15 = (unsigned)func_ptr & 0x0000FFFF;
		idt[number + 32].func_ptr_16_31 = (unsigned)func_ptr >> 16;
		break;
	default:
		panic();
	}
}

