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
// sal_input_win_di.c
// Created by Terrence Cole

#ifdef WIN32

// Includes
/////////////
#include "cog_global.h"
#include "sal_input_win_di.h"
#include <dinput.h>

// Definitions
////////////////
#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif

#define KEYBOARD_BUFFER_SIZE 150

// Local Structures
/////////////////////

// Global Prototypes
//////////////////////
/*
int in_di_Initialize(void);
void in_di_Terminate(void);
void in_di_ProcessEvents();
*/

// Local Prototypes
/////////////////////
static void in_di_ProcessKeyboardBuffered();
static void in_di_SynchKeyStates();
static void in_di_ProcessMouseImmediate();
static void in_di_PrintError( HRESULT hRet );
static void in_di_Reacquire();

// Local Variables
////////////////////
static LPDIRECTINPUT8 pDI8 = NULL;
static LPDIRECTINPUTDEVICE8 diKeyboard = NULL;
static LPDIRECTINPUTDEVICE8 diMouse = NULL;

static byte bkbLastState[256];
static DIMOUSESTATE diLastMouseState;

static varlatch vlmInvertX = NULL;
static varlatch vlmInvertY = NULL;


// *********** FUNCTIONALITY ***********
/* ------------
in_di_Initialize
------------ */
int in_di_Initialize(void)
{
    DIPROPDWORD dipdw;
    HRESULT hRet;

    con_Print( "\t<BLUE>Initializing</BLUE> Direct Input..." );

    // assert
    if( pDI8 != NULL )
    {
        con_Print( "<RED>Direct Input Init Assert Failed:  pDI8 != NULL." );
        eng_Stop( "160001" );
        return 0;
    }
    if( diKeyboard != NULL )
    {
        con_Print( "<RED>Direct Input Init Assert Failed:  diKeyboard != NULL." );
        eng_Stop( "160002" );
        return 0;
    }
    if( diMouse != NULL )
    {
        con_Print( "<RED>Direct Input Init Assert Failed:  diMouse != NULL." );
        eng_Stop( "160003" );
        return 0;
    }

    // create direct input
    hRet = DirectInput8Create( sys_handles.cInstance, DIRECTINPUT_VERSION, &IID_IDirectInput8A, &pDI8, NULL );
    if( hRet != DI_OK )
    {
        con_Print( "<RED>Direct Input Init failed at Create:  Reason:" );
        in_di_PrintError( hRet );
        return 0;
    }

    // get a keyboard device
    hRet = pDI8->lpVtbl->CreateDevice( pDI8, &(GUID_SysKeyboard), &diKeyboard, NULL );
    if( hRet != DI_OK )
    {
        con_Print( "<RED>Direct Input Init failed at Create Keyboard:  Reason:" );
        in_di_PrintError( hRet );
        in_di_Terminate();
        return 0;
    }

    // get a mouse device
    hRet = pDI8->lpVtbl->CreateDevice( pDI8, &(GUID_SysMouse), &diMouse, NULL );
    if( hRet != DI_OK )
    {
        con_Print( "<RED>Direct Input Init failed at Create Mouse:  Reason:" );
        in_di_PrintError( hRet );
        in_di_Terminate();
        return 0;
    }

    // set the mouse data format
    hRet = diKeyboard->lpVtbl->SetDataFormat( diKeyboard, &c_dfDIKeyboard );
    if( hRet != DI_OK )
    {
        con_Print( "<RED>Direct Input Init failed at Set Keyboard Data Format:  Reason:" );
        in_di_PrintError( hRet );
        in_di_Terminate();
        return 0;
    }

    // set the mouse data format
    hRet = diMouse->lpVtbl->SetDataFormat( diMouse, &c_dfDIMouse );
    if( hRet != DI_OK )
    {
        con_Print( "<RED>Direct Input Init failed at Set Mouse Data Format:  Reason:" );
        in_di_PrintError( hRet );
        in_di_Terminate();
        return 0;
    }

    // set the keyboard cooperative level
    hRet = diKeyboard->lpVtbl->SetCooperativeLevel( diKeyboard, gl_GetWindow()->hWnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE | DISCL_NOWINKEY );
    if( hRet != DI_OK )
    {
        con_Print( "<RED>Direct Input Init failed as Set Keyboard Cooperative Level:  Reason:" );
        in_di_PrintError( hRet );
        in_di_Terminate();
        return 0;
    }

    // set the mouse cooperative level
    hRet = diMouse->lpVtbl->SetCooperativeLevel( diMouse, gl_GetWindow()->hWnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE );
    if( hRet != DI_OK )
    {
        con_Print( "<RED>Direct Input Init failed as Set Mouse Cooperative Level:  Reason:" );
        in_di_PrintError( hRet );
        in_di_Terminate();
        return 0;
    }

    // enter buffered mode for keyboard access
    dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
    dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipdw.diph.dwObj        = 0;
    dipdw.diph.dwHow        = DIPH_DEVICE;
    dipdw.dwData            = KEYBOARD_BUFFER_SIZE; // Arbitary buffer size
    hRet = diKeyboard->lpVtbl->SetProperty( diKeyboard, DIPROP_BUFFERSIZE, &dipdw.diph );
    if( hRet != DI_OK )
    {
        con_Print( "<RED>Direct Input Init failed at Set Keyboard Buffer Size:  reason:" );
        in_di_PrintError( hRet );
        in_di_Terminate();
        return 0;
    }

    // acquire the keyboard
    hRet = diKeyboard->lpVtbl->Acquire( diKeyboard );
    if( hRet != DI_OK )
    {
        con_Print( "<RED>Direct Input Init has failed at Keyboard Acquire:  Reason:" );
        in_di_PrintError( hRet );
        in_di_Terminate();
        return 0;
    }

    // acquire the Mouse
    hRet = diMouse->lpVtbl->Acquire( diMouse );
    if( hRet != DI_OK )
    {
        con_Print( "<RED>Direct Input Init has failed at Mouse Acquire:  Reason:" );
        in_di_PrintError( hRet );
        in_di_Terminate();
        return 0;
    }

    // clear the data history buffers
    memset( bkbLastState, 0, 256 );
    memset( &diLastMouseState, 0, sizeof(DIMOUSESTATE) );

    // latch to the mouse inversion variables
    vlmInvertX = var_GetVarLatch( "mInvertX" );
    if( vlmInvertX == NULL )
    {
        con_Print( "<RED>Direct Input Init Failed:  Unable to latch to variable \"mInvertX\"." );
        return 0;
    }
    vlmInvertY = var_GetVarLatch( "mInvertY" );
    if( vlmInvertY == NULL )
    {
        con_Print( "<RED>Direct Input Init Failed:  Unable to latch to variable \"mInvertY\"." );
        return 0;
    }

    // clear the global mouse states
    state.mAxis[0] = state.mAxis[1] = 0;
    state.mAxisPrev[0] = state.mAxisPrev[1] = 0;
    state.mAxisDelta[0] = state.mAxisDelta[1] = 0;

    con_Print( "\t\tSuccess!" );

    return 1;
}

