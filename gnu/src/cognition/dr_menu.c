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
// dr_foreground.c
// Created 12/26/03 by Terrence Cole

// Foreground draw routines

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
void d_MenuRect( float left, float right, float bottom, float top, color clr, colormap_t *cm );
*/

// Local Prototypes
/////////////////////

// Local Variables
////////////////////
const static vec2 win_tc0 = { 0.0f, 0.0f };
const static vec2 win_tc1 = { 1.0f, 0.0f };
const static vec2 win_tc2 = { 1.0f, 1.0f };
const static vec2 win_tc3 = { 0.0f, 1.0f };

// *********** FUNCTIONALITY ***********
/* ------------
d_MenuRect( ... )
// Exported
------------ */
void d_MenuRect( float left, float right, float bottom, float top, color clr, colormap_t *cm )
{
	glColor4ubv( clr );
	SAFE_BIND( cm );
	glBegin( GL_QUADS );
		glTexCoord2fv( win_tc0 );
		glVertex2f( left, bottom );
		glTexCoord2fv( win_tc1 );
		glVertex2f( right, bottom );
		glTexCoord2fv( win_tc2 );
		glVertex2f( right, top );
		glTexCoord2fv( win_tc3 );
		glVertex2f( left, top );
	glEnd();
}
