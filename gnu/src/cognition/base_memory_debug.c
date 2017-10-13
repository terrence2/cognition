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
// memory.c
// Created 4-8-02 @ 1041 by Terrence Cole

// Includes
/////////////
#include "cog_global.h"

// Local Structures
/////////////////////
typedef struct mem_block_s
{
	void *addr;
	unsigned int size;
	unsigned long int time;
	struct mem_block_s *next;
	struct mem_block_s *prev;
} mem_block_t;

// Global Prototypes
//////////////////////
/*
int mem_Initialize(void);
void mem_Terminate(void);
void* mem_alloc( unsigned int size );
void mem_free( void *memblock );
void mem_PrintInfo( char *null );
*/

// Local Prototypes
/////////////////////
static void mem_LinkTail( mem_block_t *mb );
static void mem_Unlink( mem_block_t *mb );

// Local Variables
////////////////////
static unsigned int num_blocks = 0;
static mem_block_t *mb_head = NULL;
static mem_block_t *mb_tail = NULL;

// *********** FUNCTIONALITY ***********
/* ------------
mem_Initialize
------------ */
int mem_Initialize(void)
{
	if( (mb_head != NULL) || (mb_tail != NULL) )
	{
		mem_Terminate();
	}

	return 1;
}

/* ------------
mem_Terminate
------------ */
void mem_Terminate(void)
{
	mem_block_t *mb = mb_head;
	mem_block_t *temp;
	unsigned int stray_blocks = 0;
	unsigned int stray_size = 0;

	con_Print( "\nTerminating Memory System..." );

	while( mb != NULL )
	{
		// hand off
		temp = mb;
		mb = mb->next;

		stray_blocks++;
		stray_size += temp->size;

		con_Print( "\tFreed Block:  %d,  Size:  %d,  Time:  %d", temp->addr, temp->size, temp->time );

		if( temp->addr != NULL ) 
		{
			free(temp->addr);
			temp->addr = NULL;
		}
		if( temp != NULL ) 
		{
			free(temp);
			temp = NULL;
		}
	}

	mb_head = mb_tail = NULL;

	con_Print( "\tFreed %d stray blocks -> %d bytes recovered", stray_blocks, stray_size );
	con_Print( "\tMemory System Terminated Successfully." );
}

/* ------------
mem_alloc
------------ */
void* mem_alloc( unsigned int size )
{
	mem_block_t *mb;

	// allocate a new block
	mb = (mem_block_t*)malloc( sizeof(mem_block_t) );
	if( mb == NULL )
	{
		con_Print( "Memory Block Allocation Failed!" );
		return NULL;
	}
	memset( mb, 0, sizeof(mem_block_t) );

	// initialize the block
	mb->size = size;
	mb->time = ts_GetTime();
	mb->addr = malloc( size );
	if( mb->addr == NULL )
	{
		if( mb != NULL ) 
		{
			free(mb);
			mb = NULL;
		}
		con_Print( "Memory Allocation Failed for %d bytes", size );
		return NULL;  // act like malloc
	}
	// don't clear the memory // act like malloc

#ifdef _PARANOID
	con_Print( "\t<RED>Allocated Block: %d,  Size:  %d,  Time:  %d</RED>", mb->addr, mb->size, mb->time );
#endif
	
	// link it
	mem_LinkTail( mb );

	// increment blocks
	num_blocks++;

	return mb->addr;
}

/* ------------
mem_free
------------ */
void mem_free( void *memblock )
{
	mem_block_t *mb = mb_head;

	if( !memblock ) return;

	while( mb != NULL )
	{
		if( mb->addr == memblock )
		{
#ifdef _PARANOID
			con_Print( "\tFreeing %d bytes of Memory at Address %d", mb->size, mb->addr );
#endif
			// free memory and decrement blocks
			mem_Unlink( mb );
			if( mb->addr != NULL ) 
			{
				free(mb->addr);
				mb->addr = NULL;
			}
			if( mb != NULL ) 
			{
				free(mb);
				mb = NULL;
			}

			num_blocks--;
			return;
		}
		mb = mb->next;
	}

	// the memory was not allocated with this system... attempt to free it anyway and emit a console warning
	con_Print( "\tUnrecorded Memory Block Freed at %d", memblock ); 
	if( memblock != NULL )
	{
		free(memblock);
		memblock = NULL;
	}
}

/* ------------
mem_PrintInfo
------------ */
void mem_PrintInfo( char *null )
{
	unsigned int heap_size = 0;
	unsigned int block_size;
	mem_block_t *mb = mb_head;
	
	while( mb != NULL )
	{
		heap_size += mb->size;
		mb = mb->next;
	}

	block_size = sizeof(mem_block_t) * num_blocks;

	con_Print( "\nMemory Allocation Information:" );
	con_Print( "------------------------------" );
	con_Print( "Heap Size:  %d bytes", heap_size );
	con_Print( "Block Size:  %d bytes", block_size );
	con_Print( "Allocated Blocks:  %d blocks", num_blocks );
}

/* ------------
mem_LinkTail
------------ */
static void mem_LinkTail( mem_block_t *mb )
{
	if( mb_head == NULL ) mb_head = mb;

	if( mb_tail == NULL )
	{
		mb_tail = mb;
		mb->next = NULL;
		mb->prev = NULL;
		return;
	}

	mb->prev = mb_tail;
	mb->next = NULL;

	mb_tail->next = mb;
	mb_tail = mb;
}

/* ------------
img_Unlink
------------ */
static void mem_Unlink( mem_block_t *mb )
{
	if( mb->prev != NULL ) mb->prev->next = mb->next;
	if( mb->next != NULL ) mb->next->prev = mb->prev;

	if( mb_tail == mb ) mb_tail = mb->prev;
	if( mb_head == mb ) mb_head = mb->next;

	mb->prev = NULL;
	mb->next = NULL;
}
