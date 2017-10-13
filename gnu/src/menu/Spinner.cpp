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
#include "CogMenuGlobal.h"
#include "Spinner.h"
#include <limits>

// DEFINITIONS
////////////////
#define BUTTON_WIDTH 40
#define BUTTON_HEIGHT 18
#define BUTTON_UP_OFFSETX 110
#define BUTTON_UP_OFFSETY 22
#define BUTTON_DOWN_OFFSETX 110
#define BUTTON_DOWN_OFFSETY 0
#define TEXTEDIT_WIDTH 110

// NAMESPACE
//////////////
namespace CW {

/* ------------
Constructor
------------ */
Spinner::Spinner() : Widget() {}

/* ------------
Constructor
------------ */
Spinner::Spinner( const string &newName, Widget *newParent, 
				const int &newX, const int &newY )
	: Widget( newName, newParent, newX, newY, SPINNER_WIDTH, SPINNER_HEIGHT )
{
	// do common init
	Initialize( newX, newY );
}

/* ------------
Constructor
------------ */
bool Spinner::Create( const string &newName, Widget *newParent, 
				const int &newX, const int &newY )
{
	// do Widget stuff
	SetName( newName );
	SetParent( newParent );
	SetPosition( newX, newY );
	SetSize( SPINNER_WIDTH, SPINNER_HEIGHT );

	// do common init
	return Initialize( newX, newY );
}

/* ------------
Construction Worker
------------ */
bool Spinner::Initialize( const int &newX, const int &newY )
{
	// set widget properties
	SetPushable();
	SetFocusable();

	// set our properties
	m_bIsInteger = false;
	m_dValue = 0.0;
	m_dDelta = 1.0;

	m_dMinValue = numeric_limits<double>::infinity();
	m_dMaxValue = numeric_limits<double>::infinity();
	
	// init widgets
	m_bUp.Create( "+", "SpinnerUpButton", this, newX + BUTTON_UP_OFFSETX, 
				newY + BUTTON_UP_OFFSETY, BUTTON_WIDTH, BUTTON_HEIGHT );
	m_bDown.Create( "-", "SpinnerDownButton", this, newX + BUTTON_DOWN_OFFSETX, 
				newY + BUTTON_DOWN_OFFSETY, BUTTON_WIDTH, BUTTON_HEIGHT );
	m_teEdit.Create( "SpinnerEdit", this, newX, newY, TEXTEDIT_WIDTH );

	return true;
}

/* ------------
Destructor
------------ */
Spinner::~Spinner()
{
}

/* ------------
KeyUpEvent
------------ */
void Spinner::KeyUpEvent( const byte &key )
{
	Widget::KeyUpEvent(key);
	if( (key == KP_PLUS) || (key == '=') )
	{
		m_bUp.SetUp();
		Increment();
	}
	else if( (key == KP_MINUS) || (key == '-') )
	{
		m_bDown.SetUp();
		Decrement();
	}
	else
	{
		m_teEdit.KeyUpEvent(key);
	}
}

/* ------------
KeyDownEvent
------------ */
void Spinner::KeyDownEvent( const byte &key )
{
	Widget::KeyDownEvent(key);
	if( (key == KP_PLUS) || (key == '=') )
	{
		m_bUp.SetDown();
	}
	else if( (key == KP_MINUS) || (key == '-') )
	{
		m_bDown.SetDown();
	}
	else
	{
		m_teEdit.KeyDownEvent(key);
	}
}

/* ------------
MouseUpEvent
------------ */
void Spinner::MouseUpEvent( const byte &button, const int &x, const int &y )
{
	Widget::MouseUpEvent(button,x,y);
	
	// release any hold we have
	m_bDown.SetUp();
	m_bUp.SetUp();

	// delta
	if( m_bDown.Contains( x, y ) )
	{
		Decrement();
	}
	else if( m_bUp.Contains( x, y ) )
	{
		Increment();
	}
	else if( m_teEdit.Contains( x, y ) )
	{
		m_teEdit.MouseUpEvent( button, x, y );
	}
}

/* ------------
MouseDownEvent
------------ */
void Spinner::MouseDownEvent( const byte &button, const int &x, const int &y )
{
	Widget::MouseDownEvent(button,x,y);
	if( m_bDown.Contains( x, y ) )
	{
		m_bDown.SetDown();
	}
	else if( m_bUp.Contains( x, y ) )
	{
		m_bUp.SetDown();
	}
	else if( m_teEdit.Contains( x, y ) )
	{
		m_teEdit.MouseDownEvent( button, x, y );
	}
}

/* ------------
MouseMoveEvent
------------ */
void Spinner::MouseMoveEvent( const int &x, const int &y )
{
	Widget::MouseMoveEvent(x,y);
	m_teEdit.MouseMoveEvent( x, y );
}

/* ------------
PositionChangeEvent
------------ */
void Spinner::PositionChangeEvent( const int &x, const int &y )
{
	Widget::PositionChangeEvent(x,y);
	m_teEdit.SetPosition( x, y );
	m_bDown.SetPosition( x + BUTTON_DOWN_OFFSETX, y + BUTTON_DOWN_OFFSETY );
	m_bUp.SetPosition( x + BUTTON_UP_OFFSETX, y + BUTTON_UP_OFFSETY );
}

/* ------------
SizeChangeEvent
------------ */
void Spinner::SizeChangeEvent( const int &w, const int &h )
{
	// ignore it
	m_W = SPINNER_WIDTH;
	m_H = SPINNER_HEIGHT;
}

/* ------------
SizeChangeEvent
------------ */
void Spinner::FocusedEvent()
{
	Widget::FocusedEvent();
	m_teEdit.FocusedEvent();
}

/* ------------
UnfocusedEvent
------------ */
void Spinner::UnfocusedEvent()
{
	Widget::UnfocusedEvent();
	m_teEdit.UnfocusedEvent();
}

/* ------------
GetValue
------------ */
double Spinner::GetValue()
{
	if( m_bIsInteger )
	{
		return atoi( m_teEdit.GetText().c_str() );
	}
	else
	{
		return atof( m_teEdit.GetText().c_str() );
	}
}

/* ------------
SetDelta
------------ */
void Spinner::SetDelta( const double &newDelta )
{
	m_dDelta = newDelta;
}

/* ------------
SetValue
------------ */
void Spinner::SetValue( const double &newValue )
{
	// set the internal state
	m_dValue = newValue;
	if( m_dValue > m_dMaxValue ) m_dValue = m_dMaxValue;
	if( m_dValue < m_dMinValue ) m_dValue = m_dMinValue;

	// update the text edit
	char buffer[1024];
	if( m_bIsInteger )
	{
		snprintf( buffer, 1024, "%.0f", m_dValue );
	}
	else
	{
		snprintf( buffer, 1024, "%f", m_dValue );
	}
	m_teEdit.SetText( buffer );
}

/* ------------
SetMinimum
------------ */
void Spinner::SetMinimum( const double &newMin )
{
	m_dMinValue = newMin;
	// push up max to meet min
	if( m_dMinValue > m_dMaxValue )
	{
		m_dMaxValue = m_dValue = m_dMinValue;
		SetValue( m_dValue );
	}
	// bounds check the value
	if( m_dMinValue > m_dValue )
	{
		m_dValue = m_dMinValue;
		SetValue( m_dValue );
	}
}

/* ------------
SetMaximum
------------ */
void Spinner::SetMaximum( const double &newMax )
{
	m_dMaxValue = newMax;
	// push down min to meet max
	if( m_dMaxValue < m_dMinValue )
	{
		m_dMinValue = m_dValue = m_dMaxValue;
		SetValue( m_dValue );
	}
	// bounds check on value
	if( m_dMaxValue < m_dValue )
	{
		m_dValue = m_dMaxValue;
		SetValue( m_dValue );
	}
}

/* ------------
Draw
------------ */
void Spinner::Draw()
{
	m_teEdit.Draw();
	m_bUp.Draw();
	m_bDown.Draw();
}

}
