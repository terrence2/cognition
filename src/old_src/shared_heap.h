// Cognition
// lib_heap.h
// Created  by Terrence Cole

// A helper library for mananging and using a heap of void*'s
// This implementation uses no Cognition specific functionality so it's highly portable.
// That also means it doesn't benefit from any of the memory tracking, etc.

// The Comparator Function returns > 0 for: item 1 > item 2.
// It returns 0 for: item 1 = item 2.
// It return < 0 for: item 1 < item 2.

// This heap sorts data from "least" to "greatest".  Getting the opposite behavior
// is as simple as inverting the behavior of the comparator.
// Don't attempt to put NULL pointers in; it shouldn't work and it doesn't work.


#ifndef _HEAP_H_
#define _HEAP_H_

// Global Structures
/////////////////////
typedef struct
{
	void **data;
	int (*comparator)( void *d1, void *d2 );
	int numElmts; // currently present items
	int lastItem; // the last item on the heap
	int size; // overall size of the data
} heap_t;

// Global Prototypes
//////////////////////
heap_t *heap_Create( int (*compare)( void* d1, void *d2 ), int size );
// Desc:  The Constructor
// Args: the comparison function to distinguish two items on the heap, and the size 

void heap_Destroy( heap_t *heap );
// Desc: The Destructor
// Args:  The heap to destroy

void *heap_LookAtTop( heap_t *heap );
// Desc:  Returns the Topmost elemt on the heap without removing it
// Args:  The heap

void *heap_RemoveTop( heap_t *heap );
// Desc:  Pops and Returns the Topmost elemt on the heap
// Args:  The heap

int heap_Insert( heap_t *heap, void *data );
// Desc:  Puts a new item in the heap, returns > 0 on success or 0 on failure
// Args:  The heap, the new data to insert

int heap_IsEmpty( heap_t *heap );
// Desc:  returns non-zero for heap size > 0 or 0 if heap is empty
// Args:  The heap

int heap_IsFull( heap_t *heap );
// Desc:  returns non-zero if the heap is full or 0 if heap has room remaining
// Args:  The heap

#endif // _HEAP_H_

