#include <video/graphic.h>
#include <video/graph.h>
#include <support.h>

unsigned int p2000_mem;

static unsigned int p2000_test(void)
{
	if (testinx2(GRA_I, 0x3D, 0x3F) &&
		tstrg(0x3D6, 0x1F) &&
		tstrg(0x3D7, 0x1F))
	{
//		p2000_mem = check_mem(32, p2000_setbank);
		return 1;
	}
	return 0;
}

static char * p2000_get_name(void)
{
	return "Primus P2000";
}

static unsigned int p2000_memory(void)
{
	return p2000_mem;
}

static void p2000_setbank(unsigned int bank)
{
    if (current_bank == bank)
         return;
    current_bank = bank;
	outportb(0x3d6, bank);
	outportb(0x3d7, bank);
}

GraphicDriver p2000_driver =
{
	p2000_test,
	p2000_get_name,
	p2000_memory,
	NULL,
	NULL,
	p2000_setbank
};

char Check_P2000(GraphicDriver * driver)
{
	*driver = p2000_driver;
	return (p2000_test());
}
