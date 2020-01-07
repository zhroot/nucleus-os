#include <stdio.h>
#include <video/graphic.h>
#include <video/graph.h>
#include <drivers/pci.h>
#include <support.h>

#define	nVidia_V1	0x10
#define	nVidia_V2	0x20

#define	NV_NV1			0x001
#define	NV_STG2000		0x002
#define	NV_STG3000		0x003
#define	NV_RIVA128		0x004
#define	NV_RIVA128zx		0x005
#define	NV_TNT			0x006
#define NV_TNT2			0x007
#define NV_UTNT2		0x008
#define NV_VTNT2		0x009
#define NV_UVTNT2		0x00A
#define NV_ITNT2		0x00B
#define NV_GEFORCE256		0x00C
#define NV_GEFORCEDDR		0x00D
#define NV_QUADRO		0x00E
#define NV_GEFORCE2GTS		0x00F
#define NV_GEFORCE2GTS_1	0x010
#define NV_GEFORCE2ULTRA	0x011
#define NV_QUADRO2PRO		0x012
#define NV_GEFORCE2MX		0x013
#define NV_GEFORCE2MXDDR	0x014
#define NV_QUADRO2MXR		0x015
#define NV_GEFORCE2GO		0x016

// PCI chipsets
#define PCI_CHIP_NV1		0x0008
#define PCI_CHIP_DAC64		0x0009
#define PCI_CHIP_TNT		0x0020
#define PCI_CHIP_TNT2		0x0028
#define PCI_CHIP_UTNT2		0x0029
#define PCI_CHIP_VTNT2		0x002C
#define PCI_CHIP_UVTNT2		0x002D
#define PCI_CHIP_ITNT2		0x00A0
#define PCI_CHIP_GEFORCE256     0x0100
#define PCI_CHIP_GEFORCEDDR     0x0101
#define PCI_CHIP_QUADRO         0x0103
#define PCI_CHIP_GEFORCE2MX     0x0110
#define PCI_CHIP_GEFORCE2MXDDR  0x0111
#define PCI_CHIP_GEFORCE2GO	0x0112
#define PCI_CHIP_QUADRO2MXR     0x0113
#define PCI_CHIP_GEFORCE2GTS    0x0150
#define PCI_CHIP_GEFORCE2GTS_1  0x0151
#define PCI_CHIP_GEFORCE2ULTRA  0x0152
#define PCI_CHIP_QUADRO2PRO     0x0153

static char * nvidia_chip_name[] =
{
	"", "NV1", "SGS STG2000", "Riva128", "SGS STG3000", "Riva128zx", "RivaTNT",
	"RIVATNT2", "RIVATNT2 (Ultra)", "RIVATNT2 (Vanta)", "RIVATNT2 M64", "RIVATNT2 (Integrated)", "GeForce 256",
	"GeForce DDR", "Quadro", "GeForce2 GTS", "GeForce2 GTS (rev 1)", "GeForce2 ultra", "Quadro 2 Pro",
	"GeForce2 MX", "GeForce2 MX DDR", "Quadro 2 MXR", "GeForce 2 Go"
};

unsigned int nvidia_chip, nvidia_chip_v, nvidia_mem;
unsigned int nvidia_seg;

void RIVA_Write(unsigned long adr, unsigned long val)
{
	unsigned int x,	y, i, j;

	x = rdinx(CRT_I, 0x38);
	wrinx(CRT_I, 0x38 ,5);
	y = inportw(0x3D0);
	wrinx(CRT_I, 0x38, 3);
	i = inportw(0x3D0);
	j = inportw(0x3D2);
	outportw(0x3D2,	adr >> 16);	//Offset
	outportw(0x3D0,	adr);
	wrinx(CRT_I, 0x38,7);
	outportw(0x3D0,	val);		//Data
	outportw(0x3D2,	val >> 16);
	wrinx(CRT_I, 0x38, 3);
	outportw(0x3D2,	j);
	outportw(0x3D0,	i);
	wrinx(CRT_I, 0x38, 7);
	outportw(0x3D0,	y);
	wrinx(CRT_I, 0x38, x);
}

