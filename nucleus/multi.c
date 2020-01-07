// multi.c

// v0.1: Doug Gale
//	- Initial revision.
//	- Rewrite from earlier implementation
// v0.2: Doug Gale
//	- Added critical section reference count to handle disabling of interrupts

#include <datatypes.h>
#include <support.h>
#include <interrupts.h>
#include <string.h>
#include <stdio.h>
#include <multi.h>
#include <pagetbl.h>
#include <drivers/timer.h>
#include <drivers/mem/mem.h>

#define MULTI_DEBUG		defined(DEBUG)

#define THREADS_MAX	200		// should be enough

// This is the "save area" on the stack created by a thread switch
typedef struct {
	// Page directory base register
	unsigned r_cr3;

	// General registers
	unsigned r_eax, r_ebx, r_ecx, r_edx, r_esi, r_edi, r_ebp;

	// Segment registers
	unsigned r_ds, r_es, r_fs, r_gs;

	// FPU/MMX context
	unsigned char r_fpu[108];

	// The following are pushed by the CPU
	unsigned r_eip, r_cs, r_eflags;
} ThreadContext;

// This is the structure created on the stack when creating a thread
typedef struct {
	ThreadContext context;

	// The return address, initialized to thread_exit()
	unsigned tc_ret;

	// Argument passed to thread function
	unsigned tc_arg;
} ThreadCreate;

// =========================================================================
// Thread information
// =========================================================================

typedef enum {
	THST_UNUSED,		// This thread structure is not currently in use
	THST_EXITING,		// Needs stack deallocated
	THST_SUSPENDED,		// The thread is suspended
	THST_ACTIVE,		// Running or ready to run
	THST_MAX			// For bounds-checking enumeration
} ThreadState;

// Thread info block
typedef struct tagThreadInfo {
	// Linked list (circular)
	struct tagThreadInfo *next, *prev;	// Circular linked list pointers

	// Stack bounds
	void *stack_lo, *stack_hi;			

	// esp register to resume
	unsigned r_esp;

	// Current state of the thread
	ThreadState state;					
} ThreadInfo;

// Used to implement other circular lists of ThreadInfo structures
typedef struct {
	ThreadInfo *next, *prev;
} ThreadChain;

// Free list
ThreadChain free_threads;

// Thread identifiers are an index into the following array
// Zero is the main kernel thread.
// Active threads are linked to this thread.
// The kernel thread always has work to do:
//  - Clearing unused pages
//  - Checking page accessed and modified flags (virtual memory)
// It is expected to finish its work early and yield
static unsigned cur_thread_id;
static ThreadInfo thread_table[THREADS_MAX];

// =========================================================================
// Protection for thread chains. Manages interrupt flag.
// =========================================================================

static volatile unsigned critical_count, critical_enab_ints;

// Enter critical section of code.
// Disables interrupts and updates reference count.
// Note that for each call of multi_critical_enter you 
// must call multi_critical_exit.
static void multi_critical_enter()
{
	int ints_were_enabled = interrupts_disable();

	// Remember interrupt-enable flag state if this is the first "entry"
	if (critical_count++ == 0)
		critical_enab_ints = ints_were_enabled;
}

// Expects interrupts disabled on entry
static void multi_critical_exit()
{
	if (!critical_count)
		panic();

	if (--critical_count == 0) {
		if (critical_enab_ints)
			interrupts_enable();
	}
}

// =========================================================================
// Helpers
// =========================================================================

static ThreadInfo *thread_destroy(ThreadInfo *thread)
{
	ThreadInfo *next;

	next = thread->next;

	free(thread->stack_lo);
	thread->stack_lo = 0;
	thread->stack_hi = 0;
	thread->r_esp = 0;
	thread->state = THST_UNUSED;

	// Unlink thread from current chain
	thread->next->prev = thread->prev;
	thread->prev->next = thread->next;

	// Link into end of free chain
	thread->next = (void*)&free_threads;
	thread->prev = free_threads.prev;
	thread->next->prev = thread;
	thread->prev->next = thread;

	return next;
}

// =========================================================================
// Saves stack for current thread and selects next running thread.
// =========================================================================

// Returns esp value for new stack
// Called by IRQ handler
unsigned multi_scheduler(unsigned old_esp)
{
	ThreadInfo *cur_thread;
	int found;

	// Initialized?
	if (thread_table[0].next == 0)
		return old_esp;

	// Calculate pointer to current thread
	cur_thread = thread_table + cur_thread_id;

	// Store last esp in the current thread's ThreadInfo
	cur_thread->r_esp = old_esp;

	// Step to the next active thread
	cur_thread = cur_thread->next;

	// Search for a thread that is ready to run
	// Clean up exited threads when encountered
	found = 0;
	do {
		// Should I destroy this thread?
		switch (cur_thread->state) {
		case THST_EXITING:
			cur_thread = thread_destroy(cur_thread);
			break;
		case THST_ACTIVE:
			found = 1;
			break;
		case THST_SUSPENDED:
			cur_thread = cur_thread->next;
			break;
		default:
			panic();
		}
	} while (!found);

	// Update current thread id
	cur_thread_id = cur_thread - thread_table;

#if MULTI_DEBUG
	// DEBUG:
	// Make sure that the pointer is possible
	if (cur_thread < thread_table || 
			cur_thread >= thread_table + THREADS_MAX)
		panic();

	// DEBUG:
	// Make sure the pointer is aligned properly
	if (cur_thread != thread_table + cur_thread_id)
		panic();

	// DEBUG:
	// Validate stack pointer
	if (cur_thread->r_esp <= (unsigned)cur_thread->stack_lo ||
			cur_thread->r_esp > (unsigned)cur_thread->stack_hi) {
		printf("\nBad stack pointer, switching to thread %d!\n"
				"Old ESP: %08x, New ESP: %08x\n", 
				cur_thread_id, old_esp, cur_thread->r_esp);
		panic();
	}
#endif

	// Return esp for new thread
	return cur_thread->r_esp;
}

