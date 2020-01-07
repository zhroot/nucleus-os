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

#ifndef CPU_H
#define CPU_H

//void cpu_init(void);

/*struct cpu
{
	char *name;
	int rev;
	short takt;
	int model;
	int family;
	int typ;
	int cache;
	
	int fpu : 1;
	int vme : 1; // Virtual 8086 Enhancements
	int de : 1; // Debugging Extensions
	int pse : 1; // Page Size Extensions
	int tsc : 1; // Verfügbarkeit des RDTSC-Befehls
	int msr : 1; //RDMSR und WRMSR-Befehle
	int pae : 1; // Unterstützung für Adressen, die größer als 32 bit sind
	int mce : 1; // Machine Check Exception
	int cx8 : 1; // CMPXCH8B-Befehl
	int apic : 1; 
	int sep : 1; // Prozessor unterstützt SYSENTER/SYSEXIT bzw. SYSCALL/SYSRET (AMD)
	int acpi : 1; 
	int mmx : 1;
	int sse : 1;
	int sse2 : 1;
	int _3dnex : 1; // Extended 3DNow! (Bit30)
	int _3dn : 1; // 3DNow! Technologie
};

extern struct cpu cpu1;*/

#endif // CPU_H
