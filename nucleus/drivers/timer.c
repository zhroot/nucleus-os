/*
* Timer functions						*
* v0.1: TDS								*
*       - basic routines				*
* v0.2: TDS (26.05.2004)				*
*		- timer queued sub procedures	*
*		- detection of timer controller *
* v0.3: Doug Gale (01.08.2004)					*
*		- delay(): Improved resolution			*
*		- delay(): Works with IRQ disabled		*
*		- delay(): Corrected duration of delay	*
*		- delay(): Yields unwanted CPU time		*
*		- Corrected disabling of interrupts		*
*/
#include <stdio.h>
#include <support.h>
#include <interrupts.h>
#include <drivers/pic.h>
#include <irqa.h>
#include <multi.h>
#include <drivers/timer.h>
#include <drivers/input.h>
#include <drivers/mem/mem.h>

volatile long long timer_ticks;

// Called in timer IRQ handler
void timer_tick(void)
{
	timer_ticks++;
}

// Wait the specified number of milliseconds
// If you wait less than 16.6ms, has 1 microsecond accuracy
// If you wait more than 16.6ms, has 16.6ms microsecond accuracy
// Works when interrupts are disabled
// Could wait slightly longer than requested
void delay(int x)
{
	dword tmp;
	
	if (x <= 166 || !interrupts_query()) {
		//
		// Short delay or interrupts are disabled

		// Convert milliseconds to microseconds
		tmp = 1000 * x;
		while (tmp--)
			outportb(0x80, 0);
	} else {
		//
		// Long delay, interrupts enabled, CPU efficient

		tmp = timer_ticks + ((x * 60) / 1000);
		while (timer_ticks <= tmp)
			multi_yield();
	}
}

#define TIMER_PORT	0x43
#define PIT_LOW		0x10
#define PIT_HIGH	0x20
#define PIT_BOTH	0x30

#define PIT_MODE_0	0x0	// One shot 
#define PIT_MODE_1	0x2	// No worky 
#define PIT_MODE_2	0x4	// forever 
#define PIT_MODE_3	0x6	// forever 
#define PIT_MODE_4	0x8	// No worky 
#define PIT_MODE_5	0xA	// No worky 

#define Timer_EMUL	0		// Timer is faulty or emulated by OS 
#define Timer_8253	1		// Timer is an 8253 
#define Timer_8254	2		// Timer is an 8254 
#define TIMER_60HZ	0x4DAE	// 60Hz
#define TIMER_30HZ	0x965C	// 30Hz
#define TIMER_20HZ	0xE90B	// 20Hz
#define TIMER_18HZ	0xFFFF	// 18.2Hz (standard count)

unsigned timer_get(int channel)
{
	unsigned tmp;
	
	outportb(TIMER_PORT, channel * 0x40);
	tmp  = inportb(channel + 0x40);
	tmp |= (inportb(channel + 0x40) << 8);
	return tmp;
}

void timer_set(int channel, unsigned value)
{
	outportb(TIMER_PORT, (channel*0x40) | PIT_BOTH | PIT_MODE_3);
	outportb((0x40+channel), (value & 0xFF));
	outportb((0x40+channel), (value >> 8));
}

// FIXME: 8253 is present only in old IBM XT

// detect type of Timer chip - 8253, 8254 or emulated
static unsigned timer_get_type(void)
{
	const word TestValue = 0x55AA;
	const word Backwards = (word)((LOWORD(TestValue) << 8) + HIWORD(TestValue));
	const word EXP_CSTAT = 0x30;	// Expected counter status
	int ints_were_enabled;

	byte port61;
    byte rdb_status1, rdb_status2;
    word rdb_count1, rdb_count2;
    byte timer_type;
    word i, j;
    
	timer_type = Timer_EMUL;
	
	ints_were_enabled = interrupts_disable();

	// turn off speaker & set gate2 input to low 
	port61 = inportb(0x61);
	outportb(0x61, port61 & 0xFC);
  	// program channel 2 to mode 0, two bytes, binary 
	outportb(0x43, 0xB0);
  	outportb(0x42, LOBYTE(TestValue));
 	outportb(0x42, HIBYTE(TestValue));
	// wait until the value of counter 0 changes 
	i = j = timer_get(0);
	while ((j = timer_get(0)) == i);
	i = j;
	while ((j = timer_get(0)) == i);
	// read value from counter 2, test if readout is stable
	i = timer_get(2);
	j = timer_get(2);
	// if not then the Timer is bad or emulated
	if ((i != j) || (i != TestValue))
		goto got_type;
	// readback command will reverse lo/hi flag on a 8053
	outportb(0x43, 0xC8);
  	rdb_status1 = inportb(0x42);
	rdb_count1 = inportb(0x42);
  	rdb_count1 = (inportb(0x42) << 8) + rdb_count1;
	i = timer_get(2);
	// read again to fix hi/lo flag
	outportb(0x43, 0xC8);
	rdb_status2 = inportb(0x42);
	rdb_count2 = inportb(0x42);
	rdb_count2 = (inportb(0x42) << 8) + rdb_count2;
	j = timer_get(2);
	if ((rdb_status1 != EXP_CSTAT) && (rdb_status2 != EXP_CSTAT) && (i == Backwards) && (j == TestValue))
        timer_type = Timer_8253;
	if ((rdb_status1 == EXP_CSTAT) && (rdb_status2 == EXP_CSTAT) && (i == TestValue) && (j == TestValue))
		timer_type = Timer_8254;
got_type:

	if (ints_were_enabled)
		interrupts_enable();

	return timer_type;
}

void timer_init(void)
{
	dprintf("timer: Checking type...");
	switch(timer_get_type())
	{
		case Timer_EMUL: dprintf("emulated!\n"); break;
		case Timer_8253: dprintf("8253\n"); break;
		case Timer_8254: dprintf("8254\n"); break;
		default: dprintf("unknown!\n"); break;
	}
	timer_ticks=0;
	timer_set(0, TIMER_60HZ);

	interrupt_install(INT_HARDWARE, 0, isr_timer);
	irq_enable(0);
}
