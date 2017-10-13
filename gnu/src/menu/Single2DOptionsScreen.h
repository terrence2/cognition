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

#ifndef _SINGLE2DOPTIONSSCREEN_H_
#define _SINGLE2DOPTIONSSCREEN_H_

// INCLUDES
/////////////
#include "CogMenuGlobal.h"
#include "Screen.h"
#include "Button.h"
#include "TextLabel.h"
#include "CheckButton.h"
#include "Slider.h"
#include "RadioButton.h"
#include "RadioButtonGroup.h"
#include "SingleScreen2D.h"

// NAMESPACE
//////////////
using namespace CW;

// SINGLE2DOPTIONSSCREEN
//////////////////////////
class Single2DOptionsScreen : public Screen
{
public:
	Single2DOptionsScreen( Widget *newParent );
	~Single2DOptionsScreen();
	
	// Event Handlers
	void KeyUpEvent( const byte &key );
	void MouseUpEvent( const byte &button, const int &x, const int &y );

	// Events
	void Draw();

private:
	// Action Handlers
	void UpdateVars();
	void UpdateUseQuestions();
	void UpdateGameType();
	void UpdateWidth();
	void UpdateHeight();
	void UpdateMineCount();
	void UpdateMaxMines();
	void DescendPlay();

private:
	// widgets
	TextLabel m_tlTitle;
	
	CheckButton m_ckUseQs;

	RadioButtonGroup m_rbgGameType;
	RadioButton m_rbGame2;
	RadioButton m_rbGame1;

	TextLabel m_tlWidth;
	Slider m_sliderWidth;
	TextLabel m_tlHeight;
	Slider m_sliderHeight;
	TextLabel m_tlMines;
	Slider m_sliderMines;

	Button m_bPlay;
	Button m_bBack;

	// child screens
	SingleScreen2D *m_Screen2D;

	// option states
	varlatch vlUseQuestion; // { "2d_use_questionmarks", "0", 0, 0, 0, NULL },
		// whether or not to use checkmarks on right button press
	varlatch vlWidth; //	{ "2d_width", "10", 10, 0, 0, NULL },
		// the grid width
	varlatch vlHeight; //	{ "2d_height", "10", 10, 0, 0, NULL },
		// the grid height
	varlatch vlGameType; //	{ "2d_game_type", "0", 0, 0, NULL },
		// 0 for edge and corner, 1 for edges only
	varlatch vlNumMines; 
};

#endif // _SINGLE2DOPTIONSSCREEN_H_
