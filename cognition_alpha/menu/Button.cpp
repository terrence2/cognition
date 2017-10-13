// INCLUDES
/////////////
#include "Button.h"

// NAMESPACE
//////////////
namespace CW {

// DEFINITIONS
////////////////
#define BUTTON_CLICK_DOWN_SOUND "button"
#define BUTTON_DEF_UP_IMG24 "buttonUp24"
#define BUTTON_DEF_UP_IMG8 "buttonUp8"
#define BUTTON_DEF_OVER_IMG24 "buttonOver24"
#define BUTTON_DEF_OVER_IMG8 "buttonUp8"
#define BUTTON_DEF_DOWN_IMG24 "buttonDown24"
#define BUTTON_DEF_DOWN_IMG8 "buttonUp8"
#define BUTTON_DEF_DISABLED_IMG24 "buttonDisabled24"
#define BUTTON_DEF_DISABLED_IMG8 "buttonUp8"

/* ------------
Constructor
------------ */
Button::Button() : Widget()
{
}

/* ------------
Constructor
------------ */
Button::Button( const string &newLabel, const string &newName, Widget *newParent, 
				const int &newX, const int &newY, const int &newW, const int &newH )
				: Widget( newName, newParent, newX, newY, newW, newH )
{
	// common init
	Initialize( newLabel, newX, newY, newW, newH );
}

/* ------------
Create
------------ */
bool Button::Create( const string &newLabel, const string &newName, Widget *newParent, 
				const int &newX, const int &newY, const int &newW, const int &newH )
{
	// Constructor Stuff
	SetName( newName );
	SetParent( newParent );
	SetPosition( newX, newY );
	SetSize( newW, newH );

	// common init
	return Initialize( newLabel, newX, newY, newW, newH );
}

/* ------------
Initialize
------------ */
bool Button::Initialize( const string &newLabel, const int &newX, const int &newY, const int &newW, const int &newH )
{
	// Set Widget Properties
	SetFocusable();
	SetPushable();
	SetUp();

	// Set Button Properties
		// load images
	m_pmUp.Create( BUTTON_DEF_UP_IMG24, BUTTON_DEF_UP_IMG8, 
					"ButtonUpPM", this, newX, newY, newW, newH );
	m_pmDown.Create( BUTTON_DEF_DOWN_IMG24, BUTTON_DEF_DOWN_IMG8,
					"ButtonDownPM", this, newX, newY, newW, newH );
	m_pmOver.Create( BUTTON_DEF_OVER_IMG24, BUTTON_DEF_OVER_IMG8, 
					"ButtonOverPM", this, newX, newY, newW, newH );
	m_pmDisabled.Create( BUTTON_DEF_DISABLED_IMG24, BUTTON_DEF_DISABLED_IMG8,
					"ButtonDisabledPM", this, newX, newY, newW, newH );

	// compute a centering for the text
	m_tlLabel.Create( newLabel, "ButtonLabel", this, newX, newY );
	float fW = ei()->d_GetTextWidth( (char*)(m_tlLabel.GetText().c_str()), m_tlLabel.GetTextSize() );
	float fXOff = (newW - fW) / 2.0f;
	float fH = ei()->d_GetTextHeight( m_tlLabel.GetTextSize() );
	float fYOff = (newH - fH) / 2.0f;
	m_tlLabel.SetPosition( (int)((float)newX + fXOff), (int)((float)newY + fYOff) );

	// load the click sound
	m_bSndEnabled = true;
	m_sndDown = ei()->s_Load( BUTTON_CLICK_DOWN_SOUND );

	// Return Success
	Enable();
	return true;
}

/* ------------
Destructor
------------ */
Button::~Button()
{
	if( m_sndDown ) ei()->s_Unload( m_sndDown );
}

/* ------------
PositionChangeEvent
------------ */
void Button::PositionChangeEvent( const int &x, const int &y )
{
	// propogate position change to child widgets
	m_pmUp.SetPosition( x, y );
	m_pmDown.SetPosition( x, y );
	m_pmOver.SetPosition( x, y );
	m_pmDisabled.SetPosition( x, y );

	// compute a centering for the text
	float fW = ei()->d_GetTextWidth( (char*)(m_tlLabel.GetText().c_str()), m_tlLabel.GetTextSize() );
	float fXOff = (Width() - fW) / 2.0f;
	float fH = ei()->d_GetTextHeight( m_tlLabel.GetTextSize() );
	float fYOff = (Height() - fH) / 2.0f;
	m_tlLabel.SetPosition( (int)((float)x + fXOff), (int)((float)y + fYOff) );
}

/* ------------
SizeChangeEvent
------------ */
void Button::SizeChangeEvent( const int &w, const int &h )
{
	// propogate a size change to child widgets
	m_pmUp.SetSize( w, h );
	m_pmDown.SetSize( w, h );
	m_pmOver.SetSize( w, h );
	m_pmDisabled.SetSize( w, h );
	
	// compute a centering for the text
	float fW = ei()->d_GetTextWidth( (char*)(m_tlLabel.GetText().c_str()), m_tlLabel.GetTextSize() );
	float fXOff = (Width() - fW) / 2.0f;
	float fH = ei()->d_GetTextHeight( m_tlLabel.GetTextSize() );
	float fYOff = (Height() - fH) / 2.0f;
	m_tlLabel.SetPosition( (int)((float)x() + fXOff), (int)((float)y() + fYOff) );
}

/* ------------
PushedDownEvent
------------ */
void Button::PushedDownEvent()
{
	if( m_bSndEnabled && m_sndDown ) ei()->s_PlaySound( m_sndDown, 0, NULL, NULL );
}

/* ------------
PushedUpEvent
------------ */
void Button::PushedUpEvent()
{
}

/* ------------
Draw
------------ */
void Button::Draw()
{
	Pixmap *pm;
	
	if( !IsEnabled() )
	{
		pm = &m_pmDisabled;
	}

	else if( IsDown() )
	{
		pm = &m_pmDown;
	}

	else if( IsFocused() )
	{
		pm = &m_pmOver;
	}

	else
	{
		pm = &m_pmUp;
	}

	// do the pixmap draw
	if( pm ) pm->Draw();

	// draw on the label
	m_tlLabel.Draw();
}

/* ------------
SetUpPixmap
------------ */
void Button::SetUpPixmap( const string &colorMap, const string &alphaMap )
{
	m_pmUp.SetPixmap( colorMap, alphaMap );
}

/* ------------
SetDownPixmap
------------ */
void Button::SetDownPixmap( const string &colorMap, const string &alphaMap )
{
	m_pmDown.SetPixmap( colorMap, alphaMap );
}

/* ------------
SetOverPixmap
------------ */
void Button::SetOverPixmap( const string &colorMap, const string &alphaMap )
{
	m_pmOver.SetPixmap( colorMap, alphaMap );
}

/* ------------
SetDisabledPixmap
------------ */
void Button::SetDisabledPixmap( const string &colorMap, const string &alphaMap )
{
	m_pmDisabled.SetPixmap( colorMap, alphaMap );
}

/* ------------
SetText
------------ */
void Button::SetText( const string &newLabel )
{
	m_tlLabel.SetText( newLabel );
}

}