/* ------------
in_di_TGerminate
------------ */
void in_di_Terminate(void)
{
    con_Print( "\n<RED>Terminating</RED> Direct Input." );

    // unaquire the devices
    if( diMouse != NULL ) diMouse->lpVtbl->Unacquire( diMouse );
    if( diKeyboard != NULL ) diKeyboard->lpVtbl->Unacquire( diKeyboard );

    // release the devices
    if( diMouse != NULL ) diMouse->lpVtbl->Release( diMouse );
    if( diKeyboard != NULL ) diKeyboard->lpVtbl->Release( diKeyboard );

    // release the Direct Input
    if( pDI8 != NULL ) pDI8->lpVtbl->Release( pDI8 );

    // null everything
    pDI8 = NULL;
    diKeyboard = NULL;
    diMouse = NULL;

    con_Print( "\tSuccess..." );
}

/* ------------
in_di_ProcessEvents - exports work to special mouse and keyboard handling
------------ */
void in_di_ProcessEvents()
{
    // assert
    if( diMouse == NULL )
    {
        con_Print( "DInput Process Events Assert Failed:  diMouse is NULL" );
        eng_Stop( "160004" );
        return;
    }
    if( diKeyboard == NULL )
    {
        con_Print( "DInput Process Events Assert Failed:  diKeyboard is NULL" );
        eng_Stop( "160005" );
        return;
    }

    // process mouse movement and button states
    in_di_ProcessMouseImmediate();

    // process keyboard events
    in_di_ProcessKeyboardBuffered();
}

