#ifndef _ET4000_H_
#define _ET4000_H_

#define TSENG_ET4000        0x01
#define TSENG_ET4000W32     0x02
#define TSENG_ET4000W32i_a  0x03
#define TSENG_ET4000W32p_a  0x04
#define TSENG_ET4000W32i_b  0x05
#define TSENG_ET4000W32p_b  0x06
#define TSENG_ET4000W32p_d  0x07
#define TSENG_ET4000W32p_c  0x08
#define TSENG_ET4000W32i_c  0x09

static char * et4000_chip_name[] =
{
	"ET4000", "ET4000W32", "ET4000W32i Rev. A", "ET4000W32p Rev. A",
	"ET4000W32i Rev. B", "ET4000W32p Rev. B", "ET4000W32p Rev. D",
	"ET4000W32p Rev. C", "ET4000W32i Rev. C"
};

char Check_ET4000(GraphicDriver * driver);

#endif
