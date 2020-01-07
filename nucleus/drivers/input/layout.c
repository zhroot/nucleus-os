#include <datatypes.h>
#include <drivers/input/layout.h>
#include <drivers/input/layout/us.h>
#include <drivers/input/layout/de.h>

byte layout_translate(byte scancode, int shift)
{
	if(shift == 0)return scancodes_de[scancode];
	else return scancodes_de_shifted[scancode];
}
