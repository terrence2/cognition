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

#ifndef _SCREEN_H_
#define _SCREEN_H_

// INCLUDES
/////////////
#include "Widget.h"
#include <list>

// NAMESPACE
//////////////
namespace CW {

// SCREEN
///////////
class Screen : public Widget  
{
public:
	// Construction / Destruction
	Screen();
	Screen( const string &newName, Widget *newParent );
	virtual ~Screen();
	bool Create( const string &newName, Widget *newParent );

	// Manage the Screen's Widget List
	void AddWidget( Widget *newWidget );
	void RemoveWidget( Widget *oldWidget );

	// Focus Usability
	Widget *GetFocusedItem();

	// Events
	void KeyDownEvent( const byte &key );
	void KeyUpEvent( const byte &key );
	void MouseDownEvent( const byte &button, const int &x, const int &y  );
	void MouseUpEvent( const byte &button, const int &x, const int &y  );
	void MouseMoveEvent( const int &x, const int &y );

	// Virtual Events
	virtual void Frame( exec_state_t *state ) {}

private:
	void ChangeFocus( list<Widget*>::iterator &i );

private:
	Widget *m_FocusWidget;
	Widget *m_KeyDownWidget;
	Widget *m_MouseDownWidget;
	list<Widget*> m_WidgetList;
	list<Widget*> m_FocusableList;
	list<Widget*> m_PushableList;
	list<Widget*>::iterator m_FocusIter;
};

}
#endif // _SCREEN_H_
