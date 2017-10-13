// Cognition
// menu
// menu_main.c
// by Terrence Cole created 4/9/02

// Includes
/////////////
#include "menu_global.h"

// Definitions
////////////////

// Structures
///////////////


// Global Prototypes
//////////////////////
/*
menu_interface_t *GetMenuAPI( engine_interface_t *engine_interface );
int menu_Initialize(void);
void menu_Terminate(void);
void menu_KeyPress( byte key, byte key_state );
void menu_Animate( exec_state_t *state );
menu_t *menu_LoadMenu( unsigned int ident );
*/

// Global Variables
/////////////////////
/*
menu_interface_t mi;
engine_interface_t *ei;
*/

// Local Prototypes
/////////////////////
static void menu_UnloadCurrentMenu(void);

// static void menu_UnloadMenu( menu_t *menu );

// Local Variables
////////////////////
// static menu_t *menu_current = NULL;

// *********** FUNCTIONALITY ***********
/* ------------
GetMenuAPI
------------ */
menu_interface_t *GetMenuAPI( engine_interface_t *engine_interface )
{
	// capture the engine api
	ei = engine_interface;

	// prepare the menu api
	mi.apiversion = API_VERSION;
	mi.menu_Initialize = menu_Initialize;
	mi.menu_Terminate = menu_Terminate;
	mi.menu_KeyPress = menu_KeyPress;
	mi.menu_Animate = menu_Animate;

	return &mi;
}

/* ------------
menu_Initialize
------------ */
int menu_Initialize(void)
{
	ei->con_Print( "\t<BLUE>Initializing</BLUE> Menu Module..." );

	// init misc stuff
	m_Initialize();

	// load the background
	if( !bg_LoadMenuBG() )
	{
		ei->con_Print( "\n<RED>Menu Initialization Failed:  Unable to load the menu background animations." );
		return 0;
	}

	if( !tree_Initialize() )
	{
		ei->con_Print( "\n<RED>Menu Initialization Failed: could not create menu traversal tree." ) ;
		return 0;
	}

	return 1;
}

/* ------------
menu_Terminate
------------ */
void menu_Terminate(void)
{
	ei->con_Print( "\t<RED>Terminating</RED> Menu Module..." );
	
	// unload misc stuff
	m_Terminate();

	// unload the background
	bg_UnloadMenuBG();

	// unload the traversal parameters
	tree_Terminate();
}


static vec3 vLoc;

/* ------------
menu_Animate - advances the menu_animations by 1 step
------------ */
void menu_Animate( exec_state_t *state )
{
	bg_Animate( state );
	vLoc[0] = (float)state->mAxis[0];
	vLoc[1] = (float)state->mAxis[1];
//	cur_Update( state );
}

/* ------------
menu_KeyPress
------------ */
void menu_KeyPress( byte key, byte key_state )
{
/*
	static int iTmp1 = 0;
	static int iTmp3 = 0;
	static int iTmp2 = 0;
	
	if( key_state && key == M_LEFT ) 
	{
		iTmp1 = ei->s_PlaySound( sndBind[0].snd, 1, &vLoc, NULL );
	}
	if( !key_state && key == M_LEFT )
	{
		ei->s_StopSound( iTmp1 );
	}

	if( key_state && key == M_RIGHT ) 
	{
		iTmp2 = ei->s_PlaySound( sndBind[1].snd, 1, &vLoc, NULL );
	}
	if( !key_state && key == M_RIGHT )
	{
		ei->s_StopSound( iTmp2 );
	}
	
	if( key_state && key == M_MIDDLE ) 
	{
		iTmp3 = ei->s_PlaySound( sndBind[2].snd, 1, &vLoc, NULL );
	}
	if( !key_state && key == M_MIDDLE )
	{
		ei->s_StopSound( iTmp3 );
	}

	if( key_state && key == K_RSHIFT ) 
	{
		iTmp3 = ei->s_PlaySound( sndBind[4].snd, 1, &vLoc, NULL );
	}
	if( !key_state && key == K_RSHIFT )
	{
		ei->s_StopSound( iTmp3 );
	}

	if( key_state && key == K_SPACE ) 
	{
		iTmp3 = ei->s_PlaySound( sndBind[3].snd, 1, &vLoc, NULL );
	}
	if( !key_state && key == K_SPACE )
	{
		ei->s_StopSound( iTmp3 );
	}
	*/
	if( key == K_ESCAPE && key_state == 0 )
	{
		tree_Ascend();
	}
	else if( key == K_ENTER && key_state == 0 )
	{
		tree_Descend( 0 );
	}
	
/*	// retreat one level
	if( (key == K_ESCAPE) && (key_state == 1) )
	{
		if( menu_current != NULL )
		{
			menu_LoadMenu( menu_current->prev_menu );
		}
		return;
	}

	// advance one level
	if( (key == K_ENTER) && (key_state == 1) )
	{
		if( menu_current != NULL )
		{
			menu_LoadMenu( menu_current->default_item->next_menu );
		}
		return;
	}*/
}

/* ------------
menu_LoadMenu
------------ */
/*menu_t *menu_LoadMenu( unsigned int ident )
{
	menu_UnloadCurrentMenu();

	switch( ident )
	{
	case MENU_ROOT:
		menu_current = root_LoadMenu();
		break;
	case MENU_EXIT:
		ei->sys_PostQuit(0);
		break;
	default:
		ei->con_Print( "Unrecognized menu identifier." );
		break;
	}

	return menu_current;
}
*/

/* ------------
menu_UnloadCurrentMenu
------------ */
static void menu_UnloadCurrentMenu(void)
{
//	menu_UnloadMenu( menu_current );
//	menu_current = NULL;
}

