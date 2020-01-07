#include <video/graphic.h>
#include <support.h>
#include <drivers/timer.h>

void port_out(int value, int port)
{
	outportb(port, value);
}

void port_outw(int value, int port)
{
	outportw(port, value);
}

void port_outl(int value, int port)
{
	outportd(port, value);
}

int port_in(int port)
{
	return inportb(port);
}

int port_inw(int port)
{
	return inportw(port);
}

int port_inl(int port)
{
	return inportd(port);
} 

void graphic_delay(void)
{
	delay(10);
}

void getbios(char * tmp, unsigned int start, unsigned int len)
{
	int i;

	len = (len <= 100) ? len : 100;
	for (i=start; i<start+len; i++)
		tmp[i] = *(char *)(0xA0000 + start);
	tmp[len+1] = '\0';
}

// Wait for vertical retrace
void wait_retrace(void)
{
	return;

	// Wait for vertical retrace to end
	while ((inportb(IS1_RC) & 0x08) != 0);
	// Wait for vertical retrace to begin
	while ((inportb(IS1_RC) & 0x08) == 0);
}

// bugfixes:
// - changed CRT_I to CRT_IC
void write_regs(unsigned char *regs)
{
	unsigned i;

	if (graphic_card > GR_EGA)
	{	
		// Wait for vertical retrace
		wait_retrace();

    	/* update misc output register */
    	//printf("%04X %02X  -", MIS_W, regs[MIS]);
		outportb(MIS_W, regs[MIS]);

		/* synchronous reset on */
		outportb(SEQ_I, 0x00);
		outportb(SEQ_D, 0x01);    	

		/* write sequencer registers */
		outportb(SEQ_I, 0x01);
		outportb(SEQ_D, regs[SEQ + 1] | 0x20);
    	//printf("\nSEQ\n");
		for (i = 2; i < SEQ_C; i++)
		{
			outportb(SEQ_I, i);
			outportb(SEQ_D, regs[SEQ + i]);
	    	//printf("%04X %02X  -", SEQ_D, regs[SEQ+i]);	
		}

		/* synchronous reset off */
		outportb(SEQ_I, 0x00);
		outportb(SEQ_D, 0x03); 

		/* deprotect CRT registers 0-7 */
		outportb(CRT_IC, 0x11);
		i = inportb(CRT_DC);		
		outportb(CRT_IC, 0x11);
		outportb(CRT_DC, i & 0x7F);
    
    	//printf("\nCRT\n");
		/* write CRT registers */
		for (i = 0; i < CRT_C; i++)
		{
			outportb(CRT_IC, i);
			outportb(CRT_DC, regs[CRT + i]);
	    	//printf("%04X %02X  -", CRT_DC, regs[CRT+i]);
		}

    	//printf("\nGRA\n");
		/* write graphics controller registers */
		for (i = 0; i < GRA_C; i++)
		{
			outportb(GRA_I, i);
			outportb(GRA_D, regs[GRA+i]);
	    	//printf("%04X %02X  -", GRA_D, regs[GRA+i]);
		}

		/* write attribute controller registers */
    	//printf("\nATT\n");
		for (i = 0; i < ATT_C; i++)
		{
			(void)inportb(IS1_RC);			graphic_delay();
			outportb(ATT_I, i);				graphic_delay();
			outportb(ATT_I, regs[ATT+i]);	graphic_delay();
	    	//printf("%04X %02X  -", ATT_I, regs[ATT+i]);	
		}
	}
}
