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
// sal_input_win_win32.c
// Created by Terrence Cole

// Includes
/////////////
#include "cog_global.h"
#include "sal_input_win_win32.h"

// Definitions
////////////////
#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif 

// Local Structures
/////////////////////
typedef struct
{
	char name[16];
	byte ascii;
}
inKeyMap_t;

// Global Prototypes
//////////////////////
/*
int in_w32_Initialize(void);
void in_w32_Terminate(void);
void in_w32_ProcessEvents();
void in_w32_Event( byte key, unsigned short down ); 
byte in_w32_MapKey( int key );
void in_w32_HideMouse(void);
void in_w32_UnhideMouse(void);
*/

// Local Prototypes
/////////////////////

// Local Variables
////////////////////
static inKeyMap_t in_KeyMap[] =  {
	{ "", 0 },
	{ "Escape", K_ESCAPE },
	{ "1", '1' },
	{ "2", '2' },
	{ "3", '3' },
	{ "4", '4' }, //5
	{ "5", '5' },
	{ "6", '6' },
	{ "7", '7' },
	{ "8", '8' },
	{ "9", '9' },
	{ "0", '0' },
	{ "-", '-' },
	{ "=", '=' },
	{ "Backspace", K_BACKSPACE }, 
	{ "Tab", K_TAB }, //15 
	{ "q", 'q' },
	{ "w", 'w' },
	{ "e", 'e' },
	{ "r", 'r' },
	{ "t", 't' },
	{ "y", 'y' },
	{ "u", 'u' },
	{ "i", 'i' },
	{ "o", 'o' },
	{ "p", 'p' }, // 25
	{ "[", '[' },
	{ "]", ']' },
	{ "Enter", K_ENTER },  // KP_ENTER (28)
	{ "Left Ctrl", K_LCTRL },  // RIGHT CTRL (29)
	{ "a", 'a' },
	{ "s", 's' },
	{ "d", 'd' },
	{ "f", 'f' },
	{ "g", 'g' },
	{ "h", 'h' },// 35
	{ "j", 'j' },
	{ "k", 'k' },
	{ "l", 'l' },
	{ ";", ';' },
	{ "'", '\'' },
	{ "`", '`' }, 
	{ "Left Shift", K_LSHIFT },  // Fake Left SHIFT (42)
	{ "\\", '\\' },
	{ "z", 'z' },
	{ "x", 'x' }, // 45
	{ "c", 'c' },
	{ "v", 'v' },
	{ "b", 'b' },
	{ "n", 'n' },
	{ "m", 'm' },
	{ ",", ',' },
	{ ".", '.' },
	{ "/", '/' }, // KP_DIVIDE (53)
	{ "Right Shift", K_RSHIFT }, // Fake Right Shift (54)
	{ "Keypad *", KP_MULTIPLY }, // 55  -> Prt Scrn
	{ "Left Alt", K_LALT }, // RIGHT ALT
	{ "Space", K_SPACE }, 
	{ "Caps Lock", K_CAPSLOCK }, // no bind 
	{ "F1", K_F1 },
	{ "F2", K_F2 },
	{ "F3", K_F3 },
	{ "F4", K_F4 },
	{ "F5", K_F5 },
	{ "F6", K_F6 },
	{ "F7", K_F7 }, // 65
	{ "F8", K_F8 },
	{ "F9", K_F9 },
	{ "F10", K_F10 },
	{ "NumLock", KP_NUMLOCK },
	{ "ScrollLock", K_SCROLLLOCK }, // no bind 
	{ "Keypad 7", KP_7 }, // -> HOME
	{ "Keypad 8", KP_8 }, // -> UP
	{ "Keypad 9", KP_9 }, // -> PGUP
	{ "Keypad -", KP_MINUS }, // no bind
	{ "Keypad 4", KP_4 }, //75 -> LEFT
	{ "Keypad 5", KP_5 }, // no bind
	{ "Keypad 6", KP_6 }, // -> RIGHT
	{ "Keypad +", KP_PLUS }, // no bind
	{ "Keypad 1", KP_1 }, // -> END
	{ "Keypad 2", KP_2 }, // -> DOWN
	{ "Keypad 3", KP_3 }, // -> PGDN
	{ "Keypad 0", KP_0 }, // -> INS
	{ "Keypad .", KP_PERIOD }, // -> DELETE
	{ "", 0 }, // alt+sysreq
	{ "", 0 }, // 85
	{ "", 0 }, // win key?
	{ "F11", 133 }, // 87
	{ "F12", 134 }, // 88

	// extended keymaps
/*28*/	{ "KP Enter", KP_ENTER }, // 89
/*29*/	{ "Right Ctrl", K_RCTRL }, // 90
/*55*/	{ "Print Screen", K_PRINTSCREEN }, // 91
/*56*/    { "Right Alt", K_RALT }, // 92
/*69*/	{ "", 0 }, // 93
/*71*/    { "Home", K_HOME }, // 94
/*72*/	{ "Up", K_UP }, // 95
/*73*/	{ "Page Up", K_PGUP }, // 96
/*75*/	{ "Left", K_LEFT }, // 97
/*77*/	{ "Right", K_RIGHT }, // 98
/*79*/	{ "End", K_END }, // 99
/*80*/	{ "Down", K_DOWN }, // 100
/*81*/	{ "Page Down", K_PGDN }, // 101
/*82*/	{ "Insert", K_INSERT }, // 102
/*83*/	{ "Delete", K_DELETE }, // 103
/*53*/	{ "Keypad /", KP_DIVIDE } // 104
};

