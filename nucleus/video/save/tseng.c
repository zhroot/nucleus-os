#include <video/graphic.h>
#include <video/tseng.h>
#include <video/et3000.h>
#include <video/et4000.h>
#include <video/et6000.h>

GraphicDriver tseng_driver;

char tseng_test(void)
{
	char result;

	result = 0;
	if (Check_ET3000(&tseng_driver))
		result = 1;
	else
	if (Check_ET4000(&tseng_driver))
		result = 1;
	else
	if (Check_ET6000(&tseng_driver))
		result = 1;
	return result;
}

unsigned int tseng_chiptype(void)
{
	return tseng_driver.DetectChip();
}

unsigned int tseng_memory(void)
{
	return tseng_driver.DetectMemory();
}

char * tseng_get_name(void)
{
	return tseng_driver.DetectName();
}

void tseng_setbank(unsigned int bank)
{
	tseng_driver.SetBank(bank);
}

char Check_TSENG(GraphicDriver * driver)
{
	char res;
	
	res = tseng_test();
	if (res)
		*driver = tseng_driver;
	return res;
}
