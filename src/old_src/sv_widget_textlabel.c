// Cognition
// sv_widget_textlabel.c
// Created  by Terrence Cole 9/9/03

// Includes
/////////////
#include "cog_global.h"
#include "sv_window.h"

// Definitions
////////////////

// Local Structures
/////////////////////
/*
typedef struct widget_textlabel_s
{
	// properties
	window_t win;
	char *text;
	uint32_t font_size;
	uint8_t color[4];

	// linkings
	struct widget_textlabel_s *next;
	struct widget_textlabel_s *prev;
} widget_textlabel_t;
*/
// Global Prototypes
//////////////////////
/*
int textlabel_Initialize();
void textlabel_Terminate();
widget_textlabel_t *textlabel_Create( window_t parent, uint32_t x, uint32_t y, char *text, uint32_t font_size );
void textlabel_Destroy( widget_textlabel_t *tl );
void textlabel_SetColor( widget_textlabel_t *tl, uint8_t color[4] );
void textlabel_SetFontSize( widget_textlabel_t *tl, int font_size );
void textlabel_SetText( widget_textlabel_t *tl, const char *text );
 */

// Local Prototypes
/////////////////////
static void textlabel_LinkTail( widget_textlabel_t *textlabel );
static void textlabel_Unlink( widget_textlabel_t *textlabel );
 
// Local Variables
////////////////////
static widget_textlabel_t *textlabelHead = NULL;
static widget_textlabel_t *textlabelTail = NULL;
static int iLinkCount = 0; // number of linked items

// *********** FUNCTIONALITY ***********
int textlabel_Initialize() {return 1;}
void textlabel_Terminate()
{
	int cnt = 0;
	widget_textlabel_t *tl;
	
	con_Print( "\n<RED>Terminating</RED> Textlabel System..." );
	while( textlabelHead != NULL )
	{
		tl = textlabelHead->next;
		SAFE_RELEASE( textlabelHead->text );
		win_Destroy( textlabelHead->win );
		SAFE_RELEASE( textlabelHead );
		textlabelHead = tl;
		cnt++;
	}

	textlabelHead = textlabelTail = NULL;
	iLinkCount = 0;

	if(cnt) con_Print("\tFreed %d textlabels...", cnt );
	con_Print( "\tDone." );
}

/* ------------
textlabel_Create
 ------------ */
widget_textlabel_t *textlabel_Create( window_t parent, uint32_t x, uint32_t y, char *text, uint32_t font_size )
{
	widget_textlabel_t *tl;
	float w, h;

	// allocate
	tl = (widget_textlabel_t*)mem_alloc( sizeof(widget_textlabel_t) );
	if( tl == NULL )
	{
		con_Print( "<RED>Textlabel Create Error:  memory allocation failed." );
		return NULL;
	}
	memset( tl, 0, sizeof(widget_textlabel_t) );

	// setup window
	w = d_GetTextWidth( text, (float)font_size ) + 1.0f;
	h = d_GetTextHeight( (float)font_size ) + 1.0f;
	tl->win = win_Create( parent, x, y, (int)w, (int)h, 0, tl );
	if( tl->win == NULL_WINDOW )
	{
		con_Print( "<RED>Textlabel Create Error:  could not create window." );
		return NULL;
	}
	win_SetDrawHandler( tl->win, textlabel_Draw );

	// copy stuff
	tl->text = tcDeepStrCpy( text );
	tl->font_size = font_size;
	tl->color[0] = 255;
	tl->color[1] = 255;
	tl->color[2] = 255;
	tl->color[3] = 255;
	
	// link
	textlabel_LinkTail( tl );

	return tl;
}

/* ------------
textlabel_Destroy
 ------------ */
void textlabel_Destroy( widget_textlabel_t *tl )
{
	assert( tl );
	if( !tl )
	{
		con_Print( "<RED>Textlabel Destroy Failed:  Null textlabel" );
		return;
	}

	// unlink
	textlabel_Unlink( tl );

	// destroy internals
	SAFE_RELEASE( tl->text );

	// destroy window
	win_Destroy( tl->win );

	// destroy
	SAFE_RELEASE( tl );
}

/* ------------
textlabel_SetColor
 ------------ */
void textlabel_SetColor( widget_textlabel_t *tl, uint8_t color[4] )
{
	assert( tl );
	if( !tl )
	{
		con_Print( "<RED>Textlabel Set Color Failed:  Null textlabel" );
		return;
	}
	
	memcpy( tl->color, color, sizeof(uint8_t) * 4 );
}

/* ------------
textlabel_SetFontSize
------------ */
void textlabel_SetFontSize( widget_textlabel_t *tl, int font_size )
{
	assert(tl);
	if( !tl )
	{
		con_Print( "<RED>Textlabel Set Font Size Failed:  textlabel is NULL" );
		return;
	}

	tl->font_size = (uint32_t)font_size;
	win_SetSize( tl->win, (int)d_GetTextWidth(tl->text, (float)font_size), (int)d_GetTextHeight((float)font_size) );
}

/* ------------
textlabel_SetText
------------ */
void textlabel_SetText( widget_textlabel_t *tl, const char *text )
{
	assert(tl);
	assert(text);
	if( !tl )
	{
		con_Print( "<RED>Textlabel Set Text Failed:  textlabel is NULL" );
		return;
	}
	if( !text )
	{
		con_Print( "<RED>Textlabel Set Text Failed: text is NULL" );
		return;
	}

	SAFE_RELEASE( tl->text );
	tl->text = tcDeepStrCpy(text);
	win_SetSize( tl->win, (int)d_GetTextWidth(tl->text, (float)tl->font_size), (int)d_GetTextHeight((float)tl->font_size) );
}

/* ------------
textlabel_LinkTail 
------------ */
static void textlabel_LinkTail( widget_textlabel_t *textlabel )
{
	if( textlabelHead == NULL ) textlabelHead = textlabel;

	if( textlabelTail == NULL )
	{
		textlabelTail = textlabel;
		textlabel->next = NULL;
		textlabel->prev = NULL;
		iLinkCount++;
		return;
	}

	textlabel->prev = textlabelTail;
	textlabel->next = NULL;

	textlabelTail->next = textlabel;
	textlabelTail = textlabel;
	iLinkCount++;
}

/* ------------
textlabel_Unlink
------------ */
static void textlabel_Unlink( widget_textlabel_t *textlabel )
{
	if( textlabel == NULL ) return;

	if( textlabel->prev != NULL ) textlabel->prev->next = textlabel->next;
	if( textlabel->next != NULL ) textlabel->next->prev = textlabel->prev;

	if( textlabelTail == textlabel ) textlabelTail = textlabel->prev;
	if( textlabelHead == textlabel ) textlabelHead = textlabel->next;

	textlabel->prev = NULL;
	textlabel->next = NULL;

	iLinkCount--;

	if( iLinkCount == 0 )
	{
		textlabelHead = NULL;
		textlabelTail = NULL;
	}
}

