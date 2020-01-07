; irqa.asm

; IRQ handler code

; v0.1: Doug Gale
;	- Initial revision
;	- Moved all IRQ handlers to here
;	- Part of rewrite of threading code
;	- Added support for floating point operations in interrupt handlers
;	- Cleaned up naming of interrupt handlers

[BITS 32]

[SECTION .text]

[GLOBAL _isr_dummy_0_7]
[GLOBAL _isr_dummy_8_15]
[GLOBAL _isr_keyboard]
[GLOBAL _isr_floppy]
[GLOBAL _isr_ide_14]
[GLOBAL _isr_ide_15]
[GLOBAL _isr_timer]
[GLOBAL _isr_null]

[GLOBAL _multi_yield]

[EXTERN _keyboard_handler]
[EXTERN _floppy_process]
[EXTERN _ide_process14]
[EXTERN _ide_process15]
[EXTERN _timer_tick]
[EXTERN _multi_scheduler]
[EXTERN _irq_acknowledge]

; ==========================================================================
; Handlers for unused INTs/IRQs. Should never happen, those IRQs are masked
; ==========================================================================

_isr_null:
	iretd

_isr_dummy_0_7:
	push eax

	mov al,0x20
	out 0x20,al

	pop eax
	iretd

_isr_dummy_8_15:
	push eax

	mov al,0x20
	out 0xa0,al
	mov al,0x20
	out 0x20,al

	pop eax
	iretd

; ==========================================================================
; Keyboard IRQ
; ==========================================================================

_isr_keyboard:
	pusha
	sub esp,108
	fnsave [esp]

	call _keyboard_handler
	
	push dword 1
	call _irq_acknowledge
	pop eax

	frstor [esp]
	add esp,108
	popa
	iretd

_isr_floppy:
	pusha
	sub esp,108
	fnsave [esp]

	call _floppy_process
	
	push dword 6
	call _irq_acknowledge
	pop eax

	frstor [esp]
	add esp,108
	popa
	iretd

_isr_ide_14:
	pusha
	sub esp,108
	fnsave [esp]

	call _ide_process14
	
	push dword 14
	call _irq_acknowledge
	pop eax

	frstor [esp]
	add esp,108
	popa
	iretd

_isr_ide_15:
	pusha
	sub esp,108
	fnsave [esp]

	call _ide_process15
	
	push dword 15
	call _irq_acknowledge
	pop eax

	frstor [esp]
	add esp,108
	popa
	iretd

; Periodic IRQ handler that calls the scheduler
; This may "return" to a different thread
_isr_timer:
	;
	; Save entire cpu context on the stack
	;

	; FPU/MMX context
	sub esp,108
	fnsave [esp]

	push gs
	push fs
	push es
	push ds

	push ebp
	push edi
	push esi
	push edx
	push ecx
	push ebx
	push eax

	mov eax,cr3
	push eax

	call _timer_tick

	; Pass kernel stack pointer to scheduler
	; This function returns the new value for esp
	push esp
	call _multi_scheduler

	; Use (possibly) new stack
	mov esp,eax

	; Acknowledge IRQ
	push dword 0
	call _irq_acknowledge
	pop eax

	; Page directory base register
	; Avoid unnecessary TLB flush
	pop eax
	mov ecx,cr3
	cmp eax,ecx
	je .skip_cr3
	mov cr3,eax
.skip_cr3

	; General registers
	pop eax
	pop ebx
	pop ecx
	pop edx
	pop esi
	pop edi
	pop ebp

	; Segment registers
	pop ds
	pop es
	pop fs
	pop gs

	; FPU/MMX context
	frstor [esp]
	add esp,108

	; Restore flags, load CS:EIP, and load user stack if applicable
	iretd

; Does the same thing as _isr_timer, but does not do
; periodic processing and does not acknowledge an IRQ
; Used by multi_yield()
_soft_threadswitch:
	;
	; Save entire cpu context on the stack
	;

	; FPU/MMX context
	sub esp,108
	fnsave [esp]

	; Segment registers
	push gs
	push fs
	push es
	push ds

	push ebp
	push edi
	push esi
	push edx
	push ecx
	push ebx
	push eax

	mov eax,cr3
	push eax

	; Pass kernel stack pointer to scheduler
	; This function returns the new value for esp
	push esp
	call _multi_scheduler

	; Use (possibly) new stack
	mov esp,eax

	;
	; Restore entire cpu context from the stack
	;

	; Page directory base register
	; Avoid unnecessary TLB flush
	pop eax
	mov ecx,cr3
	cmp eax,ecx
	je .skip_cr3
	mov cr3,eax
.skip_cr3

	; General registers
	pop eax
	pop ebx
	pop ecx
	pop edx
	pop esi
	pop edi
	pop ebp

	; Segment registers
	pop ds
	pop es
	pop fs
	pop gs

	; FPU/MMX context
	frstor [esp]
	add esp,108

	; Restore flags, load CS:EIP, and load user stack if applicable
	iretd

; To force a task switch, call this function
_multi_yield:
	pushfd
	cli
	push cs
	call _soft_threadswitch
	ret