static byte w32Active = 0;
static varlatch vlmInvertX = NULL;
static varlatch vlmInvertY = NULL;

// *********** FUNCTIONALITY ***********
/* ------------
in_w32_Initialize
------------ */
int in_w32_Initialize(void)
{
	w32Active = 1;

	// setup mouse stuff
	state.mAxis[0] = state.mAxis[1] = 0;
	state.mAxisPrev[0] = state.mAxisPrev[1] = 0;
	state.mAxisDelta[0] = state.mAxisDelta[1] = 0;

	// latch to the mouse inversion variables
	vlmInvertX = var_GetVarLatch( "mInvertX" );
	if( vlmInvertX == NULL )
	{
		con_Print( "<RED>Direct Input Init Failed:	Unable to latch to variable \"mInvertX\"." );
		return 0;
	}
	vlmInvertY = var_GetVarLatch( "mInvertY" );
	if( vlmInvertY == NULL )
	{
		con_Print( "<RED>Direct Input Init Failed:	Unable to latch to variable \"mInvertY\"." );
		return 0;
	}	

	return 1;
}

/* ------------
in_w32_TGerminate
------------ */
void in_w32_Terminate(void)
{
	w32Active = 0;

	// reset states
	state.mAxis[0] = state.mAxis[1] = 0;
	state.mAxisPrev[0] = state.mAxisPrev[1] = 0;
	state.mAxisDelta[0] = state.mAxisDelta[1] = 0;

	// remove the mouse capture
	ReleaseCapture();
}

/* ------------
in_w32_ProcessEvents
------------ */
void in_w32_ProcessEvents()
{
	// reset the axis input
	memset( state.mAxisDelta, 0, sizeof(short) * 2 );
}

/* ------------
in_w32_Event
------------ */
void in_w32_Event( byte key, unsigned short down )
{
	// we should only respond to events if this module is active
	// otherwise, we will respond a second time in DirectInput
	if( !w32Active ) return;

	// FIXME:  add support for different window states
//	if(  )
//	{
//		sys_handles.cMouseCap = sys_handles.cWnd;
//		SetCapture( sys_handles.cWnd );
//	}

	// check for axis movements
	if( key == M_XAXIS )
	{
		if( (int)var_GetFloat( vlmInvertX ) > 0 ) 
		{
			state.mAxisPrev[YAW] = state.mAxis[YAW];
			state.mAxis[YAW] = down;
			state.mAxisDelta[YAW] = state.mAxis[YAW] + state.mAxisPrev[YAW];
		}
		else
		{
			state.mAxisPrev[YAW] = state.mAxis[YAW];
			state.mAxis[YAW] = down;
			state.mAxisDelta[YAW] = state.mAxis[YAW] - state.mAxisPrev[YAW];
		}

		mouse_event( MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, 32768, 32768, 0, 0 );
		return;
	}
	if( key == M_YAXIS )
	{
		if( (int)var_GetFloat( vlmInvertY ) > 0 ) 
		{
			state.mAxisPrev[PITCH] = state.mAxis[PITCH];
			state.mAxis[PITCH] = down;
			state.mAxisDelta[PITCH] = state.mAxis[PITCH] + state.mAxisPrev[PITCH];
		}
		else
		{
			state.mAxisPrev[PITCH] = state.mAxis[PITCH];
			state.mAxis[PITCH] = down;
			state.mAxisDelta[PITCH] = state.mAxis[PITCH] - state.mAxisPrev[PITCH];
		}
		
		mouse_event( MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, 32768, 32768, 0, 0 );
		return;
	}
	if( key == M_WHEEL )
	{
		state.mWheel = state.mWheel + (short)down;
		if( state.mWheel <= -(WHEEL_DELTA) )
		{
			state.mWheel = 0;
			bind_TranslateKeyPress( M_WHEELDOWN, 1 );
		}
		else if( state.mWheel >= WHEEL_DELTA )
		{
			state.mWheel = 0;
			bind_TranslateKeyPress( M_WHEELUP, 1 );
		}

		return;
	}
	
	bind_TranslateKeyPress( key, (byte)down );
}

/* ------------
in_MapKey
------------ */
byte in_w32_MapKey( int key )
{
	byte extended = (byte)((key>>24)&1);
	byte scancode = (byte)((key>>16)&255);
	
	if( extended )
	{
		if( scancode == 28 ) { scancode = 89; }
		if( scancode == 29 ) { scancode = 90; }
		if( scancode == 55 ) { scancode = 91; }
		if( scancode == 56 ) { scancode = 92; }
		if( scancode == 71 ) { scancode = 94; }
		if( scancode == 72 ) { scancode = 95; }
		if( scancode == 73 ) { scancode = 96; }
		if( scancode == 75 ) { scancode = 97; }
		if( scancode == 77 ) { scancode = 98; }
		if( scancode == 79 ) { scancode = 99; }
		if( scancode == 80 ) { scancode = 100; }
		if( scancode == 81 ) { scancode = 101; }
		if( scancode == 82 ) { scancode = 102; }
		if( scancode == 83 ) { scancode = 103; }
		if( scancode == 53 ) { scancode = 104; }
	}
	return in_KeyMap[scancode].ascii;
}

/* ------------
in_HideMouse
------------ */
void in_w32_HideMouse(void)
{
  	while( ShowCursor( FALSE ) >= 0 ) ;
}

/* ------------
in_ShowMouse
------------ */
void in_w32_UnhideMouse(void)
{
    while( ShowCursor( TRUE ) < 0 ) ;
}
