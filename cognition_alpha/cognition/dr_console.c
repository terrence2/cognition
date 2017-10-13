// Cognition
// cog_draw_console.c
// by Terrence Cole 12/12/01

// Includes
/////////////
#include "cog_global.h"
#include "cog_opengl.h"
#include "sv_console.h"

// Global Prototypes
//////////////////////
/*
void d_DrawConsole();
void d_DrawConsoleFullscreen();
void d_ConsoleLine( char *line, float pos_x, float pos_y );
void d_ConsoleInvertedLine( char *line, float pos_x, float pos_y );
*/

// Local Prototypes
/////////////////////
static float d_ConsoleChar( char c, float xScale, float pos_x, float pos_y );
static int d_con_ClrIndexFromKey( char c );

// Local Variables
////////////////////
static color cBack = { 255, 255, 255, 200 };
static color white = { 255, 255, 255, 255 };
static color blue = { 0, 0, 255, 255 };
const static vec2 win_tc0 = { 0.0f, 0.0f };
const static vec2 win_tc1 = { 1.0f, 0.0f };
const static vec2 win_tc2 = { 1.0f, 1.0f };
const static vec2 win_tc3 = { 0.0f, 1.0f };

// *********** FUNCTIONALITY ***********
/* ------------
d_DrawConsole
------------ */
void d_DrawConsole()
{
	int a;
	float y;
	int index;

	if( console.inited < 1 ) return;
	if( !state.bInConsole ) return;

	// setup the pretty scrolling star background
	glMatrixMode( GL_TEXTURE );
	glPushMatrix();

		// draw the background
		glLoadIdentity();
		glTranslatef( -(float)state.frame_time * 0.00002f, (float)state.frame_time * 0.00003f, 0.0f );

			d_MenuRect( console.bg_pos[RECT_LEFT],
						console.bg_pos[RECT_RIGHT],
						console.bg_pos[RECT_BOTTOM],
						console.bg_pos[RECT_TOP],
						cBack, console.cmBackground );
			
		// draw the overlay
		glLoadIdentity();
		glTranslatef( -(float)state.frame_time * 0.00004f, (float)state.frame_time * 0.00006f, 0.0f );

			d_MenuRect( console.bg_pos[RECT_LEFT],
						console.bg_pos[RECT_RIGHT],
						console.bg_pos[RECT_BOTTOM],
						console.bg_pos[RECT_TOP],
						cBack, console.cmOverlay );

	glPopMatrix();

	// draw the menu border
	glColor4ubv( console.borderColor );
	glLineWidth( console.borderWidth );
	glDisable( GL_TEXTURE_2D );

		glBegin( GL_LINE_STRIP );
			glVertex2f( console.bg_pos[RECT_LEFT], console.bg_pos[RECT_TOP] );
			glVertex2f( console.bg_pos[RECT_LEFT], console.bg_pos[RECT_BOTTOM] );
			glVertex2f( console.bg_pos[RECT_RIGHT], console.bg_pos[RECT_BOTTOM] );
			glVertex2f( console.bg_pos[RECT_RIGHT], console.bg_pos[RECT_TOP] );
			glVertex2f( console.bg_pos[RECT_LEFT], console.bg_pos[RECT_TOP] );
		glEnd();
		
	glEnable( GL_TEXTURE_2D );

	// draw the current input line
	d_TextLineWhite( console.input, 12.0f, 0.0f, SCR_HEIGHT - CON_HEIGHT );

	// draw visible console lines
	y = SCR_HEIGHT - CON_HEIGHT + console.cs_height;
	index = console.lastLine - console.lineOffset;
	while( index < 0 ) index += NUM_CON_LINES;
	if( index >= NUM_CON_LINES ) index = NUM_CON_LINES - 1;  // we can only go down so don't wrap around
	for( a = console.cs_DownNumVisLines ; a > 0 ; a-- )
	{
		d_TextLineWhite( console.lines[index], 12.0f, 0.0f, y );
		y += (console.cs_height);

		index--;
		while( index < 0 ) index += NUM_CON_LINES;
		if( index >= NUM_CON_LINES ) index = NUM_CON_LINES - 1;  // we can only go down so don't wrap around
	}
}

/* ------------
d_DrawConsoleFullscreen - draws the console with base functionality to the full screen
------------ */
void d_DrawConsoleFullscreen()
{
	int a;
	float y;
	int index;
	int iNumVisLines;

	// make sure we're ready
	if( console.inited < 1 ) return;

	// clear the color buffer manually since we arn't doing it ourselves
	glClear( GL_COLOR_BUFFER_BIT );

	// in fullscreen we need to cap our own projection
	d_EnterOrthoMode();

	// draw the background
	d_MenuRect( 0.0f, SCR_WIDTH, 0.0f, SCR_HEIGHT, cBack, console.cmLoading );
	d_MenuRect( 0.0f, SCR_WIDTH, 0.0f, SCR_HEIGHT, cBack, console.cmLoadingOverlay );

	// draw the menu border
	glColor4ubv( console.borderColor );
	glLineWidth( console.borderWidth );
	glDisable( GL_TEXTURE_2D );

		glBegin( GL_LINE_STRIP );
			glVertex2f( 0.0f, SCR_HEIGHT );
			glVertex2f( 0.0f, 0.0f );
			glVertex2f( SCR_WIDTH, 0.0f );
			glVertex2f( SCR_WIDTH, SCR_HEIGHT);
			glVertex2f( 0.0f, SCR_HEIGHT);
		glEnd();
		
	glEnable( GL_TEXTURE_2D );

	// draw visible console lines
	y = console.cs_height;
	index = console.lastLine - console.lineOffset;
	iNumVisLines = (int)(480.0f / console.cs_height);
	while( index < 0 ) index += NUM_CON_LINES;
	if( index >= NUM_CON_LINES ) index = NUM_CON_LINES - 1;  // we can only go down so don't wrap around
	for( a = console.cs_FSNumVisLines - 5 ; a > 0 ; a-- )
	{
		d_TextLineWhite( console.lines[index], 12.0f, 2.0f, y );
		y += (console.cs_height);

		index--;
		while( index < 0 ) index += NUM_CON_LINES;
		if( index >= NUM_CON_LINES ) index = NUM_CON_LINES - 1;  // we can only go down so don't wrap around
	}

	// draw the loading message
	d_TextLineColor( "Please Wait While Cognition Starts...", 24.0f, blue, 2.0f, y + console.cs_height );

	// in fullscreen we need to manage our own projection
	d_LeaveOrthoMode();
}


