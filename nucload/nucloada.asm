[BITS 16]

[GLOBAL _ReadCHS]
[GLOBAL _CopyToExt]
[GLOBAL _EnterKernel]
[GLOBAL _enable_A20]
[GLOBAL _disable_A20]
[GLOBAL _detect_A20_type]

[EXTERN _PrintString]

; - Compile switches begin -------------
%define USE_REAL_DELAY		1
%define USE_MSG_DELAY		0
; - Compile switches end ---------------

[SECTION .data]

; Function pointers initialized by A20 detection code
times ($$-$) & 3 nop
_ena_A20_vec	dd 0
_dis_A20_vec	dd 0

; This is the *real* GDT that gets loaded before jumping to kernel
[GLOBAL gdtr]
times ($$-$) & 3 nop
	dw 0
gdtr:
	dw gdt_end-gdt-1	; Limit
	dd gdt				; Linear address

times ($$-$) & 7 nop
gdt:
	dd 0
	dd 0

code4g_gdt:		; Code descriptor: 4 GB
	dw 0xFFFF	; Limit = 4 GB
	dw 0x0000	; Base  0-15
	db 0x00 	; Base 16-23
	db 0x9A 	; Code
	db 0xCF 	; Page granularity
	db 0x00 	; Base 24-31

data4g_gdt:		; Data descriptor: 4 GB
	dw 0xFFFF	; Limit = 4 GB
	dw 0x0000	; base Bits 0-15
	db 0x00 	; base Bits 16-23
	db 0x93 	; Data segment
	db 0xCF 	; Page granularity
	db 0x00 	; base Bits 24-31

code64k_gdt:
	dw 0xFFFF	; Limit = 64 KB
	dw 0x0000	; Base  0-15
	db 0x00 	; Base 16-23
	db 0x9A 	; Code
	db 0x00 	; Byte granularity
	db 0x00 	; Base 24-31

data64k_gdt:
	dw 0xFFFF	; Limit = 64 KB
	dw 0x0000	; base Bits 0-15
	db 0x00 	; base Bits 16-23
	db 0x93 	; Data segment
	db 0x00 	; Byte granularity
	db 0x00 	; base Bits 24-31

gdt_end:

[SECTION .text]

; ------------------
%macro safemsg 1+
%if 1
	jmp %%overtext
%%msg: db 10,%1,10,0
%%overtext:
	pushad
o32	push es
	mov ax,0xb800
	mov es,ax
	mov esi,%%msg
	xor di,di
%%another:
	lodsb
	cmp al,0
	jz %%done
	stosb
	mov al,7
	stosb
	jmp %%another
%%done:

	mov ax,0x0800
%%another2:
	stosw
	cmp di,80*2
	jb %%another2

%if USE_MSG_DELAY
	mov ecx,200000000
%else
	mov ecx,1
%endif

%%spin:
	dec ecx
	jnz %%spin

o32	pop es
	popad
%endif
%endmacro

; ------------------
%macro safemsg_pm 1+
%if 1
jmp %%overtext
%%msg db 10,%1,10,0
%%overtext:
	pushad
	mov esi,%%msg
	mov edi,0xB8000
%%another:
	lodsb
	cmp al,0
	jz %%done
	stosb
	mov al,7
	stosb
	jmp %%another
%%done:

	mov ax,0x0800
%%another2:
	stosw
	cmp edi,0xB8000 + 80*2
	jb %%another2

%if USE_MSG_DELAY
	mov ecx,200000000
%else
	mov ecx,1
%endif

%%spin:
	dec ecx
	jnz %%spin

	popad
%endif
%endmacro

; ------------------
; // Returns true on success
; STATIC word ReadCHS(void *dest, dword drv, dword c, dword h, dword s)
_ReadCHS:
	push ebp
	mov ebp,esp
	sub esp,4
	push ebx
	push esi
	push edi

	; 5 retries
	mov word [bp-4],5

.retry:
	mov bx,[bp+8]		; mem
	mov dl,[bp+12]		; drive
	mov ch,[bp+16]		; cylinder
	mov dh,[bp+20]		; head
	mov cl,[bp+24]		; sector
	mov ax,0x0201		; read sector
	int 0x13			; Disk BIOS
	jnc .ok
	dec word [bp-4]		; Retry counter
	jnz .retry
	stc
