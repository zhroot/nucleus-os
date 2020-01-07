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
#ifndef _PIC_H_
#define _PIC_H_

void irq_init(void);
void irq_enable(unsigned irq);
void irq_disable(unsigned irq);
void irq_acknowledge(unsigned irq);

#endif
