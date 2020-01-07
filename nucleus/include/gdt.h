// gdt.h

// GDT

// Initialize GDT
void gdt_init(void);

// Initialize TSS selector and load task register
void gdt_set_tss(void *tss, unsigned size);
