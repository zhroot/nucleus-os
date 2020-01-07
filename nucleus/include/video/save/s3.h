#ifndef _S3_H_
#define _S3_H_

#define S3_UNKNOWN              0x0000
#define S3_911                  0x0001
#define S3_911A_924             0x0002
#define S3_928                  0x0003
#define S3_928C                 0x0004
#define S3_928D                 0x0005
#define S3_928E                 0x0006
#define S3_801_805              0x0007
#define S3_801AB                0x0008
#define S3_805AB                0x0009
#define S3_801C                 0x000A
#define S3_805C                 0x000B
#define S3_801D                 0x000C
#define S3_805D                 0x000D
#define S3_801P                 0x000E
#define S3_805P                 0x000F
#define S3_801I                 0x0010
#define S3_805I                 0x0011
#define S3_Aurora               0x0012
#define S3_928PCI               0x0013
#define S3_Vision864            0x0014
#define S3_Vision864P           0x0015
#define S3_Vision964            0x0016
#define S3_Virge325             0x0017
#define S3_Trio32732            0x0018
#define S3_Trio64V765_p         0x0019
#define S3_Trio64764            0x001A
#define S3_Aurora64V_p          0x001B
#define S3_Trio64UV767_p        0x001C
#define S3_Aurora128            0x001D
#define S3_Vision968            0x001E
#define S3_VirgeVX988           0x001F
#define S3_Vision866            0x0020
#define S3_Vision868            0x0021
#define S3_Trio64V2DX_GX775_785 0x0022
#define S3_Trio64V2GX785        0x0023
#define S3_Trio64V2DX775        0x0024
#define S3_PlatoPX              0x0025
#define S3_Trio3D               0x0026
#define S3_VirgeGX385           0x0027
#define S3_VirgeDX375           0x0028
#define S3_VirgeGX2357          0x0029
#define S3_Savage3D391          0x002A
#define S3_VirgeMX260           0x002B
#define S3_VirgeMX280_p         0x002C

static char * s3_chip_name[] =
{
	"", "86c911", "86c911A/924", "86c928", "86c928C", "86c928D", "86c928E",
	"86c801/805", "86c801AB", "86c805AB", "86c801C", "86c805C", "86c801D",
	"86c805D", "86c801p", "86c805p", "86c801i", "86c801i", "Aurora",
	"86c928PCI", "Vision 864", "Vision 864P", "Vision 964", "Virge 325",
	"Trio32 732", "Trio64V+ 765", "Trio64 764", "Aurora64V+", "Trio64UV+ 767",
	"Aurora128", "Vision 968", "VirgeVX 988", "Vision 866", "Vision 868",
	"Trio64V2 DX/GX 775/785", "Trio64V2 GX 785", "Trio64V2 DX 775", "Plato PX",
	"Trio 3D", "Virge DX 375", "Virge GX 385", "Virge GX2 357", "Savage3D 391",
	"Virge MX 260", "Virge MX+ 280"
};

#define STATUS_0  0x03C2
#define STATUS_1  0x03DA

#define SEQ_DATA  0x03C5
#define CRTC_DATA 0x03D5

#define ATR_ADR   0x03C0            // Attribute Controller Registers: AR0-14
#define ATR_DATA  0x03C1

#define SEQ_ADR	  SR
#define CRTC_ADR  CR

#endif
