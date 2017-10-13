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

#ifndef _CHECKBUTTON_H_
#define _CHECKBUTTON_H_

// INCLUDES
/////////////
#include "CogMenuGlobal.h"
#include "Button.h"
#include "Widget.h"

// NAMESPACE
//////////////
namespace CW {

// BUTTON
///////////
class CheckButton : public Widget
{
public:
	CheckButton();
	CheckButton( const string &newLabel, const string &newName, Widget *newParent, 
						const int &newX, const int &newY );
	~CheckButton();
	bool Create( const string &newLabel, const string &newName, Widget *newParent, 
						const int &newX, const int &newY );

	// Event Handling
	void KeyUpEvent( const byte &key );
	void KeyDownEvent( const byte &key );
	void MouseUpEvent( const byte &button, const int &x, const int &y );
	void MouseDownEvent( const byte &button, const int &x, const int &y );
	void PositionChangeEvent( const int &x, const int &y );

	void PushedDownEvent();
	void PushedUpEvent();
	void EnabledEvent();
	void DisabledEvent();
	void FocusedEvent();
	void UnfocusedEvent();
	
	// Drawing Handler
	void Draw();
	
	// accessors
	void SetText( const string &newLabel ) { m_tlLabel.SetText(newLabel); }
	string GetText() const { return m_tlLabel.GetText(); }
	void SetColor( const color &newColor ) { m_tlLabel.SetColor(newColor); }
	void GetColor( color &thisColor ) { m_tlLabel.GetColor(thisColor); }

	void SetChecked() { m_IsChecked = true; m_pmCheck.Enable(); }
	void SetUnchecked() { m_IsChecked = false; m_pmCheck.Disable(); }
	bool IsChecked() const { return m_IsChecked; }

private:
	bool Initialize( const string &newLabel, const int &newX, const int &newY );
	void ToggleChecked();

private:
	// widgets
	Button m_bBox;
	TextLabel m_tlLabel;
	Pixmap m_pmCheck;

	// properties	
	bool m_IsChecked;
};

}

#endif // _CHECKBUTTON_H_
