#include <video/graphic.h>
#include <video/graph.h>
#include <support.h>

#define V7_UNKNOWN  0x0000
#define V7_VEGA     0x0010
#define V7_208_13   0x0020
#define V7_208A     0x0040
#define V7_208B     0x0080
#define V7_208CD    0x0100
#define V7_216BC    0x0200
#define V7_216D     0x0400
#define V7_216E     0x0800
#define V7_216F     0x1000

unsigned int video7_chip, video7_mem;

void video7_enable(void)
{
	outportw(0x3C4, 0xEA06);
}

void video7_disable(void)
{
	outportw(0x3C4, 0xAE06);
}

char video7_test(void)
{
 	unsigned int new_value, old_value, id, sub;
	char result;

	result = 0;
	video7_enable();
	outportb(0x3D4, 0x0C);
	old_value = inportb(0x3D5);
	outportb(0x3D5, 0x55);
	new_value = inportb(0x3D5);
	inportb(0x3D5);
	outportb(0x3D4, 0x1F);
	id = inportb(0x3D5);
	outportb(0x3D4, 0x0C);
	outportb(0x3D5, old_value);
	if (id == 0xBF)
	{
	 	result = 1;
		wrinx(SEQ_I, 6, 0xEA);	// Enable extensions
		sub = (rdinx(SEQ_I,0x8F) << 8)+rdinx(SEQ_I,0x8E);
		if (sub >= 0x8000 && sub <= 0xFFFF)
			video7_chip = V7_VEGA;
		if (sub >= 0x7000 && sub <= 0x70FF)
			video7_chip = V7_208_13;	//Fastwrite
		if (sub >= 0x7140 && sub <= 0x714F)
			video7_chip = V7_208A;		//1024i
		switch(sub)
		{
			case 0x7151: video7_chip = V7_208B; break;	// VRAm II b
			case 0x7152: video7_chip = V7_208CD; break;	// VRAm II c
			case 0x7760: video7_chip = V7_216BC; break;
			case 0x7763: video7_chip = V7_216D; break;
			case 0x7764: video7_chip = V7_216E; break;
			case 0x7765: video7_chip = V7_216F; break;
			default: video7_chip = V7_UNKNOWN;
		}
		video7_mem = 256; // Hmm...
	}
	video7_disable();
	return result;
}

unsigned int video7_chiptype(void)
{
	return video7_chip;
}

char * video7_get_name(void)
{
	switch(video7_chip)
	{
		case V7_VEGA  : return "Video7 VEGA VGA";
		case V7_208_13: return "Video7 HT208 Version 1-3";
		case V7_208A  : return "Video7 HT208 Rev. A";
		case V7_208B  : return "Video7 HT208 Rev. B";
		case V7_208CD : return "Video7 HT208 Rev. C/D";
		case V7_216BC : return "Video7 HT216 Rev. B/C";
		case V7_216D  : return "Video7 HT216 Rev. D";
		case V7_216E  : return "Video7 HT216 Rev. E";
		case V7_216F  : return "Video7 HT216 Rev. F";
	}
	return "Video7 Unknown";
}

unsigned int video7_memory(void)
{
	return video7_mem;
}

void video7_setbank(unsigned int bank)
{
	unsigned char x;

	if (current_bank == bank)
 		return;
	current_bank = bank;
	if (video7_chip < V7_208A)
	{
		if (mem_mode > _pl4)
		{
			x = inportb(0x3CC) & 0xDF;
			if (bank & 2)
				x += 32;
			outportb(0x3C2, x);
			modinx(SEQ_I, 0xF9, 1, bank);
			bank >>= 2;
		}
		modinx(SEQ_I, 0xF6, 0xF, bank*5);
	}
	else
	{
		if (mem_mode <= _pl4)
		bank *= 4;
		wrinx(SEQ_I, 0xE8, bank << 4);
		// wrinx(SEQ, 0xE9, bank << 4+8); // Don't work, why ?
	}
}

GraphicDriver video7_driver =
{
	video7_chiptype,
	video7_get_name,
	video7_memory,
	NULL,
	NULL,
	video7_setbank
};

char Check_Video7(GraphicDriver * driver)
{
	char res;
	
	res = video7_test();
	if (res)
		*driver = video7_driver;
	return res;
}
