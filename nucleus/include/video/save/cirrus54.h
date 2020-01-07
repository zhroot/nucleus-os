#ifndef _CIRRUS54_H_
#define _CIRRUS54_H_

#define CL_AVGA1     0x01
#define CL_AVGA2     0x02
#define CL_GD5401    0x03
#define CL_GD5402    0x04
#define CL_GD5402r1  0x05
#define CL_GD5420    0x06
#define CL_GD5420r1  0x07
#define CL_GD5422    0x08
#define CL_GD5424    0x09
#define CL_GD5425    0x0A
#define CL_GD5426    0x0B
#define CL_GD5428    0x0C
#define CL_GD5429    0x0D
#define CL_GD5430    0x0E
#define CL_GD54M30   0x0F
#define CL_GD5440    0x10
#define CL_GD54M40   0x11
#define CL_GD5434    0x12
#define CL_GD5436    0x13
#define CL_GD5446    0x14
#define CL_GD5462    0x15
#define CL_GD5464    0x16
#define CL_GD5465    0x17
#define CL_GD7541    0x18    //Nordic Lite
#define CL_GD7542    0x19    //Nordic
#define CL_GD7543    0x1A    //Viking
#define CL_GD7548    0x1B    //?
#define CL_GD7555    0x1C    //?
#define CL_GD7556    0x1D    //?
#define CL_GD5480    0x1E
#define CL_GD6205    0x1F
#define CL_GD6215    0x20
#define CL_GD6225    0x21
#define CL_GD6235    0x22
#define CL_GD6245    0x23

// chips
#define CL_54       0x01
#define CL_CirLAG   0x02

static char * cirrus54_chip_name[] =
{
	"", "[Acumos AVGA1 (5401)]", "[Acumos AVGA2 (5402)]", "CL-GD5401",
	"CL-GD5402", "CL-GD5402 rev 1", "CL-GD5420", "CL-GD5420 rev 1",
	"CL-GD5422", "CL-GD5424", "CL-GD5425", "CL-GD5426", "CL-GD5428",
	"CL-GD5429", "CL-GD5430", "CL-GD54M30", "CL-GD5440", "CL-GD54M40",
	"CL-GD5434", "CL-GD5436", "CL-GD5446", "CL-GD7541 (Nordic Lite)",
	"CL-GD7542 (Nordic)", "CL-GD7543 (Viking)", "CL-GD7548 (?)",
	"CL-GD7555", "CL-GD7556", "CL-GD5480", "CL-GD6205", "CL-GD6215",
	"CL-GD6225", "CL-GD6235", "CL-GD6245", "Laguna CL-GD5462",
        "Laguna CL-GD5464", "Laguna CL-GD5465"
};

char Check_Cirrus54(GraphicDriver *driver);

#endif
