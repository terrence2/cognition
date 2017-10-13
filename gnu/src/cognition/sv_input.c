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
// sv_input.c
// Created  by Terrence Cole

// Includes
/////////////
#include "cog_global.h"
#include "sal_input_win_di.h"
#include "sal_input_win_win32.h"
#include "sal_input_sdl.h"

// Definitions
////////////////
#define IN_MODE_NONE 0
#define IN_MODE_DINPUT 1
#define IN_MODE_WIN32 2
#define IN_MODE_SDL 3

// Local Structures
/////////////////////

// Global Prototypes
//////////////////////
/*
int in_Initialize(void);
void in_Terminate(void);
void in_ProcessEvents();
void in_HideMouse();
void in_UnhideMouse();
*/

// Local Prototypes
/////////////////////

// Local Variables
////////////////////
static varlatch vlMode = NULL;
static int iMode = IN_MODE_NONE;

// *********** FUNCTIONALITY ***********
/* ------------
in_Initialize - opens the requested input device 
			  - export work to another module based on the value of input_mode
------------ */
int in_Initialize(void)
{
	double tStart, tEnd;
	
	con_Print( "\n<BLUE>Initializing</BLUE> Input." );
	eng_LoadingFrame();
	tStart = ts_GetSaneTime();

	// clear the axises
	memset( state.mAxis, 0, sizeof(short) * 2 );
	memset( state.mAxisPrev, 0, sizeof(short) * 2 );
	memset( state.mAxisDelta, 0, sizeof(short) * 2 );
	state.mWheel = 0;

	// get the mode for dispatching functionality
	vlMode = var_GetVarLatch( "input_mode" );
	if( vlMode == NULL )
	{
		con_Print( "Input Init:  Get Var Latch for \"input_mode\" failed." );
		return 0;
	}

	// let the platform deal with rejecting non-extant modes
	if( tcstricmp(var_GetString(vlMode),"auto") ) {
		con_Print( "\t<BLUE>Using</BLUE> <PURPLE>auto</PURPLE> mode query." );
		con_Print( "\t\t<RED>Trying</RED> <PURPLE>direct input</PURPLE>" );
		if( !in_di_Initialize() ) {
			con_Print( "\t\t<RED>Trying</RED> <PURPLE>sdl</PURPLE>" );
			if( !in_sdl_Initialize() ) {
				con_Print( "\t\t<RED>Trying</RED> <PURPLE>win32</PURPLE>" );
				if( !in_w32_Initialize() ) {
					con_Print( "Input Init:  Could not open an input device." );
					return 0;
				} else {
					con_Print( "\t\t<GREEN>Using</GREEN> <PURPLE>win32</PURPLE>" );
					iMode = IN_MODE_WIN32;
				}
			} else {
				con_Print( "\t\t<GREEN>Using</GREEN> <PURPLE>sdl</PURPLE>" );
				iMode = IN_MODE_SDL;
			}
		} else {
			con_Print( "\t\t<GREEN>Using</GREEN> <PURPLE>direct input</PURPLE>" );
			iMode = IN_MODE_DINPUT;
		}
	}
	else if( tcstricmp(var_GetString(vlMode),"sdl") ) {
		con_Print( "\t<BLUE>Using</BLUE> <PURPLE>sdl</PURPLE> mode input." );
		if( !in_sdl_Initialize() ) {
			con_Print( "Input Init:  Could not open an input device: mode SDL." );
			return 0;
		}
		iMode = IN_MODE_SDL;
	}
	else if( tcstricmp(var_GetString(vlMode),"direct_input") ) {
		con_Print( "\t<BLUE>Using</BLUE> <PURPLE>direct input</PURPLE> mode input." );
		if( !in_di_Initialize() ) {
			con_Print( "Input Init:  Could not open an input device: mode DirectInput." );
			return 0;
		}
		iMode = IN_MODE_DINPUT;
	}
	else if( tcstricmp(var_GetString(vlMode),"win32") ) {
		con_Print( "\t<BLUE>Using</BLUE> <PURPLE>win32</PURPLE> mode input." );
		if( !in_w32_Initialize() ) {
			con_Print( "Input Init:  Could not open an input device: mode WIN32." );
			return 0;
		}
		iMode = IN_MODE_SDL;
	}
	else {
		con_Print( "\t<RED>Input Init:  Could not open an input device: unrecognized mode string</RED>" );
		iMode = IN_MODE_NONE;
	}
    
	tEnd = ts_GetSaneTime();
	con_Print( "\tDone:  %.3f sec", tEnd - tStart );
	eng_LoadingFrame();

	return 1;
}


/* ------------
in_Terminate - closes input devices
------------ */
void in_Terminate(void)
{
	// if iMode is not set, the system is not started
	if( iMode == IN_MODE_NONE ) return;

	con_Print( "\nTerminating Input System." );

	switch( iMode )
	{
	case IN_MODE_DINPUT:
		con_Print( "\tClosing Direct Input." );
		in_di_Terminate();
		break;
	case IN_MODE_WIN32:
		con_Print( "\tClosing Win32 Input." );
		in_w32_Terminate();
		break;
	case IN_MODE_SDL:
		con_Print( "\tClosing SDL Input." );
		in_sdl_Terminate();
		break;
	default:
		/* not inited yet */
		break;
	}
}

/* ------------
in_ProcessEvents - dispatches input events to the bind dereferencer
------------ */
void in_ProcessEvents()
{
	switch( iMode )
	{
	case IN_MODE_DINPUT:
		in_di_ProcessEvents();
		break;
	case IN_MODE_WIN32:
		// events under Win32 are routed from the WinProc
		// this function does by frame maintenence
		in_w32_ProcessEvents();
		break;
    case IN_MODE_SDL:
        in_sdl_ProcessEvents();
        break;
	default:
        /* not inited yet */
        break;
	}
}

/* ------------
in_HideMouse - hides the mouse
------------ */
void in_HideMouse()
{
con_Print( "CALLED IN_HIDEMOUSE" );
    switch( iMode )
	{
	case IN_MODE_DINPUT:
	case IN_MODE_WIN32:
con_Print( "DOING IN_HIDEMOUSE WIN32" );
		in_w32_HideMouse();
		break;
	case IN_MODE_SDL:
con_Print( "DOING IN_HIDEMOUSE SDL" );
		in_sdl_HideMouse();
		break;
	default:
con_Print( "DOING IN_HIDEMOUSE NONE" );
		/* not inited yet */
		break;
	}
}

/* ------------
in_UnhideMouse - makes the mouse visible
------------ */
void in_UnhideMouse()
{
con_Print( "CALLED IN_UNHIDEMOUSE" );
	switch( iMode )
	{
	case IN_MODE_DINPUT:
	case IN_MODE_WIN32:
con_Print( "DOING IN_UNHIDEMOUSE WIN32" );
		in_w32_UnhideMouse();
		break;
	case IN_MODE_SDL:
con_Print( "DOING IN_UNHIDEMOUSE SDL" );
		in_sdl_UnhideMouse();
		break;
	default:
con_Print( "DOING IN_UNHIDEMOUSE NONE" );
		/* not inited yet */
		break;
	}
}
