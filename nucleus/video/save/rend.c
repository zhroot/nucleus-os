#include <video/graphic.h>
#include <drivers/pci.h>
#include <support.h>

#define RND_UNKNOWN  0x00
#define RND_V1000    0x10
#define RND_V2100    0x20
#define RND_V2200    0x40

unsigned int rendition_chip, rendition_mem, rendition_ioaddr;

char rendition_test(void)
{
	unsigned int i, sub;

  	if (CheckPCI() && GetSpecialVendor(0x1163))
	{
		for (i=0; i<cfg_max; i++)
		{
			if (pci_list[i].vendorID != 0x1163)
				continue;
			rendition_ioaddr = pci_list[i].nonbridge.base_address0 & 0xFF00;
			sub = pci_list[i].deviceID;
			switch(sub)
			{
				case 1: rendition_chip = RND_V1000; break;
				case 0x2000:
				     rendition_chip =  (inportb(rendition_ioaddr+0x4D) == 0xC) ? RND_V2200 : RND_V2100; break;
			}
                	rendition_mem = (inportb(rendition_ioaddr+0x71) & 15)*1024;
			return 1;
		}
	}
	return 0;
}

unsigned int rendition_chiptype(void)
{
	return rendition_chip;
}

unsigned int rendition_memory(void)
{
	return rendition_mem;
}

char * rendition_get_name(void)
{
	switch(rendition_chip)
	{
		case RND_V1000: return "Rendition Verite V1000";
		case RND_V2100: return "Rendition Verite V2100";
		case RND_V2200: return "Rendition Verite V2200";
	}
	return "Rendition Unknown";
}

void rendition_setbank(unsigned int bank)
{
        if (current_bank == bank)
                return;
        current_bank = bank;
	if (current_mode > 0x13)
		outportb(rendition_ioaddr+0x76, bank);
}

GraphicDriver rendition_driver =
{
	rendition_chiptype,
	rendition_get_name,
	rendition_memory,
	NULL,
	NULL,
	rendition_setbank
};

char Check_Rendition(GraphicDriver * driver)
{
	*driver = rendition_driver;
	return (rendition_test());
}
