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
// sal_input_sdl.h
// Created by Terrence Cole

// Includes
/////////////
#include "cog_global.h"

// Global Prototypes
//////////////////////
/*
int in_sdl_Initialize(void);
void in_sdl_Terminate(void);
void in_sdl_ProcessEvents();
void in_sdl_Event( unsigned char key, unsigned short down );
byte in_sdl_MapKey( int key );
void in_sdl_HideMouse(void);
void in_sdl_UnhideMouse(void);
*/

// *********** FUNCTIONALITY ***********
#ifdef HAVE_SDL
/* ------------
in_sdl_Initialize
------------ */
int in_sdl_Initialize(void)
{
    // already inited by video
    return 1;
}

/* ------------
in_sdl_Terminate
------------ */
void in_sdl_Terminate(void)
{
    // no resources to free
}

/* ------------
in_sdl_ProcessEvents
------------ */
void in_sdl_ProcessEvents()
{
	// in sdl, events are processed, globally; as in win32
}

/* ------------
in_sdl_Event
------------ */
void in_sdl_Event( unsigned char key, unsigned short down )
{
	bind_TranslateKeyPress( key, (byte)down );
}

/* ------------
in_sdl_MapKey
------------ */
byte in_sdl_MapKey( int key )
{
        SDLKey sym = (SDLKey)key;
        byte out;

        switch( sym ) {
//        case SDLK_CLEAR: out =                 clear
        case SDLK_BACKSPACE: out = K_BACKSPACE; break;
        case SDLK_TAB: out = K_TAB; break;
        case SDLK_RETURN: out = K_ENTER; break; // '\r'    return
        case SDLK_PAUSE: out = K_PAUSE; break; // pause
        case SDLK_ESCAPE: out = K_ESCAPE; break; //        '^['    escape
        case SDLK_SPACE: out = K_SPACE; break; //         ' '     space
        case SDLK_QUOTE: out =       '\''; break; //    single quote
        case SDLK_QUOTEDBL: out =  '\''; break; //    double quote
        case SDLK_COMMA: out =      ','; break; //     comma
        case SDLK_LESS: out = 	     ','; break; //     less-than sign
        case SDLK_MINUS: out =       '-'; break; //     minus sign
        case SDLK_UNDERSCORE: out = '-'; break; //     underscore
        case SDLK_PERIOD: out =      '.'; break; //     period / full stop
        case SDLK_GREATER: out =    '.'; break; //     greater-than sign
        case SDLK_SLASH: out =       '/'; break; //     forward slash
        case SDLK_QUESTION: out =  '/'; break; //     question mark
        case SDLK_0: out =             '0'; break; //     0
        case SDLK_RIGHTPAREN: out = '0'; break; //     right parenthesis
        case SDLK_1: out =             '1'; break; //     1
        case SDLK_EXCLAIM: out =   '1'; break; //     exclamation mark
        case SDLK_2: out =   	    '2'; break; //     2
        case SDLK_AT: out =           '2'; break; //     at
        case SDLK_3: out =     	    '3'; break; //     3
        case SDLK_HASH: out =  	    '3'; break; //     hash
        case SDLK_4: out =     	    '4'; break; //     4
        case SDLK_DOLLAR: out =     '4'; break; //     dollar
        case SDLK_5: out =   	    '5'; break; //     5
        case SDLK_6: out =  		    '6'; break; //     6
        case SDLK_CARET: out =      '6'; break; //     caret
        case SDLK_7: out =  		    '7'; break; //     7
        case SDLK_AMPERSAND: out = '7'; break; //     ampersand
        case SDLK_8: out =  		     '8'; break; //     8
        case SDLK_ASTERISK: out =  '8'; break; //     asterisk
        case SDLK_9: out =  		     '9'; break; //     9
        case SDLK_LEFTPAREN: out = '9'; break; //     left parenthesis
        case SDLK_SEMICOLON: out = ';'; break; //     semicolon
        case SDLK_COLON: out =        ';'; break; //     colon
        case SDLK_EQUALS: out =      '='; break; //     equals sign
        case SDLK_PLUS: out =  	      '='; break; //     plus sign
        case SDLK_LEFTBRACKET: out =   '['; break; //     left bracket
        case SDLK_BACKSLASH: out =     '\\'; break; //    backslash
        case SDLK_RIGHTBRACKET: out =  ']'; break; //     right bracket
        case SDLK_BACKQUOTE: out =     '`'; break; //     grave
        case SDLK_a: out =     'a'; break; //     a
        case SDLK_b: out =     'b'; break; //     b
        case SDLK_c: out =     'c'; break; //     c
        case SDLK_d: out =     'd'; break; //     d
        case SDLK_e: out =     'e'; break; //     e
        case SDLK_f: out =     'f'; break; //     f
        case SDLK_g: out =     'g'; break; //     g
        case SDLK_h: out =     'h'; break; //     h
        case SDLK_i: out =     'i'; break; //     i
        case SDLK_j: out =     'j'; break; //     j
        case SDLK_k: out =     'k'; break; //     k
        case SDLK_l: out =     'l'; break; //     l
        case SDLK_m: out =     'm'; break; //     m
        case SDLK_n: out =     'n'; break; //     n
        case SDLK_o: out =     'o'; break; //     o
        case SDLK_p: out =     'p'; break; //     p
        case SDLK_q: out =     'q'; break; //     q
        case SDLK_r: out =     'r'; break; //     r
        case SDLK_s: out =     's'; break; //     s
        case SDLK_t: out =     't'; break; //     t
        case SDLK_u: out =     'u'; break; //     u
        case SDLK_v: out =     'v'; break; //     v
        case SDLK_w: out =     'w'; break; //     w
        case SDLK_x: out =     'x'; break; //     x
        case SDLK_y: out =     'y'; break; //     y
        case SDLK_z: out =     'z'; break; //     z
        case SDLK_DELETE: out = K_DELETE; break; // '^?'    delete
/*
        case SDLK_WORLD_0               world 0
        case SDLK_WORLD_1               world 1
        case SDLK_WORLD_2               world 2
        case SDLK_WORLD_3               world 3
        case SDLK_WORLD_4               world 4
        case SDLK_WORLD_5               world 5
        case SDLK_WORLD_6               world 6
        case SDLK_WORLD_7               world 7
        case SDLK_WORLD_8               world 8
        case SDLK_WORLD_9               world 9
        case SDLK_WORLD_10              world 10
        case SDLK_WORLD_11              world 11
        case SDLK_WORLD_12              world 12
        case SDLK_WORLD_13              world 13
        case SDLK_WORLD_14              world 14
        case SDLK_WORLD_15              world 15
        case SDLK_WORLD_16              world 16
        case SDLK_WORLD_17              world 17
        case SDLK_WORLD_18              world 18
        case SDLK_WORLD_19              world 19
        case SDLK_WORLD_20              world 20
        case SDLK_WORLD_21              world 21
        case SDLK_WORLD_22              world 22
        case SDLK_WORLD_23              world 23
        case SDLK_WORLD_24              world 24
        case SDLK_WORLD_25              world 25
        case SDLK_WORLD_26              world 26
        case SDLK_WORLD_27              world 27
        case SDLK_WORLD_28              world 28
        case SDLK_WORLD_29              world 29
        case SDLK_WORLD_30              world 30
        case SDLK_WORLD_31              world 31
        case SDLK_WORLD_32              world 32
        case SDLK_WORLD_33              world 33
        case SDLK_WORLD_34              world 34
        case SDLK_WORLD_35              world 35
        case SDLK_WORLD_36              world 36
        case SDLK_WORLD_37              world 37
        case SDLK_WORLD_38              world 38
        case SDLK_WORLD_39              world 39
        case SDLK_WORLD_40              world 40
        case SDLK_WORLD_41              world 41
        case SDLK_WORLD_42              world 42
        case SDLK_WORLD_43              world 43
        case SDLK_WORLD_44              world 44
        case SDLK_WORLD_45              world 45
        case SDLK_WORLD_46              world 46
        case SDLK_WORLD_47              world 47
        case SDLK_WORLD_48              world 48
        case SDLK_WORLD_49              world 49
        case SDLK_WORLD_50              world 50
        case SDLK_WORLD_51              world 51
        case SDLK_WORLD_52              world 52
        case SDLK_WORLD_53              world 53
        case SDLK_WORLD_54              world 54
        case SDLK_WORLD_55              world 55
        case SDLK_WORLD_56              world 56
        case SDLK_WORLD_57              world 57
        case SDLK_WORLD_58              world 58
        case SDLK_WORLD_59              world 59
        case SDLK_WORLD_60              world 60
        case SDLK_WORLD_61              world 61
        case SDLK_WORLD_62              world 62
        case SDLK_WORLD_63              world 63
        case SDLK_WORLD_64              world 64
        case SDLK_WORLD_65              world 65
        case SDLK_WORLD_66              world 66
        case SDLK_WORLD_67              world 67
        case SDLK_WORLD_68              world 68
        case SDLK_WORLD_69              world 69
        case SDLK_WORLD_70              world 70
        case SDLK_WORLD_71              world 71
        case SDLK_WORLD_72              world 72
        case SDLK_WORLD_73              world 73
        case SDLK_WORLD_74              world 74
        case SDLK_WORLD_75              world 75
        case SDLK_WORLD_76              world 76
        case SDLK_WORLD_77              world 77
        case SDLK_WORLD_78              world 78
        case SDLK_WORLD_79              world 79
        case SDLK_WORLD_80              world 80
        case SDLK_WORLD_81              world 81
        case SDLK_WORLD_82              world 82
        case SDLK_WORLD_83              world 83
        case SDLK_WORLD_84              world 84
        case SDLK_WORLD_85              world 85
        case SDLK_WORLD_86              world 86
        case SDLK_WORLD_87              world 87
        case SDLK_WORLD_88              world 88
        case SDLK_WORLD_89              world 89
        case SDLK_WORLD_90              world 90
        case SDLK_WORLD_91              world 91
        case SDLK_WORLD_92              world 92
        case SDLK_WORLD_93              world 93
        case SDLK_WORLD_94              world 94
        case SDLK_WORLD_95              world 95
*/
        case SDLK_KP0: out = KP_0; break; //           keypad 0
        case SDLK_KP1: out = KP_1; break; //           keypad 1
        case SDLK_KP2: out = KP_2; break; //           keypad 2
        case SDLK_KP3: out = KP_3; break; //           keypad 3
        case SDLK_KP4: out = KP_4; break; //           keypad 4
        case SDLK_KP5: out = KP_5; break; //           keypad 5
        case SDLK_KP6: out = KP_6; break; //           keypad 6
        case SDLK_KP7: out = KP_7; break; //           keypad 7
        case SDLK_KP8: out = KP_8; break; //           keypad 8
        case SDLK_KP9: out = KP_9; break; //           keypad 9
        case SDLK_KP_PERIOD: out = KP_PERIOD; break; // '.'     keypad period
        case SDLK_KP_DIVIDE: out = KP_DIVIDE; break; // '/'     keypad divide
        case SDLK_KP_MULTIPLY: out = KP_MULTIPLY; break; // '*'     keypad multiply
        case SDLK_KP_MINUS: out = KP_MINUS; break; // '-'     keypad minus
        case SDLK_KP_PLUS: out = KP_PLUS; break; // '+'     keypad plus
        case SDLK_KP_ENTER: out = KP_ENTER; break; // '\r'    keypad enter
//        case SDLK_KP_EQUALS: out =    '='     keypad equals
        case SDLK_UP: out = K_UP; break; //           up arrow
        case SDLK_DOWN: out = K_DOWN; break; //         down arrow
        case SDLK_RIGHT: out = K_RIGHT; break; //                right arrow
        case SDLK_LEFT: out = K_LEFT; break; //         left arrow
        case SDLK_INSERT: out = K_INSERT; break; //               insert
        case SDLK_HOME: out = K_HOME; break; //         home
        case SDLK_END: out = K_END; break; //          end
        case SDLK_PAGEUP: out = K_PGUP; break; //                page up
        case SDLK_PAGEDOWN: out = K_PGDN; break; //  page down
        case SDLK_F1: out = K_F1; break; //           F1
        case SDLK_F2: out = K_F2; break; //            F2
        case SDLK_F3: out = K_F3; break; //            F3
        case SDLK_F4: out = K_F4; break; //            F4
        case SDLK_F5: out = K_F5; break; //            F5
        case SDLK_F6: out = K_F6; break; //            F6
        case SDLK_F7: out = K_F7; break; //            F7
        case SDLK_F8: out = K_F8; break; //            F8
        case SDLK_F9: out = K_F9; break; //           F9
        case SDLK_F10: out = K_F10; break; //           F10
        case SDLK_F11: out = K_F11; break; //           F11
        case SDLK_F12: out = K_F12; break; //           F12
//        case SDLK_F13: out = K_F13; break; //           F13
//        case SDLK_F14: out = K_F14; break; //           F14
//        case SDLK_F15: out = K_F15; break; //           F15
//        case SDLK_NUMLOCK: out = K_NUMLOCK; break; //              numlock
        case SDLK_CAPSLOCK: out = K_CAPSLOCK; break; //             capslock
        case SDLK_SCROLLOCK: out = K_SCROLLLOCK; break; //  scrollock
        case SDLK_RSHIFT: out = K_RSHIFT; break; //  right shift
        case SDLK_LSHIFT: out = K_LSHIFT; break; //               left shift
        case SDLK_RCTRL: out = K_RCTRL; break; //                right ctrl
        case SDLK_LCTRL: out = K_LCTRL; break; //                left ctrl
        case SDLK_RALT: out = K_RALT; break; //         right alt / alt gr
        case SDLK_LALT: out = K_LALT; break; //         left alt
//        case SDLK_RMETA: out =                 right meta
//        case SDLK_LMETA: out =                 left meta
//        case SDLK_LSUPER                left windows key
//        case SDLK_RSUPER                right windows key
//        case SDLK_MODE          mode shift
//        case SDLK_COMPOSE               compose
//        case SDLK_HELP          help
        case SDLK_PRINT: out = K_PRINTSCREEN; break; //   print-screen
//        case SDLK_SYSREQ                SysRq
//        case SDLK_BREAK: out = K_                break
//        case SDLK_MENU          menu
//        case SDLK_POWER                 power
//        case SDLK_EURO          euro
        default:
        	out = ' ';
        	break;
        }
        return out;
}

