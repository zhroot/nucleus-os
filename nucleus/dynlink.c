// dynlink.c

// Dynamic linking tests

// v0.1: Doug Gale
//	- Initial revison. Builds import and export table.
//	- Unfinished

#include <dynlink.h>
#include <stdio.h>
#include <string.h>

// As a test, export all the string functions

DYN_EXPORT(strlen);
DYN_EXPORT(strcmp);
DYN_EXPORT(strcpy);
DYN_EXPORT(strncpy);
DYN_EXPORT(strupr);
DYN_EXPORT(strlwr);
DYN_EXPORT(strstr);

int dyn_test()
{
	// Lookup the function pointer for "strlen" and print the address
	extern Dyn_Symbol __export_st[], __export_en[];
	Dyn_Symbol *pexp = __export_st;

	while (pexp < __export_en) {
		printf("Found %s at %08X\n", pexp->name, pexp->value);

		if (strcmp(pexp->name, "strcmp") == 0)
			break;
		pexp++;
	}
	if (pexp >= __export_en)
		return -1;

	// I found it!
	printf("Found export named %s at address %08X\n", 
			pexp->name, pexp->value);
	return 0x00;
}

