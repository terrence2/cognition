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

// linked_list.c
// 8-25-02

// PURPOSE
// an abstract list implementation

// INCLUDES
/////////////
#include "linked_list.h"
#include "../cognition/cog_global.h"

// CONSTANTS
//////////////
#define NUM_LISTS 64

// STRUCTURE
//////////////
typedef struct LinkedItem_s
{
	void *data;
	struct LinkedItem_s *next;
} LinkedItem_t;

// GLOBAL PROTOTYPES
//////////////////////
/* - globals declared in linked_list.h
LinkedList ll_GetNewList();
int ll_DeleteList( LinkedList list );
int ll_AddUnordered( LinkedList list, void *new_data );

// iterator functions
void ll_iter_reset( LinkedList list ); 
int ll_iter_isdone( LinkedList list );
void *ll_iter_data( LinkedList list );
void ll_iter_next( LinkedList list );
void ll_iter_remove( LinkedList list );
*/ 

// LOCAL DATA
///////////////
static LinkedItem_t *Lists[NUM_LISTS];
static byte List_Usage[NUM_LISTS];
static LinkedItem_t *Iterators[NUM_LISTS];

// IMPLEMENTATION
///////////////////
/* ----------
ll_GetNewList - finds the first unused list, marks it as used and returns a handle to the list
			  - returns the new list index when successful
			  - else returns < 0
---------- */
LinkedList ll_GetNewList()
{
	int a;

	// search through all the potential lists
	for( a = 0 ; a < NUM_LISTS ; a++ )
	{
		if( List_Usage[a] < 1 )
		{
			// the list is unused
			List_Usage[a] = 1;
			return a;
		}
	}

	return LINKEDLIST_NOT_A_LIST;
}

/* ----------
ll_DeleteList - Frees and removes each linked item then marks the list as unused
			  - if data is non-null for any one LinkedItem or an error occurs, the function return is 0
			  - otherwise the function returns > 0
---------- */
int ll_DeleteList( LinkedList list )
{
	LinkedItem_t *llTmp;

	while( Lists[list] != NULL )
	{
		// store the current element
		llTmp = Lists[list];
		Lists[list] = Lists[list]->next;

		// free the list element
		SAFE_RELEASE( llTmp );
	}

	List_Usage[list] = 0;

	return 1;
}

/* ----------
ll_AddUnordered - Adds an element at the list head
			    - if data is added successfully returns > 0
			    - otherwise the function returns 0
---------- */
int ll_AddUnordered( LinkedList list, void *new_data )
{
	LinkedItem_t *li_new;

	// allocate memory for the new item
	li_new = (LinkedItem_t*)mem_alloc( sizeof(LinkedItem_t) );
	if( li_new == NULL )
	{
		return 0;
	}
	memset( li_new, 0 , sizeof(LinkedItem_t) );

	// add the data
	li_new->data = new_data;

	// add the link
	li_new->next = Lists[list];
	Lists[list] = li_new;

	return 1;
}


// ITERATOR FUNCTIONS
///////////////////////
/* ----------
ll_iter_reset - resets the iterator to the list head
---------- */
void ll_iter_reset( LinkedList list )
{
	Iterators[list] = Lists[list];
}

/* ----------
ll_iter_isdone - returns > 0 if the current list item is null
---------- */
int ll_iter_isdone( LinkedList list )
{
	if( Iterators[list] == NULL ) return 1;

	return 0;
}

/* ----------
ll_iter_data - returns the data item at the current iterator
---------- */
void *ll_iter_data( LinkedList list )
{
	if( Iterators[list] != NULL ) return Iterators[list]->data;

	return NULL;
}

/* ----------
ll_iter_next - moves the Iterator forward one item
---------- */
void ll_iter_next( LinkedList list )
{
	if( Iterators[list] != NULL ) Iterators[list] = Iterators[list]->next;
}

/* ----------
ll_iter_remove - removes the current item from the list and sets the iterator to the next item
---------- */
void ll_iter_remove( LinkedList list )
{
	LinkedItem_t *prev;

	// is there a list
	if( list < 0 ) return;
	prev = Lists[list];

	// does the list have items
	if( prev == NULL ) return;

	// is the item the first on the list
	if( prev == Iterators[list] )
	{
		Lists[list] = Iterators[list]->next;
		SAFE_RELEASE(Iterators[list]);
		Iterators[list] = Lists[list];
		return;
	}

	// search for the previous entry
	while( (prev != NULL) && (prev->next != Iterators[list]) ) 
	{
		prev = prev->next;
	}

	// did we find it?
	if( prev == NULL ) return;

	// unlink the iterator
	prev->next = Iterators[list]->next;

	// delete the reference
	SAFE_RELEASE(Iterators[list]);
	Iterators[list] = prev->next;
}
