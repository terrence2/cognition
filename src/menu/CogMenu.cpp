// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

// Cognition
// CogMenu.cpp
// Created by Terrence Cole

// Includes
/////////////
#include "CogMenu.h"
#include <assert.h>
#include <GL/gl.h>

// Definitions
////////////////

// Local Structures
/////////////////////

// Local Prototypes
/////////////////////
extern "C" menu_interface_t *GetMenuAPI( engine_interface_t *engine_interface );
static int menu_Initialize(void);
static void menu_Terminate(void);
static void menu_KeyPress( byte key, byte key_state );
static void menu_Frame( exec_state_t *state );
static void menu_Draw( exec_state_t *state );

// Local Variables
////////////////////
typedef struct menu_preload_list_s
{
	// titles
	colormap_t *menu_title; // "menu_title24" and "menu_title8"

	// button
	colormap_t *buttonUp; // "buttonUp24" and "buttonUp8"
	colormap_t *buttonDown; // "buttonDown24" and "buttonUp8"
	colormap_t *buttonOver; // "buttonOver24" and "buttonUp8"
	colormap_t *buttonDisabled; // "buttonDisabled24" and "buttonUp8"
	sound_t *buttonSound; // button.wav

	// check button
	colormap_t *checkButtonInner; // "checkButtonInner24" and "checkButtonInner8"
	colormap_t *checkButtonUp; // "checkButtonUp24" and "checkButton8"
	colormap_t *checkButtonOver; // "checkButtonOver24" and "checkButton8"
	colormap_t *checkButtonDown; // "checkButtonDown24" and "checkButton8"
	colormap_t *checkButtonDisabled; // "checkButtonDisabled24" and "checkButton8"

	// radio button
	colormap_t *radioButtonUp; // "radioButtonUp24" and "radioButton8"
	colormap_t *radioButtonDown; // "radioButtonDown24" and "radioButton8"
	colormap_t *radioButtonOver; // "radioButtonOver24" and "radioButton8"
	colormap_t *radioButtonDisabled; // "radioButtonDisabled24" and "radioButton8"
	colormap_t *radioButtonInner; // "radioButtonInner24" and "radioButtonInner8"

	// text edit
	colormap_t *teCursor; // "teCursor24" and "teCursor8"
	colormap_t *teBackground; // "teBackground24" and "teBackground8"
	colormap_t *teSelection; // "teSelection24" and "teSelection8"

	// slider
	colormap_t *sliderBar; // "sliderBar24" and "sliderBar8"
	colormap_t *sliderPos; // "sliderPos24" and "sliderPos8"
	colormap_t *sliderMark; // "sliderMark24" and "sliderMark8"
} menu_preload_list_t;

static menu_preload_list_t preload;
static CogMenu *menu = NULL;
static engine_interface_t *engVPtr = NULL;
static menu_interface_t menu_interface = 
{
	API_VERSION,		//	int apiversion;
	menu_Initialize,	//	int (*menu_Initialize)(void);
	menu_Terminate,		//	void (*menu_Terminate)(void);
	menu_KeyPress,		//	void (*menu_KeyPress)( byte key, byte key_state );
	menu_Frame,		//	void (*menu_Frame)( exec_state_t *state );	
	menu_Draw,			//	void (*menu_Draw)( exec_state_t *state );
};

// *********** FUNCTIONALITY ***********
/* ------------
ei()
// interface accessor
------------ */
inline engine_interface_t *ei() { return engVPtr; }

/* ------------
SetCurrentScreen
------------ */
void SetCurrentScreen( Screen *scr )
{
	if( menu ) menu->SetCurrentScreen( scr );
}

/* ------------
The Runtime Interfacer
------------ */
extern "C" menu_interface_t *GetMenuAPI( engine_interface_t *engine_interface )
{
	// get the engine interface
	assert( engine_interface );
	if( !engine_interface ) return NULL;
	engVPtr = engine_interface;

	// return the menu interface
	return &menu_interface;
}

