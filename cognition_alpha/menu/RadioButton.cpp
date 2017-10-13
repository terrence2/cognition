// INCLUDES
/////////////
#include "RadioButton.h"

// NAMESPACE
//////////////
namespace CW {

// DEFINITIONS
////////////////
#define RADIO_BUTTON_SIZE 32
#define RADIO_TEXT_SIZE 14
#define RADIO_TEXT_OFFSET 5
#define RADIOBUTTON_DEF_UP_IMG24 "radioButtonUp24"
#define RADIOBUTTON_DEF_UP_IMG8 "radioButton8"
#define RADIOBUTTON_DEF_OVER_IMG24 "radioButtonOver24"
#define RADIOBUTTON_DEF_OVER_IMG8 "radioButton8"
#define RADIOBUTTON_DEF_DOWN_IMG24 "radioButtonDown24"
#define RADIOBUTTON_DEF_DOWN_IMG8 "radioButton8"
#define RADIOBUTTON_DEF_DISABLED_IMG24 "radioButtonDisabled24"
#define RADIOBUTTON_DEF_DISABLED_IMG8 "radioButton8"
#define RADIOBUTTON_DEF_INNER_IMG24 "radioButtonInner24"
#define RADIOBUTTON_DEF_INNER_IMG8 "radioButtonInner8"

/* ------------
Constructor
------------ */
RadioButton::RadioButton() : Widget() {}

/* ------------
Constructor
------------ */
RadioButton::RadioButton( const string &newLabel, const string &newName, Widget *newParent, 
						const int &newX, const int &newY )
	: Widget( newName, newParent, newX, newY, 10, RADIO_BUTTON_SIZE )
{
	// do custom init
	Initialize( newLabel, newX, newY );
}

/* ------------
Constructor
------------ */
bool RadioButton::Create( const string &newLabel, const string &newName, Widget *newParent, 
						const int &newX, const int &newY )
{
	// do widget init
	SetName( newName );
	SetParent( newParent );
	SetPosition( newX, newY );
	SetSize( 10, RADIO_BUTTON_SIZE );

	// do custom init
	return Initialize( newLabel, newX, newY );
}

/* ------------
Construction Worker
------------ */
bool RadioButton::Initialize( const string &newLabel, const int &newX, const int &newY )
{
	// init the button
	m_bButton.Create( "", "RadioButton", this, newX, newY, RADIO_BUTTON_SIZE, RADIO_BUTTON_SIZE );
	m_bButton.SetUpPixmap( RADIOBUTTON_DEF_UP_IMG24, RADIOBUTTON_DEF_UP_IMG8 );
	m_bButton.SetDownPixmap( RADIOBUTTON_DEF_DOWN_IMG24, RADIOBUTTON_DEF_DOWN_IMG8 );
	m_bButton.SetOverPixmap( RADIOBUTTON_DEF_OVER_IMG24, RADIOBUTTON_DEF_OVER_IMG8 );
	m_bButton.SetDisabledPixmap( RADIOBUTTON_DEF_DISABLED_IMG24, RADIOBUTTON_DEF_DISABLED_IMG8 );
	
	// CheckButton Properties
	m_IsSelected = false;
	
	// compute the size of the widget with text
	int tw = (int)ei()->d_GetTextWidth( (char*)newLabel.c_str(), RADIO_TEXT_SIZE );
	int th = (int)ei()->d_GetTextHeight( RADIO_TEXT_SIZE );

	// locate the text
	int tx = newX + RADIO_BUTTON_SIZE + RADIO_TEXT_OFFSET;
	int ty = newY + ((RADIO_BUTTON_SIZE - th) / 2);

	// compute overall widget size
	SetSize( tw + RADIO_TEXT_OFFSET + RADIO_BUTTON_SIZE, RADIO_BUTTON_SIZE );

	// create the label
	m_tlLabel.Create( newLabel, "CheckBoxLabel", this, tx, ty );
	m_tlLabel.SetTextSize( 14.0f );
	m_pmCenter.Create( RADIOBUTTON_DEF_INNER_IMG24, RADIOBUTTON_DEF_INNER_IMG8,
						"CheckBoxCheck", this, newX, newY, RADIO_BUTTON_SIZE, RADIO_BUTTON_SIZE );
	m_pmCenter.Disable();

	Enable();
	return true;

}

/* ------------
Destructor
------------ */
RadioButton::~RadioButton()
{
}

/* ------------
PositionChangeEvent
------------ */
void RadioButton::PositionChangeEvent( const int &x, const int &y )
{
	// compute the size of the widget with text
	int th = (int)ei()->d_GetTextHeight( RADIO_TEXT_SIZE );

	// locate the text
	int tx = x + RADIO_BUTTON_SIZE + RADIO_TEXT_OFFSET;
	int ty = y + ((RADIO_BUTTON_SIZE - th) / 2);

	// do the position calls
	m_tlLabel.SetPosition( tx, ty );
	m_bButton.SetPosition( x, y );
}

/* ------------
PushedDownEvent
------------ */
void RadioButton::PushedDownEvent()
{
	m_bButton.SetDown();
}

/* ------------
PushedUpEvent
------------ */
void RadioButton::PushedUpEvent()
{
	m_bButton.SetUp();
}

/* ------------
EnabledEvent
------------ */
void RadioButton::EnabledEvent()
{
	m_bButton.Enable();
}

/* ------------
DisabledEvent
------------ */
void RadioButton::DisabledEvent()
{
	m_bButton.Disable();
}

/* ------------
FocusedEvent
------------ */
void RadioButton::FocusedEvent()
{
	m_bButton.Focus();
}

/* ------------
UnfocusedEvent
------------ */
void RadioButton::UnfocusedEvent()
{
	m_bButton.Unfocus();
}
	
/* ------------
Draw
------------ */
void RadioButton::Draw()
{
	m_bButton.Draw();
	m_tlLabel.Draw();
	m_pmCenter.Draw();
}

}



