#ifndef _COGMENU_H_
#define _COGMENU_H_

/*
	The Main Menu Class for Base Cognition
*/

#include "CogMenuGlobal.h"
#include "CogMenuBG.h"
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