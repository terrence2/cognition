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

// INCLUDES
/////////////
#include "TextEdit.h"

// DEFINITIONS
///////////////
#define TE_HEIGHT 40
#define TE_TEXT_SIZE 14.0f
#define TE_CURSOR_SIZE 24
#define TE_CURSOR_OFFSETX 5
#define TE_CURSOR_OFFSETY 7
#define TE_TEXT_OFFSETX 8
#define TE_TEXT_OFFSETY 8

#define TE_CURSOR24 "teCursor24"
#define TE_CURSOR8 "teCursor8"
#define TE_BACKGROUND24 "teBackground24"
#define TE_BACKGROUND8 "teBackground8"
#define TE_SELECTION24 "teSelection24"
#define TE_SELECTION8 "teSelection8"

// NAMESPACE
//////////////
namespace CW {

/* ------------
Constructor
------------ */
TextEdit::TextEdit() : Widget() {}

/* ------------
Constructor
------------ */
TextEdit::TextEdit( const string &newName, Widget *newParent, 
					const int &newX, const int &newY, const int &newW )
	: Widget( newName, newParent, newX, newY, newW, TE_HEIGHT )
{
	Initialize( newX, newY, newW );
}

/* ------------
Constructor
------------ */
bool TextEdit::Create( const string &newName, Widget *newParent, 
						const int &newX, const int &newY, const int &newW )
{
	// Widget init
	SetName( newName );
	SetParent( newParent );
	SetPosition( newX, newY );
	SetSize( newW, TE_HEIGHT );
	
	return Initialize( newX, newY, newW );
}

/* ------------
Construction Worker
------------ */
bool TextEdit::Initialize( const int &newX, const int &newY, const int &newW )
{
	// Widget properties
	SetFocusable();
	SetPushable();
	
	// properties
	m_fCurLength = 0;
	m_fMaxLength = newW - (2 * TE_TEXT_OFFSETX);
	m_text = "";
	m_iCursorPos = 0;
	m_bShifted = false;
	m_bInDrag = false;
	m_bHaveSelection = false;
	m_iStartDrag = 0;
	m_iEndDrag = 0;

	// Create child widgets
	m_tlText.Create( "", "TextEditText", this, newX + TE_TEXT_OFFSETX, 
										newY + TE_TEXT_OFFSETY );
	m_tlText.SetTextSize( TE_TEXT_SIZE );
	m_pmCursor.Create( TE_CURSOR24, TE_CURSOR8, "TextEditCursor", this, 
						newX + TE_CURSOR_OFFSETX, newY + TE_CURSOR_OFFSETY, 
						TE_CURSOR_SIZE, TE_CURSOR_SIZE );
	m_pmBackground.Create( TE_BACKGROUND24, TE_BACKGROUND8, "TextEditBG", this,
						newX, newY, newW, TE_HEIGHT );
	m_pmSelection.Create( TE_SELECTION24, TE_SELECTION8, "TextEditSelection", this,
						newX, newY, 10, 10 );
	m_pmSelection.Disable();

	// set default stuff
	if( IsFocused() ) m_pmCursor.Enable();
	else m_pmCursor.Disable();

	Enable();
	return true;
}

/* ------------
Destructor
------------ */
TextEdit::~TextEdit()
{
}

/* ------------
SetText
------------ */
void TextEdit::SetText( const string &newText )
{ 
	// reset properties
	m_iCursorPos = 0;
	m_bShifted = false;
	m_bInDrag = false;
	m_bHaveSelection = false;
	m_iStartDrag = 0;
	m_iEndDrag = 0;

	// truncate to max length
	float fCurLen = 0.0f;
	float fNextLen = 0.0f;
	m_text = "";
	for( unsigned int a = 0 ; a < newText.length() ; a++ )
	{
		fNextLen = ei()->d_GetCharWidth( newText[a], TE_TEXT_SIZE );
		if( fCurLen + fNextLen < m_fMaxLength )
		{
			m_text.append( 1, newText[a] );
		}
		else
		{
			m_tlText.SetText( m_text );
			return;
		}
	}
	
	m_tlText.SetText(m_text);	
}

/* ------------
PositionCursor
------------ */
void TextEdit::PositionCursor()
{
	// snap the cursor position to a relevant offset into the string
	if( m_iCursorPos > (signed)m_text.length() ) m_iCursorPos = (signed)m_text.length();
	if( m_iCursorPos < 0 ) m_iCursorPos = 0;

	// get the string through cursor position
	string start = m_text.substr( 0, m_iCursorPos );
	int width = (int)ei()->d_GetTextWidth( (char*)start.c_str(), TE_TEXT_SIZE );
	m_pmCursor.SetPosition( x() + TE_CURSOR_OFFSETX + width, y() + TE_CURSOR_OFFSETY );
}

/* ------------
GetClickPosition
------------ */
int TextEdit::GetClickPosition( const float &fx )
{
	int a = 0;
	float fLowBound = m_X + TE_TEXT_OFFSETX;
	float fHighBound;

	// catch the case where x < left of string
	if( fLowBound > fx ) 
	{
		return 0;
	}

	// iterate
	while( a < (signed)m_text.length() )
	{
		// get the high bound
		fHighBound = fLowBound + ei()->d_GetCharWidth( m_text[a], TE_TEXT_SIZE );

		// did we find the right letter
		if( fx > fLowBound && fx < fHighBound )
		{
			float fToLo = fx - fLowBound;
			float fToHi = fHighBound - fx;
			if( fToLo < fToHi )
			{
				return a;
			}
			else
			{
				return a + 1;
			}
		}

		// next
		fLowBound = fHighBound;
		a++;
	}

	// if we got here, x is past the text end
	return m_text.length();
}

/* ------------
GetOffsetPosition
------------ */
float TextEdit::GetOffsetPosition( const int &off )
{
	float fSize = 0.0f;
	for( int a = 0 ; a < off ; a++ )
	{
		fSize += ei()->d_GetCharWidth( m_text[a], TE_TEXT_SIZE );
	}
	return fSize;
}

/* ------------
RemoveSelection
// called on delete and backspace when a segment is highlighted
------------ */
void TextEdit::RemoveSelection()
{
	// do we have a selection at all?
	if( !m_bHaveSelection ) return;

	// get left and right side
	int iLeft = m_iStartDrag;
	int iRight = m_iEndDrag;

	// swap if they are reversed
	if( iLeft > iRight )
	{
		iLeft = m_iEndDrag;
		iRight = m_iStartDrag;
	}
	
	// erase a substring
	string sub = m_text.substr( iLeft, iRight - iLeft );
	m_fCurLength -= ei()->d_GetTextWidth( (char*)sub.c_str(), TE_TEXT_SIZE );
	m_text.erase( iLeft, iRight - iLeft );

	// set the position
	m_iCursorPos = iLeft;

	// clear the selection
	m_pmSelection.Disable();
	m_bHaveSelection = false;

	// lock the cursor to position
	if( m_iCursorPos < 0 ) m_iCursorPos = 0;
	if( m_iCursorPos > (signed)m_text.length() ) m_iCursorPos = (signed)m_text.length();

	// set the output
	m_tlText.SetText( m_text );
	PositionCursor();
}

/* ------------
KeyUpEvent
------------ */
void TextEdit::KeyUpEvent( const byte &key )
{
	Widget::KeyUpEvent(key);

	// clear the selection
	m_pmSelection.Disable();
	m_bHaveSelection = false;
	
	// shift out
	if( key == K_LSHIFT || key == K_RSHIFT ) m_bShifted = false;
}

/* ------------
KeyDownEvent
------------ */
void TextEdit::KeyDownEvent( const byte &key )
{
	Widget::KeyDownEvent(key);

	// we need this to be non-const so we can remap
	char k = key;

	// check for special non printing characters that work on the buffer
	switch( key )
	{
	case K_RIGHT:
		// clear the selection
		m_pmSelection.Disable();
		m_bHaveSelection = false;
		if( ei()->bind_CtrlIsDown() )
		{
			// ctrl + -> goes to end of line
			m_iCursorPos = m_text.length();
		}
		else
		{
			// -> advances once
			m_iCursorPos++;
			// make sure we aren't overextended
			if( m_iCursorPos > (signed)m_text.length() ) m_iCursorPos = (signed)m_text.length();
		}
		PositionCursor();
		return;
	case K_LEFT:
		// clear the selection
		m_pmSelection.Disable();
		m_bHaveSelection = false;
		if( ei()->bind_CtrlIsDown() ) 
		{
			m_iCursorPos = 0;
		}
		else
		{
			m_iCursorPos--;
			if( m_iCursorPos < 0 ) m_iCursorPos = 0;
		}
		PositionCursor();
		return;
	case K_END:
		// clear the selection
		m_pmSelection.Disable();
		m_bHaveSelection = false;
		m_iCursorPos = (signed)m_text.length();
		PositionCursor();
		return;
	case K_HOME:
		// clear the selection
		m_pmSelection.Disable();
		m_bHaveSelection = false;
		m_iCursorPos = 0;
		PositionCursor();
		return;
	case K_LSHIFT:
	case K_RSHIFT:
		// shift in
		// clear the selection
		m_pmSelection.Disable();
		m_bHaveSelection = false;
		m_bShifted = true;
		return;
	case K_BACKSPACE:
		// clearing a selecion region
		if( m_bHaveSelection )
		{
			RemoveSelection();
			return;
		}

		// can't backspace any farther
		if( m_iCursorPos == 0 ) return;

		// compute the length decriment
		m_fCurLength -= ei()->d_GetCharWidth( (char)m_text[m_iCursorPos-1], TE_TEXT_SIZE );

		// do the erase
		m_text.erase( m_iCursorPos - 1, 1 );
		m_iCursorPos--;

		// lock the cursor to position
		if( m_iCursorPos < 0 ) m_iCursorPos = 0;
		m_tlText.SetText( m_text );
		PositionCursor();
		return;
	case K_DELETE:
		// clearing a selecion region
		if( m_bHaveSelection )
		{
			RemoveSelection();
			return;
		}

		// single delete
		if( m_iCursorPos == (signed)m_text.length() ) return;

		// compute the length decriment
		m_fCurLength -= ei()->d_GetCharWidth( (char)m_text[m_iCursorPos-1], TE_TEXT_SIZE );

		// do the erasure
		m_text.erase( m_iCursorPos, 1 );

		// lock to position and update
		if( m_iCursorPos > (signed)m_text.length() ) m_iCursorPos = (signed)m_text.length();

		m_tlText.SetText( m_text );
		PositionCursor();
		return;
	// These ks need manual  remapping
	case KP_MINUS:	k = '-';	break;
	case KP_PLUS: 	k = '+';	break;
	case KP_MULTIPLY:k = '*';	break;
	case KP_PERIOD:	k = '.';	break;
	case KP_7:	k = '7';	break;
	case KP_8:	k = '8';	break;
	case KP_9:	k = '9';	break;
	case KP_4:	k = '4';	break;
	case KP_5:	k = '5';	break;
	case KP_6:	k = '6';	break;
	case KP_1:	k = '1';	break;
	case KP_2:	k = '2';	break;
	case KP_3:	k = '3';	break;
	case KP_0:	k = '0';	break;

	case K_ENTER:
	case KP_ENTER:
	case K_ESCAPE:
	case K_PGDN:
	case K_PGUP:
	case K_UP:
	case K_DOWN:
	case K_TAB:
	case K_F1:
	case K_F2:
	case K_F3:
	case K_F4:
	case K_F5:
	case K_F6:
	case K_F7:
	case K_F8:
	case K_F9:
	case K_F10:
	case K_F11:
	case K_F12:
	case K_SCROLLLOCK:
	case K_PAUSE:
	case K_INSERT:
	case K_LALT:
	case K_RALT:
	case K_LCTRL:
	case K_RCTRL:
	case KP_NUMLOCK:
		// these keys are ignored by us
		return;
	}

	// escape for control sequences
	if( ei()->bind_AltIsDown() || ei()->bind_CtrlIsDown() ) return;

	// remap shifted ks
	if( m_bShifted )
	{
		// the alpha set is easy
		if( (k >= 'a') && (k <= 'z') ) k -= 32;
		
		// for the top row ks
		else if( k == '`' ) k = 126;
		else if( k == '1' ) k = 33;
		else if( k == '2' ) k = 64;
		else if( k == '3' ) k = 35;
		else if( k == '4' ) k = 36;
		else if( k == '5' ) k = 37;
		else if( k == '6' ) k = 94;
		else if( k == '7' ) k = 38;
		else if( k == '8' ) k = 42;
		else if( k == '9' ) k = 40;
		else if( k == '0' ) k = 41;
		else if( k == '-' ) k = 95;
		else if( k == '=' ) k = 43;

		// map the braces and v-bar in the second from top
		else if( k == '[') k = 91;
		else if( k == ']') k = 93;

		// map the middle row
		else if( k == ';' ) k = 58;
		else if( k == '\'') k = 34;

		// for the second from bottom
		else if( k == ',' ) k = 60;
		else if( k == '.' ) k = 62;
		else if( k == '/' ) k = 63;
	}

	// remove any selection before placing this keystroke
	RemoveSelection();

	// does the buffer have room to insert this string?
	float fNewLen = m_fCurLength + ei()->d_GetCharWidth( k, TE_TEXT_SIZE );
	if( fNewLen >= m_fMaxLength ) return;
	
	// otherwise add the character to the string
	// N.B. I hate "string".
	string skey = "";
	skey.append( 1, k );
	if( m_iCursorPos == (signed)m_text.length() )
	{
		m_text.append( skey );
	}
	else
	{
		m_text.insert( m_iCursorPos, skey );
	}
	m_fCurLength = fNewLen;

	// set the label to the string
	m_tlText.SetText( m_text );

	// adjust the cursor
	m_iCursorPos++;
	PositionCursor();
}

/* ------------
MouseUpEvent
------------ */
void TextEdit::MouseUpEvent( const byte &button, const int &x, const int &y )
{
	Widget::MouseUpEvent(button,x,y);

	// get the unclick location
	m_iCursorPos = GetClickPosition( (float)x );
	PositionCursor();

	// End Drag
	m_iEndDrag = m_iCursorPos;
	m_bInDrag = false;
	if( m_iStartDrag == m_iEndDrag )
	{
		m_bHaveSelection = false;
		m_pmSelection.Disable();
	}
	else
	{
		m_bHaveSelection = true;
	}
}

/* ------------
MouseDownEvent
------------ */
void TextEdit::MouseDownEvent( const byte &button, const int &x, const int &y )
{
	Widget::MouseDownEvent(button,x,y);

	// Adjust for Click
	m_iCursorPos = GetClickPosition( (float)x );
	PositionCursor();

	// Start Drag
	m_bInDrag = true;
	m_bHaveSelection = false;
	m_iStartDrag = m_iEndDrag = m_iCursorPos;
}

/* ------------
MouseMoveEvent
------------ */
void TextEdit::MouseMoveEvent( const int &x, const int &y )
{
	Widget::MouseMoveEvent(x,y);

	if( m_bInDrag )
	{
		// Get the Current Drag Location
		m_iCursorPos = GetClickPosition( (float)x );
		
		// Get the Selection Extents
		float fLeft = GetOffsetPosition( m_iStartDrag );
		float fRight = GetOffsetPosition( m_iCursorPos );
		int iCurPos = (int)fRight;
		if( fLeft > fRight )
		{
			float fTmp = fRight;
			fRight = fLeft;
			fLeft = fTmp;
			iCurPos = (int)fLeft;
		}

		// Set up the selection rect
		m_pmSelection.Enable();
		m_pmSelection.SetPosition( m_X + TE_TEXT_OFFSETX + (int)fLeft, 
								m_Y + TE_CURSOR_OFFSETY );
		m_pmSelection.SetSize( (int)(fRight - fLeft), TE_CURSOR_SIZE );

		// keep the cursor under the mouse
		m_pmCursor.SetPosition( m_X + TE_CURSOR_OFFSETX + (int)iCurPos, m_Y + TE_CURSOR_OFFSETY );
	}
}

/* ------------
PositionChangeEvent
------------ */
void TextEdit::PositionChangeEvent( const int &x, const int &y )
{
	Widget::PositionChangeEvent(x,y);
	m_tlText.SetPosition( x + TE_TEXT_OFFSETX, y + TE_TEXT_OFFSETY );
	m_pmCursor.SetPosition( x + TE_CURSOR_OFFSETX, y + TE_CURSOR_OFFSETY );
	m_pmBackground.SetPosition( x, y );
}

/* ------------
SizeChangeEvent
------------ */
void TextEdit::SizeChangeEvent( const int &w, const int &h )
{
	Widget::SizeChangeEvent(w,h);
	m_pmBackground.SetSize( w, TE_HEIGHT );
}

/* ------------
FocusedEvent
------------ */
void TextEdit::FocusedEvent()
{
	Widget::FocusedEvent();
	m_pmCursor.Enable();
}

/* ------------
UnfocusedEvent
------------ */
void TextEdit::UnfocusedEvent()
{
	Widget::UnfocusedEvent();
	m_pmCursor.Disable();
}

/* ------------
Draw
------------ */
void TextEdit::Draw()
{
	m_pmBackground.Draw();
	m_pmSelection.Draw();
	m_tlText.Draw();
	m_pmCursor.Draw();
}


}
