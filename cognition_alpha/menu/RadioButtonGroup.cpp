// INCLUDES
/////////////
#include "RadioButtonGroup.h"

// NAMESPACE
//////////////
namespace CW {

// DEFINITIONS
////////////////

/* ------------
Constructor
------------ */
RadioButtonGroup::RadioButtonGroup() : Widget() {}
	
/* ------------
Constructor
------------ */
RadioButtonGroup::RadioButtonGroup( const string &newName, Widget *newParent )
	:Widget( newName, newParent, 0, 0, 10, 10 )
{
	Initialize();
}

/* ------------
Constructor
------------ */
bool RadioButtonGroup::Create( const string &newName, Widget *newParent )
{
	SetName( newName );
	SetParent( newParent );
	SetPosition( 0, 0 );
	SetSize( 10, 10 );
	return Initialize();
}

/* ------------
Construction Worker
------------ */
bool RadioButtonGroup::Initialize()
{
	// set properties
	SetPushable();
	SetFocusable();

	// clear
	m_rbList.clear();

	// return
	Enable();
	return 1;
}

/* ------------
Destructor
------------ */
RadioButtonGroup::~RadioButtonGroup()
{
	m_rbList.clear();
}

/* ------------
AddButton
------------ */
void RadioButtonGroup::AddButton( RadioButton &rb )
{
	// add the widget to this list
	m_rbList.push_back( &rb );

	// update our screen location
	SetMinimalArea();	
}

/* ------------
RemoveButton
------------ */
void RadioButtonGroup::RemoveButton( RadioButton &rb )
{
	// linear probe
	for( vector<RadioButton*>::iterator i =  m_rbList.begin() ; i != m_rbList.end() ; ++i )
	{
		if( (*(*i)).GetName() == rb.GetName() )
		{
			m_rbList.erase( i );
			return;
		}
	}

	// update the screen location
	SetMinimalArea();
}

/* ------------
KeyUpEvent
------------ */
void RadioButtonGroup::KeyUpEvent( const byte &key )
{
	if( key == K_ENTER || key == KP_ENTER || key == K_SPACE )
	{
		RadioButton *rb = GetKeyedButton();
		if( rb ) SetCheckedButton( rb );
	}
}

/* ------------
KeyDownEvent
------------ */
void RadioButtonGroup::KeyDownEvent( const byte &key ) { }

/* ------------
MouseUpEvent
------------ */
void RadioButtonGroup::MouseUpEvent( const byte &button, const int &x, const int &y )
{
	ei()->con_Print( "%d, %d, %d, %d", this->x(), this->y(), Width(), Height() );
	RadioButton *rb = GetClickedButton( x, y );
	if( rb ) SetCheckedButton( rb );	
}

/* ------------
MouseDownEvent
------------ */
void RadioButtonGroup::MouseDownEvent( const byte &button, const int &x, const int &y ) {}

/* ------------
EnabledEvent
------------ */
void RadioButtonGroup::EnabledEvent()
{
	for( vector<RadioButton*>::iterator i =  m_rbList.begin() ; i != m_rbList.end() ; ++i )
	{
		(*i)->Enable();
	}
}

/* ------------
DisabledEvent
------------ */
void RadioButtonGroup::DisabledEvent()
{
	for( vector<RadioButton*>::iterator i =  m_rbList.begin() ; i != m_rbList.end() ; ++i )
	{
		(*i)->Disable();
	}
}

/* ------------
GetClickedButton
------------ */
RadioButton *RadioButtonGroup::GetClickedButton( const int &x, const int &y )
{
	for( vector<RadioButton*>::iterator i =  m_rbList.begin() ; i != m_rbList.end() ; ++i )
	{
		if( (*i)->Contains(x,y) )
		{
			return (*i);
		}
	}
	return NULL;
}

/* ------------
GetKeyedButton
------------ */
RadioButton *RadioButtonGroup::GetKeyedButton()
{
	for( vector<RadioButton*>::iterator i =  m_rbList.begin() ; i != m_rbList.end() ; ++i )
	{
		if( (*i)->IsFocused() )
		{
			return (*i);
		}
	}
	return NULL;
}

/* ------------
SetCheckedButton
------------ */
void RadioButtonGroup::SetCheckedButton( RadioButton *rb )
{
	for( vector<RadioButton*>::iterator i =  m_rbList.begin() ; i != m_rbList.end() ; ++i )
	{
		if( (*i) == rb )
		{
			(*i)->SetChecked();
		}
		else
		{
			(*i)->SetUnchecked();
		}
	}
}

/* ------------
SetMinimalArea
------------ */
void RadioButtonGroup::SetMinimalArea()
{
	// empty list
	if( m_rbList.size() <= 0 )
	{
		SetPosition( 0, 0 );
		SetSize( 0, 0 );
	}

	// set us to the first widget
	vector<RadioButton*>::iterator j = m_rbList.begin();
	SetPosition( (*j)->x(), (*j)->y() );
	SetSize( (*j)->Width(), (*j)->Height() );

	// iterate, extending to fit each radio button
	for( vector<RadioButton*>::iterator i =  m_rbList.begin() ; i != m_rbList.end() ; ++i )
	{
		// get the button
		RadioButton *rb = (*i);

		// get the sides of the current rect
		int left = x();
		int right = x() + Width();
		int bottom = y();
		int top = y() + Height();

		if( rb->x() < x() ) left = rb->x();
		if( rb->y() < y() ) bottom = rb->y();
		if( (rb->x() + rb->Width()) > right ) right = rb->x() + rb->Width();
		if( (rb->y() + rb->Height()) > top ) top = rb->y() + rb->Height();
		SetPosition( left, bottom );
		SetSize( right - left, top - bottom );
	}
}

}

