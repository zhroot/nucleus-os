#include <video/graphic.h>
#include <video/graph.h>
#include <video/s3.h>
#include <support.h>

unsigned int s3_chip, s3_mem;
unsigned int rev, subrev;
    
unsigned char ReadReg(unsigned int address, unsigned char index)
// Read a VGA register.
{
	unsigned char tmp;

        if (address == ATR_ADR)
	{
		tmp = inportb(STATUS_1);
		outportb(ATR_ADR, (inportb(ATR_ADR) & 0xE0) | (index & 0x1F));
		return inportb(ATR_DATA);
	}
	else
	{
		outportb(address, index);
		return inportb(address+1);
        }
}

unsigned char WriteReg(unsigned int address, unsigned char index,
		unsigned char value)
// Write to a VGA register.
{
	unsigned char tmp;

        if (address != ATR_ADR)
	{
		outportb(address, index);
		outportb(address+1, value);
	}
	else
        {
		tmp = inportb(STATUS_1);
		outportb(ATR_ADR, (inportb(ATR_ADR) & 0xE0) | (index & 0x1F));
		outportb(ATR_ADR, value);
	}
	return 0x00;
}

char detects3(void)
{
	unsigned char tmp;
	char b;

	b = 0;
	outportb(0x3D4, 0x38);	// Index Lock Register
	tmp = inportb(0x3D5);	// save register
	outportb(0x3D5, 0x48);	// unlock extended S3-registers
	outportb(0x3D4, 0x35);	// Index Bank Select Register
	if (RegBit(0x3D5,0x0F))	// Bit 0..3 useable
	{
		outportb(0x3D4, 0x38);	// Index Lock Register
		outportb(0x3D5, 0);	// lock extended S3-registers
		outportb(0x3D4, 0x35);	// Index Bank Select Register
		b = ~RegBit(0x3D5,0x0F);// card is S3, if bits 0..3 are zero
  	}
	outportb(0x3D4, 0x38);
	outportb(0x3D5, tmp);	// restore register
	return b;
}