#if MULTI_DEBUG
static int multi_tests(void);
#endif

// =========================================================================
// Initialize
// =========================================================================

void multi_init(void)
{
	int i;
	ThreadInfo *p;

	multi_critical_enter();

	// Initialize circular list
	thread_table->next = thread_table;
	thread_table->prev = thread_table;

	// Hard-coded kernel stack (256KB)
	thread_table->stack_hi = (void*)0x00090000;
	thread_table->stack_lo = (void*)0x00050000;

	// Active
	thread_table->state = THST_ACTIVE;

	// Link the remaining structures to the free list
	free_threads.next = (void*)&free_threads;
	free_threads.prev = (void*)&free_threads;

	for (i = 1, p = thread_table + 1; i < THREADS_MAX; i++, p++) {
		// Fill link pointers in this element
		p->prev = (void*)free_threads.prev;
		p->next = (void*)&free_threads;
		// Link into chain
		p->prev->next = p;
		p->next->prev = p;

		p->state = THST_UNUSED;
	}

	multi_critical_exit();

	// Force a thread switch, to get kernel thread info initialized
	// It "switches" back to the same thread, so this function
	// returns immediately.
	// Not strictly necessary, but might be in the future
	multi_yield();

#if MULTI_DEBUG
	// Run debug tests
	multi_tests();
#endif
}

#if MULTI_DEBUG
unsigned delme_test_a;
static int multi_tests(void)
{
	int x, y;

	atomic_inc(&delme_test_a);
	atomic_dec(&delme_test_a);
	atomic_add(&delme_test_a,1);
	atomic_add(&delme_test_a,-1);
	x = atomic_xchg(&delme_test_a,2);
	y = atomic_xchg(&delme_test_a,0);

	printf("a: %08x\nx: %08x\ny: %08x\n", delme_test_a, x, y);

	return 0;
}
#endif		// MULTI_DEBUG

void multi_show(void)
{
	// Print summary of active threads
}

// =========================================================================
// Create a thread
// =========================================================================

// Create a new thread
int thread_create(unsigned stacksize, unsigned priority, 
		void (*execute)(), unsigned thread_arg)
{
	ThreadInfo *p;
	void *stack;
	ThreadCreate *context;
	unsigned stackbytes;

	// Enforce minimum stack size (64KB)
	if (stacksize < 64)
		stacksize = 64;

	// Calculate size of stack in bytes
	stackbytes = stacksize << 10;

	// Allocate stack
	stack = malloc(stackbytes);
	if (!stack)
		return -1;	// fail: out of memory

	// Protect list from corruption by thread switch
	multi_critical_enter();

#ifdef DEBUG
	if (get_eflags() & 0x200)
		panic();
#endif

	// Grab a free thread
	p = free_threads.next;
	if (p == (void*)&free_threads) {
		free(stack);
		multi_critical_exit();
		return -2;	// fail: too many threads
	}

	// Remove from the free list
	p->prev->next = p->next;
	p->next->prev = p->prev;

	// Add to the active list
	p->prev = thread_table[0].prev;
	p->next = thread_table;
	p->prev->next = p;
	p->next->prev = p;

	// Stack bounds
	p->stack_lo = stack;
	p->stack_hi = stack + stackbytes;

	// Calculate initial stack pointer
	context = stack + stackbytes - sizeof(ThreadCreate);

	// Prepare stack
	memset(stack, 0, stackbytes);
	context->context.r_eflags = EFL_DEFAULT;
	context->context.r_eip = (unsigned)execute;
	context->context.r_cs = 0x0008;
	context->context.r_ds = 0x0010;
	context->context.r_es = 0x0010;
	context->context.r_fs = 0x0010;
	context->context.r_gs = 0x0010;

	context->context.r_ebp = 0xFFFFFFFF;	// Terminates stack trace

	context->context.r_cr3 = (unsigned)page_dir;

	// Make it return to thread_exit
	context->tc_ret = (unsigned)thread_exit;

	// Pass specified parameter to thread
	context->tc_arg = thread_arg;

	// Point the initial esp to the beginning of the context
	p->r_esp = (unsigned)context;
	p->state = THST_ACTIVE;

	// Done manipulating thread chains
	multi_critical_exit();

	// Return index into thread_table
	return p - thread_table;
}

// Exit the current thread and put it on the free list
void thread_exit(void)
{
	// The next time this thread is going to be scheduled, remove it
	// This is done because I cannot free the current stack
	thread_table[cur_thread_id].state = THST_EXITING;

	multi_yield();
}

// =========================================================================
// Synchronization primitives
// =========================================================================

// TODO: Spin lock

// TODO: Sleep lock

// TODO: Mutex

