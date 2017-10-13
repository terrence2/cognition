// Cognition
// sv_widget_pixmap.c
// Created  by Terrence Cole 9/14/03

// Includes
/////////////
#include "cog_global.h"
#include "sv_window.h"

// Definitions
////////////////

// Local Structures
/////////////////////
/*
typedef struct widget_pixmap_s
{
	// properties
	window_t win;
	colormap_t *cm;
	uint8_t color[4];

	// linkings
	struct widget_pixmap_s *next;
	struct widget_pixmap_s *prev;
} widget_pixmap_t;
*/

// Global Prototypes
//////////////////////
/*
int pixmap_Initialize();
void pixmap_Terminate();
widget_pixmap_t *pixmap_Create( window_t parent, int32_t x, int32_t y, int32_t w, int32_t h, char *img24, char *img8 );
void pixmap_Destroy( widget_pixmap_t *pm );
void pixmap_SetColor( widget_pixmap_t *pm, uint8_t color[4] );
 */

// Local Prototypes
/////////////////////
static void pixmap_LinkTail( widget_pixmap_t *pixmap );
static void pixmap_Unlink( widget_pixmap_t *pixmap );
 
// Local Variables
////////////////////
static widget_pixmap_t *pixmapHead = NULL;
static widget_pixmap_t *pixmapTail = NULL;
static int iLinkCount = 0; // number of linked items

// *********** FUNCTIONALITY ***********
int pixmap_Initialize() {return 1;}
void pixmap_Terminate()
{
	int cnt = 0;
	widget_pixmap_t *pm;
	
	con_Print( "\n<RED>Terminating</RED> Pixmap System..." );

	while( pixmapHead != NULL )
	{
		pm = pixmapHead->next;
		cm_Unload( pixmapHead->cm );
		win_Destroy( pixmapHead->win );
		SAFE_RELEASE( pixmapHead );
		pixmapHead = pm;
		cnt++;
	}

	pixmapHead = pixmapTail = NULL;
	iLinkCount = 0;

	if(cnt) con_Print("\tFreed %d pixmaps...", cnt );
	con_Print( "\tDone." );
}

/* ------------
pixmap_Create
 ------------ */
widget_pixmap_t *pixmap_Create( window_t parent, int32_t x, int32_t y, int32_t w, int32_t h, char *img24, char *img8 )
{
	widget_pixmap_t *pm;
	colormap_params_t cmParams;
	char buffer[1024];

	// allocate
	pm = (widget_pixmap_t*)mem_alloc( sizeof(widget_pixmap_t) );
	if( pm == NULL )
	{
		con_Print( "<RED>Pixmap Create Error:  memory allocation failed." );
		return NULL;
	}
	memset( pm, 0, sizeof(widget_pixmap_t) );

	// setup window
	pm->win = win_Create( parent, x, y, w, h, 0, pm );
	if( pm->win == NULL_WINDOW )
	{
		con_Print( "<RED>Textlabel Create Error:  could not create window." );
		return NULL;
	}
	win_SetDrawHandler( pm->win, pixmap_Draw );

	// load the title pixmap
	cmParams.bMipmap = 1;
	cmParams.bNoRescale = 0;
	cmParams.iEnv = GL_MODULATE;
	cmParams.iFilter = GL_LINEAR;
	cmParams.iScaleBias = 0;
	cmParams.iWrap = GL_REPEAT;
	snprintf( buffer, 1024, "%s_WITH_%s", img24, img8 );
	pm->cm = cm_LoadFromFiles( buffer, img24, img8, &cmParams );
	
	// copy stuff
	pm->color[0] = 255;
	pm->color[1] = 255;
	pm->color[2] = 255;
	pm->color[3] = 255;
	
	// link
	pixmap_LinkTail( pm );

	return pm;
}

/* ------------
pixmap_Destroy
 ------------ */
void pixmap_Destroy( widget_pixmap_t *pm )
{
	assert( pm );
	if( !pm )
	{
		con_Print( "<RED>Pixmap Destroy Failed:  NULL pixmap" );
		return;
	}

	// unlink
	pixmap_Unlink( pm );

	// destroy internals
	cm_Unload( pm->cm );

	// destroy window
	win_Destroy( pm->win );

	// destroy
	SAFE_RELEASE( pm );
}

/* ------------
pixmap_SetColor
 ------------ */
void pixmap_SetColor( widget_pixmap_t *pm, uint8_t color[4] )
{
	assert( pm );
	if( !pm )
	{
		con_Print( "<RED>Pixmap Set Color Failed:  NULL pixmap" );
		return;
	}

	memcpy( pm->color, color, sizeof(uint8_t) * 4 );
}

/* ------------
pixmap_LinkTail 
------------ */
static void pixmap_LinkTail( widget_pixmap_t *pixmap )
{
	if( pixmapHead == NULL ) pixmapHead = pixmap;

	if( pixmapTail == NULL )
	{
		pixmapTail = pixmap;
		pixmap->next = NULL;
		pixmap->prev = NULL;
		iLinkCount++;
		return;
	}

	pixmap->prev = pixmapTail;
	pixmap->next = NULL;

	pixmapTail->next = pixmap;
	pixmapTail = pixmap;
	iLinkCount++;
}

/* ------------
pixmap_Unlink
------------ */
static void pixmap_Unlink( widget_pixmap_t *pixmap )
{
	if( pixmap == NULL ) return;

	if( pixmap->prev != NULL ) pixmap->prev->next = pixmap->next;
	if( pixmap->next != NULL ) pixmap->next->prev = pixmap->prev;

	if( pixmapTail == pixmap ) pixmapTail = pixmap->prev;
	if( pixmapHead == pixmap ) pixmapHead = pixmap->next;

	pixmap->prev = NULL;
	pixmap->next = NULL;

	iLinkCount--;

	if( iLinkCount == 0 )
	{
		pixmapHead = NULL;
		pixmapTail = NULL;
	}
}