/* ------------
menu_Initalize
// create the menu system and prepare for framing
------------ */
static int menu_Initialize(void)
{
	// setup the new parameters
	colormap_params_t cmParams;
	cmParams.bMipmap = 1; // (look pretty at all resolutions / scales)
	cmParams.bNoRescale = 0; // allow the engine to save texture memory
	cmParams.iEnv = GL_MODULATE; // standard blending
	cmParams.iFilter = ei()->cm_GetGlobalFilter(); // user level effects choice 
	cmParams.iScaleBias = 0; // no default biasing on rescale
	cmParams.iWrap = GL_CLAMP; // these are generally being used as labels
	
	preload.menu_title = ei()->cm_LoadFromFiles( "menu_title24_WITH_menu_title8", "menu_title24", "menu_title8", &cmParams ); // "menu_title24", "menu_title8"
	preload.buttonUp = ei()->cm_LoadFromFiles( "buttonUp24_WITH_buttonUp8", "buttonUp24", "buttonUp8", &cmParams ); // "buttonUp24", "buttonUp8"
	preload.buttonDown = ei()->cm_LoadFromFiles( "buttonDown24_WITH_buttonUp8", "buttonDown24", "buttonUp8", &cmParams ); // "buttonDown24", "buttonUp8"
	preload.buttonOver = ei()->cm_LoadFromFiles( "buttonOver24_WITH_buttonUp8", "buttonOver24", "buttonUp8" , &cmParams ); // "buttonOver24", "buttonUp8"
	preload.buttonDisabled = ei()->cm_LoadFromFiles( "buttonDisabled24_WITH_buttonUp8", "buttonDisabled24", "buttonUp8", &cmParams ); // "buttonDisabled24", "buttonUp8"
	// preload.buttonSound; // button.wav

	// check button
	preload.checkButtonInner = ei()->cm_LoadFromFiles( "checkButtonInner24_WITH_checkButtonInner8", "checkButtonInner24", "checkButtonInner8", &cmParams ); // "checkButtonInner24", "checkButtonInner8"
	preload.checkButtonUp = ei()->cm_LoadFromFiles( "checkButtonUp24_WITH_checkButton8", "checkButtonUp24", "checkButton8", &cmParams ); // "checkButtonUp24", "checkButton8"
	preload.checkButtonOver = ei()->cm_LoadFromFiles( "checkButtonOver24_WITH_checkButton8", "checkButtonOver24", "checkButton8", &cmParams ); // "checkButtonOver24", "checkButton8"
	preload.checkButtonDown = ei()->cm_LoadFromFiles( "checkButtonDown24_WITH_checkButton8", "checkButtonDown24", "checkButton8", &cmParams ); // "checkButtonDown24", "checkButton8"
	preload.checkButtonDisabled = ei()->cm_LoadFromFiles( "checkButtonDisabled24_WITH_checkButton8", "checkButtonDisabled24", "checkButton8" , &cmParams ); // "checkButtonDisabled24", "checkButton8"

	// radio button
	preload.radioButtonUp = ei()->cm_LoadFromFiles( "radioButtonUp24_WITH_radioButton8", "radioButtonUp24", "radioButton8", &cmParams ); // "radioButtonUp24", "radioButton8"
	preload.radioButtonDown = ei()->cm_LoadFromFiles( "radioButtonDown24_WITH_radioButton8", "radioButtonDown24", "radioButton8", &cmParams ); // "radioButtonDown24", "radioButton8"
	preload.radioButtonOver = ei()->cm_LoadFromFiles( "radioButtonOver24_WITH_radioButton8", "radioButtonOver24", "radioButton8", &cmParams ); // "radioButtonOver24", "radioButton8"
	preload.radioButtonDisabled = ei()->cm_LoadFromFiles( "radioButtonDisabled24_WITH_radioButton8", "radioButtonDisabled24", "radioButton8", &cmParams ); // "radioButtonDisabled24", "radioButton8"
	preload.radioButtonInner = ei()->cm_LoadFromFiles( "radioButtonInner24_WITH_radioButtonInner8", "radioButtonInner24", "radioButtonInner8", &cmParams ); // "radioButtonInner24", "radioButtonInner8"

	// text edit
	preload.teCursor = ei()->cm_LoadFromFiles( "teCursor24_WITH_teCursor8", "teCursor24", "teCursor8", &cmParams ); // "teCursor24", "teCursor8"
	preload.teBackground = ei()->cm_LoadFromFiles( "teBackground24_WITH_teBackground8", "teBackground24", "teBackground8", &cmParams ); // "teBackground24", "teBackground8"
	preload.teSelection = ei()->cm_LoadFromFiles( "teSelection24_WITH_teSelection8", "teSelection24", "teSelection8", &cmParams ); // "teSelection24", "teSelection8"

	// slider
	preload.sliderBar = ei()->cm_LoadFromFiles( "sliderBar24_WITH_sliderBar8", "sliderBar24", "sliderBar8", &cmParams ); // "sliderBar24", "sliderBar8"
	preload.sliderPos = ei()->cm_LoadFromFiles( "sliderPos24_WITH_sliderPos8", "sliderPos24", "sliderPos8", &cmParams ); // "sliderPos24", "sliderPos8"
	preload.sliderMark = ei()->cm_LoadFromFiles( "sliderMark24_WITH_sliderMark8", "sliderMark24", "sliderMark8", &cmParams ); // "sliderMark24", "sliderMark8"
	
	// create the menu
	menu = new CogMenu();
	if( !menu ) return 0;

	// tell us about it
	ei()->con_Print( "\tMenu Init Complete." );

	return 1;
}

