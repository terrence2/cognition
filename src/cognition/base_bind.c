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
// bind.c
// by Terrence Cole 12/12/01

// Includes
/////////////
#include "cog_global.h"
#include "sv_console.h"

// Definitions
////////////////
#define NUM_KEYS 104
#define COM_STR_SIZE 32

// Structures
///////////////
typedef struct
{
	byte key;  // the rational key
	char *text;
	char downCom[COM_STR_SIZE + 1];
	char upCom[COM_STR_SIZE + 1];
	int downcount;
} keybind_t;

// Global Prototypes
//////////////////////
/*
int bind_Initialize(void);
void bind_Terminate(void);
void bind_TranslateKeyPress( byte key, byte key_state );
int bind_ShiftIsDown();
int bind_AltIsDown();
int bind_CtrlIsDown();
void bind_BindKey( char *buffer );
void bind_SetKeyBind( char *key, char *command );
void bind_UnbindKey( char *buffer );
void bind_UnbindAll();
void bind_FlushKeyStates(void);
*/

// Local Prototypes
/////////////////////

// Local Variables
////////////////////
static keybind_t bindlist[] = {
	{ M_LEFT,			"LeftMouseButton", 	"uncover",		"",				0	},
	{ M_RIGHT,			"RightMouseButton", "flag",			"",				0	},
	{ M_MIDDLE,			"MiddleMouseButton", "",				"",				0	},
	{ M_WHEELDOWN,		"MouseWheelDown", "",				"",				0	},
	{ M_WHEELUP,		"MouseWheelUp",	"",				"",				0	},
	{ K_ESCAPE,			"escape",			"",			"",				0	},
	{ K_F1,				"F1",				"",				"",				0	},
	{ K_F2,				"F2",				"",				"",				0	},
	{ K_F3,				"F3",				"",				"",				0	},
	{ K_F4,				"F4",				"",				"",				0	},
	{ K_F5,				"F5",				"",				"",				0	},
	{ K_F6,				"F6",				"",				"",				0	},
	{ K_F7,				"F7",				"",				"",				0	},
	{ K_F8,				"F8",				"",				"",				0	},
	{ K_F9,				"F9",				"screenshot",		"",				0	},
	{ K_F10,				"F10",			"",				"",				0	},
	{ K_F11,				"F11",			"",				"",				0	},
	{ K_F12,				"F12",			"",				"",				0	},
	{ K_PRINTSCREEN,		"printscreen",		"screenshot",		"",				0	},
	{ K_SCROLLLOCK,		"scrolllock",		"",				"",				0	},
	{ K_PAUSE,			"pause",			"",				"",				0	},

	{ '`',					"`",				"",			"con_toggle",				0	},
	{ '1',					"1",				"",				"",				0	},
	{ '2',					"2",				"",				"",				0	},
	{ '3',					"3",				"",				"",				0	},
	{ '4',					"4",				"",				"",				0	},
	{ '5',					"5",				"",				"",				0	},
	{ '6',					"6",				"",				"",				0	},
	{ '7',					"7",				"",				"",				0	},
	{ '8',					"8",				"",				"",				0	},
	{ '9',					"9",				"",				"",				0	},
	{ '0',					"0",				"",				"",				0	},
	{ '-',					"-",				"",				"",				0	},
	{ '=',					"=",				"",				"",				0	},
	{ K_BACKSPACE,		"backspace",		"selection_clear",	"",				0	},

	{ K_TAB,				"tab",			"",				"",				0	},
	{ '[',					"[",				"",				"",				0	},
	{ ']',					"]",				"",				"",				0	},
	{ '\\',				"\\",				"",				"",				0	},

	{ K_CAPSLOCK,		"capslock",		"",				"",				0	},
	{ ';',					";",				"",				"",				0	},
	{ '\'',					"\'",				"",				"",				0	},
	{ K_ENTER,			"enter",			"",				"",				0	},

	{ K_LSHIFT,			"leftshift",			"",				"",				0	},
	{ ',',					",",				"",				"",				0	},
	{ '.',					".",				"",				"",				0	},
	{ '/',					"/",				"",				"",				0	},
	{ K_RSHIFT,			"rightshift",		"",				"",				0	},

	{ K_LCTRL,			"leftctrl",			"",				"",				0	},
	{ K_LALT,				"leftalt",			"",				"",				0	},
	{ K_SPACE,			"space",			"",				"",				0	},
	{ K_RALT,				"rightalt",			"",				"",				0	},
	{ K_RCTRL,			"rightctrl",		"",				"",				0	},

	{ K_INSERT,			"insert",			"",				"",				0	},
	{ K_DELETE,			"delete",			"",				"",				0	},
	{ K_HOME,			"home",			"",				"",				0	},
	{ K_END,				"end",			"",				"",				0	},
	{ K_PGUP,			"pageup",		"",				"",				0	},
	{ K_PGDN,			"pagedown",		"",				"",				0	},

	{ K_RIGHT,			"right",			"+rotateright", 	"-rotateright",		0	},
	{ K_LEFT,				"left",			"+rotateleft",		"-rotateleft",		0	},
	{ K_DOWN,			"down",			"+rotateup",		"-rotateup",		0	},
	{ K_UP,				"up",			"+rotatedown",	"-rotatedown",	0	},

	{ 'a',					"a",				"+moveleft",		"-moveleft",		0	},
	{ 'b',					"b",				"",				"",				0	},
	{ 'c',					"c",				"+movedown",		"-movedown",		0	},
	{ 'd',					"d",				"+moveright",		"-moveright",		0	},
	{ 'e',					"e",				"+moveup",		"-moveup",		0	},
	{ 'f',					"f",				"",				"",				0	},
	{ 'g',					"g",				"",				"",				0	},
	{ 'h',					"h",				"",				"",				0	},
	{ 'i',					"i",				"",				"",				0	},
	{ 'j',					"j",				"",				"",				0	},
	{ 'k',					"k",				"",				"",				0	},
	{ 'l',					"l",				"",				"",				0	},
	{ 'm',				"m",				"",				"",				0	},
	{ 'n',					"n",				"",				"",				0	},
	{ 'o',					"o",				"",				"",				0	},
	{ 'p',					"p",				"",				"",				0	},
	{ 'q',					"q",				"",				"",				0	},
	{ 'r',					"r",				"",				"",				0	},
	{ 's',					"s",				"+movebackward",	"-movebackward",	0	},
	{ 't',					"t",				"",				"",				0	},
	{ 'u',					"u",				"",				"",				0	},
	{ 'v',					"v",				"",				"",				0	},
	{ 'w',				"w",				"+moveforward",	"-moveforward",	0	},
	{ 'x',					"x",				"",				"",				0	},
	{ 'y',					"y",				"",				"",				0	},
	{ 'z',					"z",				"",				"",				0	},

	{ KP_NUMLOCK,		"numlock",		"",				"",				0	},
	{ KP_MULTIPLY,		"kp_multiply",		"",				"",				0	},
	{ KP_MINUS,			"kp_minus",		"",				"",				0	},
	{ KP_PLUS,			"kp_plus",		"",				"",				0	},
	{ KP_ENTER,			"kp_enter",		"",				"",				0	},
	{ KP_PERIOD,			"kp_period",		"",				"",				0	},
	{ KP_DIVIDE,			"kp_divide",		"",				"",				0	},

	{ KP_7,				"kp_7",			"",				"",				0	},
	{ KP_8,				"kp_8",			"",				"",				0	},
	{ KP_9,				"kp_9",			"",				"",				0	},
	{ KP_4,				"kp_4",			"",				"",				0	},
	{ KP_5,				"kp_5",			"",				"",				0	},
	{ KP_6,				"kp_6",			"",				"",				0	},
	{ KP_1,				"kp_1",			"",				"",				0	},
	{ KP_2,				"kp_2",			"",				"",				0	},
	{ KP_3,				"kp_3",			"",				"",				0	},
	{ KP_0,				"kp_0",			"",				"",				0	}
};
static int bindListSize = sizeof(bindlist) / sizeof(keybind_t);
static byte bShiftIsDown = 0;
static byte bCtrlIsDown = 0;
static byte bAltIsDown = 0;

 // *********** FUNCTIONALITY ***********
