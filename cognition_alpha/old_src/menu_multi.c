// Cognition
// menu_multi.c
// Created  by Terrence Cole 10/30/03

// Includes
/////////////
#include "menu_global.h"

// Definitions
////////////////

// Local Structures
/////////////////////
typedef struct menu_multi_s
{
	widget_textlabel_t *tlTitle;

} menu_multi_t;

// Global Prototypes
//////////////////////
/*
int multi_Initialize(void);
void multi_Terminate(void);
*/

// Local Prototypes
/////////////////////
static void multi_BackEvent( void *data, uint8_t key, uint8_t key_state );

// Local Variables
////////////////////
static menu_multi_t menu;
static int bInited = 0;

// *********** FUNCTIONALITY ***********
/* ------------
multi_Initialize
------------ */
int multi_Initialize(void)
{
	if( bInited ) return 1;

	ei->con_Print( "\n<BLUE>Initializing</BLUE> MultiPlayer Menu..." );

	menu.tlTitle = ei->textlabel_Create( ei->win_GetRoot(), 25, 675, "MultiPlayer", 20 );
	
	bInited = 1;
	return 1;
}

/* ------------
multi_Terminate
------------ */
void multi_Terminate(void)
{
	if( !bInited ) return;

	ei->con_Print( "\n<RED>Terminating</RED> MultiPlayer Menu..." );

	ei->textlabel_Destroy( menu.tlTitle );

	memset( &menu, 0, sizeof(menu_multi_t) );
	bInited = 0;
}

/* ------------
multi_BackEvent
------------ */
static void multi_BackEvent( void *data, uint8_t key, uint8_t key_state )
{
	widget_button_t *button = (widget_button_t*)data;
	if( !button ) return;
	if( key == M_LEFT && key_state == 0 )
	{
		tree_Descend( SINGLE_BACK );
	}
}
