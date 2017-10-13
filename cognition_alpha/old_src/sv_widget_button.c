// Cognition
// sv_widget_button.c
// Created  by Terrence Cole 10/16/03

// Includes
/////////////
#include "cog_global.h"
#include "sv_window.h"

// Definitions
////////////////
#define BUTTON_DEF_UP_IMG24 "buttonUp24"
#define BUTTON_DEF_UP_IMG8 "buttonUp8"
#define BUTTON_DEF_OVER_IMG24 "buttonOver24"
#define BUTTON_DEF_OVER_IMG8 "buttonUp8"
#define BUTTON_DEF_DOWN_IMG24 "buttonDown24"
#define BUTTON_DEF_DOWN_IMG8 "buttonUp8"
#define BUTTON_ALIGN_LEFT 0
#define BUTTON_ALIGN_CENTER 1
#define BUTTON_ALIGN_RIGHT 2

// Local Structures
/////////////////////
/*
typedef struct widget_button_s
{
	// window
	window_t win;

	// properties (personal)
	int iAlignment;

	// properties (children)
	widget_textlabel_t *tl;
	widget_pixmap_t *pmUp;
	widget_pixmap_t *pmHighlight;
	widget_pixmap_t *pmDown;

	// linkings
	struct widget_button_s *next;
	struct widget_button_s *prev;
} widget_button_t;
*/

// Global Prototypes
//////////////////////
/*
int button_Initialize();
void button_Terminate();
widget_button_t *button_Create( window_t parent, int32_t x, int32_t y, int32_t w, int32_t h, char *text );
void button_Destroy( widget_button_t *pm );

void button_SetUp( widget_button_t *button );
void button_SetOver( widget_button_t *button );
void button_SetDown( widget_button_t *button );

int button_SetDefaultPixmaps( widget_button_t *button );
void button_SetUpPixmap( widget_button_t *button, char *img24, char *img8 );
void button_SetOverPixmap( widget_button_t *button, char *img24, char *img8 );
void button_SetDownPixmap( widget_button_t *button, char *img24, char *img8 );
void button_SetFontSize( widget_button_t *button, int font_size );
void button_SetAlignment( widget_button_t *button, int align );
*/


// Local Prototypes
/////////////////////
static void button_MouseInHandler( void *data, int x, int y );
static void button_MouseOutHandler( void *data, int x, int y );
static void button_LinkTail( widget_button_t *button );
static void button_Unlink( widget_button_t *button );
 
// Local Variables
////////////////////
static widget_button_t *buttonHead = NULL;
static widget_button_t *buttonTail = NULL;
static int iLinkCount = 0; // number of linked items

// *********** FUNCTIONALITY ***********
int button_Initialize() {return 1;}
void button_Terminate()
{
	int cnt = 0;
	widget_button_t *button;
	
	con_Print( "\n<RED>Terminating</RED> Button System..." );

	while( buttonHead != NULL )
	{
		button = buttonHead->next;

		if( buttonHead->tl ) textlabel_Destroy( buttonHead->tl );
		if( buttonHead->pmUp ) pixmap_Destroy( buttonHead->pmUp );
		if( buttonHead->pmDown ) pixmap_Destroy( buttonHead->pmDown );
		if( buttonHead->pmHighlight ) pixmap_Destroy( buttonHead->pmHighlight );
		win_Destroy( buttonHead->win );
		SAFE_RELEASE( buttonHead );

		buttonHead = button;
		cnt++;
	}

	buttonHead = buttonTail = NULL;
	iLinkCount = 0;

	if(cnt) con_Print("\tFreed %d buttons...", cnt );
	con_Print( "\tDone." );
}

/* ------------
button_Create
// defaults to center alignment
------------ */
widget_button_t *button_Create( window_t parent, int32_t x, int32_t y, int32_t w, int32_t h, char *text )
{
	widget_button_t *button;
	float tmp;

	// get a new button
	button = mem_alloc( sizeof(widget_button_t) );
	if( button == NULL )
	{
		con_Print( "<RED>Button Create Failed:  Could not allocate memory for button widget struct." );
		return NULL;
	}
	memset( button, 0, sizeof(widget_button_t) );

	// create the window
	button->win = win_Create( parent, x, y, w, h, 0, button );
	if( button->win == NULL_WINDOW )
	{
		con_Print( "<RED>Button Create Failed:  Could not create new window." );
		SAFE_RELEASE( button );
		return NULL;
	}

	// create the default pixmap set
	if( !button_SetDefaultPixmaps( button ) )
	{
		con_Print( "<RED>Button Creation Failed:  Couldn't create textlabel." );
		win_Destroy( button->win );
		SAFE_RELEASE(button);
		return NULL;
	}
	
	// grab a textlabel
	tmp = d_GetTextWidth( text, 15 );
	tmp = ((float)w - tmp) / 2.0f;
	button->tl = textlabel_Create( button->win, (int)tmp + x, y + 4, text, 15 );
	if( !button->tl )
	{
		con_Print( "<RED>Button Creation Failed:  Couldn't create textlabel." );
		win_Destroy( button->win );
		SAFE_RELEASE(button);
		return NULL;
	}

	// FIXME: check to see if we need to start the state as 'over'
	win_SetHidden( button->pmHighlight->win, 1 );
	win_SetHidden( button->pmDown->win, 1 );
	win_SetHidden( button->pmUp->win, 0 );
	win_SetParent( button->tl->win, button->pmUp->win );

	// setup callbacks
	win_SetMouseInHandler( button->win, button_MouseInHandler );
	win_SetMouseOutHandler( button->win, button_MouseOutHandler );

	// link it
	button_LinkTail( button );

	// return
	return button;
}