/* ---------------
bind_Initialize
--------------- */
int bind_Initialize(void)
{
	return 1;
}

/* ---------------
bind_Terminate
--------------- */
void bind_Terminate(void)
{
}

/* ---------------
bind_TranslateKeyPress - takes a key and its movement type and emits its bound command to the command processor system 
// global key event dispatcher -- the game, console, and menu all need to cooperate to play nice together
// this function makes sure that that happens
--------------- */
void bind_TranslateKeyPress( byte key, byte key_state )
{
	int a;

	// catch the state of these for use elsewhere
	if( key == K_LALT || key == K_RALT ) bAltIsDown = key_state;
	if( key == K_LCTRL || key == K_RCTRL ) bCtrlIsDown = key_state;
	if( key == K_LSHIFT || key == K_RSHIFT ) bShiftIsDown = key_state;

	// do nothing dangerous in init
	if( state.bInInit )
	{
		if( key == K_ESCAPE && key_state == 0 ) eng_Stop( "0" );
		return;
	}
	
	// system level interupt for console
	if( key == '`'  )
	{
		if( key_state == 0 )
		{
			con_Toggle(NULL);
		}
		else
		{
		}
		return;
	}

	// system level interupt for escape key
	if( key == K_ESCAPE && key_state == 0 )
	{
		if( state.bInConsole ) 
		{
			con_Toggle(NULL);
			return;
		}
		if( state.bGameType != GAME_TYPE_NOGAME )
		{
			if( state.bDrawMenu )
			{
				state.bDrawMenu = 0;
				ms_Hide();
			}
			else
			{
				state.bDrawMenu = 1;
				ms_Show();
			}
			return;
		}
	}

	// console gets first dibs to key presses
	if( state.bInConsole )
	{
		con_KeyPress( key, key_state );
		return;
	}

	// menu gets second dibs to key events
	if( mi != NULL && state.bDrawMenu )
	{
		mi->menu_KeyPress( key, key_state );
		return;
	}	



	// otherwise the bind dereference gets a pass
	for( a=0 ; a<bindListSize ; a++ )
	{
		if( bindlist[a].key == key )
		{
			if( key_state >= 1 ) 
			{
				// track the down counts to minimize the repeated work going to the com processor
				bindlist[a].downcount++;
				if( bindlist[a].downcount > 1 ) return;

				com_Process( bindlist[a].downCom );

				return;
			}
			else
			{
				// reset the downs so we can call the command again
				bindlist[a].downcount = 0;

				com_Process( bindlist[a].upCom );

				return;
			}
		}
	}
}