/* ------------
in_di_ProcessKeyboardBuffered - attempts to read and respond to buffered keyboard events
                        - if an overflow occurs, sets bkbLastState to the last known good buffered state,
                        - then responds to keyboard action in immediate mode to resolve any discrepancies
------------ */
static void in_di_ProcessKeyboardBuffered()
{
    // assert
    // nb. speedhack - already asserted keyboard in ProcessEvents

    DIDEVICEOBJECTDATA didod[ KEYBOARD_BUFFER_SIZE ];
    DWORD dwElements = KEYBOARD_BUFFER_SIZE;
    HRESULT hRet;
    DWORD a;
    byte key;

    // get device data to current instant in time
    hRet = diKeyboard->lpVtbl->GetDeviceData( diKeyboard, sizeof(DIDEVICEOBJECTDATA), didod, &dwElements, 0 );
    if( hRet != DI_OK && hRet != DI_BUFFEROVERFLOW )
    {
        switch( hRet )
        {
        // reacquisition errors
        case DIERR_INPUTLOST:
        case DIERR_NOTACQUIRED:
            in_di_Reacquire();
            return;
        default:
            break;
        }
        con_Print( "<RED>Direct Input Error in Process Keyboard Buffered on GetDevice Data:  reason:" );
        in_di_PrintError( hRet );
        return;
    }

    // process all current items
    for( a = 0 ; a < dwElements ; a++ )
    {
        switch( didod[a].dwOfs )
        {
        case DIK_0: key = '0'; break;
        case DIK_1: key = '1'; break;
        case DIK_2: key = '2'; break;
        case DIK_3: key = '3'; break;
        case DIK_4: key = '4'; break;
        case DIK_5: key = '5'; break;
        case DIK_6: key = '6'; break;
        case DIK_7: key = '7'; break;
        case DIK_8: key = '8'; break;
        case DIK_9: key = '9'; break;
        case DIK_A: key = 'a'; break;
        case DIK_ADD: key = KP_PLUS; break;
        case DIK_APOSTROPHE: key = '\''; break;
        case DIK_B: key = 'b'; break;
        case DIK_BACK: key = K_BACKSPACE; break;
        case DIK_BACKSLASH: key = '\\'; break;
        case DIK_C: key = 'c'; break;
        case DIK_CAPITAL: key = K_CAPSLOCK; break;
        case DIK_COMMA: key = ','; break;
        case DIK_D: key = 'd'; break;
        case DIK_DECIMAL: key = KP_PERIOD; break;
        case DIK_DELETE: key = K_DELETE; break;
        case DIK_DIVIDE: key = KP_DIVIDE; break;
        case DIK_DOWN: key = K_DOWN; break;
        case DIK_E: key = 'e'; break;
        case DIK_END: key = K_END; break;
        case DIK_EQUALS: key = '='; break;
        case DIK_ESCAPE: key = K_ESCAPE; break;
        case DIK_F: key = 'f'; break;
        case DIK_F1: key = K_F1; break;
        case DIK_F2: key = K_F2; break;
        case DIK_F3: key = K_F3; break;
        case DIK_F4: key = K_F4; break;
        case DIK_F5: key = K_F5; break;
        case DIK_F6: key = K_F6; break;
        case DIK_F7: key = K_F7; break;
        case DIK_F8: key = K_F8; break;
        case DIK_F9: key = K_F9; break;
        case DIK_F10: key = K_F10; break;
        case DIK_F11: key = K_F11; break;
        case DIK_F12: key = K_F12; break;
        case DIK_G: key = 'g'; break;
        case DIK_GRAVE: key = '`'; break;
        case DIK_H: key = 'h'; break;
        case DIK_HOME: key = K_HOME; break;
        case DIK_I: key = 'i'; break;
        case DIK_INSERT: key = K_INSERT; break;
        case DIK_J: key = 'j'; break;
        case DIK_K: key = 'k'; break;
        case DIK_L: key = 'l'; break;
        case DIK_LBRACKET: key = '['; break;
        case DIK_LCONTROL: key = K_LCTRL; break;
        case DIK_LEFT: key = K_LEFT; break;
        case DIK_LMENU: key = K_LALT; break;
        case DIK_LSHIFT: key = K_LSHIFT; break;
        case DIK_M: key = 'm'; break;
        case DIK_MINUS: key = '-'; break;
        case DIK_MULTIPLY: key = KP_MULTIPLY; break;
        case DIK_N: key = 'n'; break;
        case DIK_NEXT: key = K_PGDN; break;
        case DIK_NUMLOCK: key = KP_NUMLOCK; break;
        case DIK_NUMPAD0: key = KP_0; break;
        case DIK_NUMPAD1: key = KP_1; break;
        case DIK_NUMPAD2: key = KP_2; break;
        case DIK_NUMPAD3: key = KP_3; break;
        case DIK_NUMPAD4: key = KP_4; break;
        case DIK_NUMPAD5: key = KP_5; break;
        case DIK_NUMPAD6: key = KP_6; break;
        case DIK_NUMPAD7: key = KP_7; break;
        case DIK_NUMPAD8: key = KP_8; break;
        case DIK_NUMPAD9: key = KP_9; break;
        case DIK_NUMPADENTER: key = KP_ENTER; break;
        case DIK_O: key = 'o'; break;
        case DIK_P: key = 'p'; break;
        case DIK_PAUSE: key = K_PAUSE; break;
        case DIK_PERIOD: key = '.'; break;
        case DIK_PRIOR: key = K_PGUP; break;
        case DIK_Q: key = 'q'; break;
        case DIK_R: key = 'r'; break;
        case DIK_RBRACKET: key = ']'; break;
        case DIK_RCONTROL: key = K_RCTRL; break;
        case DIK_RETURN: key = K_ENTER; break;
        case DIK_RIGHT: key = K_RIGHT; break;
        case DIK_RMENU: key = K_RALT; break;
        case DIK_RSHIFT: key = K_RSHIFT; break;
        case DIK_S: key = 's'; break;
        case DIK_SCROLL: key = K_SCROLLLOCK; break;
        case DIK_SEMICOLON: key = ';'; break;
        case DIK_SLASH: key = '/'; break;
        case DIK_SPACE: key = K_SPACE; break;
        case DIK_SUBTRACT: key = KP_MINUS; break;
        case DIK_SYSRQ: key = K_PRINTSCREEN; break;
        case DIK_T: key = 't'; break;
        case DIK_TAB: key = K_TAB; break;
        case DIK_U: key = 'u'; break;
        case DIK_UP: key = K_UP; break;
        case DIK_V: key = 'v'; break;
        case DIK_W: key = 'w'; break;
        case DIK_X: key = 'x'; break;
        case DIK_Y: key = 'y'; break;
        case DIK_Z: key = 'z'; break;
        default: key = 0; break;
        }

        // send it to the processor to dereference the bind
        bind_TranslateKeyPress( key, (byte)((didod[a].dwData >> 7) & 0x00000001) );

        // set the current state in our last known good in case we need to synch later
        bkbLastState[didod[a].dwOfs] = (byte)((didod[a].dwData >> 7) & 0x00000001);
    }

    // if we couldn't get all recent device data, sync our states with the current states
    if( hRet == DI_BUFFEROVERFLOW )
    {
        // FIXME:  take this print out for final release
        con_Print( "Direct Input:  Buffer Overflow, synching states." );
        in_di_SynchKeyStates();
    }
}


