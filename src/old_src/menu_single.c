// Cognition
// menu_single.c
// Created  by Terrence Cole 10/29/03

// Includes
/////////////
#include "menu_global.h"

// Definitions
////////////////

// Local Structures
/////////////////////
typedef struct menu_single_s
{
	widget_textlabel_t *tlTitle;
	
	widget_button_t *buttonGame;
	widget_textlabel_t *tlGame;

	widget_button_t *buttonGame2D;
	widget_textlabel_t *tlGame2D;

	widget_button_t *buttonBack;
	widget_textlabel_t *tlBack;

} menu_single_t;

// Global Prototypes
//////////////////////
/*
int single_Initialize(void);
void single_Terminate(void);
*/

// Local Prototypes
/////////////////////
static void single_GameEvent( void *data, uint8_t key, uint8_t key_state );
static void single_Game2DEvent( void *data, uint8_t key, uint8_t key_state );
static void single_BackEvent( void *data, uint8_t key, uint8_t key_state );

// Local Variables
////////////////////
static menu_single_t menu;
static int bInited = 0;

// *********** FUNCTIONALITY ***********
/* ------------
single_Initialize
------------ */
int single_Initialize(void)
{
	uint8_t titleColor[4] = { 0, 0, 255, 255 };

	if( bInited ) return 1;

	ei->con_Print( "\n<BLUE>Initializing</BLUE> Single Player Menu..." );

	menu.tlTitle = ei->textlabel_Create( ei->win_GetRoot(), 60, 710, "Single Player", 30 );
	ei->textlabel_SetColor( menu.tlTitle, titleColor );
	
	menu.buttonGame = ei->button_Create( ei->win_GetRoot(), 200, 400, 150, 32, "3D" );
	if( menu.buttonGame ) ei->win_SetEventHandler( menu.buttonGame->win, single_GameEvent );
	menu.tlGame = ei->textlabel_Create( ei->win_GetRoot(), 370, 405, "3 Degrees of Freedom.", 14 );

	menu.buttonGame2D = ei->button_Create( ei->win_GetRoot(), 200, 325, 150, 32, "2D" );
	if( menu.buttonGame2D ) ei->win_SetEventHandler( menu.buttonGame2D->win, single_Game2DEvent );
	menu.tlGame2D = ei->textlabel_Create( ei->win_GetRoot(), 370, 330, "2 Degrees of Freedom.", 14 );

	menu.buttonBack = ei->button_Create( ei->win_GetRoot(), 200, 250, 150, 32, "Back" );
	if( menu.buttonBack ) ei->win_SetEventHandler( menu.buttonBack->win, single_BackEvent );
	menu.tlBack = ei->textlabel_Create( ei->win_GetRoot(), 370, 255, "Return to Main Menu", 14 );

	bInited = 1;
	return 1;
}

/* ------------
single_Terminate
------------ */
void single_Terminate(void)
{
	if( !bInited ) return;

	ei->con_Print( "\n<RED>Terminating</RED> Single Player Menu..." );
	
	ei->textlabel_Destroy( menu.tlTitle );
	ei->button_Destroy( menu.buttonGame );
	ei->textlabel_Destroy( menu.tlGame );
	ei->button_Destroy( menu.buttonGame2D );
	ei->textlabel_Destroy( menu.tlGame2D );
	ei->button_Destroy( menu.buttonBack );
	ei->textlabel_Destroy( menu.tlBack );

	memset( &menu, 0, sizeof(menu_single_t) );
	bInited = 0;
}

/* ------------
single_GameEvent
------------ */
static void single_GameEvent( void *data, uint8_t key, uint8_t key_state )
{
	widget_button_t *button = (widget_button_t*)data;
	if( !button ) return;
	if( key == M_LEFT && key_state == 0 )
	{
		tree_Descend( SINGLE_GAME );
	}
}

/* ------------
single_Game2DEvent
------------ */
static void single_Game2DEvent( void *data, uint8_t key, uint8_t key_state )
{
	widget_button_t *button = (widget_button_t*)data;
	if( !button ) return;
	if( key == M_LEFT && key_state == 0 )
	{
		tree_Descend( SINGLE_GAME2D );
	}
}

/* ------------
single_BackEvent
------------ */
static void single_BackEvent( void *data, uint8_t key, uint8_t key_state )
{
	widget_button_t *button = (widget_button_t*)data;
	if( !button ) return;
	if( key == M_LEFT && key_state == 0 )
	{
		tree_Descend( SINGLE_BACK );
	}
}

