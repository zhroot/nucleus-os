#include <support.h>
#include <video/graphic.h>
#include <video/graph.h>

static unsigned int _3dlabs_test(void)
{
	if (testinx2(SEQ_I, 5, 0x7F) && testinx(GRA_I, 9))
		return 1;
    return 0;
}

static void _3dlabs_setbank(unsigned int bank)
{
	if (current_bank == bank)
 		return;
	current_bank = bank;
	modinx(GRA_I, 9, 7, bank);
}

static char * _3dlabs_get_name(void)
{
        return "3D Labs Unknown";
}

GraphicDriver _3dlabs_driver =
{
	_3dlabs_test,
	_3dlabs_get_name,
	NULL,
	NULL,
	NULL,
	_3dlabs_setbank,
};

char Check_3dlabs(GraphicDriver * driver)
{
	*driver = _3dlabs_driver;
        return (_3dlabs_test());
}