/* ------------
in_di_SynchKeyStates - synchronizes key states if a buffer overflow occurs in direct input
------------ */
static void in_di_SynchKeyStates()
{
    byte bkbCurState[256];
    HRESULT hRet;

    // clear transfer memory
    memset( bkbCurState, 0, 256 );

    // get the current device states from the keyboard
    hRet = diKeyboard->lpVtbl->GetDeviceState( diKeyboard, 256, bkbCurState );
    if( hRet != DI_OK )
    {
        if( hRet == DIERR_INPUTLOST )
        {
            in_di_Reacquire();
            return;
        }
        else
        {
            con_Print( "<RED>Input Error in Direct Input Process Events, Keyboard:  reason:" );
            in_di_PrintError( hRet );
            return;
        }
    }

    // iterate the keystructs, looking for edges in the input signals
    // we're doing this in list format, since individual key names may not match between my and DI's implementation
    if( bkbCurState[DIK_0] != bkbLastState[DIK_0] ) bind_TranslateKeyPress( '0', (byte)((bkbCurState[DIK_0] >> 7) & 0x01) );
    else if( bkbCurState[DIK_1] != bkbLastState[DIK_1] ) bind_TranslateKeyPress( '1', (byte)((bkbCurState[DIK_1] >> 7) & 0x01) );
    else if( bkbCurState[DIK_2] != bkbLastState[DIK_2] ) bind_TranslateKeyPress( '2', (byte)((bkbCurState[DIK_2] >> 7) & 0x01) );
    else if( bkbCurState[DIK_3] != bkbLastState[DIK_3] ) bind_TranslateKeyPress( '3', (byte)((bkbCurState[DIK_3] >> 7) & 0x01) );
    else if( bkbCurState[DIK_4] != bkbLastState[DIK_4] ) bind_TranslateKeyPress( '4', (byte)((bkbCurState[DIK_4] >> 7) & 0x01) );
    else if( bkbCurState[DIK_5] != bkbLastState[DIK_5] ) bind_TranslateKeyPress( '5', (byte)((bkbCurState[DIK_5] >> 7) & 0x01) );
    else if( bkbCurState[DIK_6] != bkbLastState[DIK_6] ) bind_TranslateKeyPress( '6', (byte)((bkbCurState[DIK_6] >> 7) & 0x01) );
    else if( bkbCurState[DIK_7] != bkbLastState[DIK_7] ) bind_TranslateKeyPress( '7', (byte)((bkbCurState[DIK_7] >> 7) & 0x01) );
    else if( bkbCurState[DIK_8] != bkbLastState[DIK_8] ) bind_TranslateKeyPress( '8', (byte)((bkbCurState[DIK_8] >> 7) & 0x01) );
    else if( bkbCurState[DIK_9] != bkbLastState[DIK_9] ) bind_TranslateKeyPress( '9', (byte)((bkbCurState[DIK_9] >> 7) & 0x01) );
    else if( bkbCurState[DIK_A] != bkbLastState[DIK_A] ) bind_TranslateKeyPress( 'a', (byte)((bkbCurState[DIK_A] >> 7) & 0x01) );
    else if( bkbCurState[DIK_B] != bkbLastState[DIK_B] ) bind_TranslateKeyPress( 'b', (byte)((bkbCurState[DIK_B] >> 7) & 0x01) );
    else if( bkbCurState[DIK_C] != bkbLastState[DIK_C] ) bind_TranslateKeyPress( 'c', (byte)((bkbCurState[DIK_C] >> 7) & 0x01) );
    else if( bkbCurState[DIK_D] != bkbLastState[DIK_D] ) bind_TranslateKeyPress( 'd', (byte)((bkbCurState[DIK_D] >> 7) & 0x01) );
    else if( bkbCurState[DIK_E] != bkbLastState[DIK_E] ) bind_TranslateKeyPress( 'e', (byte)((bkbCurState[DIK_E] >> 7) & 0x01) );
    else if( bkbCurState[DIK_F] != bkbLastState[DIK_F] ) bind_TranslateKeyPress( 'f', (byte)((bkbCurState[DIK_F] >> 7) & 0x01) );
    else if( bkbCurState[DIK_G] != bkbLastState[DIK_G] ) bind_TranslateKeyPress( 'g', (byte)((bkbCurState[DIK_G] >> 7) & 0x01) );
    else if( bkbCurState[DIK_H] != bkbLastState[DIK_H] ) bind_TranslateKeyPress( 'h', (byte)((bkbCurState[DIK_H] >> 7) & 0x01) );
    else if( bkbCurState[DIK_I] != bkbLastState[DIK_I] ) bind_TranslateKeyPress( 'i', (byte)((bkbCurState[DIK_I] >> 7) & 0x01) );
    else if( bkbCurState[DIK_J] != bkbLastState[DIK_J] ) bind_TranslateKeyPress( 'j', (byte)((bkbCurState[DIK_J] >> 7) & 0x01) );
    else if( bkbCurState[DIK_K] != bkbLastState[DIK_K] ) bind_TranslateKeyPress( 'k', (byte)((bkbCurState[DIK_K] >> 7) & 0x01) );
    else if( bkbCurState[DIK_L] != bkbLastState[DIK_L] ) bind_TranslateKeyPress( 'l', (byte)((bkbCurState[DIK_L] >> 7) & 0x01) );
    else if( bkbCurState[DIK_M] != bkbLastState[DIK_M] ) bind_TranslateKeyPress( 'm', (byte)((bkbCurState[DIK_M] >> 7) & 0x01) );
    else if( bkbCurState[DIK_N] != bkbLastState[DIK_N] ) bind_TranslateKeyPress( 'n', (byte)((bkbCurState[DIK_N] >> 7) & 0x01) );
    else if( bkbCurState[DIK_O] != bkbLastState[DIK_O] ) bind_TranslateKeyPress( 'o', (byte)((bkbCurState[DIK_O] >> 7) & 0x01) );
    else if( bkbCurState[DIK_P] != bkbLastState[DIK_P] ) bind_TranslateKeyPress( 'p', (byte)((bkbCurState[DIK_P] >> 7) & 0x01) );
    else if( bkbCurState[DIK_Q] != bkbLastState[DIK_Q] ) bind_TranslateKeyPress( 'q', (byte)((bkbCurState[DIK_Q] >> 7) & 0x01) );
    else if( bkbCurState[DIK_R] != bkbLastState[DIK_R] ) bind_TranslateKeyPress( 'r', (byte)((bkbCurState[DIK_R] >> 7) & 0x01) );
    else if( bkbCurState[DIK_S] != bkbLastState[DIK_S] ) bind_TranslateKeyPress( 's', (byte)((bkbCurState[DIK_S] >> 7) & 0x01) );
    else if( bkbCurState[DIK_T] != bkbLastState[DIK_T] ) bind_TranslateKeyPress( 't', (byte)((bkbCurState[DIK_T] >> 7) & 0x01) );
    else if( bkbCurState[DIK_U] != bkbLastState[DIK_U] ) bind_TranslateKeyPress( 'u', (byte)((bkbCurState[DIK_U] >> 7) & 0x01) );
    else if( bkbCurState[DIK_V] != bkbLastState[DIK_V] ) bind_TranslateKeyPress( 'v', (byte)((bkbCurState[DIK_V] >> 7) & 0x01) );
    else if( bkbCurState[DIK_W] != bkbLastState[DIK_W] ) bind_TranslateKeyPress( 'w', (byte)((bkbCurState[DIK_W] >> 7) & 0x01) );
    else if( bkbCurState[DIK_X] != bkbLastState[DIK_X] ) bind_TranslateKeyPress( 'x', (byte)((bkbCurState[DIK_X] >> 7) & 0x01) );
    else if( bkbCurState[DIK_Y] != bkbLastState[DIK_Y] ) bind_TranslateKeyPress( 'y', (byte)((bkbCurState[DIK_Y] >> 7) & 0x01) );
    else if( bkbCurState[DIK_Z] != bkbLastState[DIK_Z] ) bind_TranslateKeyPress( 'z', (byte)((bkbCurState[DIK_Z] >> 7) & 0x01) );
    else if( bkbCurState[DIK_ADD] != bkbLastState[DIK_ADD] ) bind_TranslateKeyPress( KP_PLUS, (byte)((bkbCurState[DIK_ADD] >> 7) & 0x01) );
    else if( bkbCurState[DIK_APOSTROPHE] != bkbLastState[DIK_APOSTROPHE] ) bind_TranslateKeyPress( '\'', (byte)((bkbCurState[DIK_APOSTROPHE] >> 7) & 0x01) );
    else if( bkbCurState[DIK_BACK] != bkbLastState[DIK_BACK] ) bind_TranslateKeyPress( K_BACKSPACE, (byte)((bkbCurState[DIK_BACK] >> 7) & 0x01) );
    else if( bkbCurState[DIK_BACKSLASH] != bkbLastState[DIK_BACKSLASH] ) bind_TranslateKeyPress( '\\', (byte)((bkbCurState[DIK_BACKSLASH] >> 7) & 0x01) );
    else if( bkbCurState[DIK_CAPITAL] != bkbLastState[DIK_CAPITAL] ) bind_TranslateKeyPress( K_CAPSLOCK, (byte)((bkbCurState[DIK_CAPITAL] >> 7) & 0x01) );
    else if( bkbCurState[DIK_COMMA] != bkbLastState[DIK_COMMA] ) bind_TranslateKeyPress( ',', (byte)((bkbCurState[DIK_COMMA] >> 7) & 0x01) );
    else if( bkbCurState[DIK_DECIMAL] != bkbLastState[DIK_DECIMAL] ) bind_TranslateKeyPress( KP_PERIOD, (byte)((bkbCurState[DIK_DECIMAL] >> 7) & 0x01) );
    else if( bkbCurState[DIK_DELETE] != bkbLastState[DIK_DELETE] ) bind_TranslateKeyPress( K_DELETE, (byte)((bkbCurState[DIK_DELETE] >> 7) & 0x01) );
    else if( bkbCurState[DIK_DIVIDE] != bkbLastState[DIK_DIVIDE] ) bind_TranslateKeyPress( KP_DIVIDE, (byte)((bkbCurState[DIK_DIVIDE] >> 7) & 0x01) );
    else if( bkbCurState[DIK_DOWN] != bkbLastState[DIK_DOWN] ) bind_TranslateKeyPress( K_DOWN, (byte)((bkbCurState[DIK_DOWN] >> 7) & 0x01) );
    else if( bkbCurState[DIK_END] != bkbLastState[DIK_END] ) bind_TranslateKeyPress( K_END, (byte)((bkbCurState[DIK_END] >> 7) & 0x01) );
    else if( bkbCurState[DIK_EQUALS] != bkbLastState[DIK_EQUALS] ) bind_TranslateKeyPress( '=', (byte)((bkbCurState[DIK_EQUALS] >> 7) & 0x01) );
    else if( bkbCurState[DIK_ESCAPE] != bkbLastState[DIK_ESCAPE] ) bind_TranslateKeyPress( K_ESCAPE, (byte)((bkbCurState[DIK_ESCAPE] >> 7) & 0x01) );
    else if( bkbCurState[DIK_F1] != bkbLastState[DIK_F1] ) bind_TranslateKeyPress( K_F1, (byte)((bkbCurState[DIK_F1] >> 7) & 0x01) );
    else if( bkbCurState[DIK_F2] != bkbLastState[DIK_F2] ) bind_TranslateKeyPress( K_F2, (byte)((bkbCurState[DIK_F2] >> 7) & 0x01) );
    else if( bkbCurState[DIK_F3] != bkbLastState[DIK_F3] ) bind_TranslateKeyPress( K_F3, (byte)((bkbCurState[DIK_F3] >> 7) & 0x01) );
    else if( bkbCurState[DIK_F4] != bkbLastState[DIK_F4] ) bind_TranslateKeyPress( K_F4, (byte)((bkbCurState[DIK_F4] >> 7) & 0x01) );
    else if( bkbCurState[DIK_F5] != bkbLastState[DIK_F5] ) bind_TranslateKeyPress( K_F5, (byte)((bkbCurState[DIK_F5] >> 7) & 0x01) );
    else if( bkbCurState[DIK_F6] != bkbLastState[DIK_F6] ) bind_TranslateKeyPress( K_F6, (byte)((bkbCurState[DIK_F6] >> 7) & 0x01) );
    else if( bkbCurState[DIK_F7] != bkbLastState[DIK_F7] ) bind_TranslateKeyPress( K_F7, (byte)((bkbCurState[DIK_F7] >> 7) & 0x01) );
    else if( bkbCurState[DIK_F8] != bkbLastState[DIK_F8] ) bind_TranslateKeyPress( K_F8, (byte)((bkbCurState[DIK_F8] >> 7) & 0x01) );
    else if( bkbCurState[DIK_F9] != bkbLastState[DIK_F9] ) bind_TranslateKeyPress( K_F9, (byte)((bkbCurState[DIK_F9] >> 7) & 0x01) );
    else if( bkbCurState[DIK_F10] != bkbLastState[DIK_F10] ) bind_TranslateKeyPress( K_F10, (byte)((bkbCurState[DIK_F10] >> 7) & 0x01) );
    else if( bkbCurState[DIK_F11] != bkbLastState[DIK_F11] ) bind_TranslateKeyPress( K_F11, (byte)((bkbCurState[DIK_F11] >> 7) & 0x01) );
    else if( bkbCurState[DIK_F12] != bkbLastState[DIK_F12] ) bind_TranslateKeyPress( K_F12, (byte)((bkbCurState[DIK_F12] >> 7) & 0x01) );
    else if( bkbCurState[DIK_GRAVE] != bkbLastState[DIK_GRAVE] ) bind_TranslateKeyPress( '`', (byte)((bkbCurState[DIK_GRAVE] >> 7) & 0x01) );
    else if( bkbCurState[DIK_HOME] != bkbLastState[DIK_HOME] ) bind_TranslateKeyPress( K_HOME, (byte)((bkbCurState[DIK_HOME] >> 7) & 0x01) );
    else if( bkbCurState[DIK_INSERT] != bkbLastState[DIK_INSERT] ) bind_TranslateKeyPress( K_INSERT, (byte)((bkbCurState[DIK_INSERT] >> 7) & 0x01) );
    else if( bkbCurState[DIK_LBRACKET] != bkbLastState[DIK_LBRACKET] ) bind_TranslateKeyPress( '[', (byte)((bkbCurState[DIK_LBRACKET] >> 7) & 0x01) );
    else if( bkbCurState[DIK_LCONTROL] != bkbLastState[DIK_LCONTROL] ) bind_TranslateKeyPress( K_LCTRL, (byte)((bkbCurState[DIK_LCONTROL] >> 7) & 0x01) );
    else if( bkbCurState[DIK_LEFT] != bkbLastState[DIK_LEFT] ) bind_TranslateKeyPress( K_LEFT, (byte)((bkbCurState[DIK_LEFT] >> 7) & 0x01) );
    else if( bkbCurState[DIK_LMENU] != bkbLastState[DIK_LMENU] ) bind_TranslateKeyPress( K_LALT, (byte)((bkbCurState[DIK_LMENU] >> 7) & 0x01) );
    else if( bkbCurState[DIK_LSHIFT] != bkbLastState[DIK_LSHIFT] ) bind_TranslateKeyPress( K_LSHIFT, (byte)((bkbCurState[DIK_LSHIFT] >> 7) & 0x01) );
    else if( bkbCurState[DIK_MINUS] != bkbLastState[DIK_MINUS] ) bind_TranslateKeyPress( '-', (byte)((bkbCurState[DIK_MINUS] >> 7) & 0x01) );
    else if( bkbCurState[DIK_MULTIPLY] != bkbLastState[DIK_MULTIPLY] ) bind_TranslateKeyPress( KP_MULTIPLY, (byte)((bkbCurState[DIK_MULTIPLY] >> 7) & 0x01) );
    else if( bkbCurState[DIK_NEXT] != bkbLastState[DIK_NEXT] ) bind_TranslateKeyPress( K_PGDN, (byte)((bkbCurState[DIK_NEXT] >> 7) & 0x01) );
    else if( bkbCurState[DIK_NUMLOCK] != bkbLastState[DIK_NUMLOCK] ) bind_TranslateKeyPress( KP_NUMLOCK, (byte)((bkbCurState[DIK_NUMLOCK] >> 7) & 0x01) );
    else if( bkbCurState[DIK_NUMPAD0] != bkbLastState[DIK_NUMPAD0] ) bind_TranslateKeyPress( KP_0, (byte)((bkbCurState[DIK_NUMPAD0] >> 7) & 0x01) );
    else if( bkbCurState[DIK_NUMPAD1] != bkbLastState[DIK_NUMPAD1] ) bind_TranslateKeyPress( KP_1, (byte)((bkbCurState[DIK_NUMPAD1] >> 7) & 0x01) );
    else if( bkbCurState[DIK_NUMPAD2] != bkbLastState[DIK_NUMPAD2] ) bind_TranslateKeyPress( KP_2, (byte)((bkbCurState[DIK_NUMPAD2] >> 7) & 0x01) );
    else if( bkbCurState[DIK_NUMPAD3] != bkbLastState[DIK_NUMPAD3] ) bind_TranslateKeyPress( KP_3, (byte)((bkbCurState[DIK_NUMPAD3] >> 7) & 0x01) );
    else if( bkbCurState[DIK_NUMPAD4] != bkbLastState[DIK_NUMPAD4] ) bind_TranslateKeyPress( KP_4, (byte)((bkbCurState[DIK_NUMPAD4] >> 7) & 0x01) );
    else if( bkbCurState[DIK_NUMPAD5] != bkbLastState[DIK_NUMPAD5] ) bind_TranslateKeyPress( KP_5, (byte)((bkbCurState[DIK_NUMPAD5] >> 7) & 0x01) );
    else if( bkbCurState[DIK_NUMPAD6] != bkbLastState[DIK_NUMPAD6] ) bind_TranslateKeyPress( KP_6, (byte)((bkbCurState[DIK_NUMPAD6] >> 7) & 0x01) );
    else if( bkbCurState[DIK_NUMPAD7] != bkbLastState[DIK_NUMPAD7] ) bind_TranslateKeyPress( KP_7, (byte)((bkbCurState[DIK_NUMPAD7] >> 7) & 0x01) );
    else if( bkbCurState[DIK_NUMPAD8] != bkbLastState[DIK_NUMPAD8] ) bind_TranslateKeyPress( KP_8, (byte)((bkbCurState[DIK_NUMPAD8] >> 7) & 0x01) );
    else if( bkbCurState[DIK_NUMPAD9] != bkbLastState[DIK_NUMPAD9] ) bind_TranslateKeyPress( KP_9, (byte)((bkbCurState[DIK_NUMPAD9] >> 7) & 0x01) );
    else if( bkbCurState[DIK_NUMPADENTER] != bkbLastState[DIK_NUMPADENTER] ) bind_TranslateKeyPress( KP_ENTER, (byte)((bkbCurState[DIK_NUMPADENTER] >> 7) & 0x01) );
    else if( bkbCurState[DIK_PAUSE] != bkbLastState[DIK_PAUSE] ) bind_TranslateKeyPress( K_PAUSE, (byte)((bkbCurState[DIK_PAUSE] >> 7) & 0x01) );
    else if( bkbCurState[DIK_PERIOD] != bkbLastState[DIK_PERIOD] ) bind_TranslateKeyPress( '.', (byte)((bkbCurState[DIK_PERIOD] >> 7) & 0x01) );
    else if( bkbCurState[DIK_PRIOR] != bkbLastState[DIK_PRIOR] ) bind_TranslateKeyPress( K_PGUP, (byte)((bkbCurState[DIK_PRIOR] >> 7) & 0x01) );
    else if( bkbCurState[DIK_RBRACKET] != bkbLastState[DIK_RBRACKET] ) bind_TranslateKeyPress( ']', (byte)((bkbCurState[DIK_RBRACKET] >> 7) & 0x01) );
    else if( bkbCurState[DIK_RCONTROL] != bkbLastState[DIK_RCONTROL] ) bind_TranslateKeyPress( K_RCTRL, (byte)((bkbCurState[DIK_RCONTROL] >> 7) & 0x01) );
    else if( bkbCurState[DIK_RETURN] != bkbLastState[DIK_RETURN] ) bind_TranslateKeyPress( K_ENTER, (byte)((bkbCurState[DIK_RETURN] >> 7) & 0x01) );
    else if( bkbCurState[DIK_RIGHT] != bkbLastState[DIK_RIGHT] ) bind_TranslateKeyPress( K_RIGHT, (byte)((bkbCurState[DIK_RIGHT] >> 7) & 0x01) );
    else if( bkbCurState[DIK_RMENU] != bkbLastState[DIK_RMENU] ) bind_TranslateKeyPress( K_RALT, (byte)((bkbCurState[DIK_RMENU] >> 7) & 0x01) );
    else if( bkbCurState[DIK_RSHIFT] != bkbLastState[DIK_RSHIFT] ) bind_TranslateKeyPress( K_RSHIFT, (byte)((bkbCurState[DIK_RSHIFT] >> 7) & 0x01) );
    else if( bkbCurState[DIK_SCROLL] != bkbLastState[DIK_SCROLL] ) bind_TranslateKeyPress( K_SCROLLLOCK, (byte)((bkbCurState[DIK_SCROLL] >> 7) & 0x01) );
    else if( bkbCurState[DIK_SEMICOLON] != bkbLastState[DIK_SEMICOLON] ) bind_TranslateKeyPress( ';', (byte)((bkbCurState[DIK_SEMICOLON] >> 7) & 0x01) );
    else if( bkbCurState[DIK_SLASH] != bkbLastState[DIK_SLASH] ) bind_TranslateKeyPress( '/', (byte)((bkbCurState[DIK_SLASH] >> 7) & 0x01) );
    else if( bkbCurState[DIK_SPACE] != bkbLastState[DIK_SPACE] ) bind_TranslateKeyPress( K_SPACE, (byte)((bkbCurState[DIK_SPACE] >> 7) & 0x01) );
    else if( bkbCurState[DIK_SUBTRACT] != bkbLastState[DIK_SUBTRACT] ) bind_TranslateKeyPress( KP_MINUS, (byte)((bkbCurState[DIK_SUBTRACT] >> 7) & 0x01) );
    else if( bkbCurState[DIK_TAB] != bkbLastState[DIK_TAB] ) bind_TranslateKeyPress( K_TAB, (byte)((bkbCurState[DIK_TAB] >> 7) & 0x01) );
    else if( bkbCurState[DIK_UP] != bkbLastState[DIK_UP] ) bind_TranslateKeyPress( K_UP, (byte)((bkbCurState[DIK_UP] >> 7) & 0x01) );

    // this state now = last state
    memcpy( bkbLastState, bkbCurState, 256 );

}