/* ---------------
bind_XIsDown - accessors for standard state keys
--------------- */
int bind_ShiftIsDown() { return bShiftIsDown; }
int bind_AltIsDown() { return bAltIsDown; }
int bind_CtrlIsDown() { return bCtrlIsDown; }

/* ---------------
bind_BindKey - Pull 2 tokens from the buffer and pass them to the set bind fcn
--------------- */
void bind_BindKey( char *buffer )
{
	int to, len, cpyLen;
	char key[COM_STR_SIZE + 1];
	char cmd[COM_STR_SIZE + 1];

	// get to first token
	if( !par_GetNextToken( buffer, &to, &len ) )
	{
		con_Print( "Bind failed:  Could not find a key name." );
		con_Print( "Usage:  bind <key> <command>" );
		return;
	}

	// advance and copy the token
	buffer += to;
	if( len > COM_STR_SIZE ) cpyLen = COM_STR_SIZE;
	else cpyLen = len;
	tcstrncpy( key, buffer, cpyLen );
	key[cpyLen] = '\0';
	buffer += len;

	// get the next token
	if( !par_GetNextToken( buffer, &to, &len ) )
	{
		con_Print( "Bind failed:  Could not find a command name." );
		con_Print( "Usage:  bind <key> <command>" );
		return;
	}

	// advance and copy the token
	buffer += to;
	if( len > COM_STR_SIZE ) cpyLen = COM_STR_SIZE;
	else cpyLen = len;
	tcstrncpy( cmd, buffer, cpyLen );
	cmd[cpyLen] = '\0';

	// update the value
	bind_SetKeyBind( key, cmd );
}