/* ------------
button_Destroy
------------ */
void button_Destroy( widget_button_t *button )
{
	if( !button )
	{
		con_Print( "<RED>Button Destroy Failed:  button is NULL." );
		return;
	}
	assert( button );
	
	textlabel_Destroy( button->tl );
	win_Destroy( button->win );
	button_Unlink( button );
	SAFE_RELEASE( button );
}

/* ------------
button_SetDefaultPixmaps
------------ */
int button_SetDefaultPixmaps( widget_button_t *button )
{
	// assert
	if( !button )
	{
		con_Print( "<RED>Button SetDefaultPixmaps Failed:  button is NULL." );
		return 0;
	}
	assert( button );

	// distribute the work
	button_SetUpPixmap( button, BUTTON_DEF_UP_IMG24, BUTTON_DEF_UP_IMG8 );
	button_SetOverPixmap( button, BUTTON_DEF_OVER_IMG24, BUTTON_DEF_OVER_IMG8 );
	button_SetDownPixmap( button, BUTTON_DEF_DOWN_IMG24, BUTTON_DEF_DOWN_IMG8 );

	return 1;
}

/* ------------
button_SetUp
// makes button appearance 'up'
------------ */
void button_SetUp( widget_button_t *button )
{
	assert( button );
	win_SetHidden( button->pmHighlight->win, 1 );
	win_SetHidden( button->pmDown->win, 1 );
	win_SetHidden( button->pmUp->win, 0 );
	win_SetParent( button->tl->win, button->pmUp->win );
}

/* ------------
button_SetOver
------------ */
void button_SetOver( widget_button_t *button )
{
	assert( button );
	win_SetHidden( button->pmHighlight->win, 0 );
	win_SetHidden( button->pmDown->win, 1 );
	win_SetHidden( button->pmUp->win, 1 );
	win_SetParent( button->tl->win, button->pmHighlight->win );
}

/* ------------
button_SetOver
------------ */
void button_SetDown( widget_button_t *button )
{
	assert( button );
	win_SetHidden( button->pmHighlight->win, 1 );
	win_SetHidden( button->pmDown->win, 0 );
	win_SetHidden( button->pmUp->win, 1 );
	win_SetParent( button->tl->win, button->pmDown->win );
}

/* ------------
button_SetUpPixmap
------------ */
void button_SetUpPixmap( widget_button_t *button, char *img24, char *img8 )
{
	int x, y, w, h;

	// assert
	if( !button )
	{
		con_Print( "<RED>Button Set UP Pixmap Failed:  NULL button." );
		return;
	}
	if( !img24 )
	{
		con_Print( "<RED>Button Set UP Pixmap Failed:  NULL img24." );
		return;
	}
	if( !img8 )
	{
		con_Print( "<RED>Button Set UP Pixmap Failed:  NULL img8." );
		return;
	}
	assert( button );
	assert( img24 );
	assert( img24 );

	// clear previous pixmaps
	if( button->pmUp != NULL )
	{
		pixmap_Destroy( button->pmUp );
	}

	// get properties and allocate
	win_GetPosition( button->win, &x, &y );
	win_GetSize( button->win, &w, &h );
	button->pmUp = pixmap_Create( button->win, x, y, w, h, img24, img8 ); 
}

/* ------------
button_SetOverPixmap
------------ */
void button_SetOverPixmap( widget_button_t *button, char *img24, char *img8 )
{
	int x, y, w, h;

	// assert
	if( !button )
	{
		con_Print( "<RED>Button Set OVER Pixmap Failed:  NULL button." );
		return;
	}
	if( !img24 )
	{
		con_Print( "<RED>Button Set OVER Pixmap Failed:  NULL img24." );
		return;
	}
	if( !img8 )
	{
		con_Print( "<RED>Button Set OVER Pixmap Failed:  NULL img8." );
		return;
	}
	assert( button );
	assert( img24 );
	assert( img24 );

	// clear previous pixmaps
	if( button->pmHighlight != NULL )
	{
		pixmap_Destroy( button->pmHighlight );
	}

	// get properties and allocate
	win_GetPosition( button->win, &x, &y );
	win_GetSize( button->win, &w, &h );
	button->pmHighlight = pixmap_Create( button->win, x, y, w, h, img24, img8 ); 
}