/* ------------
in_di_ProcessMouseImmediate - responds to the current state of the mouse (normal operation)
------------ */
static void in_di_ProcessMouseImmediate()
{
    HRESULT hRet;
    DIMOUSESTATE diCurMouseState;

    // assert
    // nb speed hack, already asserted mouse in ProcessEvents

    // clear the input buffer
    memset( &diCurMouseState, 0, sizeof(DIMOUSESTATE) );

    // get the mouse states
    hRet = diMouse->lpVtbl->GetDeviceState( diMouse, sizeof(DIMOUSESTATE), &diCurMouseState );
    if( hRet != DI_OK )
    {
        if( hRet == DIERR_INPUTLOST || hRet == DIERR_NOTACQUIRED )
        {
            in_di_Reacquire();
            return;
        }
        else
        {
            con_Print( "<RED>Input Error in Direct Input Process Events, Mouse:  reason:" );
            in_di_PrintError( hRet );
            return;
        }
    }

    // now for mouse button events
    if( diCurMouseState.rgbButtons[0] != diLastMouseState.rgbButtons[0] ) bind_TranslateKeyPress( M_LEFT, (byte)((diCurMouseState.rgbButtons[0] >> 7) & 0x01) );
    if( diCurMouseState.rgbButtons[1] != diLastMouseState.rgbButtons[1] ) bind_TranslateKeyPress( M_RIGHT, (byte)((diCurMouseState.rgbButtons[1] >> 7) & 0x01) );
    if( diCurMouseState.rgbButtons[2] != diLastMouseState.rgbButtons[2] ) bind_TranslateKeyPress( M_MIDDLE, (byte)((diCurMouseState.rgbButtons[2] >> 7) & 0x01) );

    // we need to check for a mouse inversion
    if( (int)var_GetFloat( vlmInvertX ) < 1 ) diCurMouseState.lX = -diCurMouseState.lX;
    if( (int)var_GetFloat( vlmInvertY ) < 1 ) diCurMouseState.lY = -diCurMouseState.lY;

    // record any mouse movement in the last frame
    state.mAxisPrev[YAW] = state.mAxis[YAW];
    state.mAxisDelta[YAW] = (short)diCurMouseState.lX;
    state.mAxis[YAW] = state.mAxis[YAW] + (short)diCurMouseState.lX;

    state.mAxisPrev[PITCH] = state.mAxis[PITCH];
    state.mAxisDelta[PITCH] = (short)diCurMouseState.lY;
    state.mAxis[PITCH] = state.mAxis[PITCH] + (short)diCurMouseState.lY;

    // the mouse wheel needs to snap to WHEEL_DELTA chunks for input
    state.mWheel = state.mWheel + (short)diCurMouseState.lZ;
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

    // this state now = last state
    memcpy( &diLastMouseState, &diCurMouseState, sizeof(DIMOUSESTATE) );
}

