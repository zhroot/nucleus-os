[BITS 32]

; v0.1: ???
; v0.2: Doug Gale
;	- Uses linker to resolve symbols in the kernel
;	- Now a part of the link process. No concatenation.
;	- Loader reads entry point from entry_addr
; v0.3: Doug Gale
;	- Initializes stack
;	- Clears uninitialized data area (.bss section)
;	- Creates stack frame to assist stack traces
;	- Changed magic

; Here goes our mulitboot header

[SECTION .head]

[EXTERN ___bss_en]
[EXTERN ___bss_st]
[EXTERN ___load_end_addr]
[EXTERN _main]

[GLOBAL ___header_end]

magic         dd 0x1BADB002
flags         dd 0x00010000
checksum      dd 0xE4514FFE		; ???
header_addr   dd 0x00100000
load_addr     dd 0x00100000
load_end_addr dd ___load_end_addr
bss_end_addr  dd ___bss_en
entry_addr    dd __entry

; reserved for future expansion and/or header (maybe ELF)

times 384-($-$$) db 0

[GLOBAL __entry]
__entry:
	; Setup stack
	mov esp,0x00090000

	; Clear bss
	mov ecx,___bss_en
	mov edi,___bss_st
	sub ecx,edi
	shr ecx,4
	xor eax,eax
	cld
	rep
	stosd

	; Create terminator stack frame
	push dword 0xFFFFFFFF
	mov ebp,esp
	call _main
.forever
	hlt
	jmp .forever

times 507-($-$$) db 0

; The language setting for Nucleus
language	db 0x01	; 0 = German, 1 = English, 2 = Latin, 3 = Portugese

; The Debuglevel Nucleus will be using (--> nucleus.h)
debug_level    db  0x02

; Variables used for the Textfunctions
cursor_page    db  0x00
cursor_row     db  0x10
cursor_column  db  0x00

___header_end:
