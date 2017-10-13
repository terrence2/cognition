// Cognition
// sv_widget_textedit.c
// Created  by Terrence Cole 9/9/03

// Includes
/////////////
#include "cog_global.h"
#include "sv_window.h"
#include "sv_console.h"

// Definitions
////////////////

// Local Structures
/////////////////////
/*
typedef struct widget_textedit_s
{
	// properties
	window_t win;
	widget_pixmap_t *pm; // the cursor
	char buffer[TEXTEDIT_MAX_SIZE];
	uint32_t len;

	// linkings
	struct widget_textedit_s *next;
	struct widget_textedit_s *prev;
} widget_textedit_t;
*/

// Global Prototypes
//////////////////////
/*
int textedit_Initialize();
void textedit_Terminate();
widget_textedit_t *textedit_Create( window_t parent, uint32_t x, uint32_t y, uint32_t w );
void textedit_Destroy( widget_textedit_t *te );
char *textedit_GetText( widget_textedit_t *te );
void textedit_SetText( widget_textedit_t *te, char *text );
*/

// Local Prototypes
/////////////////////
static void textedit_EventHandler( void *data, uint8_t key, uint8_t key_state );
static void textedit_MouseInHandler( void *data, int x, int y );
static void textedit_MouseOutHandler( void *data, int x, int y );
static void textedit_LinkTail( widget_textedit_t *textedit );
static void textedit_Unlink( widget_textedit_t *textedit );
 
// Local Variables
////////////////////
static widget_textedit_t *texteditHead = NULL;
static widget_textedit_t *texteditTail = NULL;
static int iLinkCount = 0; // number of linked items

// *********** FUNCTIONALITY ***********
int textedit_Initialize() { return 1; }
void textedit_Terminate()
{
	int cnt = 0;
	widget_textedit_t *te;
	
	con_Print( "\n<RED>Terminating</RED> Textedit Widget System..." );
	while( texteditHead != NULL )
	{
		te = texteditHead->next;

		pixmap_Destroy( texteditHead->pm );
		win_Destroy( texteditHead->win );
		SAFE_RELEASE( texteditHead );

		texteditHead = te;
		cnt++;
	}

	texteditHead = texteditTail = NULL;
	iLinkCount = 0;

	if(cnt) con_Print("\tFreed %d textedit widgets...", cnt );
	con_Print( "\tDone." );
}

/* ------------
textedit_Create
 ------------ */
widget_textedit_t *textedit_Create( window_t parent, uint32_t x, uint32_t y, uint32_t w )
{
	widget_textedit_t *te;

	// allocate
	te = (widget_textedit_t*)mem_alloc( sizeof(widget_textedit_t) );
	if( te == NULL )
	{
		con_Print( "<RED>Textedit Create Error:  memory allocation failed." );
		return NULL;
	}
	memset( te, 0, sizeof(widget_textedit_t) );

	// get a window
	te->win = win_Create( parent, x, y, w, TEXTEDIT_HEIGHT, 0, te );
	if( te->win == NULL_WINDOW )
	{
		con_Print( "<RED>Textedit Create Error:  could not create window." );
		SAFE_RELEASE( te );
		return NULL;
	}

	// set window properties
	win_SetFilled( te->win, 1 );
	win_SetBordered( te->win, 1 );

	// get the cursor
	te->pm = pixmap_Create( te->win, x + TEXTEDIT_HORZ_OFFSET, 
									y, TEXTEDIT_HEIGHT, TEXTEDIT_HEIGHT, 
									"teCursor24", "teCursor8" );
	if( te->pm == NULL )
	{
		con_Print( "<RED>Textedit Create Error:  could not create pixmap cursor." );
		win_Destroy( te->win );
		SAFE_RELEASE( te );
		return NULL;
	}
	win_SetHidden( te->pm->win, 1 );

	// setup callbacks
	win_SetDrawHandler( te->win, textedit_Draw );
	win_SetEventHandler( te->win, textedit_EventHandler );
	win_SetMouseInHandler( te->win, textedit_MouseInHandler );
	win_SetMouseOutHandler( te->win, textedit_MouseOutHandler );

	tcstrcpy( te->buffer, "Hello World!" );
	te->len = 14;
	
	// link
	textedit_LinkTail( te );

	return te;
}