unsigned long RIVA_Read(unsigned long adr)
{
	unsigned int x, y, i, j;
	unsigned long l, result;

	x = rdinx(CRT_I, 0x38);
	wrinx(CRT_I, 0x38, 5);
	y = inportw(0x3D0);
	wrinx(CRT_I, 0x38, 3);
	i = inportw(0x3D0);
	j = inportw(0x3D2);
	outportw(0x3D2,	adr >> 16);  //Offset
	outportw(0x3D0,	adr);
	wrinx(CRT_I,0x38, 5);
	l = inportw(0x3D2);	   //Read data
	result = inportw(0x3D0)+(l << 16);
	wrinx(CRT_I, 0x38, 3);
	outportw(0x3D2,	j);
	outportw(0x3D0,	i);
	wrinx(CRT_I, 0x38, 7);
	outportw(0x3D0,	y);
	wrinx(CRT_I, 0x38, x);

	return result;
}

char nvidia_test(void)
{
	unsigned int sub;
	unsigned long l;
	unsigned char counter;

	wrinx(CRT_I, 0x1F, 0x99);
	if ((!testinx(CRT_I, 0x1D) && ! testinx(CRT_I, 0x1E)))
	{
		wrinx(CRT_I, 0x1F, 0x57);
		if ((testinx(CRT_I, 0x1D) && testinx(CRT_I, 0x1E)) &&
			(CheckPCI()))
		{
			nvidia_chip_v = nVidia_V2;
			nvidia_chip = NV_TNT;
			l = RIVA_Read(0);
			sub = (l & 0xFF)+((l & 0xFF) >> 8);
			l = RIVA_Read(0x100000);
			switch (l & 3)
                        {
				case 0: nvidia_mem = 32768; break;
				case 1: nvidia_mem = 4096; break;
				case 2: nvidia_mem = 8192; break;
				case 3: nvidia_mem = 16384; break;
			}
	                return 1;	//Skip test for nV1/STG2000
		}
	}
	wrinx(SEQ_I, 6, 0);
	if (! testinx(CRT_I, 0x1D) && !testinx(CRT_I, 0x1E))
	{
		wrinx(SEQ_I, 6 ,0x57);
		if (testinx(CRT_I, 0x1D) && testinx(CRT_I, 0x1E))
		{
			nvidia_chip_v = nVidia_V2;
			nvidia_chip = NV_RIVA128;
			l = RIVA_Read(0);
			sub = (l & 0xFF)+((l & 0xFF0000) >> 8);
			if ((l & 0xF0) >= 0x30)	// Not sure if revC is automatically a 128zx?
				nvidia_chip = NV_RIVA128zx;
			l = RIVA_Read(0x100000);
			switch(l & 3)
                        {
				case 0: nvidia_mem = 8192; break;
				case 1: nvidia_mem = 2048; break;
				case 2: nvidia_mem = 4096; break;
			}
			return 1;	// Skip test for nV1/STG2000
		}
	}
	if (CheckPCI() && GetSpecialVendor(0x10DE))
	{
		for (counter=0; counter<cfg_max; counter++)
		{
			if (pci_list[counter].vendorID != 0x10DE)
				continue;
			sub = pci_list[counter].deviceID;
			switch (sub)
			{
				case PCI_CHIP_NV1		: nvidia_chip = NV_NV1; break;
				case PCI_CHIP_TNT		: nvidia_chip = NV_TNT; break;
				case PCI_CHIP_TNT2		: nvidia_chip = NV_TNT2; break;
				case PCI_CHIP_UTNT2		: nvidia_chip = NV_UTNT2; break;
				case PCI_CHIP_VTNT2		: nvidia_chip = NV_VTNT2; break;
				case PCI_CHIP_UVTNT2		: nvidia_chip = NV_UVTNT2; break;
				case PCI_CHIP_ITNT2		: nvidia_chip = NV_ITNT2; break;
				case PCI_CHIP_GEFORCE256	: nvidia_chip = NV_GEFORCE256; break;
				case PCI_CHIP_GEFORCEDDR	: nvidia_chip = NV_GEFORCEDDR; break;
				case PCI_CHIP_QUADRO		: nvidia_chip = NV_QUADRO; break;
				case PCI_CHIP_GEFORCE2MX	: nvidia_chip = NV_GEFORCE2MX; break;
				case PCI_CHIP_GEFORCE2MXDDR	: nvidia_chip = NV_GEFORCE2MXDDR; break;
				case PCI_CHIP_GEFORCE2GO	: nvidia_chip = NV_GEFORCE2GO; break;
				case PCI_CHIP_QUADRO2MXR	: nvidia_chip = NV_QUADRO2MXR; break;
				case PCI_CHIP_GEFORCE2GTS	: nvidia_chip = NV_GEFORCE2GTS; break;
				case PCI_CHIP_GEFORCE2GTS_1	: nvidia_chip = NV_GEFORCE2GTS_1; break;
				case PCI_CHIP_GEFORCE2ULTRA	: nvidia_chip = NV_GEFORCE2ULTRA; break;
				case PCI_CHIP_QUADRO2PRO	: nvidia_chip = NV_QUADRO2PRO; break;
			}
			if (nvidia_chip)
				break;
		}
	}
	//nV1/STG2000 test
/*	nvidia_seg = 0xB8000;
	x = rdinx(GRA_I, 6);
	wrinx(GRA_I, 6, 5);	//Map A000-BFFF
	l1 = inmemd(0xB8000, 0x1E04);
	write32(0x1E04, 0x564E6F47);
	l2 = inmemd(0xB8000, 0x1E04);
	write32(0x1E40, 0xFFFFFFFF);
	l3 = inmemd(0xB8000, 0x1E04);
	if (l == 0x1FFE000)
	{
		write32(0x1E40, 0x600000); // Max 600000-601FFF at B000:2000-3fff
		tmp = inmemd(0xB8000, 0x0000);
		switch(tmp & 3)
		{
			case 0: nvidia_mem = 1024; break;
			case 1: nvidia_mem = 2048; break;
			case 2: nvidia_mem = 4096; break;
		}
		if (GetSpecialVendor(0x10DE))	// Look for any nVidia
			nvidia_chip = NV_NV1;
		else
			nvidia_chip = NV_STG2000;
		nvidia_chip_v = nVidia_V1;
		write32(0x1E40, 0x608000);
		write32(0x3010, 0);
		write32(0x3014, 0);	//DAC rg 5: Index hi
		x1 = inmemb(0xB8000, 0x3018);
		write32(0x3010, 0x1010101);
		x = inmemb(0xB8000, 0x3018);
                x = (x << 8) + x1;
//		switch (x)
//		{
//			case 0x3244: SetDAC(_dacSTG1732,'STG1732'); break;
//			case 0x6444: SetDAC(_dacSTG1764,'STG1764'); break;
//		}
		l1 = 0x564E6F4E;
		return 1;
	}
	write32(0x1E40, l2);
	write32(0x1E04, l1);
	wrinx(GRA_I, 6, x);*/
	return 0;
}

char * nvidia_get_name(void)
{
	if (nvidia_chip)
	{
	 	return nvidia_chip_name[nvidia_chip];
	}
	return "Unknown";
}

void nvidia_setbank(unsigned int bank)
{
	if (current_bank == bank)
 		return;
	current_bank = bank;
//	write32(0x1E04, 0x564E6F47);	//Enable MMap regs
//	write32(0x1E10, bank & 7);
//	write32(0x1E04, 0x564E6F4E);	//Enable MMap regs
}

unsigned int nvidia_chiptype(void)
{
        return nvidia_chip;
}

unsigned int nvidia_memory(void)
{
        return nvidia_mem;
}

GraphicDriver nvidia_driver =
{
	nvidia_chiptype,
	nvidia_get_name,
	nvidia_memory,
	NULL,
	NULL,
	nvidia_setbank
};

char Check_Nvidia(GraphicDriver * driver)
{
	*driver = nvidia_driver;
 	return (nvidia_test());
}

