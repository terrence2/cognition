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

// Local Variables
////////////////////
static unsigned int num_blocks = 0;

// *********** FUNCTIONALITY ***********
/* ------------
mem_Initialize
------------ */
int mem_Initialize(void)
{
	if( num_blocks > 0 ) mem_Terminate();
	return 1;
}

/* ------------
mem_Terminate
------------ */
void mem_Terminate(void)
{
	con_Print( "\nTerminating Memory System..." );
	con_Print( "\tLost %d stray blocks", num_blocks );
	num_blocks = 0;
}

/* ------------
mem_alloc
------------ */
void* mem_alloc( unsigned int size )
{
	void *tmp;

	// get memory
	tmp = malloc( size );

	// increment blocks
	num_blocks++;

#ifdef _PARANOID
	con_Print( "Allocated %d bytes of Memory at Address %d, block number %d", mb->size, mb->addr, num_blocks );
#endif

	return tmp;
}

/* ------------
mem_free
------------ */
void mem_free( void *memblock )
{
	free(memblock);
	num_blocks--;
}

/* ------------
mem_PrintInfo
------------ */
void mem_PrintInfo( char *null )
{
	con_Print( "\nMemory Allocation Information:" );
	con_Print( "------------------------------" );
	con_Print( "Allocated Blocks:  %d blocks", num_blocks );
}
