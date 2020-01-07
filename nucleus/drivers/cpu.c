/*
	NUCLEUSKL - OS Kernel based on XERXYS NUCLEUS 0.01 by Christian Lins
	(C)Copyright 2003 by Christian Lins <christian@netvader.de>
	
	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
	as published by the Free Software Foundation; 
	either version 2 of the License, or (at your option) any later version. 

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
	See the GNU General Public License for more details. 

	You should have received a copy of the GNU General Public License along with this program; 
	if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA. 
	
	Letzte Änderung: 16.05.2003
*/

//#include <drivers/cpu.h>

//struct cpu cpu1;

//char sys_cpu_idstr[] = "UnknownCPU/Intel 486 compatible";

//void cpu_init(void) // inline entfernt, eine Compilerdirektive übernimmt das später
//{
	//int funktionsnummer;
	
	//cpu1.name = sys_cpu_idstr;

	/*asm // Muss in AT&T Assembler geschrieben werden oder in einer extra Datei
			 mit NASM compiliert werden
	{
		// Verfügbarkeit des CPUID-Befehls überprüfen
		pushfd				// Flag-Register auf dem Stack ablegen
		pop eax				// nach eax kopieren
		mov ebx, eax			// kopie nach ebx
		xor eax, 00200000h		// bit 21 ändern
		push eax
		popfd
		pushfd
		pop eax
		xor eax, ebx
		jz ende

		// Maximale Funktionsnummer abfragen
		mov eax, 0
		cpuid
		mov funktionsnummer, eax

		ende:
	}
	if(funktionsnummer>=1)
	{
		asm
		{
			mov eax, 80000001h
			cpuid
		}
	}
	if(funktionsnummer>=2)
	{
		asm
		{
			mov eax, 80000002h
			cpuid
		}
	}*/
//}
