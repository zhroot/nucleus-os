// pic.c

// v0.1: Doug Gale
//    - rewrite
//	- Documented values being programmed into PIC registers
//	- Added irq_acknowledge()
//	- Uses specific EOI command

#include <drivers/pic.h>
#include <interrupts.h>
#include <support.h>
#include <drivers/pic.h>

// Programmable interrupt controller (8259 PIC)

static word irq_mask = 0xFFFB;		// IRQ 2 is always enabled

#define PIC1CMD		0x20
#define PIC2CMD		0xa0
#define PIC1DATA	(PIC1CMD + 1)
#define PIC2DATA	(PIC2CMD + 1)

// Initialize the programmable interrupt controller (8259 PIC)
void irq_init(void)
{
	// The first out sets the PIC to "initialization mode"
	// The following 3 out's setup the PIC

	// Initialization Command Word 1 at Port 20h and A0h
	// 	|7|6|5|4|3|2|1|0|  ICW1
	// 	 | | | | | | | `---- 1=ICW4 is needed, 0=no ICW4 needed
	// 	 | | | | | | `----- 1=single 8259, 0=cascading 8259's
	// 	 | | | | | `------ 1=4 byte interrupt vectors, 0=8 byte int vectors
	// 	 | | | | `------- 1=level triggered mode, 0=edge triggered mode
	// 	 | | | `-------- must be 1 for ICW1 (port must also be 20h or A0h)
	// 	 `------------- must be zero for PC systems
	//   0 0 0 1 1 0 0 1 = 0x19

	outportb(PIC1CMD, 0x11);
	outportb(PIC2CMD, 0x11);

	// IRQ00-IRQ07 = INT 0x20-0x27
	// IRQ08-IRQ15 = INT 0x28-0x2F

	// Initialization Command Word 2 at Port 21h and A1h
	// |7|6|5|4|3|2|1|0|  ICW2
	//  | | | | | `-------- 000= on 80x86 systems
	//  `----------------- A7-A3 of 80x86 interrupt vector

	outportb(PIC1DATA, 0x20);
	outportb(PIC2DATA, 0x28);

	// Initialization Command Word 3 at Port 21h and A1h
	// |7|6|5|4|3|2|1|0|  ICW3 for Master Device
	//  | | | | | | | `---- 1=interrupt request 0 has slave, 0=no slave
	//  | | | | | | `----- 1=interrupt request 1 has slave, 0=no slave
	//  | | | | | `------ 1=interrupt request 2 has slave, 0=no slave
	//  | | | | `------- 1=interrupt request 3 has slave, 0=no slave
	//  | | | `-------- 1=interrupt request 4 has slave, 0=no slave
	//  | | `--------- 1=interrupt request 5 has slave, 0=no slave
	//  | `---------- 1=interrupt request 6 has slave, 0=no slave
	//  `----------- 1=interrupt request 7 has slave, 0=no slave
	// |7|6|5|4|3|2|1|0|  ICW3 for Slave Device
	//  | | | | | `-------- master interrupt request slave is attached to
	//  `----------------- must be zero
	// 0x04 (cascade from IRQ2)
	// 0x02 (attached to IRQ2)

	outportb(PIC1DATA, 0x04);
	outportb(PIC2DATA, 0x02);

	// Initialization Command Word 4 at Port 21h and A1h
	// |7|6|5|4|3|2|1|0|  ICW4
	//  | | | | | | | `---- 1 for 80x86 mode, 0 = MCS 80/85 mode
	//  | | | | | | `----- 1 = auto EOI, 0=normal EOI
	//  | | | | `-------- slave/master buffered mode
	//  | | | `--------- 1 = special fully nested mode (SFNM), 0=sequential
	//  `-------------- unused (set to zero)
	//  0 0 0 0 1 1 0 1 = 0x0d = master
	//  0 0 0 0 1 0 0 1 = 0x09 = slave

	outportb(PIC1DATA, 0x0D);
	outportb(PIC2DATA, 0x09);

	// Initialization complete, PIC back to operational mode

	// Disable all IRQs
	outportb(PIC1DATA, irq_mask);
	outportb(PIC2DATA, irq_mask >> 8);

	// Throw away any pending interrupts
	outportb(PIC1CMD, 0x20);
	outportb(PIC2CMD, 0x20);
}

// Enable an IRQ
void irq_enable(unsigned irq)
{
	// Update mask
	irq_mask &= ~(1 << irq);

	// Program new mask into PIC
	if (irq >= 8) {
		outportb(PIC2DATA, irq_mask >> 8);
	} else {
		outportb(PIC1DATA, irq_mask);
	}
}

// Disable an IRQ
void irq_disable(unsigned irq)
{
	// Update mask
	irq_mask |= (1 << irq);

	// Program new mask into PIC
	if (irq >= 8) {
		outportb(PIC2DATA, irq_mask >> 8);
	} else {
		outportb(PIC1DATA, irq_mask);
	}
}

// Acknowledge specific IRQ
void irq_acknowledge(unsigned irq)
{
	// Handles standard interrupt controllers only
	if (irq >= 16)
		panic();

	if (irq >= 8) {
		// Acknowledge slave controller
		outportb(PIC2CMD, 0x60 + irq - 8);

		// Acknowledge master controller
		outportb(PIC1CMD, 0x60 + 2);
	} else {
		outportb(PIC1CMD, 0x60 + irq);
	}
}

