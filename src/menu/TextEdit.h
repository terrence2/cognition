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

#ifndef _TEXTEDIT_H_
#define _TEXTEDIT_H_

// INCLUDES
/////////////
#include "CogMenuGlobal.h"
#include "Widget.h"
#include "TextLabel.h"
#include "Pixmap.h"
#include <string>

// NAMESPACE
//////////////
namespace CW {

// TEXTEDIT
/////////////
class TextEdit : public Widget
{
public:
	TextEdit();
	TextEdit( const string &newName, Widget *newParent, 
				const int &newX, const int &newY, const int &newW );
	~TextEdit();
	bool Create( const string &newName, Widget *newParent, 
				const int &newX, const int &newY, const int &newW );

	// Manipulators
	string GetText() const { return m_tlText.GetText(); }
	void SetText( const string &newText ); 

	// Event Handlers
	void KeyUpEvent( const byte &key );
	void KeyDownEvent( const byte &key );
	void MouseUpEvent( const byte &button, const int &x, const int &y );
	void MouseDownEvent( const byte &button, const int &x, const int &y );
	void MouseMoveEvent( const int &x, const int &y );
	void PositionChangeEvent( const int &x, const int &y );
	void SizeChangeEvent( const int &w, const int &h );
	void FocusedEvent();
	void UnfocusedEvent();

	// Drawing handler
	void Draw();

private:
	bool Initialize( const int &newX, const int &newY, const int &newW );
	void PositionCursor();
	int GetClickPosition( const float &fx );
	float GetOffsetPosition( const int &off );
	void RemoveSelection();

private:
	// widgets
	TextLabel m_tlText;
	Pixmap m_pmCursor;
	Pixmap m_pmBackground;
	Pixmap m_pmSelection;

	// states
	float m_fCurLength;
	float m_fMaxLength;
	string m_text;
	int m_iCursorPos;
	bool m_bShifted;
	bool m_bInDrag;
	bool m_bHaveSelection;
	int m_iStartDrag;
	int m_iEndDrag;
		
};

}

#endif // _TEXTEDIT_H_
