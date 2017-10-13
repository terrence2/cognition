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

// Cognition
// shared_queue.c
// Created  by Terrence Cole 1/11/04

// Includes
/////////////
#include "shared_queue.h"
#include <stdlib.h>
#include <assert.h>
#include <memory.h>

// Definitions
////////////////
#define NUM_QUEUES 512

// Global Prototypes
//////////////////////
/*
int q_Initialize(void);
void q_Terminate(void);
queue_t q_Create( int size );
void q_Delete( queue_t q );
int q_Enqueue( queue_t q, void *data );
void *q_Dequeue( queue_t q );
void *q_LookAtHead( queue_t q );
void *q_GetAt( queue_t q, int offset );
int q_Count( queue_t q );
void **q_GetBase( queue_t q );
*/

// Local Vars
/////////////
static int iNumHeaps = 0;
static void **queues[NUM_QUEUES];
static int heads[NUM_QUEUES];
static int tails[NUM_QUEUES];
static int sizes[NUM_QUEUES];

/* ------------
q_Initialize
------------ */
int q_Initialize(void)
{
	int a;
	for( a = 0 ; a < NUM_QUEUES ; a++ )
	{
		queues[a] = NULL;
		heads[a] = tails[a] = sizes[a] = 0;
	}
	return 1;
}

/* ------------
q_Terminate
------------ */
void q_Terminate(void)
{
	int a;
	for( a = 0 ; a < NUM_QUEUES ; a++ ) q_Delete( a );
}

/* ------------
q_Create
------------ */
queue_t q_Create( int size )
{
	int a = 0;
	queue_t q = NULL_QUEUE;

	// sanity check
	assert( size > 0 );

	// find an empty queue
	while( (q == NULL_QUEUE) && (a < NUM_QUEUES) )
	{
		if( queues[a] == NULL ) q = a;
		a++;
	}

	// check for an error
	if( q == NULL_QUEUE ) return NULL_QUEUE;

	// allocate size elmts
	queues[q] = (void**)malloc( sizeof(void*) * size );
	if( !queues[q] ) return NULL_QUEUE;
	memset( queues[q], 0, sizeof(void*) * size );

	// set head and tail
	heads[q] = tails[q] = 0;

	// set size
	sizes[q] = size;

	// give back
	return q;
}

/* ------------
q_Delete
------------ */
void q_Delete( queue_t q )
{
	// sanity check
	assert( q >= 0 );
	assert( q < NUM_QUEUES );

	// free it
	if( queues[q] != NULL )
	{
		free( queues[q] );
		queues[q] = NULL;
	}

	// clear head/tail
	heads[q] = tails[q] = 0;

	// clear size
	sizes[q] = 0;
}

/* ------------
q_Enueue
// returns 0 on fail
------------ */
int q_Enqueue( queue_t q, void *data )
{
	int ins;

	// sanity checks
	assert( q >= 0 );
	assert( q < NUM_QUEUES );
	assert( queues[q] != NULL );

	// get a trial insertion point
	ins = tails[q] + 1;
	while( ins >= sizes[q] ) ins -= sizes[q];

	// fail if we have gone all the way around
	if( ins == heads[q] ) return 0;

	// do the insert and move the tail
	queues[q][ins] = data;
	tails[q] = ins;
	
	return 1;
}

/* ------------
q_Dequeue
------------ */
void *q_Dequeue( queue_t q )
{
	void *item;

	// sanity checks
	assert( q >= 0 );
	assert( q < NUM_QUEUES );
	assert( queues[q] != NULL );

	// case 0: list empty
	if( heads[q] == tails[q] )
	{
		return NULL;
	}

	// advance the head
	heads[q]++;
	while( heads[q] >= sizes[q] ) heads[q] -= sizes[q];

	// get the head item
	item = queues[q][heads[q]];
	queues[q][heads[q]] = NULL;

	// give us the removed item
	return item;
}

/* ------------
q_LookAtHead
------------ */
void *q_LookAtHead( queue_t q )
{
	int pos;
	
	// sanity checks
	assert( q >= 0 );
	assert( q < NUM_QUEUES );
	assert( queues[q] != NULL );

	// case 0: list empty
	if( heads[q] == tails[q] ) return NULL;

	// get the head pos
	pos = heads[q] + 1;
	while( pos >= sizes[q] ) pos -= sizes[q];

	return queues[q][pos];
}

/* ------------
q_GetAt
------------ */
void *q_GetAt( queue_t q, int offset )
{
	int pos;
	
	// sanity checks
	assert( q >= 0 );
	assert( q < NUM_QUEUES );
	assert( queues[q] != NULL );

	// get a position
	pos = heads[q] + 1 + offset;
	while( pos >= sizes[q] ) pos -= sizes[q];

	return queues[q][pos];
}

/* ------------
q_Count
------------ */
int q_Count( queue_t q )
{	
	int offset;
	
	// sanity checks
	assert( q >= 0 );
	assert( q < NUM_QUEUES );
	assert( queues[q] != NULL );

	// count size
	offset = tails[q] - heads[q];
	if( offset < 0 ) offset += sizes[q];
	return offset;
}

/* ------------
q_GetBase
// return the base memory pointer of q
// this breaks the abstraction but is massivly needed for speed
------------ */
void **q_GetBase( queue_t q )
{	
	// sanity checks
	assert( q >= 0 );
	assert( q < NUM_QUEUES );
	assert( queues[q] != NULL );

	return queues[q];
}
