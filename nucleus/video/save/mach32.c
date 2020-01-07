#include <support.h>
#include <stdio.h>
#include <string.h>
#include <video/graphic.h>
#include <drivers/mem/mem.h>

/* Some stuff for the ATI VGA */
#define ATIPORT         0x1ce
#define ATIOFF          0x80
#define ATISEL(reg)     (ATIOFF+reg)

/* Ports we use: */
#define SUBSYS_CNTL     0x42E8
#define GE_STAT         0x9AE8
#define CONF_STAT1      0x12EE
#define CONF_STAT2      0x16EE
#define MISC_OPTIONS    0x36EE
#define MEM_CFG         0x5EEE
#define MEM_BNDRY       0x42EE
#define SCRATCH_PAD_0   0x52EE
#define DESTX_DIASTP    0x8EE8
#define R_SRC_X         0xDAEE
#define R_EXT_GE_CONF   0x8EEE
#define CHIP_ID		0xFAEE
#define MAX_WAITSTATES	0x6AEE
#define LOCAL_CNTL	0x32EE
#define R_MISC_CNTL	0x92EE
#define PCI_CNTL	0x22EE
#define DISP_STATUS	0x2E8
#define DISP_CNTL	0x22E8
#define CLOCK_SEL	0x4AEE
#define H_DISP		0x06E8
#define H_TOTAL		0x02E8
#define H_SYNC_WID	0x0EE8
#define H_SYNC_STRT	0x0AE8
#define V_DISP		0x16E8
#define V_SYNC_STRT	0x1AE8
#define V_SYNC_WID	0x1EE8
#define V_TOTAL		0x12E8
#define	R_H_TOTAL	0xB2EE
#define	R_H_SYNC_STRT	0xB6EE
#define	R_H_SYNC_WID	0xBAEE
#define	R_V_TOTAL	0xC2EE
#define	R_V_DISP	0xC6EE
#define	R_V_SYNC_STRT	0xCAEE
#define	R_V_SYNC_WID	0xD2EE

/* Bit masks: */
#define GE_BUSY         0x0200

/* Chip_id's */
#define ATI68800_3	('A'*256+'A')
#define ATI68800_6	('X'*256+'X')
#define ATI68800_6HX	('H'*256+'X')
#define ATI68800LX	('L'*256+'X')
#define ATI68800AX	('A'*256+'X')

unsigned int mach32_chip, mach32_mem;
unsigned short eeprom[128];

static void mach32_i_bltwait()
{
    int i;

    for (i = 0; i < 100000; i++)
	if (!(inportw(GE_STAT) & (GE_BUSY | 1)))
	    break;
#ifdef DEBUG
    if (i >= 100000)
	printf("GE idled out");
#endif
}

static int mach32_detect()
{
	int result = 0;
	short tmp;

	tmp = inportw(SCRATCH_PAD_0);
	outw(SCRATCH_PAD_0, 0x5555);
	mach32_i_bltwait();
	if (inportw(SCRATCH_PAD_0) == 0x5555)
	{
		outw(SCRATCH_PAD_0, 0x2a2a);
		mach32_i_bltwait();
		if (inportw(SCRATCH_PAD_0) == 0x2a2a)
		{
			/* Aha.. 8514/a detected.. */
			result = 1;
		}
	}
	outw(SCRATCH_PAD_0, tmp);
	if (!result)
		goto quit;
	/* Now ensure it is not a plain 8514/a: */
	result = 0;
	outw(DESTX_DIASTP, 0xaaaa);
	mach32_i_bltwait();
	if (inportw(R_SRC_X) == 0x02aa)
	{
		outw(DESTX_DIASTP, 0x5555);
		mach32_i_bltwait();
		if (inportw(R_SRC_X) == 0x0555)
			result = 1;
	}
	quit:
		return result;
}

char mach32_test(void)
{
	int i, j, lastfound, mask, index, flag;

 	if (!mach32_detect()) return 0;

	lastfound = inportw(CHIP_ID) & 0x3ff;
	flag = 0;
	for (i = 0; i < 10240; i++)
	{
		j = inportw(CHIP_ID) & 0x3ff;
		index = (j >> 4);
		mask = 1 << (j & 15);
		if (!(eeprom[index] & mask))
		    printf("\tfound id: %c%c\n\r",
			   0x41 + ((j >> 5) & 0x1f), 0x41 + (j & 0x1f));
		eeprom[index] |= mask;
		if (lastfound != j)
			flag = 1;
	}
	/* Build chip_id from last found id: */
	mach32_chip = (j & 0x1f) + ((j << 3) & 0x1f00);
	mach32_chip += ATI68800_3;
	return flag;
}

char * mach32_get_name(void)
{
 	switch (mach32_chip)
	{
		case ATI68800_3: return "ATI68800-3 (guessed)";
		case ATI68800_6: return "ATI68800-6";
		case ATI68800_6HX: return "ATI68800-6 (HX-id)";
		case ATI68800LX: return "ATI68800LX";
		case ATI68800AX: return "ATI68800AX";
	}
	mach32_chip = ATI68800_3;
	return "Unknown (assuming ATI68800-3)";
}

unsigned int mach32_chiptype(void)
{
        return mach32_chip;
}

unsigned int mach32_memory(void)
{
        return mach32_mem;
}

GraphicDriver mach32_driver =
{
	mach32_chiptype,
	mach32_get_name,
	mach32_memory
};

char Check_Mach32(GraphicDriver * driver)
{
	unsigned int j;

	*driver = mach32_driver;

	memset(eeprom, 0, sizeof(eeprom));

	if (mach32_test())
        {
		j = inportw(MISC_OPTIONS);
                mach32_mem = (1 << ((j >> 2) & 3)) * 512;
                return 1;
	}
	return 0;
}