.ok:
	setnc al
	movzx eax,al

	pop edi
	pop esi
	pop ebx
o32	leave
o32	ret

; ------------------
_CopyToExt:
	push ebp
	mov ebp,esp

	push ebx
	push esi
	push edi
	push ebp

	call dword _enable_A20

	mov eax,gdtr
	lgdt [eax]

	mov eax,cr0
	; Enable native FPU exception handling and enable protected mode
	or eax,0x00000021
	mov cr0,eax

	jmp .clear_pfq
	nop
.clear_pfq:

	mov esi,[bp+8]
	mov edi,[bp+12]
	mov ecx,[bp+16]

	jmp dword 0x0008:.pmode
;	push dword 8
;	push dword .pmode
;o32	retf

	nop
.pmode:
[BITS 32]
	; Protected mode

	mov eax,0x10
	mov ds,ax
	mov es,ax
	mov ss,ax

	safemsg_pm "In protected mode"

;	mov esi,[ebp+8]
;	mov edi,[ebp+12]
;	mov ecx,[ebp+16]
	cld
	rep movsb

	safemsg_pm "Done copy"

	; Load CS register for real mode
	jmp dword 0x0018:.loadcsrm
.loadcsrm:

	; Load segment registers for real mode
	mov eax,0x0020
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax
	mov ss,ax

	mov eax,cr0
	dec eax
	mov cr0,eax

	jmp .clear_pfq2
	nop
.clear_pfq2:

	jmp dword 0x0000:.rmode

	nop
.rmode:
[BITS 16]
	; Real mode

	xor ax,ax
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax
	mov ss,ax

	safemsg "Back to real mode"

	call dword _disable_A20

	pop ebp
	pop edi
	pop esi
	pop ebx
o32	leave
o32	ret

; ------------------
; Delay approximately 20ms
_shortdelay:
	push eax
	push ecx
%if USE_REAL_DELAY
	mov ecx,20000
%else
	mov ecx,1
%endif
	mov al,0
.again:
	out 0x80,al
	loop .again
	pop ecx
	pop eax
o32	ret

; ------------------
_EnterKernel:
;	safemsg "Cleaning flags"

	; Clean eflags
	push dword 0x00000002
	popfd

;	safemsg "Enabling A20"

	call dword _enable_A20

;	safemsg "Loading GDT"

	lgdt [dword gdtr]

;	safemsg "Disabling IRQs"

	; Disable every IRQ
	mov al,0xff
	out 0x21,al
	out 0xa1,al

;	safemsg "Discarding pending IRQs"

	; Throw away pending interrupts
	mov al,0x20
	out 0x20,al
	out 0xa0,al

;	safemsg "Enabling protected mode"

	mov eax,cr0
	; Enable native FPU exception handling and enable protected mode
	or eax,0x00000021
	mov cr0,eax

	jmp .clear_pfq
.clear_pfq:

	jmp dword 0x0008:.pmode
	nop
.pmode:
[BITS 32]

	; Load segment registers
	mov eax,0x10
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax

	; Calculate temporary protected mode stack from real mode stack
	movzx eax,sp
	push dword 0x0010
	push eax
	lss esp,[ds:esp]

;	safemsg_pm "Initializing FPU"

	; Reset FPU
;	clts
;	fninit

	or eax,-1
	mov ebx,eax
	mov ecx,eax
	mov edx,eax
	mov esi,eax
	mov edi,eax
	mov ebp,eax

;	safemsg_pm "Cleaning flags"

	; Clean eflags
	push dword 0x00000002
	popfd

;	safemsg_pm "Jumping to kernel"

	; The entry point is stored in the header
	jmp [0x0010001c]

[BITS 16]

; ============================================================================
; A20 Control
; ============================================================================

; ------------------
; Wrapper jumps off vector
_enable_A20:
	cli
	cmp dword [dword _ena_A20_vec],0
	jnz .detect_done
	call dword _detect_A20_type
	jc .failed

.detect_done:
	call dword [dword _ena_A20_vec]

	call dword _shortdelay

