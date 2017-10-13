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

#ifndef _ROOTSCREEN_H_
#define _ROOTSCREEN_H_

// INCLUDES
/////////////
#include "Screen.h"
#include "TextLabel.h"
#include "Pixmap.h"
#include "Button.h"
#include "SingleScreen.h"
#include "OptionScreenRoot.h"

// NAMESPACE
//////////////
using namespace CW;

// ROOTSCREEN
///////////////
class RootScreen : public Screen  
{
public:
	RootScreen();
	~RootScreen();
	
	// Event Handlers
	void KeyUpEvent( const byte &key );
	void MouseUpEvent( const byte &button, const int &x, const int &y );

	// Drawing Handler
	void Draw();

private:
	void DescendSingle();
	void DescendMulti();
	void DescendOptions();
	void DescendExit();

private:
	Pixmap m_pmTitle;
	TextLabel m_tlSingle;
	TextLabel m_tlMulti;
	TextLabel m_tlOptions;
	TextLabel m_tlExit;
	Button m_bSingle;
	Button m_bMulti;
	Button m_bOptions;
	Button m_bExit;

	SingleScreen *m_SingleScreen;
	// MultiScreen *m_MultiScreen;
	OptionScreenRoot *m_OptionsScreen;
};

#endif // _ROOTSCREEN_H_
