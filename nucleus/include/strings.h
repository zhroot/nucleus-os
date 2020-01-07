#ifndef _STRINGS_H_
#define _STRINGS_H_

#define STRINGS_LANG_GERMAN		0
#define STRINGS_LANG_ENGLISH	1
#define STRINGS_LANG_LATIN		2

#define STRINGS_NUCLOAD1	 0
#define STRINGS_NUCLOAD2	 1
#define STRINGS_DEBUG		 2 // 2 ^= 2, 3 & 4
#define STRINGS_DEBUG_NS	 5
#define STRINGS_CPUSEARCH	 6
#define STRINGS_CPUFOUND1	 7
#define STRINGS_CPUFOUND2	 8
#define STRINGS_INTINIT		 9
#define STRINGS_PICINIT		10
#define STRINGS_NOINIT		11
#define STRINGS_KERNELEXIT	12
#define STRINGS_SHUTDOWN	13
#define STRINGS_OK			14

void strings_setlang(int langid);
char* strings_get(int string);

#endif