.failed:
o32	ret

; ------------------
; Wrapper jumps off vector
_disable_A20:
	cmp dword [dword _dis_A20_vec],0
	jnz .detect_done
	call dword _detect_A20_type
	jc .failed

.detect_done:
	call dword [dword _dis_A20_vec]

	call dword _shortdelay

.failed:
	sti
o32	ret

; Null function
_a20_func_null:
o32	ret

; ------------------
; Helpers to synchronize with keyboard controller
a20wait_tosend:
	push ax
.L0:
	in al,0x64
	test al,2
	jnz .L0
	pop ax
o32	ret

a20wait_torecv:
	push ax
.L0:
	in al,0x64
	test al,1
	jz .L0
	pop ax
o32	ret

; ----------------
; Use BIOS
_a20_func_ena_bios:
	pushad
	mov ax,0x2401
	int 0x15
	popad
o32	ret

_a20_func_dis_bios:
	pushad
	mov ax,0x2400
	int 0x15
	popad
o32	ret

; ----------------
; Use keyboard controller
_a20_func_ena_kbc:
	push eax
	push edx
	call dword a20wait_tosend
	mov al,0xD1
	out 0x64,al
	call dword a20wait_tosend
	mov al,0xDF
	out 0x60,al
	call dword a20wait_tosend
	pop edx
	pop eax
o32	ret

_a20_func_dis_kbc:
	push ax
	push dx
	call dword a20wait_tosend
	mov al,0xD1
	out 0x64,al
	call dword a20wait_tosend
	mov al,0xDD
	out 0x60,al
	call dword a20wait_tosend
	pop dx
	pop ax
o32	ret

; ----------------
; Use port 0xEE
_a20_func_ena_port_ee:
	push ax
	in al,0xEE
	pop ax
o32	ret

_a20_func_dis_port_ee:
	push ax
	mov al,0
	out 0xEE,al
	pop ax
o32	ret

; ----------------
; Use port 0x92
_a20_func_ena_port_92:
	push ax
	in al,0x92
	and al,~1
	or al,2
	out 0x92,al
	pop ax
o32	ret

_a20_func_dis_port_92:
	push ax
	in al,0x92
	and al,~(1 | 2)
	out 0x92,al
	pop ax
o32	ret

; ----------------
; Use keyboard controller AND port 0x92
_a20_func_ena_port_92_kbc:
	call dword _a20_func_ena_kbc
	call dword _a20_func_ena_port_92
o32	ret

_a20_func_dis_port_92_kbc:
	call dword _a20_func_dis_kbc
	call dword _a20_func_dis_port_92
o32	ret

; ----------------
; Use keyboard controller special command
_a20_func_ena_kbc_cmd:
	push ax
	call dword a20wait_tosend
	mov al,0xDF
	out 0x64,al
	; Twice!
	call dword a20wait_tosend
	mov al,0xDF
	out 0x64,al
	call dword a20wait_tosend
	pop ax
o32	ret

_a20_func_dis_kbc_cmd:
	push ax
	call dword a20wait_tosend
	mov al,0xDD
	out 0x64,al
	; Twice!
	call dword a20wait_tosend
	mov al,0xDD
	out 0x64,al
	call dword a20wait_tosend
	pop ax
o32	ret

; ----------------
; Detect the type of A20 control this chipset uses
[GLOBAL _detect_A20_type]
_detect_A20_type:
	pushfd
	cli
	pushad

	safemsg "Probing for A20 control method"

	; Use gs register for wraparound segment
	mov ax,0xffff
	mov gs,ax

	;
	; See if A20 is already enabled
	;

	call dword .test_wraparound
	jz .null_A20

	; At this point, I know the A20 line is disabled

	;
	; The safest method is to use the BIOS, so try to use that first
	;

	call dword _a20_func_ena_bios
	jnc .bios_A20