/* ------------
in_di_PrintError - send the generic error message to the console printer
------------ */
static void in_di_PrintError( HRESULT hRet )
{
    if( hRet == DI_OK ) return;

    switch( hRet )
    {
    case DIERR_BETADIRECTINPUTVERSION: // The application was written for an unsupported prerelease version of DirectInput.
        con_Print( "<RED>\tCurrently Installed Direct Input Version is a BETA." );
        break;
    case DIERR_INVALIDPARAM: // An invalid parameter was passed to the returning function, or the object was not in a state that permitted the function to be called. This value is equal to the E_INVALIDARG standard COM return value.
        con_Print( "<RED>\tAn invalid argument was specified." );
        break;
    case DIERR_OLDDIRECTINPUTVERSION: // The application requires a newer version of DirectInput.
        con_Print( "<RED>\tCurrently Installed version of Direct Input is old.  Please upgrade to the current version." );
        break;
    case DIERR_OUTOFMEMORY: // The DirectInput subsystem couldn't allocate sufficient memory to complete the call. This value is equal to the E_OUTOFMEMORY standard COM return value.
        con_Print( "<RED>\tUnable to allocate memory." );
        break;
    case DIERR_DEVICENOTREG: // The device or device instance is not registered with Microsoft® DirectInput®. This value is equal to the REGDB_E_CLASSNOTREG standard COM return value.
        con_Print( "<RED>\tThe Device Instance is not registered with DirectInput8" );
        break;
    case DIERR_NOINTERFACE: // The specified interface is not supported by the object. This value is equal to the E_NOINTERFACE standard COM return value.
        con_Print( "<RED>\tThe specified interface is not supported." );
        break;
    case DIERR_NOTINITIALIZED: // The object has not been initialized.
        con_Print( "<RED>\tThe Device has not been initialized." );
        break;
    case E_HANDLE: // The HWND parameter is not a valid top-level window that belongs to the process.
        con_Print( "<RED>\tThe HWND parameter is not a valid top-level window that belongs to this process." );
        break;
    case DIERR_ACQUIRED: // The operation cannot be performed while the device is acquired.
        con_Print( "<RED>\tThe device is already acquired." );
        break;
    case DIERR_OTHERAPPHASPRIO: // Another application has a higher priority level, preventing this call from succeeding. This value is equal to the E_ACCESSDENIED standard COM return value. This error can be returned when an application has only foreground access to a device but is attempting to acquire the device while in the background.
        con_Print( "<RED>\tThe application cannot acquire the device because it does not meet the prerequisites or another app has a higher priority." );
        break;
    case DIERR_NOTACQUIRED: // The operation cannot be performed unless the device is acquired.
        con_Print( "<RED>\tThe operation cannot be performed unless the device is acquired." );
        break;
    case E_PENDING: // Data is not yet available.
        con_Print( "<RED>\tDevice data is not yet available." );
        break;
    case DIERR_NOTBUFFERED:  // The device is not buffered. Set the DIPROP_BUFFERSIZE property to enable buffering.
        con_Print( "<RED>\tThis Device is not in buffered mode." );
        break;
    default:
        con_Print( "\t<RED>An unknown error has occurred." );
        break;
    }
}

