#include <stdio.h>
#include <support.h>
#include <video/graphic.h>
#include <video/graph.h>
#include <video/cirrus.h>
#include <video/cirrus54.h>
#include <drivers/pci.h>

unsigned int cirrus54_chip, cirrus54_chip_v, cirrus54_mem;

static char cirrus54_test(void)
{
	unsigned int x, old, sub;

	cirrus54_chip = 0;
        old = rdinx(SEQ_I, 6);
	wrinx(SEQ_I, 6, 0);
	if (rdinx(SEQ_I, 6) == 0xF)
        {
		wrinx(SEQ_I, 6, 0x12);
		if (rdinx(SEQ_I, 6) == 0x12 && testinx2(SEQ_I, 0x1E, 0x3F))
			// && testinx2(CRT_I, 0x1B,0xff)
		{
			cirrus54_chip_v = CL_54;
			switch(rdinx(SEQ_I, 0xA) & 0x18)	// Alternate method:
                        {
				case 0: cirrus54_chip = 256;	// switch(rdinx(SEQ_I,0xF) & 0x18) {
				case 8: cirrus54_chip = 512;	// 	case 0x10: cv.mm = 1024; break;
				case 16: cirrus54_chip = 1024;	//	case 0x18: cv.mm = 2048; break;  //May not work
				case 24: cirrus54_chip = 2048;	//	default: cv.mm = 512; }
			}
			sub = rdinx(CRT_I, 0x27);
			if (testinx(GRA_I, 9))
			{
				switch(sub)
				{
					case 0x18: cirrus54_chip = CL_AVGA2; break;
					case 0x80:
					case 0x81:
					case 0x82:
					case 0x83: cirrus54_chip = CL_GD5425; break;
					case 0x88: cirrus54_chip = CL_GD5402; break;
					case 0x89: cirrus54_chip = CL_GD5402r1; break;
					case 0x8A: cirrus54_chip = CL_GD5420; break;
					case 0x8B: cirrus54_chip = CL_GD5420r1; break;
					case 0x8C:
					case 0x8D:
					case 0x8E:
					case 0x8F: cirrus54_chip = CL_GD5422; break;
					case 0x90:
					case 0x91:
					case 0x92:
					case 0x93: cirrus54_chip = CL_GD5426; break;
					case 0x94:
					case 0x95:
					case 0x96:
					case 0x97: cirrus54_chip = CL_GD5424; break;
					case 0x98:
					case 0x99:
					case 0x9A:
					case 0x9B: cirrus54_chip = CL_GD5428; break;
					case 0x9C:
					case 0x9D:
					case 0x9E:
					case 0x9F: cirrus54_chip = CL_GD5429; break;	// Might not get here ??
					case 0xA0:
					case 0xA1:
					case 0xA2:
					case 0xA3:
						{
							x = rdinx(CRT_I, 0x28);
							sub += (x << 8);
							switch(x)
                                                        {
								case 0xFF: cirrus54_chip = CL_GD5430; break;
								case 1: cirrus54_chip = CL_GD54M30; break;
								case 3: cirrus54_chip = CL_GD5440; break;
								case 7: cirrus54_chip = CL_GD54M40; break;
							}
						} break;
					// 0xA4..0xA7: cirrus54_chip = CL_GD543x;  // Probably does not exist
					case 0xA8:
					case 0xA9:
					case 0xAA:
					case 0xAB: cirrus54_chip = CL_GD5434; break;
					case 0xAC:
					case 0xAD:
					case 0xAE:
					case 0xAF: cirrus54_chip = CL_GD5436; break;
					case 0xB8:
					case 0xB9:
					case 0xBA:
					case 0xBB: cirrus54_chip = CL_GD5446; break;
					case 0x2C:
					case 0x2D:
					case 0x2E:
					case 0x2F: cirrus54_chip = CL_GD7542; break;	//Nordic
					case 0x30:
					case 0x31:
					case 0x32:
					case 0x33: cirrus54_chip = CL_GD7543; break;	//Viking
					case 0x34:
					case 0x35:
					case 0x36:
					case 0x37: cirrus54_chip = CL_GD7541; break;	//Nordic Lite
					case 0x38:
					case 0x39:
					case 0x3A:
					case 0x3B: cirrus54_chip = CL_GD7548; break;	//?
					default: cirrus54_chip = CL_UNKNOWN;
				}
				if (cirrus54_chip >= CL_GD7541)
				{
					switch(rdinx(SEQ_I, 9) & 15)
                                        {
						case 0: cirrus54_mem = 256; break;
						case 1: cirrus54_mem = 512; break;
						case 2: cirrus54_mem = 1024; break;
						case 3: cirrus54_mem = 2048; break;
						case 4: cirrus54_mem = 4096; break;
					}
				}
                        	else
				if (cirrus54_chip >= CL_GD5430)
				{
					switch(rdinx(SEQ_I, 0x15) & 15)		//  Alternate method:
					{
                                        	case 0: cirrus54_mem = 256;	// switch(rdinx(SEQ_I,0xF) & 0x18) {
						case 1: cirrus54_mem = 512;	//	case 0x10: cirrus54_mem = 1024; break;
						case 2: cirrus54_mem = 1024;	//	case 0x18: cirrus54_mem = 2048; break;
						case 3: cirrus54_mem = 2048;	//	default: cirrus54_mem =512; }
						case 4: cirrus54_mem = 4096;
          				}					// if (rdinx(SEQ_I,0xF) & 0x80)
				}						// 	cirrus54_mem *= 2;
			}
			else
			if (testinx(SEQ_I, 0x19))
			{
				switch(sub >> 6)
				{
	                        	case 0: cirrus54_chip = ((sub >> 4) == 1) ? CL_GD6245 : CL_GD6205; break;
					case 1: cirrus54_chip = CL_GD6235; break;
					case 2: cirrus54_chip = CL_GD6215; break;
					case 3: cirrus54_chip = CL_GD6225; break;
				}
				cirrus54_mem = 512;
			}
			else
			{
				cirrus54_chip = CL_AVGA2;
				switch(rdinx(SEQ_I, 0xA) & 3)
				{
					case 0: cirrus54_mem = 256; break;
					case 1: cirrus54_mem = 512; break;
					case 2: cirrus54_mem = 1024; break;
				}
			}
		}
	}
	else
	if (CheckPCI() && GetSpecialVendor(0x1013))
	{
		wrinx(SEQ_I, 6, old);
		for (x=0; x<cfg_max; x++)
		{
			if (pci_list[x].vendorID != 0x1013)
				continue;
			cirrus54_chip_v = CL_CirLAG;
			switch(pci_list[x].deviceID)
			{
				case 0xD0:
				case 0xD1:
				case 0xD2:
				case 0xD3: cirrus54_chip = CL_GD5462; break;
				case 0xD4:
				case 0xD5: cirrus54_chip = CL_GD5464; break;
				case 0xD6: cirrus54_chip = CL_GD5465; break;
			}
			if (cirrus54_chip)
			{
				cirrus54_mem = ((rdinx(SEQ_I, 0x14) & 7)+1)*1024;
				break;
			}
		}
	}
	return cirrus54_chip;
}

static unsigned int cirrus54_chiptype(void)
{
	return cirrus54_chip;
}

static unsigned int cirrus54_memory(void)
{
	return cirrus54_mem;
}

static char * cirrus54_get_name(void)
{
        char *s = "";

        if (cirrus54_chip)
	{
	        sprintf(s, "Cirrus %s", cirrus54_chip_name[cirrus54_chip]);
        	return s;
	}
	return "Cirrus Unknown";
}

static void cirrus54_setbank(unsigned int bank)
{
	if (current_bank == bank)
 		return;
	current_bank = bank;
	if (! (rdinx(GRA_I, 0xB) & 32))
		bank <<= 2;
	wrinx(GRA_I, 9, bank << 2);
}

GraphicDriver cirrus54_driver =
{
	cirrus54_chiptype,
	cirrus54_get_name,
	cirrus54_memory,
	NULL,
	NULL,
	cirrus54_setbank
};


char Check_Cirrus54(GraphicDriver *driver)
{
	if (cirrus54_test())
	{
        	*driver = cirrus54_driver;
                return 1;
        }
	return 0;
}