.bios_A20_failed:

	;
	; Try to use the keyboard controller
	;

	call dword _a20_func_ena_kbc

	call dword .test_wraparound
	jz .kbc_A20

	; Turn it off, so I can see if I need to set BOTH kbc and port 0x92 later
	call dword _a20_func_dis_kbc

	;
	; Try to use port 0xEE
	;

	call dword _a20_func_ena_port_ee

	call dword .test_wraparound
	jz .kbc_port_EE

	; Turn it off
	call dword _a20_func_dis_port_ee

	;
	; Try to use port 0x92
	;

	call dword _a20_func_ena_port_92

	call dword .test_wraparound
	jz .kbc_port_92

	; Turn it off
	call dword _a20_func_dis_port_92

	;
	; Try to use BOTH port 0x92 and kbc
	;

	call dword _a20_func_ena_port_92_kbc

	call dword .test_wraparound
	jz .kbc_port_92_kbc

	; Turn it off
	call dword _a20_func_dis_port_92_kbc

	;
	; Try issuing keyboard controller command (HP Vectra)
	;

	call dword _a20_func_ena_kbc_cmd

	call dword .test_wraparound
	jz .kbc_cmd

	call dword _a20_func_ena_kbc_cmd

	;
	; Forget it. This motherboard is too incompatible.
	;

	stc
	jmp .done

.null_A20:
	safemsg "A20: Already on"
	; The A20 line was already enabled. Don't touch it ever.
	mov dword [dword _ena_A20_vec],_a20_func_null
	mov dword [dword _dis_A20_vec],_a20_func_null
	clc
	jmp .done

.bios_A20:
	safemsg "A20: Using BIOS"
	; Turn it off again, I was just testing
	call dword _a20_func_dis_bios
	jc .bios_A20_failed
	; Success
	mov dword [dword _ena_A20_vec],_a20_func_ena_bios
	mov dword [dword _dis_A20_vec],_a20_func_dis_bios
	clc
	jmp .done

.kbc_A20:
	safemsg "A20: Using KBC"
	; Turn it off again, I was just testing
	call dword _a20_func_dis_kbc
	; Success
	mov dword [dword _ena_A20_vec],_a20_func_ena_kbc
	mov dword [dword _dis_A20_vec],_a20_func_dis_kbc
	clc
	jmp .done

.kbc_port_EE:
	safemsg "A20: Using port EE"
	; Turn it off again, I was just testing
	call dword _a20_func_dis_port_ee
	; Success
	mov dword [dword _ena_A20_vec],_a20_func_ena_port_ee
	mov dword [dword _dis_A20_vec],_a20_func_dis_port_ee
	clc
	jmp .done

.kbc_port_92:
	safemsg "A20: Using port 92"
	; Turn it off again, I was just testing
	call dword _a20_func_dis_port_92
	; Success
	mov dword [dword _ena_A20_vec],_a20_func_ena_port_92
	mov dword [dword _dis_A20_vec],_a20_func_dis_port_92
	clc
	jmp .done

.kbc_port_92_kbc:
	safemsg "A20: Using port 92 and kbc"
	; Turn it off again, I was just testing
	call dword _a20_func_dis_port_92_kbc
	; Success
	mov dword [dword _ena_A20_vec],_a20_func_ena_port_92_kbc
	mov dword [dword _dis_A20_vec],_a20_func_dis_port_92_kbc
	clc
	jmp .done

.kbc_cmd:
	safemsg "A20: Using KBC command"
	; Turn it off again, I was just testing
	call dword _a20_func_dis_kbc_cmd
	; Success
	mov dword [dword _ena_A20_vec],_a20_func_ena_kbc_cmd
	mov dword [dword _dis_A20_vec],_a20_func_dis_kbc_cmd
	clc
	jmp .done

.done:
	popad
o32	push cs
o32	pop gs
	popfd
o32	ret

; The following code is shared above to test for wraparound
; Returns with ZF set if NO wraparound occurred (the A20 line is enabled)
.test_wraparound:
	call dword _shortdelay

	; Use INT 0x80 vector for scratch area to detect wraparound
	mov word [0x80*4],0x1234
	; Write to wraparound segment
	mov word [gs:0x80*4 + 16],0x5678
	; See if a wraparound occurred
	cmp word [0x80*4],0x1234
o32	ret

[SECTION .data]
[GLOBAL _footer_sig]
_footer_sig:	db 0xE0

stupid:
times 1024 db 0