/* ------------
in_di_Reacquire - attempts to reaquire lost controllers in the event they are unacquired
------------ */
static void in_di_Reacquire()
{
    HRESULT hRet;

    // assert
    if( diMouse == NULL )
    {
        con_Print( "<RED>Direct Input Reacquire Assert Failed:  diMouse is NULL!" );
        eng_Stop( "160006" );
        return;
    }
    if( diKeyboard == NULL )
    {
        con_Print( "<RED>Direct Input Reacquire Assert Failed:  diKeyboard is NULL!" );
        eng_Stop( "160007" );
        return;
    }

    // Commented these -- noisy, annoying and unneccesary
//  con_Print( "Reaquiring Direct Input Control..." );

    // reacquire mouse
    hRet = diMouse->lpVtbl->Acquire( diMouse );
    if( hRet != DI_OK )
    {
//      con_Print( "<RED>Reacquire failed for Mouse:  reason:" );
//      in_di_PrintError( hRet );
        return;
    }

    // reacquire the keyboard
    hRet = diKeyboard->lpVtbl->Acquire( diKeyboard );
    if( hRet != DI_OK )
    {
//      con_Print( "<RED>Reacquire failed for Keyboard:  reason:" );
//      in_di_PrintError( hRet );
        return;
    }
}

#endif
