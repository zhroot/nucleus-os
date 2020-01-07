#ifndef _MEM_H_
#define _MEM_H_

void memset(void *dest, int c, unsigned long count);
void memcpy(void * dest, const void * src, unsigned long count);
void memmove(void *dest, const void *src, unsigned long count);

void memsetw(void *dest, int c, unsigned long count);
void memsetd(void *dest, int c, unsigned long count);

// initialization of the memory management
void mm_init(void);
unsigned long detect_memory(void);

// malloc reserves some memory, if possible and returns a pointer to it
// and free does free this memory again, just like the ANSI C functions
void* malloc(unsigned long size);
void  free(void* ptr);
void* kmalloc(unsigned long size);
void  kfree(void* ptr);

void mem_walk(void);

// Allocate/free a single entire kernel page
void *mm_page_alloc(void);
void mm_page_free(void *page);

// Get physical memory size
unsigned long mm_get_memsize(void);

#endif // _MEM_H_
