// INCLUDES
////////////
#include "SingleScreen.h"

// DEFINITIONS
///////////////

// NAMESPACE
//////////////
using namespace CW;
	
/* ------------
Constructor
------------ */
SingleScreen::SingleScreen( Widget *newParent ) 
	: Screen( "SingleScreen", newParent )
{
	color blue = { 0, 0, 255, 255 };
	
	// The Title
	m_tlTitle.Create( "Single Player Game", "SinglePlayerTitle", this, 70, 610 );
	m_tlTitle.SetTextSize( 64.0f );
	m_tlTitle.SetColor( blue );

	// 3D
	m_tl3D.Create( "3 Degrees of Freedom", "SinglePlayer3DLabel", this, 370, 405 );
	m_b3D.Create( "3D Game", "SinglePlayer3DButton", this, 200, 400, 150, 32 );

	// 2D
	m_tl2D.Create( "2 Degrees of Freedom", "SinglePlayer2DLabel", this, 370, 330 );
	m_b2D.Create( "2D Game", "SinglePlayer2DButton", this, 200, 325, 150, 32 );

	// back button
	m_bBack.Create( "Back", "SinglePlayerBackButton", this, 850, 32, 150, 32 );

	// Add widgets to screen
	AddWidget( &m_tlTitle );
	AddWidget( &m_tl3D );
	AddWidget( &m_tl2D );
	AddWidget( &m_b3D );
	AddWidget( &m_b2D );
	AddWidget( &m_bBack );

	// null the children screens
	m_Screen2D = NULL;
	m_Screen3D = NULL;
}

/* ------------
Destructor
------------ */
SingleScreen::~SingleScreen()
{
	SAFE_DELETE( m_Screen2D );
	SAFE_DELETE( m_Screen3D );
}

/* ------------
KeyUpEvent
------------ */
void SingleScreen::KeyUpEvent( const byte &key )
{
	// pass it to the parent
	Screen::KeyUpEvent( key );

	// do custom processing
	if( key == K_ESCAPE )
	{
		SetCurrentScreen( (Screen*)GetParent() );
	}
	else if( key == K_ENTER || key == KP_ENTER || key == K_SPACE )
	{
		// Get the focused Item
		Widget *w = GetFocusedItem();
		if( w->GetName() == "SinglePlayer3DButton" )
		{
			Descend3D();
		}
		else if( w->GetName() == "SinglePlayer2DButton" )
		{
			Descend2D();
		}
		else if( w->GetName() == "SinglePlayerBackButton" )
		{
			SetCurrentScreen( (Screen*)GetParent() );
		}
	}
}

/* ------------
MouseUpEvent
------------ */
void SingleScreen::MouseUpEvent( const byte &button, const int &x, const int &y )
{
	// pass to the screen
	Screen::MouseUpEvent( button, x, y );
		
	// Check to see if we are in a widget
	if( m_b3D.Contains( x, y ) )
	{
		Descend3D();
	}
	else if( m_b2D.Contains( x, y ) )
	{
		Descend2D();
	}
	else if( m_bBack.Contains( x, y ) )
	{
		SetCurrentScreen( (Screen*)GetParent() );
	}
}

/* ------------
Draw
------------ */
void SingleScreen::Draw()
{
	// Do the Drawing for this Frame
	m_tlTitle.Draw();
	m_tl3D.Draw();
	m_tl2D.Draw();
	m_b3D.Draw();
	m_b2D.Draw();
	m_bBack.Draw();
}

/* ------------
Descend2D
------------ */
void SingleScreen::Descend2D()
{
	if( !m_Screen2D ) m_Screen2D = new Single2DOptionsScreen( this );
	else SetCurrentScreen( m_Screen2D );
}

/* ------------
Descend3D
------------ */
void SingleScreen::Descend3D()
{
	if( !m_Screen3D ) m_Screen3D = new SingleScreen3D( this );
	else SetCurrentScreen( m_Screen3D );
}

