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
// dr_vbo.c
// by Terrence Cole 6/29/04

// Includes
/////////////
#include "cog_global.h"
#include "cog_opengl.h"

// Definitions
////////////////

// Structures
///////////////

// Global Prototypes
//////////////////////
/*
*/
typedef struct vbo_element_range_s
{
	unsigned int name;
	unsigned int mode;
	unsigned int element_count;
} vbo_element_range_t;

typedef struct vbo_s
{
	unsigned int name;
	unsigned int mode;

	unsigned int vert_pointer_offset;
	unsigned int mvert_pointer_offset;
	unsigned int norm_pointer_offset;

	unsigned int num_elem_ranges;
	
} vbo_t;

unsigned int d_VBOAllocate( mesh_t *mesh );
void d_VBODeallocate( unsigned int buffer );
void d_VBODraw( vbo_t *vbo );

// Local Prototypes
/////////////////////


// Local Variables
////////////////////
static int vbo_count = 0;

// *********** FUNCTIONALITY ***********
/* ------------
d_VBOAllocate
------------ */
unsigned int d_VBOAllocate( mesh_t *mesh )
{
	unsigned int buf;

	// allocate and get access
	//gl_state.extensions.glGenBuffersARB( 1, &buf );
	//gl_state.extensions.glBindBufferARB( GL_ARRAY_BUFFER_ARB, buf );
	glGenBuffersARB( 1, &buf );
	glBindBufferARB( GL_ARRAY_BUFFER_ARB, buf );
	
	// copy in
	//gl_state.extensions.glBufferDataARB( buf, size, data, GL_STATIC_DRAW_ARB );
	//glBufferDataARB( buf, size, data, GL_STATIC_DRAW_ARB );
	
	// increment
	vbo_count++;
	return buf;
}

/* ------------
d_VBODeallocate
------------ */
void d_VBODeallocate( unsigned int buffer )
{
	// acquire it
	//gl_state.extensions.glBindBufferARB( GL_ARRAY_BUFFER_ARB, buffer );
	glBindBufferARB( GL_ARRAY_BUFFER_ARB, buffer );
	
	// delete it
	//gl_state.extensions.glDeleteBuffersARB( 1, &buffer );
	glDeleteBuffersARB( 1, &buffer );
	
	// decriment
	vbo_count--;
}

/* ------------
d_VBODeallocate
------------ */
void d_VBODraw( vbo_t *vbo )
{
	// setup array pointers / iterators
//	glNormalPointer( GL_FLOAT, 0, vbo->norm_pointer_offset );
//	glTexCoordPointer( 2, GL_FLOAT, 0, vbo->mvert_pointer_offset );
//	glVertexPointer( 3, GL_FLOAT, 0, vbo->vert_pointer_offset );
	
//	glDrawRangeElements(mode , start , end , count , type , indices );
}
