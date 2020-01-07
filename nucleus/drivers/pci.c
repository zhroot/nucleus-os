#include <stdio.h>
#include <drivers/pci.h>
#include <support.h>

unsigned char cfg_check = 0xFF;
unsigned char cfg_mech = 0x00;
unsigned char cfg_max = 0x00;

struct pci_cfg pci_list[0x0F] = { };

static unsigned long PCIAddress(int bus, int device, int func, int reg)
{
	return (0x80000000 | ((unsigned long)(bus & 0xFF) << 16) |
              ((unsigned long)(device & 0x1F) << 11) |
              ((unsigned long)(func & 0x07) << 8) | (reg & 0xFC));
}

static char read_dword_register(int bus, int device, int func, int reg,
			unsigned int *lo, unsigned int *hi)
{
	unsigned long value, addr, orig;
	unsigned char oldenable, oldbus;

        if (cfg_mech == 1)
        {
		addr = PCIAddress(bus, device, func, reg);
		orig = inportd(0xCF8);
		outportd(0xCF8, addr);
		value = inportd(0xCFC);
		outportd(0xCF8, orig);
	}
	else
	{
		if (device > 0x0F)
                {
			*lo = 0xFFFF;
			*hi = 0xFFFF;
                        return 0;
		}
		oldenable = inportb(0xCF8);
		oldbus = inportb(0xCFA);
		outportb(0xCFA,bus);
		outportb(0xCF8,0x80);
		addr = (0xC000 | ((device & 0x0F) << 8) | (reg & 0xFF));
		value = inportd(addr);
		outportb(0xCFA,oldbus);
		outportb(0xCF8,oldenable);
	}
	*hi = (value >> 16);
	*lo = (value & 0xFFFF);
	return 1;
}

void write_dword_register(int bus, int device, int func, int reg,
			unsigned int lo, unsigned int hi)
{
	unsigned long value, addr, orig;
	unsigned char oldenable, oldbus;

	value = (((unsigned long)(hi) << 16) | lo);
        if (cfg_mech == 1)
	{
		addr = PCIAddress(bus, device, func, reg);
		orig = inportd(0xCF8);
		outportd(0xCF8,addr);
		outportd(0xCFC,value);
		outportd(0xCF8,orig);
	}
	else
	{
		if (device > 0x0F)
                	return;
		oldenable = inportb(0xCF8);
		oldbus = inportb(0xCFA);
		outportb(0xCFA, bus);
		outportb(0xCF8, 0x80);
		addr = (0xC000 | ((device & 0x0F) << 8) | reg);
		outportd(addr,value);
		outportb(0xCFA, oldbus);
		outportb(0xCF8, oldenable);
	}
}


static struct pci_cfg * read_PCI_config(int bus, int device, int func)
{
	unsigned int hi, lo, i;
	static struct pci_cfg cfg;

	for (i = 0; i < sizeof(struct pci_cfg)/sizeof(unsigned long); i++)
	{
		if (!read_dword_register(bus,device,func,i*sizeof(unsigned long),&lo,&hi))
			return 0;
		((unsigned int *)&cfg)[2*i] = lo;
		((unsigned int *)&cfg)[2*i+1] = hi;
	}
	return &cfg ;
}

char GetSpecialVendor(unsigned int vendor)
{
	int i;

	for (i=0;i<cfg_max;i++)
	{
		if (pci_list[i].vendorID == vendor)
			return 1;
	}
	return 0x00;
}

char GetSpecialDevice(unsigned int vendor, unsigned int device)
{
	int i;

	for (i=0;i<cfg_max;i++)
	{
		if (pci_list[i].vendorID == vendor &&
			pci_list[i].deviceID == device)
			return 1;
	}
	return 0x00;
}

static int dump_PCI_config(int bus, int device, int func, int * is_multifunc)
{
	struct pci_cfg *cfg = read_PCI_config(bus, device, func);

	if (!cfg || cfg->vendorID == 0xFFFF || cfg->deviceID == 0xFFFF)
		return 0;

	if ((cfg->header_type & 0x80) != 0)	// make multi-function flag sticky
		*is_multifunc = 1;		// since some devs only show on func 0
	pci_list[cfg_max] = *cfg;
	cfg_max++;
	return *is_multifunc;
}

void GetPCIList(void)
{
  	int bus, device, func, maxbus;

        cfg_max = 0;
        maxbus = (cfg_mech == 1) ? 15 : 0xFF;
	for (bus = 0; bus <= maxbus; bus++)
	{
		for (device = 0; device < 32; device++)
		{
			for (func = 0; func < 8; func++)
			{
				int multifunc = 0;
				if (!dump_PCI_config(bus,device,func,&multifunc))
					break;
			}
		}
       }
}


typedef struct BIOS32
{
	unsigned long	magic;
	unsigned long	phys_bsd_entry;
	unsigned char	vers;
	unsigned char	prg_lens;
	unsigned char	crc;
} BIOS32;

char check_pci_bios(void)
{
	unsigned long p, bios32_call;
	BIOS32 *x;
	unsigned char flag, crc, tmp;
	int i;

	bios32_call = 0;
 	flag = 0;
	p = 0xA0000;
	while (!flag && p < 0x100000)
	{
		(BIOS32 *)x = (BIOS32 *)(p);
		if (x->magic == 0x5F32335F)		// _32_
		{
			for(i=0, crc=0; i < (x->prg_lens * 16); i++)
			{
				tmp = (*(unsigned char *)(p + i));
				crc += tmp;
			}
			if (crc == 0)
			{
				flag = 1;
				bios32_call = x->phys_bsd_entry;
			}			
		}
		else
			p += 0x10;
	}
	if (flag)
	{
		printf("pci: Found PCI-BIOS at 0x%lX (%04X:%04X)\n\r", (unsigned long)p,
			(unsigned int)((p >> 16) << 12), (unsigned int)(p & 0xFFFF));
		printf("pci: Found PCI-BIOS service at 0x%lX\n\r", bios32_call);
	}
	return flag;
}

char CheckPCI(void)
{
	unsigned long tmp;

	if (cfg_check != 0xFF)
		return cfg_check;
	outportb(0xCFB, 0x01);
	tmp = inportd(0xCF8);
	outportd(0xCF8, 0x80000000);
	if (inportd(0xCF8) == 0x80000000)
		cfg_mech = 1;
	outportd(0xCF8, tmp); // restore register
	if (cfg_mech == 0xFF)
    {
		outportb(0xCFB, 0x00);
		outportb(0xCF8, 0x00);
		outportb(0xCFA, 0x00);
		if ((inportb(0xCF8) == 0x00) && (inportb(0xCFA) == 0x00))
			cfg_mech = 2;
	}
	cfg_check = cfg_mech;
	if (cfg_mech == 0x00)
      	return 0x00;
	GetPCIList();
    return 0x01;
}
