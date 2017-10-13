// INCLUDES
/////////////
#include "CheckButton.h"

// NAMESPACE
//////////////
namespace CW {

// DEFINITIONS
////////////////
#define CHECK_BOX_SIZE 32
#define CHECK_TEXT_SIZE 14
#define CHECK_TEXT_OFFSET 5
#define CHECKBUTTON_DEF_UP_IMG24 "checkButtonUp24"
#define CHECKBUTTON_DEF_UP_IMG8 "checkButton8"
#define CHECKBUTTON_DEF_OVER_IMG24 "checkButtonOver24"
#define CHECKBUTTON_DEF_OVER_IMG8 "checkButton8"
#define CHECKBUTTON_DEF_DOWN_IMG24 "checkButtonDown24"
#define CHECKBUTTON_DEF_DOWN_IMG8 "checkButton8"
#define CHECKBUTTON_DEF_DISABLED_IMG24 "checkButtonDisabled24"
#define CHECKBUTTON_DEF_DISABLED_IMG8 "checkButton8"
#define CHECKBUTTON_DEF_INNER_IMG24 "checkButtonInner24"
#define CHECKBUTTON_DEF_INNER_IMG8 "checkButtonInner8"

/* ------------
Constructor
------------ */
CheckButton::CheckButton() : Widget() {}

/* ------------
Constructor
------------ */
CheckButton::CheckButton( const string &newLabel, const string &newName, 
						Widget *newParent, const int &newX, const int &newY )
	: Widget( newName, newParent, newX, newY, 10, 10 )
{
	Initialize( newLabel, newX, newY );
}

/* ------------
Constructor
------------ */
bool CheckButton::Create( const string &newLabel, const string &newName, Widget *newParent, 
						const int &newX, const int &newY )
{
	// Do Constructor Stuff
	SetName( newName );
	SetParent( newParent );
	SetPosition( newX, newY );

	// do init
	return Initialize( newLabel, newX, newY );
}

/* ------------
Construction Worker
------------ */
bool CheckButton::Initialize( const string &newLabel, const int &newX, const int &newY )
{
	// set properties
	SetPushable();
	SetFocusable();

	// CheckButton Properties
	m_IsChecked = false;
	
	// compute the size of the widget with text
	int tw = (int)ei()->d_GetTextWidth( (char*)newLabel.c_str(), CHECK_TEXT_SIZE );
	int th = (int)ei()->d_GetTextHeight( CHECK_TEXT_SIZE );

	// locate the text
	int tx = newX + CHECK_BOX_SIZE + CHECK_TEXT_OFFSET;
	int ty = newY + ((CHECK_BOX_SIZE - th) / 2);

	// compute overall widget size
	SetSize( tw + CHECK_TEXT_OFFSET + CHECK_BOX_SIZE, CHECK_BOX_SIZE );

	// create the widgets
	m_bBox.Create( "", "CheckBoxButton", this, newX, newY, CHECK_BOX_SIZE, CHECK_BOX_SIZE );
	m_tlLabel.Create( newLabel, "CheckBoxLabel", this, tx, ty );
	m_tlLabel.SetTextSize( 14.0f );
	m_pmCheck.Create( CHECKBUTTON_DEF_INNER_IMG24, CHECKBUTTON_DEF_INNER_IMG8,
						"CheckBoxCheck", this, newX, newY, CHECK_BOX_SIZE, CHECK_BOX_SIZE );
	m_pmCheck.Disable();

	// change out the pixmap pictures
	m_bBox.SetUpPixmap( CHECKBUTTON_DEF_UP_IMG24, CHECKBUTTON_DEF_UP_IMG8 );
	m_bBox.SetOverPixmap( CHECKBUTTON_DEF_OVER_IMG24, CHECKBUTTON_DEF_OVER_IMG8 );
	m_bBox.SetDownPixmap( CHECKBUTTON_DEF_DOWN_IMG24, CHECKBUTTON_DEF_DOWN_IMG8 );
	m_bBox.SetDisabledPixmap( CHECKBUTTON_DEF_DISABLED_IMG24, CHECKBUTTON_DEF_DISABLED_IMG8 );

	Enable();
	return true;
}

/* ------------
Destructor
------------ */
CheckButton::~CheckButton()
{
}

/* ------------
KeyUpEvent
------------ */
void CheckButton::KeyUpEvent( const byte &key )
{
	Widget::KeyUpEvent(key);
	if( (key == K_ENTER) || (key == KP_ENTER) || (key == K_SPACE) ) ToggleChecked();
}

/* ------------
KeyDownEvent
------------ */
void CheckButton::KeyDownEvent( const byte &key )
{
	Widget::KeyDownEvent(key);
}

/* ------------
MouseUpEvent
------------ */
void CheckButton::MouseUpEvent( const byte &button, const int &x, const int &y )
{
	Widget::MouseUpEvent(button,x,y);
	ToggleChecked();
}

/* ------------
MouseDownEvent
------------ */
void CheckButton::MouseDownEvent( const byte &button, const int &x, const int &y )
{
	Widget::MouseDownEvent(button,x,y);
}

/* ------------
PositionChangeEvent
------------ */
void CheckButton::PositionChangeEvent( const int &x, const int &y )
{
	m_bBox.SetPosition( x, y );
	
	// compute the size of the widget with text
	int th = (int)ei()->d_GetTextHeight( CHECK_TEXT_SIZE );

	// locate the text
	int tx = x + CHECK_BOX_SIZE + CHECK_TEXT_OFFSET;
	int ty = y + ((CHECK_BOX_SIZE - th) / 2);

	m_tlLabel.SetPosition( tx, ty );
}

/* ------------
PushedDownEvent
------------ */
void CheckButton::PushedDownEvent()
{
	m_bBox.SetDown();
}

/* ------------
PushedUpEvent
------------ */
void CheckButton::PushedUpEvent()
{
	m_bBox.SetUp();
}

/* ------------
EnabledEvent
------------ */
void CheckButton::EnabledEvent()
{
	m_bBox.Enable();
}

/* ------------
DisabledEvent
------------ */
void CheckButton::DisabledEvent()
{
	m_bBox.Disable();
}

/* ------------
FocusedEvent
------------ */
void CheckButton::FocusedEvent()
{
	m_bBox.Focus();
}

/* ------------
UnfocusedEvent
------------ */
void CheckButton::UnfocusedEvent()
{
	m_bBox.Unfocus();
}
	
/* ------------
Draw
------------ */
void CheckButton::Draw()
{
	m_bBox.Draw();
	m_tlLabel.Draw();
	m_pmCheck.Draw();
}

/* ------------
ToggleChecked
------------ */
void CheckButton::ToggleChecked()
{
	// toggle
	if( m_IsChecked )
	{
		m_IsChecked = false;
		m_pmCheck.Disable();
	}
	else
	{
		m_IsChecked = true;
		m_pmCheck.Enable();
	}
}	


}