/* ------------
textedit_Destroy
 ------------ */
void textedit_Destroy( widget_textedit_t *te )
{
	if( te == NULL )
	{	
		con_Print( "<RED>Textedit Destroy Failed:  tl is NULL" );
		return;
	}
	assert( te );

	// unlink
	textedit_Unlink( te );

	// destroy data
	pixmap_Destroy( te->pm );
	win_Destroy( te->win );
	SAFE_RELEASE( te );
}

/* ------------
textedit_GetText
------------ */
char *textedit_GetText( widget_textedit_t *te )
{
	assert( te );
	return te->buffer;	
}

/* ------------
textedit_SetText
------------ */
void textedit_SetText( widget_textedit_t *te, char *text )
{
	assert( te );
	assert( text );
	tcstrncpy( te->buffer, text, TEXTEDIT_MAX_SIZE );
}

/* ------------
textedit_EventHandler
------------ */
static void textedit_EventHandler( void *data, uint8_t key, uint8_t key_state )
{
	widget_textedit_t *te = (widget_textedit_t*)data;
	int pos, loc;
	int len, offset;
	int y;
	float fp, fLastTrans;

	if( !te ) return;
	
	if( key == M_LEFT && key_state == 1 )
	{
		// span the string until pos > than click loc + len
		ms_GetPosition( &loc, NULL );
		win_GetPosition( te->win, &pos, &y );
		pos += TEXTEDIT_HORZ_OFFSET;
		fp = fLastTrans = 0.0f;
		len = tcstrlen( te->buffer );
		offset = 0;

		while( (pos + (int)fp) < loc && offset < len )
		{
			fLastTrans = d_GetCharWidth( te->buffer[offset], (float)TEXTEDIT_FONTSIZE );
			fp += fLastTrans;
			offset++;
			con_Print( "SIZE: %f, POS: %d", fLastTrans, pos + (int)fp );
		}
		if( offset != len ) fp -= fLastTrans;
		con_Print( "\n" );

		win_SetHidden( te->pm->win, 0 );
		win_SetPosition( te->pm->win, pos + (int)fp, y );
	}
}

/* ------------
textedit_MouseInHandlerl 
------------ */
static void textedit_MouseInHandler( void *data, int x, int y )
{
	widget_textedit_t *te = (widget_textedit_t*)data;
	if( !te ) return;
	win_SetHidden( te->pm->win, 0 );
}

/* ------------
textedit_MouseOutHandler 
------------ */
static void textedit_MouseOutHandler( void *data, int x, int y )
{
	widget_textedit_t *te = (widget_textedit_t*)data;
	if( !te ) return;
	win_SetHidden( te->pm->win, 1 );
}

/* ------------
textedit_LinkTail 
------------ */
static void textedit_LinkTail( widget_textedit_t *textedit )
{
	if( texteditHead == NULL ) texteditHead = textedit;

	if( texteditTail == NULL )
	{
		texteditTail = textedit;
		textedit->next = NULL;
		textedit->prev = NULL;
		iLinkCount++;
		return;
	}

	textedit->prev = texteditTail;
	textedit->next = NULL;

	texteditTail->next = textedit;
	texteditTail = textedit;
	iLinkCount++;
}

/* ------------
textedit_Unlink
------------ */
static void textedit_Unlink( widget_textedit_t *textedit )
{
	if( textedit == NULL ) return;

	if( textedit->prev != NULL ) textedit->prev->next = textedit->next;
	if( textedit->next != NULL ) textedit->next->prev = textedit->prev;

	if( texteditTail == textedit ) texteditTail = textedit->prev;
	if( texteditHead == textedit ) texteditHead = textedit->next;

	textedit->prev = NULL;
	textedit->next = NULL;

	iLinkCount--;

	if( iLinkCount == 0 )
	{
		texteditHead = NULL;
		texteditTail = NULL;
	}
}


