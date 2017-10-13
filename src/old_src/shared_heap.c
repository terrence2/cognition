// Cognition
// lib_heap.c
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

// Includes
/////////////
#include "shared_heap.h"
#include <stdlib.h>
#include <assert.h>
#include <memory.h>

// Definitions
////////////////
#define NULL_ITEM 0xFFFFFFFF // -1

// Global Structures
/////////////////////
/*
typedef struct
{
	void **data;
	int (*comparator)( void *d1, void *d2 );
	int numElmts; // currently present items
	int lastItem; // the last item on the heap
	int size; // overall size of the data
} heap_t;
*/

// Global Prototypes
//////////////////////
/*
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
*/

// Local Prototypes
/////////////////////
static int heap_Parent( int cur );
static int heap_LeftChild( int cur );
static int heap_RightChild( int cur );
static void heap_SiftUp( heap_t *heap, int pos );
static void heap_SiftDown( heap_t *heap, int pos );

// Local Variables
////////////////////

// *********** FUNCTIONALITY ***********

/* ------------
heap_Create
// Desc:  The Constructor
// Args: the comparison function to distinguish two items on the heap, and the size 
------------ */
heap_t *heap_Create( int (*compare)( void* d1, void *d2 ), int size )
{
	heap_t *h;

	// assert on all errors
	assert( compare );
	assert( size > 0 );

	// allocate a new heap	
	h = (heap_t*)malloc( sizeof(heap_t) );
	assert( h );
	if( !h ) return NULL;
	memset( h, 0, sizeof(heap_t) );
	
	// fill in given data
	h->comparator = compare;
	h->size = size;
	h->numElmts = 0;
	h->lastItem = NULL_ITEM;

	// allocate data
	h->data = (void**)malloc( sizeof(void*) * h->size );
	assert( h->data );
	if( !h->data ) return NULL;
	memset( h->data, 0, sizeof(void*) * h->size );

	return h;
}

/* ------------
heap_Destroy
// Desc: The Destructor
// Args:  The heap to destroy
------------ */
void heap_Destroy( heap_t *heap )
{
	assert( heap );
	if( heap->data ) free( heap->data );
	free( heap );
}

/* ------------
heap_LookAtTop
// Desc:  Returns the Topmost elemt on the heap without removing it
// Args:  The heap
------------ */
void *heap_LookAtTop( heap_t *heap )
{
	assert(heap);
	return heap->data[0];
}

/* ------------
heap_LookAtTop
// Desc:  Pops and Returns the Topmost elemt on the heap
// Args:  The heap
------------ */
void *heap_RemoveTop( heap_t *heap )
{
	void *top;
	assert( heap );

	// is the heap empty
	if( heap->numElmts <= 0 ) return NULL;

	// store and remove the top item
	top = heap->data[0];
	heap->data[0] = heap->data[heap->lastItem];
	heap->data[heap->lastItem] = NULL;
	heap->numElmts--;
	heap->lastItem--;

	// sift the bubble down
	heap_SiftDown( heap, 0 );

	// give back the removed elmt
	return top;
}

/* ------------
heap_Inserts
// Desc:  Puts a new item in the heap, returns > 0 on success or 0 on failure
// Args:  The heap, the new data to insert
------------ */
int heap_Insert( heap_t *heap, void *data )
{
	assert( heap );

	// check for full condition
	if( heap->numElmts >= heap->size ) return 0;

	// increment pointers
	heap->numElmts++;
	heap->lastItem++;

	// add item
	heap->data[heap->lastItem] = data;

	// sift it up
	heap_SiftUp( heap, heap->lastItem );

	return 1;
}

/* ------------
heap_IsEmpty
// Desc:  returns non-zero for heap size > 0 or 0 if heap is empty
// Args:  The heap
------------ */
int heap_IsEmpty( heap_t *heap )
{
	assert( heap );
	return !heap->numElmts;
}

/* ------------
heap_IsFull
// Desc:  returns non-zero if the heap is full or 0 if heap has room remaining
// Args:  The heap
------------ */
int heap_IsFull( heap_t *heap )
{
	assert( heap );
	return (heap->numElmts == heap->size);
}

// *********** MAINTAINENCE ***********
/* ------------
heap_Parent
// returns the parent's position relative to the given position
------------ */
static int heap_Parent( int cur )
{
	// if we are already the parent
	if( !cur ) return NULL_ITEM;

	// odd implementation can just snap down
	if( cur & 1 )
	{
		return cur / 2;
	}
	// even implementation needs to decriment
	else
	{
		return cur / 2 - 1;
	}
}

