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

#ifndef DATATYPES_H
#define DATATYPES_H

typedef long long __int64;
typedef unsigned long dword;
typedef unsigned short word;
typedef unsigned char byte;

#ifdef NULL
#undef NULL
#endif

#define NULL 0
typedef enum { FALSE=0, TRUE=1 } BOOL;

#endif
