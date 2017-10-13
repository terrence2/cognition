// INCLUDES
/////////////
#include "CogMenuGlobal.h"
#include "TextLabel.h"

// NAMESPACE
//////////////
namespace CW {

/* ------------
Constructor
------------ */
TextLabel::TextLabel() : Widget()
{
}

/* ------------
Constructor
------------ */
TextLabel::TextLabel( const string &newLabel, const string &newName, Widget *newParent, 
					 const int &newX, const int &newY ) 
	: Widget( newName, newParent, newX, newY, 10, 10 ), m_label(newLabel), m_size(12)
{
	// Set Widget Properties
	SetUnfocusable();
	SetUnpushable();

	// SetTextlabel Properties
	m_clr[0] = m_clr[1] = m_clr[2] = m_clr[3] = 255;
	m_label = newLabel;
	m_size = 14;
}

/* ------------
Destructor
------------ */
bool TextLabel::Create( const string &newLabel, const string &newName, 
					Widget *newParent, const int &newX, const int &newY ) 
{
	// Constructor Stuff
	SetName( newName );
	SetParent( newParent );
	SetPosition( newX, newY );

	// Widget Properties
	SetUnfocusable();
	SetUnpushable();
	
	// Set TextLabel Properties
	m_clr[0] = m_clr[1] = m_clr[2] = m_clr[3] = 255;
	m_label = newLabel;
	m_size = 14;

	// Return Success
	Enable();
	return true;
}

/* ------------
Destructor
------------ */
TextLabel::~TextLabel() 
{
}

/* ------------
Draw
------------ */
void TextLabel::Draw()
{
	if( !IsEnabled() ) return;
	ei()->d_TextLineColor( (char*)m_label.c_str(), m_size, m_clr, m_X, m_Y );
}

}
