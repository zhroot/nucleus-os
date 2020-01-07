#ifndef _GRAPH_H_
#define _GRAPH_H_

unsigned int rdinx(unsigned int pt, unsigned int inx);
unsigned int rdinx2(unsigned int pt, unsigned int inx);
void wrinx(unsigned int pt, unsigned int inx, unsigned int val);
char testinx2(unsigned int pt, unsigned int rg, unsigned int msk);
char testinx(unsigned int pt, unsigned int rg);
void modinx(unsigned int pt, unsigned int inx, unsigned int mask, unsigned int nwv);
void setinx(unsigned int pt, unsigned int inx, unsigned int val);
void clrinx(unsigned int pt, unsigned int inx, unsigned int val);
void modreg(unsigned int reg, unsigned int mask, unsigned int nwv);
char tstrg(unsigned int pt, unsigned int msk);

extern char (Check_Acer(GraphicDriver * driver));
extern char (Check_Ahead(GraphicDriver * driver));
extern char (Check_ALG(GraphicDriver * driver));	
extern char (Check_Alliance(GraphicDriver * driver));
extern char (Check_Ark(GraphicDriver * driver));		
extern char (Check_Ati(GraphicDriver * driver));		
extern char (Check_Cirrus(GraphicDriver * driver));	
extern char (Check_TSENG(GraphicDriver * driver));	
extern char (Check_Genoa(GraphicDriver * driver));	
extern char (Check_Mach32(GraphicDriver * driver));	
extern char (Check_Mach64(GraphicDriver * driver));	
extern char (Check_OAK(GraphicDriver * driver));		
extern char (Check_Paradise(GraphicDriver * driver));
extern char (Check_Nvidia(GraphicDriver * driver));	
extern char (Check_Trident(GraphicDriver * driver));	
extern char (Check_Video7(GraphicDriver * driver));	
extern char (Check_3dlabs(GraphicDriver * driver));	
extern char (Check_HiQ(GraphicDriver * driver));		
extern char (Check_Compaq(GraphicDriver * driver));	
extern char (Check_Cyrix(GraphicDriver * driver));	
extern char (Check_HMC(GraphicDriver * driver));		
extern char (Check_MXIC(GraphicDriver * driver));	
extern char (Check_NCR(GraphicDriver * driver));		
extern char (Check_NeoMagic(GraphicDriver * driver));
extern char (Check_OPTi(GraphicDriver * driver));	
extern char (Check_P2000(GraphicDriver * driver));	
extern char (Check_Realtek(GraphicDriver * driver));	
extern char (Check_Sierra(GraphicDriver * driver));	
extern char (Check_Sigma(GraphicDriver * driver));	
extern char (Check_SiS(GraphicDriver * driver));		
extern char (Check_SMOS(GraphicDriver * driver));	
extern char (Check_UMC(GraphicDriver * driver));		
extern char (Check_3dfx(GraphicDriver * driver));	
extern char (Check_Imagine(GraphicDriver * driver));	
extern char (Check_Rendition(GraphicDriver * driver));
extern char (Check_S3(GraphicDriver * driver));		
extern char (Check_VGA(GraphicDriver * driver));		
extern char (Check_EGA(GraphicDriver * driver));		

#endif

