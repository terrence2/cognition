// Cognition
// menu
// menu_root.c
// by Terrence Cole created 4/1/02

// Includes
/////////////
#include "menu_global.h"

// Definitions
////////////////

// Structures
///////////////
typedef struct menu_root_s
{
	widget_pixmap_t *pmTitle;
	widget_button_t *buttonSingle;
	widget_textlabel_t *tlSingle;
	widget_button_t *buttonMulti;
	widget_textlabel_t *tlMulti;
	widget_button_t *buttonOptions;
	widget_textlabel_t *tlOptions;
	widget_button_t *buttonExit;
	widget_textlabel_t *tlExit;
} menu_root_t;

// Global Prototypes
//////////////////////
/*
int root_Initialize(void);
void root_Terminate(void);
*/

// Local Prototypes
/////////////////////
static void root_SinglePlayerEvent( void *data, uint8_t key, uint8_t key_state );
static void root_MultiPlayerEvent( void *data, uint8_t key, uint8_t key_state );
static void root_OptionsEvent( void *data, uint8_t key, uint8_t key_state );
static void root_ExitEvent( void *data, uint8_t key, uint8_t key_state );

// Local Variables
////////////////////
static menu_root_t root;
static int bInited = 0;

// *********** FUNCTIONALITY ***********
/* ------------
root_Initialize
------------ */
int root_Initialize(void)
{
	window_t rootwin;
	if( bInited ) return 1;

	ei->con_Print( "\n<BLUE>Initializing</BLUE> Root Menu..." );

	// cache this, we'll be using it
	rootwin = ei->win_GetRoot();

	// Title
	root.pmTitle = ei->pixmap_Create( rootwin, 25, 768 - 300, 600, 300, "menu_title24", "menu_title8" );

	// Single Player
	root.buttonSingle = ei->button_Create( rootwin, 200, 400, 150, 32, "Single Player" );
	if( root.buttonSingle ) ei->win_SetEventHandler( root.buttonSingle->win, root_SinglePlayerEvent );
	root.tlSingle = ei->textlabel_Create( rootwin, 370, 405, "Start a new Single Player game.", 14 );
	
	// Multiplayer
	root.buttonMulti = ei->button_Create( rootwin, 200, 325, 150, 32, "Multi-Player" );
	if( root.buttonMulti ) ei->win_SetEventHandler( root.buttonMulti->win, root_MultiPlayerEvent );
	root.tlMulti = ei->textlabel_Create( rootwin, 370, 330, "Start a new Multi-Player game.", 14 );

	// Options
	root.buttonOptions = ei->button_Create( rootwin, 200, 250, 150, 32, "Options" );
	if( root.buttonOptions ) ei->win_SetEventHandler( root.buttonOptions->win, root_OptionsEvent );
	root.tlOptions = ei->textlabel_Create( rootwin, 370, 255, "Change game options.", 14 );

	// Exit
	root.buttonExit = ei->button_Create( rootwin, 200, 175, 150, 32, "Exit" );
	if( root.buttonExit ) ei->win_SetEventHandler( root.buttonExit->win, root_ExitEvent );
	root.tlExit = ei->textlabel_Create( rootwin, 370, 180, "Leave Cognition.", 14 );

	ei->textedit_Create( rootwin, 120, 450, 150 );

	bInited = 1;
	return 1;
}

/* ------------
root_Terminate
------------ */
void root_Terminate(void)
{
	if( !bInited ) return;

	ei->con_Print( "\n<RED>Terminating</RED> Root Menu..." );
		
	ei->pixmap_Destroy( root.pmTitle );
	ei->button_Destroy( root.buttonSingle );
	ei->textlabel_Destroy( root.tlSingle );
	ei->button_Destroy( root.buttonMulti );
	ei->textlabel_Destroy( root.tlMulti );
	ei->button_Destroy( root.buttonOptions );
	ei->textlabel_Destroy( root.tlOptions );
	ei->button_Destroy( root.buttonExit );
	ei->textlabel_Destroy( root.tlExit );
	memset( &root, 0, sizeof(menu_root_t) );
	
	bInited = 0;
}

/* ------------
root_SinglePlayerEvent
------------ */
static void root_SinglePlayerEvent( void *data, uint8_t key, uint8_t key_state )
{
	widget_button_t *button;

	button = (widget_button_t*)data;
	if( !button ) return;
	if( key == M_LEFT && key_state == 0 )
	{
		tree_Descend( ROOT_SINGLE_PLAYER );
	}
}

/* ------------
root_MultiPlayerEvent
------------ */
static void root_MultiPlayerEvent( void *data, uint8_t key, uint8_t key_state )
{
	widget_button_t *button = (widget_button_t*)data;
	if( !button ) return;
	if( key == M_LEFT && key_state == 0 )
	{
		tree_Descend( ROOT_MULTI_PLAYER );
	}
}

/* ------------
root_OptionsEvent
------------ */
static void root_OptionsEvent( void *data, uint8_t key, uint8_t key_state )
{
	widget_button_t *button = (widget_button_t*)data;
	if( !button ) return;
	if( key == M_LEFT && key_state == 0 )
	{
		tree_Descend( ROOT_OPTIONS );
	}
}

/* ------------
root_ExitEvent
------------ */
static void root_ExitEvent( void *data, uint8_t key, uint8_t key_state )
{
	if( key == M_LEFT && key_state == 0 )
	{
		tree_Ascend();
	}
}
