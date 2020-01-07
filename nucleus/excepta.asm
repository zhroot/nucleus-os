; excepta.asm

; Exception stubs

; v0.1: Doug Gale
;	- Initial revision

[BITS 32]

[SECTION .text]

[EXTERN _exception_handler]

[GLOBAL _halt]

; See page 145 of IA32 System Programming Reference

; Passes a pointer to the following structure to the C handler
; The C handler can modify any of the values, but changes to
; the stack pointer registers will be ignored
; SS:ESP are the values before the exception occurred
; typedef struct {
;	dword r_eax;
;	dword r_ebx;
;	dword r_ecx;
;	dword r_edx;
;	dword r_esi;
;	dword r_edi;
;	dword r_ebp;
;	dword r_esp;		// NOT restored upon return
;	dword r_ss;			// NOT restored upon return
;	dword r_ds;
;	dword r_es;
;	dword r_fs;
;	dword r_gs;
;	dword r_cr2;		// Page fault linear address, not restored
;	dword r_dr6;		// Debug register, not restored
;	dword r_type;		// The exception code, 
;	dword r_error_code;	// Exception error code, if present, zero otherwise
;	dword r_eip;
;	dword r_cs;
;	dword r_eflags;
; } Exception_Context;

; If there was a priviledge transition, the SS:ESP in 
; ExceptionContext points to this structure:
; You can tell by the RPL of CS
; typedef struct {
;	dword r_user_eflags;
;	dword r_user_esp;
;	dword r_user_ss;
; } Exception_ContextTransition;

%define OFS_USER	80
%define OFS_EFLAGS	76
%define OFS_CS		72
%define OFS_EIP		68
%define OFS_ERROR	64
%define OFS_TYPE	60
%define OFS_DR6		56
%define OFS_CR2		52
%define OFS_GS		48
%define OFS_FS		44
%define OFS_ES		40
%define OFS_DS		36
%define OFS_SS		32
%define OFS_ESP		28
%define OFS_EBP		24
%define OFS_EDI		20
%define OFS_ESI		16
%define OFS_EDX		12
%define OFS_ECX		8
%define OFS_EBX		4
%define OFS_EAX		0

%macro ExceptionStub_NoErrorCode 1
[global _exception_stub_%1]
_exception_stub_%1:
	push dword 0		; 64: error code placeholder
	push dword %1		; 60: type of exception
	jmp _exception_stub_common
%endmacro

%macro ExceptionStub_ErrorCode 1
[global _exception_stub_%1]
_exception_stub_%1:
	push dword %1		; 60: type of exception
	jmp _exception_stub_common
%endmacro

; Exception stubs (2 or 3 instructions each)
ExceptionStub_NoErrorCode	0x00	; Divide error
ExceptionStub_NoErrorCode	0x01	; Debug
ExceptionStub_NoErrorCode	0x02	; NMI
ExceptionStub_NoErrorCode	0x03	; Breakpoint
ExceptionStub_NoErrorCode	0x04	; Overflow
ExceptionStub_NoErrorCode	0x05	; Bound range exceeded
ExceptionStub_NoErrorCode	0x06	; Invalid opcode
ExceptionStub_NoErrorCode	0x07	; Coprocessor not available
ExceptionStub_ErrorCode		0x08	; Double fault
ExceptionStub_NoErrorCode	0x09	; Coprocessor segment overrun
ExceptionStub_ErrorCode		0x0a	; Invalid TSS
ExceptionStub_ErrorCode		0x0b	; Segment not present
ExceptionStub_ErrorCode		0x0c	; Stack fault
ExceptionStub_ErrorCode		0x0d	; General protection fault
ExceptionStub_ErrorCode		0x0e	; Page fault
ExceptionStub_NoErrorCode	0x0f	; (Intel Reserved)
ExceptionStub_NoErrorCode	0x10	; FPU error
ExceptionStub_NoErrorCode	0x11	; Alignment fault
ExceptionStub_NoErrorCode	0x12	; Machine check
ExceptionStub_NoErrorCode	0x13	; SSE/SSE2 fault
ExceptionStub_NoErrorCode	0x14	; (Intel Reserved)
ExceptionStub_NoErrorCode	0x15	; (Intel Reserved)
ExceptionStub_NoErrorCode	0x16	; (Intel Reserved)
ExceptionStub_NoErrorCode	0x17	; (Intel Reserved)
ExceptionStub_NoErrorCode	0x18	; (Intel Reserved)
ExceptionStub_NoErrorCode	0x19	; (Intel Reserved)
ExceptionStub_NoErrorCode	0x1a	; (Intel Reserved)
ExceptionStub_NoErrorCode	0x1b	; (Intel Reserved)
ExceptionStub_NoErrorCode	0x1c	; (Intel Reserved)
ExceptionStub_NoErrorCode	0x1d	; (Intel Reserved)
ExceptionStub_NoErrorCode	0x1e	; (Intel Reserved)
ExceptionStub_NoErrorCode	0x1f	; (Intel Reserved)

_exception_stub_common:
	push eax			; 56: dr6
	push eax			; 52: cr2
	push gs				; 48:  gs
	push fs				; 44:  fs
	push es				; 40:  es
	push ds				; 36:  ds
	push ss				; 32:  ss (before exception)
	push eax			; 28: esp (before exception)
	push ebp			; 24:
	push edi			; 20:
	push esi			; 16:
	push edx			; 12:
	push ecx			;  8:
	push ebx			;  4:
	push eax			;  0:

	; Scheduled ahead
	lea eax,[esp+OFS_USER]
	mov ebx,dr6
	mov ecx,cr2

	; Insert segment registers into stack space reserved above
	mov [esp+OFS_CR2],ecx
	mov [esp+OFS_DR6],ebx
	mov [esp+OFS_ESP],eax

	; Call C exception handler
	push esp
	call _exception_handler
	pop eax
	
	pop eax
	pop ebx
	pop ecx
	pop edx
	pop esi
	pop edi
	pop ebp

	add esp,8

	pop ds
	pop es
	pop fs
	pop gs

	add esp,OFS_EIP-OFS_CR2

	iretd

