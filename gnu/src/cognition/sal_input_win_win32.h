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
// sal_input_win_win32.h
// Created by Terrence Cole

// input system global prototypes for the win32 input driver

#ifndef SAL_INPUT_WIN_WIN32_H
#define SAL_INPUT_WIN_WIN32_H

int in_w32_Initialize(void);
void in_w32_Terminate(void);
void in_w32_ProcessEvents();
void in_w32_Event( unsigned char key, unsigned short down );
byte in_w32_MapKey( int key );
void in_w32_HideMouse(void);
void in_w32_UnhideMouse(void);

#ifndef WIN32
int in_w32_Initialize(void) {
    con_Print( "Input: Cannot initialize a win32 only subsystem on a non-win32 system." );
    return 0;
}

void in_w32_Terminate(void) {
    con_Print( "Input: Cannot terminate a win32 only subsystem on a non-win32 system." );
}

void in_w32_ProcessEvents() {
    con_Print( "Input Win32 called from non-win32 system." );
    assert(1==0);
}

void in_w32_Event( unsigned char key, unsigned short down ) {
    con_Print( "Input Win32 called from non-win32 system." );
    assert(1==0);
}

byte in_w32_MapKey( int key ) {
    con_Print( "Input Win32 called from non-win32 system." );
    assert(1==0);
    return 0;
}

void in_w32_HideMouse(void) {
    con_Print( "Input Win32 called from non-win32 system." );
    assert(1==0);
}

void in_w32_UnhideMouse(void) {
    con_Print( "Input Win32 called from non-win32 system." );
    assert(1==0);
}
#endif

#endif // SAL_INPUT_WIN_WIN32_H
