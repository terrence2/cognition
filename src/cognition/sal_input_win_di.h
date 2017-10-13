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
// sal_input_win_di.h
// Created by Terrence Cole

// input system global prototypes for the DirectInput input driver

#ifndef SAL_INPUT_WIN_DI_H
#define SAL_INPUT_WIN_DI_H

// the current version in this release
#define DIRECTINPUT_VERSION 0x0800

int in_di_Initialize(void);
void in_di_Terminate(void);
void in_di_ProcessEvents();

#ifndef WIN32
int in_di_Initialize(void) {
    con_Print( "Input: Cannot initialize a win32 only subsystem on a non-win32 system." );
    return 0;
}

void in_di_Terminate(void) {
    con_Print( "Input: Cannot terminate a win32 only subsystem on a non-win32 system." );
}

void in_di_ProcessEvents() {
    con_Print( "Input DirectInput called from non-win32 system." );
    assert(1==0);
}
#endif

#endif // SAL_INPUT_WIN_DI_H