/* ---------------
bind_SetKeyBind - associte a given key with a command to interpret when the key is pressed
--------------- */
void bind_SetKeyBind( char *key, char *command )
{
	int a;
	int com, bind;

	// we need to verify the tokens
	com = com_VerifyCommand( command );

	// the key
	for( a=0, bind=-1 ; a<bindListSize ; a++ )
	{
		if( tcstrincmp( key, bindlist[a].text, tcstrlen(bindlist[a].text) ) )
		{
			bind = a;
			break;
		}
	}

	// check the results
	if( !com ) // failure
	{
		con_Print( "Unable to find command %s", command );
		return;
	}
	if( bind == -1 ) // failure
	{
		con_Print( "Unable to find key %s", key ); 
		return;
	}

	// now that we know we can complete the operation, delete the previous instance of the command
	for( a=0 ; a<bindListSize ; a++ )
	{
		if( tcstrncmp( command, bindlist[a].downCom, tcstrlen(bindlist[a].downCom) ) )
		{
			// delete the command from the old key
			memset( bindlist[a].downCom, 0, COM_STR_SIZE );
			memset( bindlist[a].upCom,   0, COM_STR_SIZE );
		}
	}

	// replace the current command
	memset( bindlist[bind].downCom, 0, COM_STR_SIZE );
	memset( bindlist[bind].upCom, 0, COM_STR_SIZE );

	tcstrncpy( bindlist[bind].downCom, command, tcstrlen(command) );
	if( bindlist[bind].downCom[0] == '+' )
	{
		tcstrncpy( bindlist[bind].upCom, command, tcstrlen(command) );
		bindlist[bind].upCom[0] = '-';
	}

	con_Print( "Key %s bound to command %s", bindlist[bind].text, bindlist[bind].downCom );

	return;
}

/* ---------------
bind_UnbindKey - zeros the command associated with key
--------------- */
void bind_UnbindKey( char *buffer )
{
	int to, len, cpyLen, a;
	char key[COM_STR_SIZE + 1];
	
	// get to first token
	if( !par_GetNextToken( buffer, &to, &len ) )
	{
		con_Print( "Unbind failed:  Could not find a key name." );
		con_Print( "Usage:  unbind <key>" );
		return;
	}

	// advance and copy the token
	buffer += to;
	if( len > COM_STR_SIZE ) cpyLen = COM_STR_SIZE;
	else cpyLen = len;
	tcstrncpy( key, buffer, cpyLen );
	key[cpyLen] = '\0';
	buffer += len;

	// find the key
	for( a = 0 ; a < bindListSize ; a++ )
	{
		if( tcstrcmp( bindlist[a].text, key ) )
		{
			memset( bindlist[a].downCom, 0, sizeof(char) * COM_STR_SIZE );
			memset( bindlist[a].upCom, 0, sizeof(char) * COM_STR_SIZE );
			con_Print( "Key \"%s\" unbound." );
		}
	}
}

/* ---------------
bind_UnbindAll
--------------- */
void bind_UnbindAll()
{
	int a;

	for( a = 0 ; a < bindListSize ; a++ )
	{
		memset( bindlist[a].downCom, 0, sizeof(char) * COM_STR_SIZE );
		memset( bindlist[a].upCom, 0, sizeof(char) * COM_STR_SIZE );
	}

	con_Print( "All keys unbound." );
}

/* ---------------
bind_FlushKeyStates
// this should tell all of our "special" keys to be up
// we'll call this whenever we reinit the input system
--------------- */
void bind_FlushKeyStates(void)
{
	bAltIsDown = 0;
	bCtrlIsDown = 0;
	bShiftIsDown = 0;
}
