// INCLUDES
/////////////
#include "RootScreen.h"
#include "CogMenuGlobal.h"
#include "YesNoDialog.h"

/* ------------
Constructor
------------ */
RootScreen::RootScreen() : Screen( "root", NULL )
{
	// Title
	m_pmTitle.Create( "menu_title24", "menu_title8", "Title", this, 25, 768 - 275, 600, 150 );

	// Single Player
	m_tlSingle.Create( "Start a new Single Player game.", "SinglePlayerText", this, 370, 405 );
	m_bSingle.Create( "Single Player", "SinglePlayerButton", this, 200, 400, 150, 32 );

	// Multiplayer
	m_tlMulti.Create( "Start a new Multi-Player game.", "MultiPlayerText", this, 370, 330 );
	m_bMulti.Create( "Multi-Player", "MultiPlayerButton", this, 200, 325, 150, 32 );

	// Options
	m_tlOptions.Create( "Change game options.", "OptionsText", this, 370, 255 );
	m_bOptions.Create( "Options", "OptionsButton", this, 200, 250, 150, 32 );

	// Exit
	m_tlExit.Create( "Leave Cognition.", "ExitText", this, 370, 180 );
	m_bExit.Create( "Exit", "ExitButton", this, 200, 175, 150, 32 );

	// Add widgets to the screen
	AddWidget( &m_pmTitle );
	AddWidget( &m_tlSingle );
	AddWidget( &m_tlMulti );
	AddWidget( &m_tlOptions );
	AddWidget( &m_tlExit );
	AddWidget( &m_bSingle );
	AddWidget( &m_bMulti );
	AddWidget( &m_bOptions );
	AddWidget( &m_bExit );

	// null other stuff
	m_SingleScreen = NULL;
	// m_MultiScreen = NULL;
	m_OptionsScreen = NULL;

}

/* ------------
Destructor
------------ */
RootScreen::~RootScreen()
{
	SAFE_DELETE( m_SingleScreen );
	// SAFE_DELETE( m_MultiScreen );
	SAFE_DELETE( m_OptionsScreen );
}

/* ------------
KeyDownEvent
------------ */
void RootScreen::KeyUpEvent( const byte &key )
{
	// pass it to the parent
	Screen::KeyUpEvent( key );

	// do custom processing
	if( key == K_ESCAPE )
	{
		DescendExit();
	}
	else if( key == K_ENTER || key == KP_ENTER || key == K_SPACE )
	{
		// Get the focused Item
		Widget *w = GetFocusedItem();
		if( w->GetName() == "SinglePlayerButton" )
		{
			DescendSingle();
		}
		else if( w->GetName() == "MultiPlayerButton" )
		{
			DescendMulti();
		}
		else if( w->GetName() == "OptionsButton" )
		{
			DescendOptions();
		}
		else if( w->GetName() == "ExitButton" )
		{
			DescendExit();
		}
	}
}

/* ------------
MouseUpEvent
------------ */
void RootScreen::MouseUpEvent( const byte &button, const int &x, const int &y )
{
	// pass to the screen
	Screen::MouseUpEvent( button, x, y );
		
	// Check to see if we are in a widget
	if( m_bSingle.Contains( x, y ) )
	{
		DescendSingle();
	}
	else if( m_bMulti.Contains( x, y ) )
	{
		DescendMulti();
	}
	else if( m_bOptions.Contains( x, y ) )
	{
		DescendOptions();
	}
	else if( m_bExit.Contains( x, y ) )
	{
		DescendExit();
	}
}

/* ------------
Draw
------------ */
void RootScreen::Draw()
{
	// Do the Drawing for this Frame
	m_pmTitle.Draw();
	m_tlSingle.Draw();
	m_tlMulti.Draw();
	m_tlOptions.Draw();
	m_tlExit.Draw();
	m_bSingle.Draw();
	m_bMulti.Draw();
	m_bOptions.Draw();
	m_bExit.Draw();
}

/* ------------
DescendSingle
------------ */
void RootScreen::DescendSingle()
{
	if( !m_SingleScreen ) m_SingleScreen = new SingleScreen( this );
	else SetCurrentScreen( m_SingleScreen );
}

/* ------------
DescendMulti
------------ */
void RootScreen::DescendMulti()
{
//	if( !m_MultiScreen ) m_MultiScreen = new MultiScreen( this );
//	else SetCurrentScreen( m_MultiScreen );
}

/* ------------
DescendOptions
------------ */
void RootScreen::DescendOptions()
{
	if( !m_OptionsScreen ) m_OptionsScreen = new OptionScreenRoot( this );
	else SetCurrentScreen( m_OptionsScreen );
}

/* ------------
ShowExitOptions
------------ */
void RootScreen::DescendExit()
{
	ei()->eng_Stop( "0" );
}


