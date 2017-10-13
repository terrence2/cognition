// INCLUDES
/////////////
#include "CogMenuGlobal.h"
#include "Screen.h"
#include <assert.h>
#include <vector>

// NAMESPACE
//////////////
namespace CW {

/* ------------
Constructor
------------ */
Screen::Screen() : Widget()
{
}

/* ------------
Constructor
------------ */
Screen::Screen( const string &newName, Widget *newParent = (Widget*)NULL )
	: Widget( newName, newParent, 0, 0, SCR_WIDTH, SCR_HEIGHT )
{
	// Hijack the screen
	SetCurrentScreen( this );

	// Clear the Lists
	m_WidgetList.clear();
	m_FocusableList.clear();
	m_PushableList.clear();

	// Set properties
	SetUnfocusable();

	// Clear the Focus
	m_FocusWidget = NULL;
	m_KeyDownWidget = NULL;
	m_MouseDownWidget = NULL;
	m_FocusIter = m_FocusableList.begin();
}

/* ------------
Constructor
------------ */
bool Screen::Create( const string &newName, Widget *newParent = (Widget*)NULL )
{
	// Hijack the screen 
	SetCurrentScreen( this );

	// Setup Basic Properties
	SetName( newName );
	SetParent( newParent );
	SetPosition( 0, 0 );
	SetSize( SCR_WIDTH, SCR_HEIGHT );

	// Clear the Lists
	m_WidgetList.clear();
	m_FocusableList.clear();
	m_PushableList.clear();

	// Set properties
	SetUnfocusable();

	// Clear the Focus
	m_FocusWidget = NULL;
	m_KeyDownWidget = NULL;
	m_MouseDownWidget = NULL;
	m_FocusIter = m_FocusableList.begin();

	// return success
	return true;
}

/* ------------
Destructor
------------ */
Screen::~Screen()
{
	// Clear Lists
	m_WidgetList.clear();
	m_FocusableList.clear();
	m_PushableList.clear();

	// clear states
	m_FocusWidget = NULL;
	m_KeyDownWidget = NULL;
	m_MouseDownWidget = NULL;
}

/* ------------
AddWidget
------------ */
void Screen::AddWidget( Widget *newWidget )
{
	assert( newWidget );
	if( !newWidget ) return;

	// add to the main widget list
	m_WidgetList.push_back( newWidget );

	// add to the focus list
	if( newWidget->IsFocusable() )
	{
		m_FocusableList.push_back( newWidget );

		// reset the focus iterator
		m_FocusIter = m_FocusableList.begin();
		ChangeFocus( m_FocusIter );
	}

	// add to the focus pushable list
	if( newWidget->IsPushable() )
	{
		m_PushableList.push_back( newWidget );
	}
}

/* ------------
RemoveWidget
------------ */
void Screen::RemoveWidget( Widget *oldWidget )
{
	assert( oldWidget );
	if( !oldWidget ) return;

	// remove from the main list
	m_WidgetList.remove( oldWidget );
	
	// remove from the focusable list
	m_FocusableList.remove( oldWidget );

	// remove from the pushable list
	m_PushableList.remove( oldWidget );

	// reset the focus iterator
	if( m_FocusableList.size() > 0 ) 
	{ 
		m_FocusIter = m_FocusableList.begin(); 
		ChangeFocus( m_FocusIter );
	}
}

/* ------------
GetFocusedItem
------------ */
Widget *Screen::GetFocusedItem()
{
	// return the focus position only if it is valid
	if( m_FocusableList.size() > 0 )
	{
		return *m_FocusIter;
	}
	return NULL;
}

/* ------------
KeyDownEvent
------------ */
void Screen::KeyDownEvent( const byte &key )
{
	// Focus change events
	if( m_FocusableList.size() > 0 )
	{
		// Focus Forward
		if( (key == K_TAB && !(ei()->bind_ShiftIsDown())) || key == K_DOWN )
		{
			// move the iterator
			++m_FocusIter;
			if( m_FocusIter == m_FocusableList.end() ) m_FocusIter = m_FocusableList.begin();

			// change the focus
			ChangeFocus( m_FocusIter );
		}
		// Focus Backward
		else if( (key == K_TAB && (ei()->bind_ShiftIsDown())) || key == K_UP )
		{
			// move the iterator
			if( m_FocusIter == m_FocusableList.begin() ) m_FocusIter = m_FocusableList.end();
			--m_FocusIter;

			// change the focus
			ChangeFocus( m_FocusIter );
		}
		// Set Focused Pushable Down
		else if( key == K_ENTER || key == KP_ENTER || key == K_SPACE )
		{
			// get the focused item
			if( m_FocusWidget && m_FocusWidget->IsPushable() )
			{
				m_KeyDownWidget = m_FocusWidget;
				m_KeyDownWidget->SetDown();
				m_KeyDownWidget->KeyDownEvent(key);
			}
		}
		// other keys pass through to focusable items
		else
		{
			if( m_FocusWidget )
			{
				m_FocusWidget->KeyDownEvent(key);
			}
		}
	}
}

/* ------------
KeyUpEvent
------------ */
void Screen::KeyUpEvent( const byte &key )
{
	// Restore downed widgets
	if( m_KeyDownWidget )
	{
		m_KeyDownWidget->SetUp();
		m_KeyDownWidget = NULL;
	}

	// Pass Key Up Events to the focused widget
	Widget *w = GetFocusedItem();
	if( w ) w->KeyUpEvent( key );
}

/* ------------
MouseDownEvent
------------ */
void Screen::MouseDownEvent( const byte &button, const int &x, const int &y  )
{
	// check for pushes
	for( list<Widget*>::iterator i = m_PushableList.begin() ; i != m_PushableList.end() ; ++i )
	{
		Widget *w = *i;
		
		if( !w->IsDown() && w->Contains( x, y ) )
		{
			m_MouseDownWidget = w;
			w->SetDown();
			w->MouseDownEvent(button,x,y);
		}
	}
}

/* ------------
MouseUpEvent
------------ */
void Screen::MouseUpEvent( const byte &button, const int &x, const int &y  )
{
	if( m_MouseDownWidget )
	{
		m_MouseDownWidget->SetUp();
		m_MouseDownWidget->MouseUpEvent(button,x,y);
		m_MouseDownWidget = NULL;
	}
}

/* ------------
MouseMoveEvent
------------ */
void Screen::MouseMoveEvent( const int &x, const int &y )
{
	// compute new focusing
	for( list<Widget*>::iterator i = m_FocusableList.begin() ; i != m_FocusableList.end() ; ++i )
	{
		Widget *w = *i;
		// focus follows mouse sloppily
		if( (w != m_FocusWidget) && (w->Contains( x, y )) )
		{
			// change the mouse overness
			w->MouseInEvent( x, y );

			// Refocus
			ChangeFocus( i );
		}
		else if( (w == m_FocusWidget) && !(w->Contains( x, y )) )
		{
			// mouse out
			w->MouseOutEvent( x, y );
			
			// Don't refocus until we hit another item
		}
	}
	if( m_FocusWidget ) m_FocusWidget->MouseMoveEvent(x,y);
}

/* ------------
Change Focus
// Sets a new Focused object
------------ */
void Screen::ChangeFocus( list<Widget*>::iterator &i )
{
	// unfocus the previous
	if( m_FocusWidget ) m_FocusWidget->Unfocus();

	// focus the new
	if( *i ) (*i)->Focus();

	// swap the focus
	m_FocusWidget = *i;

	// update the traversal position
	m_FocusIter = i;
}

}
