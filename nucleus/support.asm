[BITS 32]

; v0.1: Doug Gale
;	- Added preliminary code for extensive stack checks. (currently disabled)
;	  Needs thread local storage to complete.
;	- Cleaned up and fixed some bugs in port I/O routines
;	  NOTE: These will be used rarely, now implemented as inline assembly
;	  in C source.
;	- Updated interrupts_(enable/disable) to return prior state of interrupt flag
;	- Added interrupts_query to ask for current state of interrupt flag without 
;	  changing it
;	- Added some DJGPP compatibility functions to simplify porting
; v0.2: Doug Gale
;	- halt() now takes a parameter. Pass true to print traceback before halting

[GLOBAL _inb]
[GLOBAL _outb]
[GLOBAL _inw]
[GLOBAL _outw]
[GLOBAL _ind]
[GLOBAL _outd]
[GLOBAL _interrupts_enable]
[GLOBAL _interrupts_disable]
[GLOBAL _interrupts_query]
[GLOBAL _halt]
[GLOBAL _load_idt]
[GLOBAL _get_eflags]

[GLOBAL __dosmemputb]
[GLOBAL __dosmemgetb]
[GLOBAL __dosmemputw]
[GLOBAL __dosmemgetw]
[GLOBAL __dosmemputl]
[GLOBAL __dosmemgetl]
[GLOBAL __inportb]
[GLOBAL __inportw]
[GLOBAL __inportl]
[GLOBAL __outportb]
[GLOBAL __outportw]
[GLOBAL __outportl]
[GLOBAL __enable]
[GLOBAL __disable]
[GLOBAL _enable]
[GLOBAL _disable]
[GLOBAL _multi_yield]

[GLOBAL ___cyg_profile_func_enter]
[GLOBAL ___cyg_profile_func_exit]

[EXTERN _dopanic]
[EXTERN _exception_print_traceback]
[EXTERN _interrupts_ok]

[SECTION .text]

; Reboot. Is this used?
[GLOBAL _bios_reboot]
_bios_reboot:
	; No interrupts
	cli
	; Turn off protected mode
	mov eax,cr0
	and eax,~1
	mov cr0,eax
	; Clean up segment registers
	xor eax,eax
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax
	mov ss,ax
	; Jump into BIOS reboot entry point
	jmp 0xFFFF:0x0000

; Stack checking code. (Same function used on func entry AND exit)
; Stack pointer must be between 0x00090000 and 0x00050000
; This is only used when you compile with -finstrument-functions
; Raises panic if stack pointer is out-of-bounds
; Completely transparent. Even preserves flags.
___cyg_profile_func_enter:
	nop              ; This is so disassembly shows proper different symbol for entry/exit
___cyg_profile_func_exit:
jmp .not_yet
	pushfd
	cmp esp,0x00050000
	jbe _stack_overflowed
	cmp esp,0x00090000
	ja _stack_overflowed
	popfd
.not_yet
	ret

_stackoverflowmsg db 'Stack overflow!',0

_stack_overflowed:
	; Due to stack overflow, I can't push parameters for dopanic,
	; so flush/reset stack, dopanic freezes execution anyways
	mov esp,0x00090000
	push dword 0
	push _stackoverflowmsg
	call _dopanic

__inportb:	
_inb:
	push ebp
	mov ebp, esp
	
	mov edx, [ebp + 8] ; port = first parameter
	in al, dx
	movzx eax,al

	leave
	ret

__outportb:	
_outb:
	push ebp
	mov ebp, esp

	mov dx, [ebp + 8]  ; port = first parameter
	mov al, [ebp + 12] ; value = second parameter

	out dx, al

	leave
	ret

__inportw:
_inw:
	push ebp
	mov ebp, esp
	
	push edx

	mov edx, [ebp + 8] ; port = first parameter
	in ax, dx
	movzx eax,ax

	pop edx
	
	leave
	ret

__outportw:	
_outw:
	push ebp
	mov ebp, esp

	mov dx, [ebp + 8]  ; port = first parameter
	mov ax, [ebp + 12] ; value = second parameter

	out dx, ax
	
	leave
	ret

__inportl:
_ind:
	push ebp
	mov ebp, esp
	
	mov dx, [ebp + 8] ; port = first parameter
	in eax, dx

	leave
	ret

__outportl:	
_outd:
	push ebp
	mov ebp, esp
	sub esp, 0

	mov dx, [ebp + 8]  ; port = first parameter
	mov eax, [ebp + 12] ; value = second parameter

	out dx, eax
	
	leave
	ret

; Returns true if interrupts were enabled
__enable:
_enable:
_interrupts_enable:
	; DEBUG!
	cmp dword [_interrupts_ok],1
	jz .ok

	push 0
	push .panicmsg
	call _dopanic

.ok
	call _interrupts_query
	sti
	ret

.panicmsg db "Attempt to enable interrupts prematurely!",0

; Returns true if interrupts were enabled
__disable:
_disable:
_interrupts_disable:
	call _interrupts_query
	cli
	ret

; Returns true if interrupts are enabled
_interrupts_query:
	pushfd				; Get eflags
	pop eax
	test eax,0x200		; Are interrupts enabled?
	setnz al
	movzx eax,al
	ret

_halt:
	cmp dword [esp+4],0
	je .no_traceback
	push ebp
	call _exception_print_traceback
	add esp,4
.no_traceback
	cli
	hlt
	jmp .no_traceback

; Portability wrappers (put and get are equivalent)

__dosmemputb: nop
__dosmemgetb:
	push esi
	push edi
	mov esi,[esp+12]
	mov ecx,[esp+16]
	mov edi,[esp+20]
	cld
	rep movsb
	pop edi
	pop esi
	ret

__dosmemputw: nop
__dosmemgetw:
	push esi
	push edi
	mov esi,[esp+12]
	mov ecx,[esp+16]
	mov edi,[esp+20]
	cld
	rep movsw
	pop edi
	pop esi
	ret

__dosmemputl: nop
__dosmemgetl:
	push esi
	push edi
	mov esi,[esp+12]
	mov ecx,[esp+16]
	mov edi,[esp+20]
	cld
	rep movsd
	pop edi
	pop esi
	ret

_load_idt:
	push ebp
	mov ebp, esp
	sub esp, 12
	
	push eax

	mov eax,[ebp+8]
	mov [esp+2],eax
	mov ax,[ebp+12]
	mov [esp],ax
	lidt [esp]

	pop eax

	leave
	ret

_get_eflags:
	pushfd
	pop eax
	ret

