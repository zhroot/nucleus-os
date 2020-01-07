#include <stdio.h>
#include <video/graphic.h>
#include <support.h>

#define ATI_M64_UNKNOWN  0x00
#define ATI_M64_GX       0x01
#define ATI_M64_CX       0x02
#define ATI_M64_CT       0x03
#define ATI_M64_ET       0x04
#define ATI_M64_VT       0x05
#define ATI_M64_GT       0x06   //RAGE I
#define ATI_M64_VT2      0x07
#define ATI_M64_GT2      0x08   //RAGE II+
#define ATI_M64_GTP      0x09   //RAGE PrO

static char * mach64_chip_name[] =
{
	"", "88800CX", "88800GX", "264CT", "264ET", "264VT", "264GT RAGE",
	"264VT2", "264GT RAGE II", "264GT RAGE Pro"
};

unsigned int mach64_chip, mach64_mem, mach64_io;

unsigned int Mach64IO(unsigned int addr)
{
	static unsigned char M64CT[32] =	// 0x2EC..0x7EEC remap address
		{0x00, 0x04, 0x08, 0x0C, 0x10, 0x14, 0x18, 0x1C, 0x40, 0x44,
		 0x48, 0x60, 0x64, 0x68, 0x6C, 0x70, 0x80, 0x84, 0x90, 0xA0,
		 0xB0, 0xB4, 0xB8, 0xC0, 0xC4, 0xD0, 0xDC, 0xE0, 0xE4, 0xEC,
		 0x00, 0x00};

	if ((mach64_chip >= ATI_M64_CT) && (mach64_io >= 0x400))
		return mach64_io+(addr & 3)+M64CT[addr >> 10];
	else
		return mach64_io+(addr & 0xFC03);
}

char mach64_test(void)
{
	unsigned int sub, tmp;
	unsigned long mall = 0, mvga, l;
	char bios[100];

	_dosmemgetw((0xC000 << 4) + 0x40, 1, &tmp);
	getbios(bios, 0x31, 9);	
	if ((bios == "761295520") && (tmp = 0x3133))
	{
		_dosmemgetb((0xC000 << 4) + 0x43, 1, &sub);
		if (sub >= 0x20 && sub <= 0x4F)	// Mach64
		{
			//regs.ax = 0xA012;	//listened in Interrupt List, but
						//no function description}
			//regs.dx = 0xFF55;
			//Call(0x10, &regs);
			//if (regs.dx == 0xFF55)
				//mach64_io = 0x2EC;
			//else
				//mach64_io = regs.dx;
			mach64_chip = ATI_M64_CT;	//Cheat for Mach64io()
			sub = inportw(Mach64IO(0x6EEC));
			switch(sub)
			{
				case 0x57: mach64_chip = ATI_M64_CX; break;
				case 0xD7: mach64_chip = ATI_M64_GX; break;
				case 0x4354: mach64_chip = ATI_M64_CT; break;
				case 0x4554: mach64_chip = ATI_M64_ET; break;
				case 0x5654:
				     	_dosmemgetb((0xC000 << 4) + 0x43, 1, &tmp);
					mach64_chip = (tmp == 0x3C) ? ATI_M64_GT : ATI_M64_VT;
					break;
				case 0x4755: mach64_chip = ATI_M64_GT2; break;
				case 0x5655: mach64_chip = ATI_M64_VT2; break;
				case 0x4744:
				case 0x4749:
				case 0x4750:
				case 0x4751: mach64_chip = ATI_M64_GTP; break;
				default: mach64_chip = ATI_M64_UNKNOWN;
			}
			l = inportd(Mach64IO(0x52EC));
			if (mach64_chip < ATI_M64_VT2)
			{
				switch(l & 7)	//Maybe not all VT/GTs
				{
					case 0x00: mall = 512; break;
					case 0x01: mall = 1024; break;
					case 0x02: mall = 2048; break;
					case 0x03: mall = 4096; break;
					case 0x04: mall = 6144; break;
					case 0x05: mall = 8192; break;
				}
			}
			else	//VT,GT and later
			{
				switch(l & 15)
				{
					case 0x00: mall = 512; break;
					case 0x01: mall = 1024; break;
					case 0x03: mall = 2048; break;
					case 0x07: mall = 4096; break;
					case 0x09: mall = 6144; break;
					case 0x0B: mall = 8192; break;
				}
			}
			mvga = mall;
			if (l & 0x40000)
			{
				switch((l >> 16) & 3)
				{
					case 0x00: mvga = 0; break;
					case 0x01: mvga = 256; break;
					case 0x02: mvga = 512; break;
					case 0x03: mvga = 1024; break;
				}
				if (mvga > mall)
					mvga = mall;
			}
			mach64_mem = mvga;
			return 1;
		}
	}
	return 0;
}

char * mach64_get_name(void)
{
	char * s = "";

	if (mach64_chip)
	{
	 	sprintf(s, "ATI Mach64 %s", mach64_chip_name[mach64_chip]);
		return s;
	}
	return "ATI Mach64 Unknown";
}

unsigned int mach64_chiptype(void)
{
	return mach64_chip;
}

unsigned int mach64_memory(void)
{
	return mach64_mem;
}

void mach64_setbank(unsigned int bank)
{
        if (current_bank == bank)
                return;
        current_bank = bank;
	bank *= 2;
	outportb(Mach64IO(0x56EC), bank);
	outportb(Mach64IO(0x56EE), bank+1);
	outportb(Mach64IO(0x5AEC), bank);
	outportb(Mach64IO(0x5AEE), bank+1);
}

GraphicDriver mach64_driver =
{
	mach64_chiptype,
	mach64_get_name,
	mach64_memory,
	NULL,
	NULL,
	mach64_setbank
};

char Check_Mach64(GraphicDriver * driver)
{
	*driver = mach64_driver;
	return (mach64_test());
}

