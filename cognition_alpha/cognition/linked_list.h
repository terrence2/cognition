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