void detect_chiptype(void)
{
 	unsigned int sub;

	switch (rev)
	{
		case 0x81: s3_chip = S3_911; break;
		case 0x82: s3_chip = S3_911A_924; break;
		case 0x90: s3_chip = S3_928; break;
		case 0x91: s3_chip = S3_928C; break;
		case 0x94: s3_chip = S3_928D; break;
		case 0x95: s3_chip = S3_928E; break;
		case 0xA0: s3_chip = (rdinx(CRT_I, 0x36) & 2) ? S3_801AB : S3_805AB; break;
		case 0xA1: s3_chip = S3_801_805; break;
		case 0xA2:
                case 0xA3:
                case 0xA4: s3_chip = (rdinx(CRT_I, 0x36) & 2) ? S3_801C : S3_805C; break;
		case 0xA5:
                case 0xA7: s3_chip = (rdinx(CRT_I, 0x36) & 2) ? S3_801D : S3_805D; break;
		case 0xA6: s3_chip = (rdinx(CRT_I, 0x36) & 2) ? S3_801P : S3_805P; break;
		case 0xA8: s3_chip = (rdinx(CRT_I, 0x36) & 2) ? S3_801I : S3_805I; break;
		case 0xB0: s3_chip = S3_928PCI; break;
		case 0xC0: s3_chip = S3_Vision864; break;
		case 0xC1: s3_chip = S3_Vision864P; break;
		case 0xD0:
                case 0xD1: s3_chip = S3_Vision964; break;
		case 0xE0:
                case 0xE1:
			{
			  sub = rdinx(CRT_I, 0x2E)+rdinx(CRT_I, 0x2D)*256;
			  switch (rdinx(CRT_I, 0x2E))	// Not sure of this yet
			  {
				case 0x5631: s3_chip = S3_Virge325; break; //Virge
				case 0x8810: s3_chip = S3_Trio32732; break;
				case 0x8811:
				  s3_chip = (((rdinx(CRT_I, 0x2F)) >> 4) == 5) ? S3_Trio64V765_p : S3_Trio64764; break;
				case 0x8812: s3_chip = S3_Aurora; break;  // Not quite sure: 0x1288 or 0x1289}
				case 0x8814: s3_chip = S3_Trio64UV767_p; break;
				case 0x883D: s3_chip = S3_VirgeVX988; break; //Virge/VX
				case 0x8880: s3_chip = S3_Vision866; break;
				case 0x8890: s3_chip = S3_Vision868; break;
				case 0x88B0:
                                case 0x88F0: s3_chip = S3_Vision968; break;
				case 0x8901:
                                  s3_chip = ((rdinx(CRT_I, 0x6F) & 1) > 0) ? S3_Trio64V2GX785 : S3_Trio64V2DX775; break;
				case 0x8902: s3_chip = S3_PlatoPX; break;
				case 0x8904: s3_chip = S3_Trio3D; break;
				case 0x8A01:
                                  s3_chip = ((rdinx(CRT_I, 0x6F) & 1) > 0) ? S3_VirgeGX385 : S3_VirgeDX375; break;
				case 0x8A10: s3_chip = S3_VirgeGX2357; break;
				case 0x8A20: s3_chip = S3_Savage3D391; break;
				case 0x8C01: s3_chip = S3_VirgeMX260; break;
				case 0x8C03: s3_chip = S3_VirgeMX280_p; break;
				default: s3_chip = S3_UNKNOWN;
			  }
			}
		case 0x5631: s3_chip = S3_Virge325; break;
		case 0x8810: s3_chip = S3_Trio32732; break;
		case 0x8811: s3_chip = (subrev & 0x40) ? S3_Trio64V765_p : S3_Trio64764; break;
		case 0x8812: s3_chip = S3_Aurora64V_p; break;
		case 0x8814: s3_chip = S3_Trio64UV767_p; break;
		case 0x8815: s3_chip = S3_Aurora128; break;
		case 0x8802:
                case 0x88B0:
                case 0x88F0: s3_chip = S3_Vision968; break;
		case 0x883D: s3_chip = S3_VirgeVX988; break;
		case 0x8880: s3_chip = S3_Vision866; break;
		case 0x8890: s3_chip = S3_Vision868; break;
		case 0x8900: s3_chip = S3_Trio64V2DX_GX775_785; break;
		case 0x8901: s3_chip = (subrev & 1) ? S3_Trio64V2GX785 : S3_Trio64V2DX775; break;
		case 0x8902: s3_chip = S3_PlatoPX; break;
		case 0x8904: s3_chip = S3_Trio3D; break;
		case 0x8A01: s3_chip = (subrev & 1) ?  S3_VirgeGX385 : S3_VirgeDX375; break;
		case 0x8A10: s3_chip = S3_VirgeGX2357; break;
		case 0x8A20: s3_chip = S3_Savage3D391; break;
		case 0x8C01: s3_chip = S3_VirgeMX260; break;
		case 0x8C03: s3_chip = S3_VirgeMX280_p; break;
                default: s3_chip = S3_UNKNOWN;
	}
	if (s3_chip <= S3_911A_924)
        {
		if ((rdinx(CRT_I, 0x36) & 0x20) == 0)
			s3_mem = 1024;   //911 and 924
		else
			s3_mem = 512;
	}
	else
	if (s3_chip == S3_VirgeVX988)	//ViRGE/VX
	{
		switch ((rdinx(CRT_I, 0x36) >> 5) & 3)
                {
			case 0: s3_mem = 2048; break;
			case 1: s3_mem = 4096; break;
			case 2: s3_mem = 6144; break;
			case 3: s3_mem = 8192; break;
		}
		switch ((rdinx(CRT_I, 0x37) >> 5) & 3) //Remove OffScreen memory
		{
                 	case 1: s3_mem -= 2048; break;
			case 2: s3_mem -= 4096; break;
		}
	}
	else
	if (s3_chip >= S3_VirgeGX2357)	//ViRGE/GX2,Savage,MX
	{
		switch (rdinx(CRT_I, 0x36) >> 6)
		{
			case 0: s3_mem = 8192; break;
			case 1: s3_mem = 4096; break;
			default: s3_mem = 2048;
		}
        }
	else
	if (s3_chip >= S3_PlatoPX)
	{
		switch (rdinx(CRT_I, 0x36) >> 5)
                {
			case 0: s3_mem = 4096; break;
			case 2: s3_mem = 8192; break;
			case 4: s3_mem = 2048; break;
		}
	}
	else
	{
		switch (rdinx(CRT_I, 0x36) >> 5)
		{
			case 0: s3_mem = 4096; break; //Not 732
			case 2: s3_mem = 3072; break; //Not 7xx
			case 3: s3_mem = 8192; break; //Not 7xx
			case 4: s3_mem = 2048; break;
			case 5: s3_mem = 5120; break; //Not 7xx
			case 6: s3_mem = 1024; break;
		}
	}
}


unsigned char Read3D4(unsigned char ah)
{
 	outportb(0x3D4, ah);
        return inportb(0x3D5);
}

char s3_detect(void)
{
 	unsigned char tmp[2];

	if (! detects3())
        	return 0;

	tmp[0] = Read3D4(0x30); // rev.word, ah = 0
        if (tmp[0] >= 0xE0)
        {
                tmp[0] = Read3D4(0x2D);
                tmp[1] = Read3D4(0x2E);
	}
        rev = (tmp[0] << 8) + tmp[1];
        subrev = tmp[1];
	detect_chiptype();
	return 1;
}

char * s3_get_name(void)
{
        if (s3_chip == 0)
        {
         	return "S3 Unknown";	// sub, sub_rev
        }
	else
	{
                return s3_chip_name[s3_chip];
	}
}

void s3_setbank(unsigned int bank)
{
	if (current_bank == bank)
 		return;
	current_bank = bank;
	if (mem_mode <= _pl4)
		bank *= 4;
	wrinx(CRT_I, 0x39, 0xA5);
	if (s3_chip >= S3_Vision864)
		wrinx(CRT_I, 0x6A, bank);
	else
	{
		wrinx(CRT_I, 0x38, 0x48);
		setinx(CRT_I, 0x31, 9);
		modinx(CRT_I, 0x35, 0xF, bank);
		if (s3_chip > S3_911A_924)
			modinx(CRT_I, 0x51, 0xC, bank >> 2);
		wrinx(CRT_I, 0x38, 0);
	}
	wrinx(CRT_I, 0x39, 0x5A);
}

unsigned int s3_chiptype(void)
{
        return s3_chip;
}

unsigned int s3_memory(void)
{
        return s3_mem;
}

GraphicDriver s3_driver =
{
	s3_chiptype,
	s3_get_name,
	s3_memory,
	NULL,
	NULL,
	s3_setbank
};

char Check_S3(GraphicDriver * driver)
{
	*driver = s3_driver;
	return (s3_detect());
}
