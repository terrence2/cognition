// Cognition
// dr_foreground.c
// Created  by Terrence Cole

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
void d_DrawForegroundBase();
*/

// Local Prototypes
/////////////////////

// Local Variables
////////////////////

// *********** FUNCTIONALITY ***********
/* ------------
d_DrawForegroundBase - draws all of the foreground objects in the main list
------------ */
void d_DrawForegroundBase()
{
	//get the head
	fgo_t *fgoCur = *(state.fgo_list);
	colormap_t *map;
	int a;
	char buffer[1024];	

	// initialize a new projection
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();
	
	// prepare an ortho view
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
		glLoadIdentity();
		glOrtho( 0, SCR_WIDTH, 0, SCR_HEIGHT, -3, 3 );

		// set states
		glDepthMask( GL_FALSE );
		glDisable( GL_DEPTH_TEST );
		glDisable( GL_LIGHTING );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	// iterate
	while( fgoCur != NULL )
	{
		// draw the shader background
		if( fgoCur->shader != NULL )
		{
			for( a = 0 ; a < 2 ; a++ )
			{
				// setup material states
				if( fgoCur->shader->bUseMaterial )
				{
					glColor4fv( fgoCur->shader->material.ambient );
				}
			
				// select a map
				switch( a )
				{
					case 0: map = fgoCur->shader->ColorMap; break;
					case 1: map = fgoCur->shader->subColorMap; break;
					default: map = NULL; break;
				}
			
				// do some drawing
				if( map != NULL )
				{
					SAFE_BIND( map );

					glBegin( GL_QUADS );
						glNormal3f( 0.0f, 0.0f, 1.0f );
				
						glTexCoord2f( 0.0f, 0.0f );
						glVertex2f( fgoCur->shX, 					fgoCur->shY );
						
						glTexCoord2f( 1.0f, 0.0f );
						glVertex2f( fgoCur->shX + fgoCur->shW, 	fgoCur->shY );
						
						glTexCoord2f( 1.0f, 1.0f );
						glVertex2f( fgoCur->shX + fgoCur->shW, 	fgoCur->shY + fgoCur->shH );
				
						glTexCoord2f( 0.0f, 1.0f );
						glVertex2f( fgoCur->shX, 					fgoCur->shY + fgoCur->shH );
					glEnd();
				}
			
				glColor4ub( 255, 255, 255, 255 );
			}
		}

		// draw the text
		if( fgoCur->cpText != NULL )
		{
		}

		// draw the int pointer
		if( fgoCur->ipData != NULL )
		{
			memset( buffer, 0, 1024 );
			sprintf( buffer, "%d", *(fgoCur->ipData) );

		//	d_ColorText( buffer, fgoCur->ipH, fgoCur->ipColor, fgoCur->ipX, fgoCur->ipY );
		}

		if( fgoCur->fpData != NULL )
		{
			memset( buffer, 0, 1024 );
			sprintf( buffer, "%.2f", *(fgoCur->fpData) );

		//	d_ColorText( buffer, fgoCur->fpH, fgoCur->fpColor, fgoCur->fpX, fgoCur->fpY );
		}
		
		if( fgoCur->ent != NULL )
		{
		}

		// next
		fgoCur = fgoCur->next;
	}

	
		// unset states
		glDepthMask( GL_TRUE );
		glEnable( GL_DEPTH_TEST );
		glEnable( GL_LIGHTING );

		// restore perspective view
		glPopMatrix();

	// restore the model/view
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();
}

