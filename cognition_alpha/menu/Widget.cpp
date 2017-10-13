// INCLUDES
/////////////
#include "CogMenuGlobal.h"
#include "Widget.h"

// NAMESPACE
//////////////
namespace CW {

// FUNCTIONALITY
//////////////////
/* ------------
Constructor
------------ */
Widget::Widget() : 	
	m_X(0), m_Y(0), m_W(100), m_H(100),
	m_Name( "Unnamed Widget" ), m_Parent(NULL),
	m_Enabled(false), m_Focused(false), m_IsDown(false), m_IsPushable(false)
{
	SetPosition( 0, 0 );
}

/* ------------
Constructor
------------ */
Widget::Widget( const string &newName, Widget *newParent, 
	    const int &newX, const int &newY, 
		const int &newW, const int &newH ) :
	m_X(newX), m_Y(newY), m_W(newW), m_H(newH),
	m_Name(newName), m_Parent(NULL),
	m_Enabled(true), m_Focused(false), m_IsDown(false), m_IsPushable(false)
{
	// set parent / position / verts / size ...
	SetParent( newParent );
	SetPosition( newX, newY );

	// get the current cursor position to find the state
	int x, y;
	ei()->ms_GetPosition( &x, &y );
	m_bMouseIsOver = ( Contains( x, y ) ) ? true : false;

	// set focus follows mouse properly
	m_Focused = MouseIsOver();
}

/* ------------
Destructor
------------ */
Widget::~Widget()
{
}

/* ------------
SetPosition
------------ */
void Widget::SetPosition( const int &x, const int &y )
{
	m_X = x;
	m_Y = y;
	m_Top = m_Y + m_H;
	m_Right = m_X + m_W;
	PositionChangeEvent( x, y );
}

/* ------------
SetSize
------------ */
void Widget::SetSize( const int &w, const int &h )
{
	m_W = w;
	m_H = h;
	m_Top = m_Y + m_H;
	m_Right = m_X + m_W;
	SizeChangeEvent( w, h );
}

/* ------------
Contains
------------ */
bool Widget::Contains( const int &x, const int &y ) const
{
	return  (x > m_X) && (x < m_Right) &&
			(y > m_Y) && (y < m_Top);
}

/* ------------
Contains
------------ */
bool Widget::Contains( const float &x, const float &y ) const
{
	return  ((int)x > m_X) && ((int)x < m_Right) &&
			((int)y > m_Y) && ((int)y < m_Top);
}
	

/* ------------
SetName
------------ */
void Widget::SetName( const string &newName )
{
	m_Name = newName;
	NameChangeEvent( newName );
}

/* ------------
SetParent
------------ */
void Widget::SetParent( Widget *&newParent )
{
	m_Parent = newParent;
	ParentChangeEvent( newParent );
}

/* ------------
<-
------------ */
Widget &Widget::operator =( const Widget &w )
{
	m_X = w.m_X;
	m_Y = w.m_Y;
	m_W = w.m_W;
	m_H = w.m_H;
	m_Top = w.m_Top;
	m_Right = w.m_Right;
	m_Name = w.m_Name;
	m_Parent = w.m_Parent;
	m_Enabled = w.m_Enabled;
	m_Focused = w.m_Focused;
	return *this;
}

}
