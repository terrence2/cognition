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

#ifndef _RADIOBUTTONGROUP_H_
#define _RADIOBUTTONGROUP_H_

// INCLUDES
/////////////
#include "CogMenuGlobal.h"
#include "RadioButton.h"
#include <vector>

// NAMESPACE
//////////////
namespace CW {

// RADIOBUTTONGROUP
//////////////////////
class RadioButtonGroup : public Widget
{
public:
	RadioButtonGroup();
	RadioButtonGroup( const string &newName, Widget *newParent );
	~RadioButtonGroup();
	bool Create( const string &newName, Widget *newParent );

	// interface
	void AddButton( RadioButton &rb );
	void RemoveButton( RadioButton &rb );

	// events
	void KeyUpEvent( const byte &key );
	void KeyDownEvent( const byte &key );
	void MouseUpEvent( const byte &button, const int &x, const int &y );
	void MouseDownEvent( const byte &button, const int &x, const int &y );
	void EnabledEvent();
	void DisabledEvent();

	// logical abstraction only
	void Draw() {}

private:
	bool Initialize();
	RadioButton *GetClickedButton( const int &x, const int &y );
	RadioButton *GetKeyedButton();
	void SetCheckedButton( RadioButton *rb );
	void SetMinimalArea();

private:
	vector<RadioButton*> m_rbList;
};

}

#endif // _RADIOBUTTONGROUP_H_
