// multi.h

#ifndef _MULTI_H_
#define _MULTI_H_

// Spawn a new thread. Returns thread identifier
// Negative return values are errors
int thread_create(unsigned stacksize, unsigned priority, 
		void (*execute)(), unsigned thread_arg);

// Get the thread id for the currently running thread
unsigned thread_pid(void);

// Suspend or resume threads
void thread_set_state(unsigned pid, unsigned state);

// Get the thread switching system ready to process thread switches
void multi_init(void);

// Print all the active threads
void multi_show(void);

// Force an immediate thread switch
// If there are no other active threads, returns immediately
void multi_yield(void);

// Exit the currently running thread
void thread_exit(void);

// Scheduler
unsigned multi_scheduler(unsigned old_esp);

// =========================================================================
// CPU context
// =========================================================================

// x86 EFLAGS register
#define EFL_EMPTY	(0x00000002)	// These bits are always set
#define EFL_CF		(1<<0)			// Carry
#define EFL_PF		(1<<2)			// Parity
#define EFL_AF		(1<<4)			// Auxillary carry
#define EFL_ZF		(1<<6)			// Zero
#define EFL_SF		(1<<7)			// Sign
#define EFL_TF		(1<<8)			// Trap
#define EFL_IF		(1<<9)			// Interrupt
#define EFL_DF		(1<<10)			// Direction
#define EFL_OF		(1<<11)			// Overflow
#define EFL_IOPL0	(0<<12)			// I/O Priviledge level
#define EFL_IOPL1	(1<<12)			// I/O Priviledge level
#define EFL_IOPL2	(2<<12)			// I/O Priviledge level
#define EFL_IOPL3	(3<<12)			// I/O Priviledge level
#define EFL_NT		(1<<14)			// Nested task
#define EFL_RF		(1<<16)			// Resume
#define EFL_VM		(1<<17)			// Virtual mode
#define EFL_AC		(1<<18)			// Alignment check
#define EFL_VIF		(1<<19)			// Virtual interrupt flag
#define EFL_VIP		(1<<20)			// Virtual interrupt pending
#define EFL_ID		(1<<21)			// ID

// Default EFLAGS value (for starting threads)
#define EFL_DEFAULT	(EFL_EMPTY|EFL_IF)

// =========================================================================
// Atomic (uninterruptible) operations
// =========================================================================

static inline void atomic_inc(unsigned *var)
{
	__asm__ __volatile__ (
		"incl %0"
		: "=m" (*var)
		: "m" (*var)
		: "cc"
	);
}

static inline void atomic_dec(unsigned *var)
{
	__asm__ __volatile__ (
		"decl %0"
		: "=m" (*var)
		: "m" (*var)
		: "cc"
	);
}

static inline void atomic_add(unsigned *var, unsigned value)
{
	__asm__ __volatile__ (
		"addl %1,%0"
		: "=m" (*var)
		: "r" (value)
		: "cc"
	);
}

static inline unsigned atomic_xchg(unsigned *var, unsigned value)
{
	__asm__ __volatile__ (
		"xchgl %1,%0"
		: "=m" (*var), "=r" (value)
		: "1" (value)
		: "cc"
	);
	return value;
}

// Atomic compare and exchange 
//
// Compares *var to *ifequals
// If they match
//   value is stored in *var
//   returns true
// Else
//   *val is stored in *ifequals
//   return false
// Endif
//
// Always returns initial value of *var
// FIXME: only works on 486 or better, call slow function on 386
static inline unsigned atomic_cmpxchg(unsigned *var, 
		unsigned value, unsigned *ifequals)
{
	__asm__ __volatile__ (
		"cmpxchgl %2,%0\n"		// if eax==%0 { %0 = %3 } else { eax=%0 }
		"setz %%cl\n"			// if xchg happened { ecx=1 } else { ecx=0 }
		"movzbl %%cl,%2\n"		// original value returned in eax
		: "=m" (*var), "=a" (*ifequals), "=c" (value)
		: "m" (*var), "2" (value), "1" (*ifequals)
		: "cc"
	);
	return value;
}

// FIXME: only works on 486 or better, call slow function on 386
static inline unsigned atomic_xadd(unsigned *var, unsigned value)
{
	__asm__ __volatile__ (
		"xaddl %1,%0"
		: "=m" (*var), "=r" (value)
		: "1" (value)
		: "cc"
	);
	return value;
}

#endif	// _MULTI_H_