/* ------------
heap_LeftChild
// returns the left child's position with respect to current
------------ */
static int heap_LeftChild( int cur )
{
	return 2 * cur + 1;
}

/* ------------
heap_RightChild
// returns the right child's position with respect to current
------------ */
static int heap_RightChild( int cur )
{
	
	return 2 * cur + 2;
}

/* ------------
heap_SiftUp
// sifts up the item at position 'pos' in heap 'heap'
------------ */
static void heap_SiftUp( heap_t *heap, int pos )
{
	int parent;

	assert( heap );
	assert( heap->size > pos );
	assert( pos >= 0 );

	// if we have reached the root we have sifted up as far as we can go
	if( !pos ) return;

	// otherwise, get the parent
	parent = heap_Parent( pos );

	// do we need to do a sift for this level?
	if( heap->comparator( heap->data[parent], heap->data[pos] ) > 0 )
	{
		// swap values
		void *tmp = heap->data[pos];
		heap->data[pos] = heap->data[parent];
		heap->data[parent] = tmp;

		// do this again for the parent
		heap_SiftUp( heap, parent );
	}
}

/* ------------
heap_SiftDown
// The current location has been removed, fill up with the smallest path
// Sift the Gap in the heap down to the bottom, then repair the gap
------------ */
static void heap_SiftDown( heap_t *heap, int pos )
{
	int left;
	int right;

	assert( heap );
	assert( heap->size > pos );
	assert( pos >= 0 );

	// get the left and right child
	left = heap_LeftChild(pos);
	right = heap_RightChild(pos);

	// case 0: empty
	if( heap->data[pos] == NULL ) return;

	// case 1: the left is null
	if( heap->data[left] == NULL )
	{
		// case 1a: last elmt in tree on this branch
		if( heap->data[right] == NULL ) return;

		// flip the values if the bottom is smaller
		if( heap->comparator( heap->data[pos], heap->data[right] ) > 0 )
		{
			void *tmp = heap->data[right];
			heap->data[right] = heap->data[pos];
			heap->data[pos] = tmp;

			// descend
			heap_SiftDown( heap, right );
		}

		return;
	}

	// case 2: the right node only is null
	if( heap->data[right] == NULL )
	{
		if( heap->data[left] == NULL ) return;

		if( heap->comparator( heap->data[pos], heap->data[left] ) > 0 )
		{
			void *tmp = heap->data[left];
			heap->data[left] = heap->data[pos];
			heap->data[pos] = tmp;
			heap_SiftDown( heap, left );
		}

		return;
	}

	// case 3: neither node is null
	if( heap->comparator( heap->data[right], heap->data[left] ) < 0 )
	{
		// left > right
		if( heap->comparator( heap->data[pos], heap->data[right] ) > 0 )
		{
			// swap and descend
			void *tmp = heap->data[right];
			heap->data[right] = heap->data[pos];
			heap->data[pos] = tmp;
			heap_SiftDown( heap, right );
		}
		return;
	}
	else
	{
		// right > left
		if( heap->comparator( heap->data[pos], heap->data[left] ) > 0 )
		{
			// swap and descend
			void *tmp = heap->data[left];
			heap->data[left] = heap->data[pos];
			heap->data[pos] = tmp;
			heap_SiftDown( heap, left );
		}
		return;
	}

	assert( 1 );

	// check to see if we have reached the bottom of the heap
	if( left > heap->size || right > heap->size || heap->data[left] == NULL || heap->data[right] == NULL )
	{
		// pos is now a null item sitting at the bottom of the stack.
		// Put the last item in the gap and sift the gap item back up.
		heap->data[pos] = heap->data[heap->lastItem];
		heap->data[heap->lastItem] = NULL;
		heap->lastItem--;
		heap_SiftUp( heap, pos );
		return;
	}

	// left < right: advance the left
	if( heap->comparator( heap->data[left], heap->data[right] ) < 0 )
	{
		void *tmp = heap->data[pos];
		heap->data[pos] = heap->data[left];
		heap->data[left] = tmp;
		heap_SiftDown( heap, left );
	}
	// right < left: advance the right
	else
	{
		void *tmp = heap->data[pos];
		heap->data[pos] = heap->data[right];
		heap->data[right] = tmp;
		heap_SiftDown( heap, right );
	}		
}

