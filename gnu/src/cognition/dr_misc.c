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
// dr_misc.c
// Created 3/20/03 by Terrence Cole

// Includes
/////////////
#include "cog_global.h"

// Definitions
////////////////

// Local Structures
/////////////////////

// Global Prototypes
//////////////////////
/*
void d_EnterOrthoMode();
void d_LeaveOrthoMode();
*/

// Local Prototypes
/////////////////////


// Local Variables
////////////////////

// *********** FUNCTIONALITY ***********
/* ------------
d_EnterOrthoMode
------------ */
void d_EnterOrthoMode()
{
	// we'll be painting this all on painter style
	glDepthMask( GL_FALSE );
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_LIGHTING );

	// do this ortho
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	glOrtho( 0, SCR_WIDTH, 0, SCR_HEIGHT, -10, 10 );
}

/* ------------
d_LeaveOrthoMode
------------ */
void d_LeaveOrthoMode()
{
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();

	// reenable depth stuff
	glEnable( GL_LIGHTING );
	glDepthMask( GL_TRUE );
	glEnable( GL_DEPTH_TEST );
}
