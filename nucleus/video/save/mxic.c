#include <pc.h>
#include <video/graphic.h>
#include <video/graph.h>
#include <support.h>

#define MX_UNKNOWN  0x00
#define MX_86000    0x10
#define MX_86010    0x20
#define MX_86100    0x30
#define MX_86101    0x40
#define MX_86200    0x50
#define MX_86250    0x60
#define MX_86251    0x70

//chips
#define MX_MXIC     0x01
#define MX_MX2      0x02

unsigned int mxic_chip, mxic_chip_v, mxic_mem;

char mxic_test(void)
{
	unsigned int old, sub;
	char result;

	result = 0;
	old = rdinx(SEQ_I, 0xA7);
	wrinx(SEQ_I, 0xA7, 0);	//disable extensions
	if (! testinx(SEQ_I, 0xC5))
	{
		wrinx(SEQ_I, 0xA7, 0x87);	//enable extensions
		if (testinx(SEQ_I, 0xC5))
		{
			result = 1;
			mxic_chip_v = MX_MXIC;
			if ((rdinx(SEQ_I, 0x26) & 1) == 0)
				mxic_chip = MX_86010;
			else
				mxic_chip = MX_86000;	//Does this work, else test 85h bit 1 ??
			switch((rdinx(SEQ_I, 0xC2) >> 2) & 3)
			{
				case 0: mxic_mem = 256; break;
				case 1: mxic_mem = 512; break;
				case 2: mxic_mem = 1024; break;
			}
		}
	}
	wrinx(SEQ_I, 0xA7,old);
	old = rdinx(CRT_I, 0x19);
	wrinx(CRT_I, 0x19, 0);	//Disable extensions
	if (! testinx2(CRT_I, 0x34, 0x3f))
	{
		outportw(CRT_I, 0x8819);	//Enable extensions
		if (testinx2(CRT_I, 0x34, 0x3f))
		{
		 	result = 1;
			mxic_chip_v = MX_MX2;
			sub = rdinx2(CRT_I, 0xF1);
			switch(sub)
			{
				case 0x8625: mxic_chip = MX_86250; break;
				case 0x8626: mxic_chip = MX_86251; break;
				default: mxic_chip = MX_UNKNOWN;
			}
			switch(rdinx(CRT_I, 0x1A) & 0x60)
			{
				case 0: mxic_mem = 1024; break;
				case 0x20: mxic_mem = 2048; break;
				case 0x40: mxic_mem = 4096; break;
			}
		}
	}
	wrinx(CRT_I, 0x19, old);
	return result;
}

unsigned int mxic_chiptype(void)
{
	return mxic_chip;
}

unsigned int mxic_memory(void)
{
	return mxic_mem;
}

char * mxic_get_name(void)
{
	switch(mxic_chip)
	{
		case MX_86000 : return "MXIC MX86000";
		case MX_86010 : return "MXIC MX86010";
		case MX_86100 : return "MXIC MX86100";
		case MX_86101 : return "MXIC MX86101";
		case MX_86200 : return "MXIC MX86200";
		case MX_86250 : return "MXIC MX86250";
		case MX_86251 : return "MXIC MX86251";
	}
	return "MXIC Unknown";
}

void mxic_setbank(unsigned int bank)
{
        if (current_bank == bank)
                return;
        current_bank = bank;
	if (mxic_chip_v == MX_MXIC)
		wrinx(SEQ_I, 0xc5, bank*17);
	else
	if (mxic_chip == MX_86251)
		wrinx(CRT_I, 0x34, bank);
	else
		wrinx(CRT_I, 0x2e, bank);
}

GraphicDriver mxic_driver =
{
	mxic_chiptype,
	mxic_get_name,
	mxic_memory,
	NULL,
	NULL,
	mxic_setbank,
};

char Check_MXIC(GraphicDriver * driver)
{
	*driver = mxic_driver;
	return (mxic_test());
}

