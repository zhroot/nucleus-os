#ifndef SUPPORT_H
#define SUPPORT_H

#include <datatypes.h>

dword get_eflags();

#define inportb inb
#define outportb outb
#define inportw inw
#define outportw outw
#define inportd ind
#define outportd outd

static inline void outb(word port, byte data)
{
	__asm__ __volatile__ (
		"outb %%al,%%dx\n"
		:
		: "a" (data), "d" (port)
	);
}
static inline void outw(word port, word data)
{
	__asm__ __volatile__ (
		"outw %%ax,%%dx\n"
		:
		: "a" (data), "d" (port)
	);
}
static inline void outd(word port, dword data)
{
	__asm__ __volatile__ (
		"outl %%eax,%%dx\n"
		:
		: "a" (data), "d" (port)
	);
}

static inline byte inb(word port)
{
	byte ret;
	__asm__ __volatile__ (
		"inb %%dx,%%al\n"
		: "=a" (ret)
		: "d" (port)
	);
	return ret;
}
static inline word inw(word port)
{
	word ret;
	__asm__ __volatile__ (
		"inw %%dx,%%ax\n"
		: "=a" (ret)
		: "d" (port)
	);
	return ret;
}
static inline dword ind(word port)
{
	dword ret;
	__asm__ __volatile__ (
		"inl %%dx,%%eax\n"
		: "=a" (ret)
		: "d" (port)
	);
	return ret;
}

int interrupts_enable(void);
int interrupts_disable(void);
int interrupts_query(void);
void load_idt(dword idt, word limit);

//
// Handle unrecoverable fatal errors
//

// Freeze forever with interrupts disabled
void halt(int print_traceback);

#define panic() dopanic(__FILE__, __LINE__)
void dopanic(char *pFile, int nLine);

//
// Utility macros
//

#define LO_BYTE(x)	(x & 0x00FF)
#define HI_BYTE(x)	((x & 0xFF00) >> 8)

#define MAKELONG(a, b)      ((unsigned long)(((word)(a)) | (((dword)((word)(b))) << 16)))
#define MAKEWORD(a, b)		((unsigned int)(((byte)(b) << 8) + ((byte)(a))))

#ifndef _HIBYTE
#define _HIBYTE(n) ((unsigned char)(((n) >> 8) & 0xff))
#endif
#ifndef _LOBYTE
#define _LOBYTE(n) ((unsigned char)(((n)) & 0xff))
#endif
#ifndef _HIWORD
#define _HIWORD(n) ((unsigned short)(((n) >> 16) & 0xffff))
#endif
#ifndef _LOWORD
#define _LOWORD(n) ((unsigned short)(((n)) & 0xffff))
#endif

#ifndef LOBYTE
#define LOBYTE _LOBYTE
#endif
#ifndef HIBYTE
#define HIBYTE _HIBYTE
#endif
#ifndef LOWORD
#define LOWORD _LOWORD
#endif
#ifndef HIWORD
#define HIWORD _HIWORD
#endif

struct regs {
 unsigned long int eax, ebx, ecx, edx;
 unsigned long int esi, edi, ebp, esp ;
 unsigned long int eip, eflags ;
 unsigned short int cs, ds, ss, es, fs, gs ;
};

//
// Portability wrappers
//

// bytes = units * 1
void _dosmemgetb(unsigned long addr, unsigned long units, void *buf);
void _dosmemputb(const void *buf, unsigned long units, unsigned long addr);
// bytes = units * 2
void _dosmemgetw(unsigned long addr, unsigned long units, void *buf);
void _dosmemputw(const void *buf, unsigned long units, unsigned long addr);
// bytes = units * 4
void _dosmemgetl(unsigned long addr, unsigned long units, void *buf);
void _dosmemputl(const void *buf, unsigned long units, unsigned long addr);

int enable();
int disable();
int _disable();
int _enable();

#ifndef NULL
#define NULL 0
#endif
                                
#endif // SUPPORT_H
