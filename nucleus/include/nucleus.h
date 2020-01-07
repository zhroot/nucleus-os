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

#ifndef _NUCLEUS_H_
#define _NUCLEUS_H_

#include <datatypes.h>

#define NUC_MAJOR_VERSION	((nucleus_version & 0xF00) >> 8)
#define NUC_MINOR_VERSION	(nucleus_version & 0xFF)
#define NUC_REVISION		nucleus_revision

#define NUC_STATUS_OFF			0
#define NUC_STATUS_STARTING		1
#define NUC_STATUS_SHUTDOWN		2
#define NUC_STATUS_RUNNING		3

#define NUC_DEBUG_NONE		0
#define NUC_DEBUG_HALF		1
#define NUC_DEBUG_FULL		2

// Changed to use linker to locate the data structure
extern char __header_end[];
#define NUCLOAD_END			( (unsigned)__header_end )

#define NUCLOAD_CURSOR_COL	(NUCLOAD_END - 1)
#define NUCLOAD_CURSOR_ROW	(NUCLOAD_END - 2)
#define NUCLOAD_CURSOR_PAGE	(NUCLOAD_END - 3)
#define NUCLOAD_DEBUGLEVEL	(NUCLOAD_END - 4)
#define NUCLOAD_LANGUAGE	(NUCLOAD_END - 5)

// Versionskonstanten
extern const int nucleus_version; // enthält die aktuelle Versionsnummer in BCD (z.B. 0x120 für Version 1.20)
extern const char nucleus_revision[]; // enthält die Revisionsnummer (z.B. "r0" für stabile Revision 0, "b5" für beta Version 5 oder "a1" für alpha Version 1)

// Systemvariablen
extern char nucleuskl_debug; // NUC_DEBUG_NONE, NUC_DEBUG_HALF oder NUC_DEBUG_FULL
extern char nucleuskl_status; // NUC_OFF, NUC_STARTING, NUC_SHUTDOWN oder NUC_RUNNING

// ASM Variablen
extern word sel_cs, sel_ds, sel_cs4g, sel_ds4g, sel_ss;

// Altenativer Name für 4 GB Datenselektor
#define sel_4g sel_ds4g

// Nucleus Basisadresse
extern dword nucleuskl_base;

// Prototypen
void entrypoint(void); // Diese Funktion darf nur von "nucload" aufgerufen werden!!!
void shutdown(void);
void terror_nuclei(const char *nachricht); // Sollte etwas schiefgehen -> "Kernel Panic"
int main(void); // Diese Funktion darf nur von entrypoint aufgerufen werden!!!

void trace_message(char *msg);

#endif
