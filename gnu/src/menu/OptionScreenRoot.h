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

#ifndef _OPTIONSCREENROOT_H_
#define _OPTIONSCREENROOT_H_

// INCLUDES
/////////////
#include "CogMenuGlobal.h"
#include "Screen.h"
#include "TextLabel.h"
#include "Pixmap.h"
#include "Button.h"
#include "SingleScreen2D.h"

// NAMESPACE
//////////////
using namespace CW;

// OPTIONSCREENROOT
//////////////////////
class OptionScreenRoot : public Screen
{
public:
	OptionScreenRoot( Widget *newParent );
	~OptionScreenRoot();

	// Event Handlers
	void KeyUpEvent( const byte &key );
	void MouseUpEvent( const byte &button, const int &x, const int &y );

	// Drawing Handler
	void Draw();

private:
	void DescendPlayer();
	void DescendGame();
	void DescendControls();
	void DescendVideo();
	void DescendSound();

private:
	TextLabel m_tlTitle;
	TextLabel m_tlPlayer;
	TextLabel m_tlGame;
	TextLabel m_tlControls;
	TextLabel m_tlVideo;
	TextLabel m_tlSound;
	Button m_bPlayer;
	Button m_bGame;
	Button m_bControls;
	Button m_bVideo;
	Button m_bSound;
	Button m_bBack;

//	SingleScreen2D *m_Screen2D;
//	SingleScreen3D *m_Screen3D;


};

#endif // _OPTIONSCREENROOT_H_
