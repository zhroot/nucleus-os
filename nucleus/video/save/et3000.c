#include <video/graphic.h>
#include <video/graph.h>
#include <support.h>

static unsigned int et3000_mem;

static void ExtensionOn_Tseng(void)
{
	outportb(0x3BF, 0x03);
	outportb(CRT_I+4, 0xA0);
}

static void ExtensionOff_Tseng(void)
{
	outportb(0x3BF, 0x29);
	outportb(CRT_I+4, 0x01);
}

static unsigned int et3000_test(void)
{
	unsigned char old, val, x;
	unsigned int base;
	unsigned int vs;
	unsigned long tmp = 0x12345678;

	old = port_in(0x3cd);
	port_out(old ^ 0x3f, 0x3cd);
	val = port_in(0x3cd);
	port_out(old, 0x3cd);

	if (val != (old ^ 0x3f))
		return 0;
	if (port_in(0x3cc) & 1)
		base = 0x3d4;
	else
		base = 0x3b4;
	port_out(0x1b, base);
	old = port_in(base + 1);
	port_out(old ^ 0xff, base + 1);
	val = port_in(base + 1);
	port_out(old, base + 1);

	if (val != (old ^ 0xff))
		return 0;
	x = inportb(CRT_I+6);
	x = rdinx(0x3C0, 0x36);
	outportb(0x3C0, x | 0x10);
	switch((rdinx(GRA_I, 6) >> 2) & 3)
	{
		case 0:
		case 1: vs = 0xA0000; break;
		case 2: vs = 0xB0000; break;
		case 3: vs = 0xB8000; break;
	}
	
	_dosmemputl(&tmp, 1, vs);
	_dosmemgetw(vs, 1, &vs);	// vs destroyed and filled
	if (vs == 0x3456)
		et3000_mem = 512;
	wrinx(0x3C0,0x36,x);	// reset value and reenable DAC
	return 1;
}

static unsigned int et3000_memory(void)
{
	return et3000_mem;
}

static char * et3000_get_name(void)
{
	return "ET3000";
}

static void et3000_setbank(unsigned int bank)
{
        if (current_bank == bank)
                return;
        current_bank = bank;
	outportb(0x3CD, bank*9+0x40);
}

GraphicDriver et3000_driver =
{
	et3000_test,
	et3000_get_name,
	et3000_memory,
	NULL,
	NULL,
	et3000_setbank
};

char Check_ET3000(GraphicDriver * driver)
{
	if (et3000_test())
	{
	 	*driver = et3000_driver;
		return 1;
        }
	return 0;
}
