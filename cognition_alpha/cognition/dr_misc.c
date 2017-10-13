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