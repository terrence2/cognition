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