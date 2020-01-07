; XERXYS BOOTLOADER 0.01
; (C)Copyright 2003 by Christian Lins

; This program is free software; you can redistribute it and/or modify it under the terms of the GNU 
; General Public License as published by the Free Software Foundation; either version 2 of the License, ; or (at your option) any later version. 
; This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without 
; even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General ; Public License for more details. 
; You should have received a copy of the GNU General Public License along with this program; if not, 
; write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA. 

; Letzte Änderung: 24.04.2003
; Enthält den Bootsektor, der den Kernelstarter (nucload) aufruft.

start:

	org 0x7c00 		; Legt die Position im Segment fest. BIOS läd von 0x7c00

	mov si, msg_osname
	call displaystring

	mov si, msg_copyright
	call displaystring

	mov si, msg_ver
	call displaystring

	mov si, msg_nucload		
	call displaystring

; Bootlaufwerk bestimmen
	
	cli			; Interrupts abschalten
	mov ax, 0x9000		; Adresse des Stack bestimmen
	mov ss, ax		; ss=9000, Stackadresse festlegen
	xor sp, sp		; Stackpointer auf Null setzen
	sti			; Interrupts wieder an

	mov [bootdrive], dl	; Laufwerk (Info in DL) in bootdrive speichern 
	

	mov si, msg_dot		
	call displaystring

; nucload von Floppy lesen und in Speicher kopieren

floppy_reset:
	push ds			
	xor ax, ax		; ax=0	
	mov dl, [bootdrive]
	int 13h			; Interrupt 13h wird aufgerufen
	pop ds			
	jc short floppy_reset	; Jump if Carry-Flag gesetzt

	;push es
floppy_load:
	mov ax, 0x1000		; es:ax= Segment 10000 (1000h)
	mov es, ax 
	xor bx, bx		; bx=0
	mov ah, 2		; Funktion Sektor laden
	mov al, 5		; Anzahl der Sektoren
	mov cx, 2		; Spur 0, Sektor 2 (ch/cl)
	xor dx, dx		; dx=0, Seite=0 Laufwerk=0
	int 13h
	jc short floppy_load

	mov si, msg_dot		
	call displaystring

; Sprung zu nucload

	mov si, msg_dot		
	call displaystring

	mov ax, 0x1000		; nucload ist im Segment 1000h
	mov es, ax
	mov ds, ax
	push ax
	xor ax, ax		; ax=0
	push ax
	retf


; Funktionen
displaystring:			
	lodsb
	or al,al
	jz short finish
	mov ah,0x0E		; Zeigt ein Char an
	mov bx,0x0007
	int 0x10
	jmp displaystring
finish:
	retn

; Strings
	msg_osname db 13, 10, 13, 10, 13, 10, 'XERXYS 3.5.05', 13, 10, 0
	msg_copyright db '(C)2003 by Christian Lins', 13, 10, 13, 10, 0
	msg_nucload db 'Lade //boot/nucload', 0
	msg_dot db '.', 0
	msg_ver db 'Bootloader Version 0.01 (24.04.2003)', 13, 10, 0
	bootdrive db 0

times 512-($-$$)-2 db 0		 
dw 0AA55h		