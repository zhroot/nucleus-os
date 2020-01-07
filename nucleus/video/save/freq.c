#include <stdio.h>
#include <support.h>
#include <interrupts.h>
#include <video/graphic.h>
#include <video/freq.h>

static unsigned char vga_regs[TOTAL_REGS];

void init_timer(void)
{
 	outportb(0x43, 0x34);
 	outportb(0x40, 0x00);
 	outportb(0x40, 0x00);
}

void ReadTimer(unsigned int *dst)
{
 	unsigned char tmp1, tmp2;

 	outportb(0x43, 0x04);
	tmp1 = inportb(0x40);
	tmp2 = inportb(0x40);
	*dst = ~((tmp2 << 8) + tmp1);
}

void _wait_(unsigned int PORT, unsigned char VAL)
{
 	unsigned char tmp = 0;

        while (! (tmp & VAL))
		tmp = inportb(PORT);
        while (tmp & VAL)
		tmp = inportb(PORT);
}

void wait_horizontal(unsigned int port)
{
	_wait_(port, 8);
}

void wait_vertical(unsigned int port)
{
	_wait_(port, 1);
}

void __loop__(unsigned int port, unsigned char loops, unsigned char msk)
{
	while (loops--)
	{
		_wait_(port, msk);
        }
}

void loop_vertical(unsigned int port, unsigned char loops)
{
	__loop__(port, loops, 1);
}

int interlaced(void)
{
	return (vga_regs[EXT+5] & 0x80) ? 1 : 0;
}

int calc_vtotal(void)
{
	int total;

	total = (vga_regs[EXT+5]&2) ? 1024 : 0;
	switch (vga_regs[CRT+7]&0x21)
	{
		case 0x01 : total += 256; break;
		case 0x20 : total += 512; break;
		case 0x21 : total += 768; break;
	}
	total += vga_regs[CRT+6];
	return total + 2;
}

double measure_horizontal(void)
{
	unsigned int start, stop;
	long diff;
	int ints_were_enabled = interrupts_disable();

	interrupts_disable();
	init_timer();
	wait_horizontal(0x3da);
	wait_vertical(0x3da);
	ReadTimer(&start);
	loop_vertical(0x3da, 200);
	ReadTimer(&stop);
	if (ints_were_enabled)
		interrupts_enable();
	diff = stop-start;
	if (diff < 0) diff += 65536L;
	return (200/(TickTime*((double)diff)));
}

void init_freq(void)
{
 	unsigned char i;
	double vertical, horizontal;
	int ints_were_enabled = interrupts_disable();

	for (i=0; i<CRT_C; i++)
        {
		port_out(i, CRT_I);
		vga_regs[CRT+i] = port_in(CRT_D);
	}
	/* get extended CRT registers */
	for (i=0; i<8; i++)
	{
		port_out(0x30+i, CRT_I);
		vga_regs[EXT+i] = port_in(CRT_D);
	}
	port_out(0x3f, CRT_I);
	vga_regs[EXT+8] = port_in(CRT_D);
	/* get extended sequencer register */
	port_out(7, SEQ_I);
	vga_regs[EXT+9] = port_in(SEQ_D);
	/* get some other ET4000 specific registers */
	vga_regs[EXT+10] = port_in(0x3c3);
	vga_regs[EXT+11] = port_in(0x3cd);
	if (ints_were_enabled)
        interrupts_enable();
	/* Measure video timing */
	horizontal = (double)(measure_horizontal());
	vertical = (double)(horizontal / calc_vtotal());
        horizontal /= 1000.0;
	printf("graphic: Vertical: %ldKHz, Horizontal: %ldHz %s\n",
		(long)vertical, (long)horizontal, interlaced() ? "(interlaced)" : "");

}
