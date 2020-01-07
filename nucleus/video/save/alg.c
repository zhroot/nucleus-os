#include <video/graphic.h>
#include <video/graph.h>
#include <support.h>

#define ALG_UNKNOWN 0x00
#define ALG_2101    0x10
#define ALG_2228    0x20
#define ALG_2301    0x40
#define ALG_2201    0x80

unsigned int alg_chip, alg_mem;

static char alg_test(void)
{
	unsigned int old, sub;
        char result;

        result = 0;
	old = rdinx(CRT_I, 0x1A);
	clrinx(CRT_I, 0x1A, 0x10);
	if (! testinx2(CRT_I, 0x19, 0xCF))
        {
		setinx(CRT_I, 0x1A, 0x10);
		if (testinx2(CRT_I, 0x19, 0xCF) && testinx2(CRT_I, 0x1A, 0x3F))
		{
			result = 1;
			sub = rdinx(CRT_I, 0x1A);
			switch(sub >> 6)
                        {
				case 3: alg_chip = ALG_2101; break;
				case 2: if (rdinx(CRT_I, 0x1B) & 4)
						alg_chip = ALG_2228;
					else
						alg_chip = ALG_2301;
					break;
				// The 2228/2301/230x should probably be ID'd from the PCI ID ?
				case 1: alg_chip = ALG_2201; break;
				default: alg_chip = ALG_UNKNOWN;
			}
			switch (rdinx(CRT_I, 0x1E) & 3)
                        {
				case 0: alg_mem = 256; break;
				case 1: alg_mem = 512; break;
				case 2: alg_mem = 1024; break;
				case 3: alg_mem = 2048; break;
			}
		}
	}
	wrinx(CRT_I, 0x1A, old);
        return result;
}

static unsigned int alg_chiptype(void)
{
	return alg_chip;
}

static char * alg_get_name(void)
{
	switch(alg_chip)
        {
		case ALG_2101: return "Avance Logic 2101";
		case ALG_2228: return "Avance Logic 2228";
		case ALG_2301: return "Avance Logic 2301";
		case ALG_2201: return "Avance Logic 2201";
	}
	return "Avance Logic Unknown";
}

static unsigned int alg_memory(void)
{
	return alg_mem;
}

static void alg_setbank(unsigned int bank)
{
	if (current_bank == bank)
 		return;
	current_bank = bank;
	outportb(0x3d7, bank);
	outportb(0x3D6, bank);
}

GraphicDriver alg_driver =
{
	alg_chiptype,
	alg_get_name,
	alg_memory,
	NULL,
	NULL,
	alg_setbank
};

char Check_ALG(GraphicDriver * driver)
{
	*driver = alg_driver;
	return (alg_test());
}
