#include <video/graphic.h>
#include <video/graph.h>
#include <support.h>

#define TRIDENT_UNKNOWN   0x0000
#define TRIDENT_8800BR    0x0001
#define TRIDENT_8800CS    0x0002
#define TRIDENT_8900B     0x0003
#define TRIDENT_8900C     0x0004
#define TRIDENT_9000      0x0005
#define TRIDENT_LCD9100B  0x0006
#define TRIDENT_LX8200    0x0007
#define TRIDENT_LCD9320   0x0008
#define TRIDENT_IITAGX    0x0009
#define TRIDENT_9000i     0x000A
#define TRIDENT_9000C     0x000B
#define TRIDENT_8900CL    0x000C
#define TRIDENT_9200CXr   0x000D
#define TRIDENT_9400CXi   0x000E
#define TRIDENT_GUI9420   0x000F
#define TRIDENT_GUI9430   0x0010
#define TRIDENT_GUI9440   0x0011
#define TRIDENT_GUI9440_1 0x0012
#define TRIDENT_GUI9470   0x0013
#define TRIDENT_GUI9660   0x0014
#define TRIDENT_GUI9680   0x0015
#define TRIDENT_GUI9682   0x0016
#define TRIDENT_GUI9685   0x0017
#define TRIDENT_GUI9692   0x0018
#define TRIDENT_Cyb9382   0x0019	// Cyber 938x
#define TRIDENT_Cyb9385   0x001A	// Cyber 938x
#define TRIDENT_Cyb9388   0x001B	// Cyber 938x
#define TRIDENT_Cyb9397   0x001C	// Cyber 938x
#define TRIDENT_3D9750    0x001D	// 3DImage975
#define TRIDENT_3D9850    0x001E	// 3DImage98

static char * trident_chip_name[] =
{
	"", "TVGA 8800BR", "TVGA 8800CS", "TVGA 8900B", "TVGA 8900C",
	"TVGA 9000", "LCD 9100B", "LX 8200", "LCD 9320", "IIT AGX builtin",
	"TVGA 9000i", "TVGA 9000C", "TVGA 8900CL", "TVGA 9200CXr",
	"TVGA 9400CXi", "TGUI 9420DGi", "TGUI 9430", "TGUI 9440AGi",
	"TGUI 9440AGi-1", "TGUI 9470", "TGUI 9660XGi", "TGUI 9680XGi",
	"TGUI 9682", "TGUI 9685", "TGUI 9692", "Cyber9382", "Cyber9385",
	"Cyber9388", "Cyber9397", "3DImage975", "3DImage985"
};

#define IIT_AGX1x         0x001F
#define IIT_AGX14         0x0020
#define IIT_AGX15         0x0021
#define IIT_AGX16         0x0022
#define IIT_AGX17         0x0023	// Does the AGX-017 really exist?

unsigned int trident_chip, trident_mem;
char iit_found;
unsigned int iit_chip, iit_mem;