/* ------------
button_SetDownPixmap
------------ */
void button_SetDownPixmap( widget_button_t *button, char *img24, char *img8 )
{
	int x, y, w, h;

	// assert
	if( !button )
	{
		con_Print( "<RED>Button Set DOWN Pixmap Failed:  NULL button." );
		return;
	}
	if( !img24 )
	{
		con_Print( "<RED>Button Set DOWN Pixmap Failed:  NULL img24." );
		return;
	}
	if( !img8 )
	{
		con_Print( "<RED>Button Set DOWN Pixmap Failed:  NULL img8." );
		return;
	}
	assert( button );
	assert( img24 );
	assert( img24 );

	// clear previous pixmaps
	if( button->pmDown != NULL )
	{
		pixmap_Destroy( button->pmDown );
	}

	// get properties and allocate
	win_GetPosition( button->win, &x, &y );
	win_GetSize( button->win, &w, &h );
	button->pmDown = pixmap_Create( button->win, x, y, w, h, img24, img8 ); 
}

/* ------------
button_SetFontSize
// this gets reimplemented in button so we can re-center
------------ */
void button_SetFontSize( widget_button_t *button, int font_size )
{
	int width, tmp;
	int x, y;
	int w, h;

	assert(button);
	assert(button->tl );
	if( !button )
	{
		con_Print( "<RED>Button Set Font Size Failed:  button is NULL." );
		return;
	}

	textlabel_SetFontSize( button->tl, font_size );
	w = (int)d_GetTextWidth( button->tl->text, (float)font_size );
	win_GetSize( button->win, &width, &h );
	win_GetPosition( button->win, &x, &y );
	tmp = width - w;
	tmp /= 2;
	tmp += x;
	win_SetPosition( button->tl->win, tmp, y - h ); 
}

/* ------------
button_SetAlignment 
------------ */
void button_SetAlignment( widget_button_t *button, int align )
{
	int x, y, w, h, tmp;
	
	assert(button);
	assert(button->tl);
	if( !button )
	{
		con_Print( "<RED>Button Set Alignment Failed:  Button is NULL" );
		return;
	}
	if( !button->tl )
	{
		con_Print( "<RED>Button Set Alignment Failed:  Button has no textlabel." );
		return;
	}

	if( align == BUTTON_ALIGN_LEFT )
	{
		win_GetPosition( button->win, &x, &y );
		win_GetSize( button->win, &w, &h );
		win_SetPosition( button->tl->win, x, y + h );
	}
	else if( align == BUTTON_ALIGN_CENTER )
	{
		win_GetPosition( button->win, &x, &y );
		win_GetSize( button->win, &w, &h );
		tmp = (int)d_GetTextWidth( button->tl->text, (float)button->tl->font_size );
		tmp = (w - tmp) / 2;
		win_SetPosition( button->tl->win, x + tmp, y + h );
	}
	else if( align == BUTTON_ALIGN_RIGHT )
	{
		win_GetPosition( button->win, &x, &y );
		win_GetSize( button->win, &w, &h );
		tmp = (int)d_GetTextWidth( button->tl->text, (float)button->tl->font_size );
		win_SetPosition( button->tl->win, x + w - tmp, y + h );
	}
}

/* ------------
button_MouseInHandler 
------------ */
static void button_MouseInHandler( void *data, int x, int y )
{
	widget_button_t *button = (widget_button_t*)data;
	button_SetOver( button );
}

/* ------------
button_MouseOutHandler
------------ */
static void button_MouseOutHandler( void *data, int x, int y )
{
	widget_button_t *button = (widget_button_t*)data;
	button_SetUp( button );
}

/* ------------
button_LinkTail 
------------ */
static void button_LinkTail( widget_button_t *button )
{
	if( buttonHead == NULL ) buttonHead = button;

	if( buttonTail == NULL )
	{
		buttonTail = button;
		button->next = NULL;
		button->prev = NULL;
		iLinkCount++;
		return;
	}

	button->prev = buttonTail;
	button->next = NULL;

	buttonTail->next = button;
	buttonTail = button;
	iLinkCount++;
}

/* ------------
button_Unlink
------------ */
static void button_Unlink( widget_button_t *button )
{
	if( button == NULL ) return;

	if( button->prev != NULL ) button->prev->next = button->next;
	if( button->next != NULL ) button->next->prev = button->prev;

	if( buttonTail == button ) buttonTail = button->prev;
	if( buttonHead == button ) buttonHead = button->next;

	button->prev = NULL;
	button->next = NULL;

	iLinkCount--;

	if( iLinkCount == 0 )
	{
		buttonHead = NULL;
		buttonTail = NULL;
	}
}

