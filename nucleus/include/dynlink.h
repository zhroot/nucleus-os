// dynlink.h

// Declarations for handling imports and exports

// The import and export areas are arrays of this structure
typedef struct {
	char *name;
	unsigned long value;
} Dyn_Symbol;

// The beginning of the .text section contains one of these structures
typedef struct {
	Dyn_Symbol *export_st;	// Beginning of export array
	Dyn_Symbol *export_en;	// End of export array
	Dyn_Symbol *import_st;	// Beginning of import array
	Dyn_Symbol *import_en;	// End of import array
} Dyn_Header;

int dyn_test(void);

// Export a symbol
#define DYN_EXPORT(name)						\
		__asm__ (								\
			".section .export\n"				\
			".long __export_name_" #name "\n"	\
			".long _" #name "\n"				\
			".section .expnam\n"				\
			"__export_name_" #name ":\n"		\
			".asciz \"" #name "\"\n"			\
			".section .text\n"					\
		)

// Import a symbol
#define DYN_IMPORT(name)						\
		__asm__ (								\
			".section .import\n"				\
			".long __impvec_" #name "\n"		\
			".long __import_name_" #name "\n"	\
			".section .impnam\n"				\
			"__import_name_" #name ":\n"		\
			".asciz \"" #name "\"\n"			\
			".section .text\n"					\
			".global "_" #name "\n"				\
			"_" #name ":\n"						\
			"jmp *__impvec_" #name "\n"			\
		)

