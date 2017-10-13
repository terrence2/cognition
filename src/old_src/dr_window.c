// Cognition
// dr_window.c
// Created  by Terrence Cole 9/9/03

// Includes
/////////////
#include "cog_global.h"
#include "sv_window.h"

// Definitions
////////////////

// Local Structures
/////////////////////


// Global Prototypes
//////////////////////
/*
void win_Draw();
void win_DefaultDrawHandler( void *data );
void textedit_Draw( void *data );
void textlabel_Draw( void *data );
void pixmap_Draw( void *data );
*/

// Local Prototypes
/////////////////////
static void win_DrawNode( win_struct_t *win );
static void win_DrawToplevel( win_struct_t *windows, int size );

// Local Variables
////////////////////


// *********** FUNCTIONALITY ***********
/* ------------
win_Draw
------------ */
void win_Draw()
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
	glOrtho( 0, 1024, 0, 768, -10, 10 );

		// recursive draw of all children
		win_DrawNode( &windows[win_GetRoot()] );

		// reiterate for toplevel
		win_DrawToplevel( windows, WIN_MAX_WINDOWS );

	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();

	// reenable depth stuff
	glEnable( GL_LIGHTING );
	glDepthMask( GL_TRUE );
	glEnable( GL_DEPTH_TEST );
}

/* ------------
win_DefaultDrawHandler
------------ */
void win_DefaultDrawHandler( void *data )
{
}

/* ------------
textedit_Draw
// drawing function for textedit widgets
------------ */
void textedit_Draw( void *data )
{
	int x, y;
	uint8_t clr[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
	widget_textedit_t *te = (widget_textedit_t*)data;

	win_GetPosition(te->win, &x, &y );
	x += TEXTEDIT_HORZ_OFFSET;
	y += TEXTEDIT_VERT_OFFSET;
	d_TextLineColor( te->buffer, TEXTEDIT_FONTSIZE, clr, (float)x, (float)y );
}

/* ------------
textlabel_Draw
// drawing function for textlabels
------------ */
void textlabel_Draw( void *data )
{
	int x, y;
	widget_textlabel_t *tl = (widget_textlabel_t*)data;
	win_struct_t *win;
	if( tl == NULL ) return;
	
	win = &windows[tl->win];
	if( win->bHidden ) return;

	win_GetPosition( tl->win, &x, &y );
	d_TextLineColor( tl->text, (float)tl->font_size, tl->color, (float)x, (float)y );
}

/* ------------
pixmap_Draw
------------ */
void pixmap_Draw( void *data )
{
	widget_pixmap_t *pm = (widget_pixmap_t*)data;
	win_struct_t *win;
	if( pm == NULL ) return;

	win = &windows[pm->win];
	if( win->bHidden ) return;
	
	glColor4ubv( pm->color );
	SAFE_BIND( pm->cm );
	glBegin( GL_QUADS );
		glTexCoord2fv( win_tc0 );
		glVertex2fv( win->corners[0] );
		glTexCoord2fv( win_tc1 );
		glVertex2fv( win->corners[1] );
		glTexCoord2fv( win_tc2 );
		glVertex2fv( win->corners[2] );
		glTexCoord2fv( win_tc3 );
		glVertex2fv( win->corners[3] );
	glEnd();
}

/* ------------
win_DrawNode
// draws this window and it's children
------------ */
static void win_DrawNode( win_struct_t *win )
{
	int a, cnt;
	
	if( !win->bInUse ) return;
	if( win->bTopmost ) return;
	if( win->bHidden ) return;

	// set global color scheme
	glColor4ubv( win->color );

	// draw the windows stuff
	if( win->bFilled )
	{
		SAFE_BIND( cmFiller );
		
		glBegin( GL_QUADS );
			glTexCoord2fv( win_tc0 );
			glVertex2fv( win->corners[0] );
			glTexCoord2fv( win_tc1 );
			glVertex2fv( win->corners[1] );
			glTexCoord2fv( win_tc2 );
			glVertex2fv( win->corners[2] );
			glTexCoord2fv( win_tc3 );
			glVertex2fv( win->corners[3] );
		glEnd();
	}
	if( win->bBorder )
	{
		SAFE_BIND( cmBorder );

		glBegin( GL_QUADS );
			glTexCoord2fv( win_tc0 );
			glVertex2fv( win->corners[0] );
			glTexCoord2fv( win_tc1 );
			glVertex2fv( win->corners[1] );
			glTexCoord2fv( win_tc2 );
			glVertex2fv( win->corners[2] );
			glTexCoord2fv( win_tc3 );
			glVertex2fv( win->corners[3] );
		glEnd();
	}

	// draw the custom stuff
	win->draw( win->data );

	// draw children; 
	// FIXME: we should do checking to make sure this doesn't circle.  oh well
	cnt = 0;
	a = 0;
	while( cnt < win->iNumChildren && a < WIN_MAX_WINDOWS )
	{
		if( win->children[a] != NULL )
		{
			win_DrawNode( win->children[a] );
			cnt++;
		}
		a++;
	}
}

/* ------------
win_DrawToplevel
// draws toplevel windows in arbitrary order after all other window drawing
// does not recurse children
------------ */
static void win_DrawToplevel( win_struct_t *windows, int size )
{
	int a;
	win_struct_t *win;

	for( a = 0 ; a < size ; a++ )
	{
		if( windows[a].bTopmost && !windows[a].bHidden )
		{
			win = &windows[a];
		
			// set global color scheme
			glColor4ubv( win->color );

			// draw the windows stuff
			if( win->bFilled )
			{
				SAFE_BIND( cmFiller );
				
				glBegin( GL_QUADS );
					glTexCoord2fv( win_tc0 );
					glVertex2fv( win->corners[0] );
					glTexCoord2fv( win_tc1 );
					glVertex2fv( win->corners[1] );
					glTexCoord2fv( win_tc2 );
					glVertex2fv( win->corners[2] );
					glTexCoord2fv( win_tc3 );
					glVertex2fv( win->corners[3] );
				glEnd();
			}
			if( win->bBorder )
			{
				SAFE_BIND( cmBorder );

				glBegin( GL_QUADS );
					glTexCoord2fv( win_tc0 );
					glVertex2fv( win->corners[0] );
					glTexCoord2fv( win_tc1 );
					glVertex2fv( win->corners[1] );
					glTexCoord2fv( win_tc2 );
					glVertex2fv( win->corners[2] );
					glTexCoord2fv( win_tc3 );
					glVertex2fv( win->corners[3] );
				glEnd();
			}

			// draw the custom stuff
			win->draw( win->data );

		}
	}
}

