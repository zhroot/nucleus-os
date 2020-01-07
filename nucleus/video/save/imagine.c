#include <video/graphic.h>
#include <drivers/pci.h>
#include <support.h>

#define IMG_UNKNOWN  0x00
#define IMG_128      0x10  //Imagine-128
#define IMG_128v2    0x20  //Imagine-128 series 2
#define IMG_t2r      0x40  //Imagine-128 Revolution 3D
#define IMG_t2r4     0x80  //Imagine-128 Revolution IV

unsigned int imagine_chip, imagine_mem, imagine_ioaddr;

char imagine_test(void)
{
	unsigned int i, sub;

	if (CheckPCI() && GetSpecialVendor(0x105D))
	{
		for (i=0; i<cfg_max; i++)
		{
		 	if (pci_list[i].vendorID != 0x105D)
			  	continue;
			imagine_ioaddr = pci_list[i].nonbridge.base_address5;
			switch(inportb(imagine_ioaddr+0x18) >> 6)
			{
				case 0: imagine_mem = 4096; break;
				case 1: imagine_mem = 8192; break;
				case 2: imagine_mem = 16384; break;
				case 3: imagine_mem = 32768; break;
			}
			sub = pci_list[i].deviceID;
			switch(sub)
			{
				case 0x2309: imagine_chip = IMG_128; break;
				case 0x2339: imagine_chip = IMG_128v2; break;
				case 0x493D: imagine_chip = IMG_t2r; break;
				case 0x5348: imagine_chip = IMG_t2r4; break;
				default: imagine_chip = IMG_UNKNOWN;
			}
                	return 1;
		}
	}
	return 0;
}

unsigned int imagine_chiptype(void)
{
	return imagine_chip;
}

unsigned int imagine_memory(void)
{
	return imagine_mem;
}

char * imagine_get_name(void)
{
	switch(imagine_chip)
	{
		case IMG_128   : return "Imagine-128";
		case IMG_128v2 : return "Imagine-128 series 2";
		case IMG_t2r   : return "Imagine-128 Revolution 3D";
		case IMG_t2r4  : return "Imagine-128 Revolution IV";
	}
	return "Imagine Unknown";
}

void imagine_setbank(unsigned int bank)
{
        if (current_bank == bank)
                return;
        current_bank = bank;
	outportw(imagine_ioaddr+0x52, bank);
}

GraphicDriver imagine_driver =
{
	imagine_chiptype,
	imagine_get_name,
	imagine_memory,
	NULL,
	NULL,
	imagine_setbank
};

char Check_Imagine(GraphicDriver * driver)
{
	*driver = imagine_driver;
	return (imagine_test());
}

