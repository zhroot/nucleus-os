#include <drivers/input.h>
#include <drivers/input/keyboard.h>

int input_getch(void)
{
	return readkey();
}
