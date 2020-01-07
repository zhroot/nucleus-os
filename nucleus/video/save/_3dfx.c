#include <drivers/pci.h>
#include <video/graphic.h>
#include <support.h>

#define _3dfx_UNKNOWN 0x00
#define _3dfx_Voodoo  0x10
#define _3dfx_Voodoo2 0x20
#define _3dfx_Banshee 0x30

static unsigned int _3dfx_chip, _3dfx_mem, _3dfx_ioaddr;

static char _3dfx_test(void)
{
	unsigned char i;

        if (CheckPCI() && GetSpecialVendor(0x121A))
        {
		for (i=0; i<cfg_max; i++)
                {
			if (pci_list[i].vendorID != 0x121A)
				continue;
			switch(pci_list[i].deviceID)
			{
				case 1: {
					_3dfx_chip = _3dfx_Voodoo;
					_3dfx_mem = 4096;
					_3dfx_ioaddr = pci_list[i].nonbridge.base_address2 & 0xFFFE;
					} break;
				case 2: {
					_3dfx_chip = _3dfx_Voodoo2;
					_3dfx_mem = 8192;
					_3dfx_ioaddr = pci_list[i].nonbridge.base_address2 & 0xFFFE;
					} break;
				case 3: {
					_3dfx_chip = _3dfx_Banshee;
					_3dfx_mem = 16384;
					_3dfx_ioaddr = pci_list[i].nonbridge.base_address2 & 0xFFFE;
					} break;
			}
			if (_3dfx_chip) break;
		}
	}
	return (_3dfx_chip != 0);
}

static char * _3dfx_get_name(void)
{
	switch(_3dfx_chip)
        {
         	case _3dfx_Voodoo : return "3Dfx Voodoo";
         	case _3dfx_Voodoo2: return "3Dfx Voodoo 2";
         	case _3dfx_Banshee: return "3Dfx Banshee";
	}
	return "3Dfx Unknown";
}

static void _3dfx_setbank(unsigned int bank)
{
	if (current_bank == bank)
 		return;
	current_bank = bank;
	outportd(_3dfx_ioaddr+0x2C,bank*(unsigned long)(0x802) | 0x100000);
}

static unsigned int _3dfx_chiptype(void)
{
        return _3dfx_chip;
}

static unsigned int _3dfx_memory(void)
{
        return _3dfx_mem;
}

GraphicDriver _3dfx_driver =
{
	_3dfx_chiptype,
	_3dfx_get_name,
	_3dfx_memory,
	NULL,
	NULL,
	_3dfx_setbank
};

char Check_3dfx(GraphicDriver * driver)
{
	*driver = _3dfx_driver;
	return (_3dfx_test());
}