/* ------------
menu_Terminate
// free all active memory and prepare for unloading
------------ */
static void menu_Terminate(void)
{
	if( menu ) delete menu;
}

/* ------------
menu_KeyPress
// dispatcher for key events calls
------------ */
static void menu_KeyPress( byte key, byte key_state )
{
	menu->KeyPress( key, key_state );
}

/* ------------
menu_Animate
// dispatcher for framing calls
------------ */
static void menu_Frame( exec_state_t *state )
{
	menu->Frame( state );
}

/* ------------
menu_Draw
// dispatcher for drawing calls
------------ */
static void menu_Draw( exec_state_t *state )
{
	menu->Draw( state );
}

/* ------------
Constructor
------------ */
CogMenu::CogMenu()
{
	m_RootScreen = new RootScreen();
	m_CurScreen = m_RootScreen;
	ei()->ms_Show();
}

/* ------------
Destructor
------------ */
CogMenu::~CogMenu()
{
	SAFE_DELETE( m_RootScreen );
}

/* ------------
Frame
// Toplevel animation handler
------------ */
void CogMenu::Frame( exec_state_t *state )
{
	// check and dispatch mouse movement
	if( state->mAxisDelta[X_AXIS] || state->mAxisDelta[Y_AXIS] )
	{
		int x, y;
		ei()->ms_GetPosition( &x, &y );
		m_CurScreen->MouseMoveEvent( x, y );
	}

	// dispatch frame calls to the current screen
	m_CurScreen->Frame( state );
}

/* ------------
Draw
// draw everything
------------ */
void CogMenu::Draw( exec_state_t *state )
{
	m_CurScreen->Draw();
}

/* ------------
KeyPress
// Toplevel Key Event Handler
------------ */
void CogMenu::KeyPress( byte key, byte key_state )
{
	int x, y;

	// divide up events into proper categories
	if( key_state )
	{
		// check the event type
		if( key >= M_EVENTS_LOW && key <= M_EVENTS_HIGH )
		{
			ei()->ms_GetPosition( &x, &y );
			m_CurScreen->MouseDownEvent( key, x, y );
		}
		else
		{
			m_CurScreen->KeyDownEvent( key );
		}
	}
	else
	{
		// check the event type
		if( key >= M_EVENTS_LOW && key <= M_EVENTS_HIGH )
		{
			ei()->ms_GetPosition( &x, &y );
			m_CurScreen->MouseUpEvent( key, x, y );
		}
		else
		{
			m_CurScreen->KeyUpEvent( key );
		}
	}
}
