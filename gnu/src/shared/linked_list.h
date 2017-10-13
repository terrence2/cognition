// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

// linked_list.h
// 8-25-02

// PURPOSE
// an abstract list implementation

#ifndef _LINKED_LIST_H
#define _LINKED_LIST_H

// DATA TYPES
///////////////
typedef int LinkedList;

// DEFINITIONS
////////////////
#define LINKEDLIST_NOT_A_LIST -1

// GLOBAL PROTOTYPES
//////////////////////
LinkedList ll_GetNewList();
int ll_DeleteList( LinkedList list );
int ll_AddUnordered( LinkedList list, void *new_data );

// iterator functions
void ll_iter_reset( LinkedList list ); 
int ll_iter_isdone( LinkedList list );
void *ll_iter_data( LinkedList list );
void ll_iter_next( LinkedList list );
void ll_iter_remove( LinkedList list );

#endif // _LINKED_LIST_H
