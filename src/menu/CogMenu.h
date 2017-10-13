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

#ifndef _COGMENU_H_
#define _COGMENU_H_

/*
	The Main Menu Class for Base Cognition
*/

#include "CogMenuGlobal.h"
//#include "CogMenuBG.h"
#include "RootScreen.h"
#include "Screen.h"

class CogMenu
{
public:
	CogMenu();
	~CogMenu();

	// Events
	void Frame( exec_state_t *state ); // a single animation step
	void Draw( exec_state_t *state ); // draw the menu
	void KeyPress( byte key, byte key_state ); // events

	// Accessors
	void SetCurrentScreen( Screen *scr ) { m_CurScreen = scr; }
	Screen *GetCurrentScreen() { return m_CurScreen; }

private:
	RootScreen *m_root;
	Screen *m_CurScreen;
	RootScreen *m_RootScreen;
};

#endif // _COGMENU_H_