byte in_sdl_MapButton( uint8_t button )
{
	byte out;
	
	switch( button ) {
	case SDL_BUTTON_LEFT: out = M_LEFT; break;
	case SDL_BUTTON_MIDDLE: out = M_MIDDLE; break;
	case SDL_BUTTON_RIGHT: out = M_RIGHT; break;
	case SDL_BUTTON_WHEELUP: out = M_WHEELUP; break;
	case SDL_BUTTON_WHEELDOWN: out = M_WHEELDOWN; break;
	default: out = K_SPACE; break;
	}
	return out;
}

/* ------------
in_sdl_HideMouse
------------ */
void in_sdl_HideMouse(void)
{
	con_Print( "**********************" );
	con_Print( "HIDING MOUSE HIDING MOUSE" );
	con_Print( "**********************" );
    return;
	SDL_WM_GrabInput( SDL_GRAB_ON );
	SDL_ShowCursor(SDL_DISABLE);
}

/* ------------
in_sdl_UnhideMouse
------------ */
void in_sdl_UnhideMouse(void)
{
	con_Print( "**********************" );
	con_Print( "SHOWING MOUSE SHOWING MOUSE" );
	con_Print( "**********************" );
    return;
	SDL_WM_GrabInput( SDL_GRAB_OFF );
	SDL_ShowCursor(SDL_ENABLE);
}

#else

// for the case that we compile without SDL support
int in_sdl_Initialize(void) {
    con_Print( "Input: Cannot initialize an SDL only subsystem without SDL enabled." );
    return 0;
}

void in_sdl_Terminate(void) {
    con_Print( "Input: Cannot terminate an SDL only subsystem without SDL enabled." );
}

void in_sdl_ProcessEvents() {
    con_Print( "Input SDL called from non-SDL enabled system." );
    assert(1==0);
}

void in_sdl_Event( unsigned char key, unsigned short down ) {
    con_Print( "Input SDL called from non-SDL enabled system." );
    assert(1==0);
}

byte in_sdl_MapKey( int key ) {
    con_Print( "Input SDL called from non-SDL enabled system." );
    assert(1==0);
    return 0;
}

void in_sdl_HideMouse(void) {
    con_Print( "Input SDL called from non-SDL enabled system." );
    assert(1==0);
}

void in_sdl_UnhideMouse(void) {
    con_Print( "Input SDL called from non-SDL enabled system." );
    assert(1==0);
}
#endif

