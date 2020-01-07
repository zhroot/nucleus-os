#include <video/graphic.h>
#include <support.h>

unsigned int rdinx(unsigned int pt, unsigned int inx)       // read register PT index INX
{
 	unsigned int x, result;

        if (pt == 0x3C0)
	{
		x = inportb(CRT_I+6);		// Reset Attribute Data/Address Flip-Flop
		outportb(0x3C0, inx & 0xDF);	// Clear bit 5 of index
		for (x=1; x<= 10; x++) {};	// delay
		result = inportb(0x3C1);
		x = inportb(CRT_I+6);		// Reset Attribute Data/Address Flip-Flop
		for (x=1; x<= 10; x++) {};
		outportb(0x3C0,0x20);		// Set index bit 5 to keep display alive
		x = inportb(CRT_I+6);		// Reset Attribute Data/Address Flip-Flop
                return result;
	}
	else
	{
		outportb(pt, inx);
		return inportb(pt+1);
	}
}

unsigned int rdinx2(unsigned int pt, unsigned int inx)
{
	return (rdinx(pt, inx)+rdinx(pt, inx+1)*256);
}

void wrinx(unsigned int pt, unsigned int inx, unsigned int val)	// write VAL to register PT index INX
{
 	unsigned int x;

	if (pt == 0x3C0)
        {
		x =inportb(CRT_I+6);
		outportb(0x3C0,inx & 0xDF);
		outportb(0x3C0,val);
		x = inportb(CRT_I+6);	// If Attribute Register then reset Flip-Flop
		outportb(0x3C0,0x20);
		x = inportb(CRT_I+6);
	}
	else
	{
		outportb(pt,inx);
		outportb(pt+1,val);
	}
}

char testinx2(unsigned int pt, unsigned int rg, unsigned int msk)
// Returns true if the bits in MSK of register PT index RG are read/writable
{
 	unsigned int old, nw1, nw2;

	old = rdinx(pt,rg);
	wrinx(pt,rg,old &~ msk);
	nw1 = rdinx(pt,rg) & msk;
	wrinx(pt,rg,old | msk);
	nw2 = rdinx(pt,rg) & msk;
	wrinx(pt,rg,old);
	return ((nw1 == 0) && (nw2 == msk));
}

char testinx(unsigned int pt, unsigned int rg)
// Returns true if all bits of register PT index RG are read/writable.
{
	return testinx2(pt,rg,0xff);
}

void modinx(unsigned int pt, unsigned int inx, unsigned int mask, unsigned int nwv)
// In register PT index INX sets the bits in MASK as in NWV
// the other are left unchanged
{
 	unsigned int temp;

        temp = ((rdinx(pt,inx) &~ mask)+(nwv & mask));
	wrinx(pt,inx,temp);
}

void setinx(unsigned int pt, unsigned int inx, unsigned int val)
{
 	unsigned int x;

	x = rdinx(pt,inx);
	wrinx(pt,inx,x | val);
}

void clrinx(unsigned int pt, unsigned int inx, unsigned int val)
{
 	unsigned int x;

	x = rdinx(pt,inx);
	wrinx(pt,inx,x &~ val);
}

void modreg(unsigned int reg, unsigned int mask, unsigned int nwv)
// In register REG sets the bits in MASK as in NWV other are left unchanged
{
 	unsigned int temp;

        temp = ((inportb(reg) &~ mask)+(nwv & mask));
	outportb(reg,temp);
}

char tstrg(unsigned int pt, unsigned int msk)
// Returns true if the bits in MSK of register PT are read/writable
{
 	unsigned int old,nw1,nw2;

	old = inportb(pt);
	outportb(pt, old &~ msk);
	nw1 = inportb(pt) & msk;
	outportb(pt, old | msk);
	nw2 = inportb(pt) & msk;
	outportb(pt, old);
	return ((nw1 == 0) && (nw2 == msk));
}
