; Written by Moritz Bartl for "Nucleus"
; License : GPL
; Version : 0.50

%if 0		; Replaced by Doug Gale. Major threading rewrite.

%define KERNEL_DS       0x10

; TASK STRUKTUR variabel !
%define REGS_SIZE       512
%define REGS_EAX        0
%define REGS_EBX        4
%define REGS_ECX        8
%define REGS_EDX        12
%define REGS_ESI        16
%define REGS_EDI        20
%define REGS_ESP        24
%define REGS_EBP        28
%define REGS_EIP        32
%define REGS_EFLAGS     36
%define REGS_CS         40
%define REGS_DS         42
%define REGS_ES         44
%define REGS_SS         46
%define REGS_FS         48
%define REGS_GS         50
%define REGS_FPU        52 

SECTION .bss

GLOBAL _stss_savedtss
_stss_savedtss:  resb REGS_SIZE
stack_switch:   resb 8

SECTION .bss
        resb    1024
intstack:

SECTION .data
istack:
        dd      intstack
        dw      KERNEL_DS

SECTION .text

GLOBAL _getflags
_getflags:
        pushfd
        pop     eax
        ret

EXTERN _timer_func

GLOBAL _timer_int
_timer_int:
        push    ds
        push    eax
        mov     ax,     KERNEL_DS
        mov     ds,     ax

        mov     [_stss_savedtss+REGS_EBX], ebx
        pop     ebx
        mov     [_stss_savedtss+REGS_EAX], ebx
        pop     ebx
        mov     [_stss_savedtss+REGS_DS], bx
        mov     [_stss_savedtss+REGS_ECX], ecx
        mov     [_stss_savedtss+REGS_EDX], edx
        mov     [_stss_savedtss+REGS_ESI], esi
        mov     [_stss_savedtss+REGS_EDI], edi
        mov     [_stss_savedtss+REGS_EBP], ebp
        mov     [_stss_savedtss+REGS_ES], es
        mov     [_stss_savedtss+REGS_FS], fs
        mov     [_stss_savedtss+REGS_GS], gs

        pop     ebx
        mov     [_stss_savedtss+REGS_EIP], ebx
        pop     ebx
        mov     [_stss_savedtss+REGS_CS], bx
        pop     ebx
        mov     [_stss_savedtss+REGS_EFLAGS], ebx

;new
        mov     bx,     [_stss_savedtss+REGS_CS]
        lar     ax,     bx
        shr     ax,     13
        and     ax,     011b
        cmp     ax,     0
        jne     .privchange
        mov     [_stss_savedtss+REGS_ESP], esp
        mov     [_stss_savedtss+REGS_SS], ss
        lss     esp,    [istack]
        jmp     .stacksaved
.privchange:
        pop     ebx
        mov     [_stss_savedtss+REGS_ESP], ebx
        pop     ebx
        mov     [_stss_savedtss+REGS_SS], ebx
.stacksaved:

        fsave   [_stss_savedtss+REGS_FPU]

        call    _timer_func

        frstor  [_stss_savedtss+REGS_FPU]

        mov     bx,     [_stss_savedtss+REGS_CS]
        lar     ax,     bx
        shr     ax,     13
        and     ax,     011b
        cmp     ax,     0
        jne     .privchange2
        mov     eax,    [_stss_savedtss+REGS_ESP]
        mov     [stack_switch], eax
        mov     ax,     [_stss_savedtss+REGS_SS]
        mov     [stack_switch+4], ax
        lss     esp,    [stack_switch]
        jmp     .stackrestored
.privchange2:
        mov     eax,    [_stss_savedtss+REGS_SS]
        push    eax
        mov     eax,    [_stss_savedtss+REGS_ESP]
        push    eax
.stackrestored:

        mov     ebx,    [_stss_savedtss+REGS_EFLAGS]
        push    ebx
        mov     bx,     [_stss_savedtss+REGS_CS]
        push    ebx
        mov     ebx,    [_stss_savedtss+REGS_EIP]
        push    ebx

        mov     bx,     [_stss_savedtss+REGS_DS]
        push    ebx

        mov     eax,    [_stss_savedtss+REGS_EAX]
        mov     ebx,    [_stss_savedtss+REGS_EBX]
        mov     ecx,    [_stss_savedtss+REGS_ECX]
        mov     edx,    [_stss_savedtss+REGS_EDX]
        mov     esi,    [_stss_savedtss+REGS_ESI]
        mov     edi,    [_stss_savedtss+REGS_EDI]
        mov     ebp,    [_stss_savedtss+REGS_EBP]
        mov     es,     [_stss_savedtss+REGS_ES]
        mov     fs,     [_stss_savedtss+REGS_FS]
        mov     gs,     [_stss_savedtss+REGS_GS]

        pop     ds

        push    eax
        mov     al,     0x20
        out     0x20,    al
        pop     eax

        iretd

%endif