char trident_init(void)
{
	unsigned int old, sub, val, x;
	unsigned int IOadr;

	iit_found = 0;
	wrinx(SEQ_I, 0xB, 0);	// Force old mode
	sub = inportb(SEQ_I+1);	// new mode
	old = rdinx(SEQ_I,0xE);
	outportb(SEQ_I+1, old ^ 0x55);
	val = inportb(SEQ_I+1);
	outportb(SEQ_I+1, old);
	if (((val ^ old) & 15) == 7)	// Check for inverting bit 1
	{
		outportb(0x3c5, old ^ 2);	// Trident should restore bit 1 reversed
		switch(sub)
		{
			case 1: trident_chip = TRIDENT_8800BR; break;	// This'll never happen - no new mode}
			case 2: trident_chip = TRIDENT_8800CS; break;
			case 3: trident_chip = TRIDENT_8900B; break;
			case 4:
			case 0x13: trident_chip = TRIDENT_8900C; break;
			case 0x23: trident_chip = TRIDENT_9000; break;
			case 0x33: trident_chip = (rdinx(CRT_I, 0x28) & 0x80) ? TRIDENT_8900CL : TRIDENT_9000C; break;
			case 0x43: trident_chip = TRIDENT_9000i; break;
			case 0x53: trident_chip = TRIDENT_9200CXr; break;
			case 0x63: trident_chip = TRIDENT_LCD9100B; break;
			case 0x73: trident_chip = TRIDENT_GUI9420; break;	// Haven't seen this yet ?
			case 0x83: trident_chip = TRIDENT_LX8200; break;
			case 0x93: trident_chip = TRIDENT_9400CXi; break;
			case 0xA3: trident_chip = TRIDENT_LCD9320; break;
			case 0xC3: trident_chip = TRIDENT_GUI9420; break;
			case 0xD3:
				{
					x = rdinx(SEQ_I, 9);
					sub += 256*x;
					switch(x)
					{
						case 0: trident_chip = TRIDENT_GUI9660; break;
						case 1: trident_chip = TRIDENT_GUI9680; break;
						case 2: trident_chip = TRIDENT_GUI9680; break;	// 9680 CR ??
						case 0x10:
						case 0x11:
						case 0x12:
						case 0x13:
						case 0x14:
						case 0x15:
						case 0x16:
						case 0x17:
						case 0x18:
						case 0x19:
						case 0x1A:
						case 0x1B:
						case 0x1C:
						case 0x1D:
						case 0x1E:
						case 0x1F: trident_chip = TRIDENT_GUI9682; break;
						case 0x20:
						case 0x21:
						case 0x22:
						case 0x23:
						case 0x24:
						case 0x25:
						case 0x26:
						case 0x27:
						case 0x28:
						case 0x29:
						case 0x2A:
						case 0x2B:
						case 0x2C:
						case 0x2D:
						case 0x2E:
						case 0x2F: trident_chip = TRIDENT_GUI9685; break;
						case 0x30:
						case 0x31:
						case 0x32: trident_chip = TRIDENT_Cyb9382; break;	// Other combos?
						case 0x33:
						case 0x34:
						case 0x35:
						case 0x36:
						case 0x37:
						case 0x38:
						case 0x39:
						case 0x3A:
						case 0x3B:
						case 0x3C:
						case 0x3D:
						case 0x3E:
						case 0x3F: trident_chip = TRIDENT_Cyb9385; break;
						case 0x50: trident_chip = TRIDENT_GUI9692; break;
						default: trident_chip = TRIDENT_GUI9660;	// 9660/9680 ??
					}
				} break;
			case 0xE3:
				{
					x = rdinx(SEQ_I, 9);
					sub += 256*x;
					switch(x)
					{
						case 1: trident_chip = TRIDENT_GUI9440_1; break;
						case 0x82: trident_chip = TRIDENT_GUI9470; break;
						default: trident_chip = TRIDENT_GUI9440;	// 9440 ??
					}
                                } break;
			case 0xF3:
				{
					x = rdinx(SEQ_I, 9);
					sub += 256*x;
					switch(x)
					{
						case 0: trident_chip = TRIDENT_3D9750; break;
						case 0xB: trident_chip = TRIDENT_3D9850; break;
						default: trident_chip = TRIDENT_GUI9430;	// 9430 ??
					}
                                } break;
			// The 0x63, 0x73, 0x83, 0xA3 entries are still in doubt
			default: trident_chip = TRIDENT_UNKNOWN;
		}
		if (trident_chip < TRIDENT_8900CL)
			trident_mem = ((rdinx(CRT_I, 0x1F) & 3)+1)*256;
		else
		if (trident_chip >= TRIDENT_3D9750)
			trident_mem = ((rdinx(CRT_I, 0x1F) & 15)+1)*256;
		switch(rdinx(CRT_I,0x1F) & 7)
		{
			case 0:
 			case 4: trident_mem = 256; break;
			case 1:
 			case 5: trident_mem = 512; break;
			case 2:
 			case 6: trident_mem = 768; break;
			case 3: trident_mem = 1024; break;
		}
        	if ((sub == 2) && (tstrg(0x2168, 0xF)))	// IIT detection
		{
			iit_found = 1;
			trident_chip = TRIDENT_IITAGX;
			trident_mem = 512;	// Might be able to address 1Mb, but scroll etc only
			// works in the first 512K anyhow !
			IOadr = 0x2160;
			old = inportb(IOadr);
			modreg(IOadr, 7, 4);	// Enable XGA mode
			if (testinx2(IOadr+10, 0x7F, 0x30))
				iit_chip = IIT_AGX1x;
			else
			if (testinx2(IOadr+10, 0x71, 0xF))
				iit_chip = IIT_AGX16;
			else
			if (rdinx(IOadr+10, 0x6C) & 1)
				iit_chip = IIT_AGX15;
			else
				iit_chip = IIT_AGX14;
			outportb(IOadr, old);
//			iit_mem = check_mem(32, trident_setbank);
		}
        	return 1;
	}
	else
	{	// Trident 8800BR tests
		if ((sub == 1) && testinx2(SEQ_I, 0xE, 6))
		{
			trident_chip = TRIDENT_8800BR;
			if (rdinx(CRT_I, 0x1F) & 2)
				trident_mem = 512;
			return 1;
		}
	}
	return 0;
}

char trident_test(void)
{
	unsigned int origVal, neuVal;
	unsigned int save0b;

	outportb(0x3c4, 0x0b);
	save0b = inportb(0x3c5);
	outportw(0x3C4, 0x000B);
	inportb(0x3C5);
	outportb(0x3C4, 0x0E);
	origVal = inportb(0x3C5);
	outportb(0x3C5, 0x00);
	neuVal = inportb(0x3C5) & 0x0F;
	outportb(0x3C5, (origVal ^ 0x02));
	if (neuVal != 2)
	{
		outportb(0x3c5, origVal);
		outportb(0x3c4, 0x0b);
		outportb(0x3c5, save0b);
		return 0;
	}
	return trident_init();
}

unsigned int trident_chiptype(void)
{
	return trident_chip;
}

unsigned int trident_memory(void)
{
	if (iit_found)
		return iit_mem;
	return trident_mem;
}

char * trident_get_name(void)
{
	if (iit_found)
	{
		switch(iit_chip)
		{
			case IIT_AGX1x: return "IIT AGX-010/1";
			case IIT_AGX14: return "IIT AGX-014";
			case IIT_AGX15: return "IIT AGX-015";
			case IIT_AGX16: return "IIT AGX-016";
			case IIT_AGX17: return "IIT AGX-017";
		}
		return "IIT Unknown";
	}
	else
	if (trident_chip)
	{
                return trident_chip_name[trident_chip];
	}
	return "Trident Unknown";
}

void trident_setbank(unsigned int bank)
{
        if (current_bank == bank)
                return;
        current_bank = bank;
	if (trident_chip == TRIDENT_8800BR)
		modinx(SEQ_I, 0xE, 6, bank);
	else
	if (trident_chip >= TRIDENT_9000C)
	{
		outportb(0x3D8, bank);
		outportb(0x3D9, bank);
	}
	else
	{
		wrinx(SEQ_I, 0xB, 0);
		if (! rdinx(SEQ_I, 0xB)) {};	// New mode
		if ((mem_mode <= _pl4) && (bank > 1))
			bank += 2;
		modinx(SEQ_I, 0xE, 0xF, bank ^ 2);
	}
}

GraphicDriver trident_driver =
{
	trident_chiptype,
	trident_get_name,
	trident_memory,
	NULL,
	NULL,
	trident_setbank
};

char Check_Trident(GraphicDriver * driver)
{
	*driver = trident_driver;
	return (trident_test());
}
