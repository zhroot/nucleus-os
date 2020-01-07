#include <support.h>
#include <stdio.h>
#include <video/graphic.h>
#include <video/graph.h>
#include <video/tseng.h>
#include <video/et4000.h>

unsigned int et4000_chip, et4000_mem;

static void et4000_init(void)
{
	unsigned char old, neu, val;

  	old = inportb(0x3cb);
	outportb(0x3cb, 0x33);
	neu = inportb(0x3cb);
	outportb(0x3cb, old);
	if (neu != 0x33)
		et4000_chip = TSENG_ET4000;
	else
	{
		outportb(0x217a, 0xec);
		val = inportb(0x217b) >> 4;
		switch(val)
		{
			case 0: et4000_chip = TSENG_ET4000W32; break;
			case 1: et4000_chip = TSENG_ET4000W32i_a; break;
			case 2: et4000_chip = TSENG_ET4000W32p_a; break;
			case 3: et4000_chip = TSENG_ET4000W32i_b; break;
			case 5: et4000_chip = TSENG_ET4000W32p_b; break;
			case 6: et4000_chip = TSENG_ET4000W32p_d; break;
			case 7: et4000_chip = TSENG_ET4000W32p_c; break;
			case 11: et4000_chip = TSENG_ET4000W32i_c; break;
			default: et4000_chip = TSENG_UNKNOWN;
		}
	}
	outportb(0x3BF, 3);
	outportb(CRT_I+4, 0xA0);	// Enable Tseng 4000 extensions
	if (tstrg(0x3CD, 0x3F))
	{
		outportb(0x3d4, 0x37);
		val = inportb(0x3d5);
		if (val & 0x08)
			et4000_mem = 256;
		else
			et4000_mem = 64;
		switch(val & 3)
		{
			case 2: et4000_mem *= 2; break;
			case 3: et4000_mem *= 4; break;
		}
		if (val & 0x80)
			et4000_mem *= 2;
		outportb(0x3d4, 0x32);
		if (inportb(0x3d5) & 0x80)
			et4000_mem *= 2;
	}
}

static void et4000_unlock(void)
{
	port_out(3, 0x3bf);
	if (port_in(0x3cc) & 1)
		port_out(0xa0, 0x3d8);
	else
		port_out(0xa0, 0x3b8);
}

/*
old detection code has a strange behaviour ...

static char et4000_test(void)
{
	unsigned char temp, origVal, newVal;
	int iobase;

	iobase = CRT_I;
	// *
	// * Check first that there is a ATC[16] register and then look at
	// * CRTC[33]. If both are R/W correctly it's a ET4000 !
	 
	temp = inportb(iobase + 0x0A);
	et4000_unlock();		       // only ATC 0x16 is protected by KEY 
	outportb(0x3C0, 0x16 | 0x20);
	origVal = inportb(0x3C1);
	outportb(0x3C0, origVal ^ 0x10);
	outportb(0x3C0, 0x16 | 0x20);
	newVal = inportb(0x3C1);
	outportb(0x3C0, origVal);
	//    TsengLock();    FIXME: RESTORE OLD CONTENTS INSTEAD ! 
	if (newVal != (origVal ^ 0x10))
		return 0;
	outportb(iobase + 0x04, 0x33);
	origVal = inportb(iobase + 0x05);
	outportb(iobase + 0x05, origVal ^ 0x0F);
	newVal = inportb(iobase + 0x05);
	outportb(iobase + 0x05, origVal);
	if (newVal != (origVal ^ 0x0F))
		return 0;
	return 1;
}
*/

static char et4000_test(void)
{
    unsigned char newval, oldval, val;
    int base;

    et4000_unlock();

    /* test for Tseng clues */
    oldval = inportb(0x3cd);
    outportb(0x3cd, 0x55);
    newval = inportb(0x3cd);
    outportb(0x3cd, oldval);

    /* return false if not Tseng */
    if (newval != 0x55)
		return 0;

    /* test for ET4000 clues */
    if (inportb(0x3cc) & 1)
		base = 0x3d4;
    else
		base = 0x3b4;
    outportb(base, 0x33);
    oldval = inportb(base + 1);
    newval = oldval ^ 0xf;
    outportb(base + 1, newval);
    val = inportb(base + 1);
    outportb(base + 1, oldval);

    /* return true if ET4000 */
    if (val == newval)
    {
		et4000_init();
		return 1;
    }
    return 0;
}

static unsigned et4000_chiptype(void)
{
	return et4000_chip;
}

static unsigned int et4000_memory(void)
{
	return et4000_mem;
}

static char * et4000_get_name(void)
{
 	char * s = "";

	if (et4000_chip)
	{
	 	sprintf(s, "Tseng %s", et4000_chip_name[et4000_chip]);
		return s;
       	}
	return "Tseng ET4000 Unknown";
}

static void et4000_setbank(unsigned int bank)
{
        if (current_bank == bank)
                return;
        current_bank = bank;
	outportb(0x3CD, (bank & 15)*17);
	if (et4000_chip != TSENG_ET4000)
		outportb(0x3CB,(bank >> 4)*17);
}

GraphicDriver et4000_driver =
{
	et4000_chiptype,
	et4000_get_name,
	et4000_memory,
	NULL,
	NULL,
	et4000_setbank
};

char Check_ET4000(GraphicDriver *driver)
{
	if (et4000_test())
	{
	 	*driver = et4000_driver;
	 	return 1;
	}
	return 0;
}
