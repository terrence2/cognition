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
////////////
#include "OptionScreenRoot.h"

// DEFINITIONS
///////////////

// NAMESPACE
//////////////
using namespace CW;
	
/* ------------
Constructor
------------ */
OptionScreenRoot::OptionScreenRoot( Widget *newParent ) : Screen( "options", newParent )
{
	color blue = { 0, 0, 255, 255 };
	
	// The Title
	m_tlTitle.Create( "Options", "OptionsTitle", this, 70, 610 );
	m_tlTitle.SetTextSize( 64.0f );
	m_tlTitle.SetColor( blue );

	// Player
	m_tlPlayer.Create( "Modify Player Settings", "OptionsPlayerLabel", this, 370, 480 );
	m_bPlayer.Create( "Player", "OptionsPlayerButton", this, 200, 475, 150, 32 );

	// Game
	m_tlGame.Create( "Modify Game Settings", "OptionsGameLabel", this, 370, 405 );
	m_bGame.Create( "Game", "OptionsGameButton", this, 200, 400, 150, 32 );

	// Controls
	m_tlControls.Create( "Modify Control Settings", "OptionsControlsLabel", this, 370, 330 );
	m_bControls.Create( "Controls", "OptionsControlsButton", this, 200, 325, 150, 32 );

	// Video
	m_tlVideo.Create( "Modify Video Settings", "OptionsVideoLabel", this, 370, 255 );
	m_bVideo.Create( "Video", "OptionsVideoButton", this, 200, 250, 150, 32 );

	// Sound
	m_tlSound.Create( "Modify Sound Settings", "OptionsSoundLabel", this, 370, 180 );
	m_bSound.Create( "Sound", "OptionsSoundButton", this, 200, 175, 150, 32 );

	// back button
	m_bBack.Create( "Back", "OptionsBackButton", this, 850, 32, 150, 32 );

	// Add widgets to screen
	AddWidget( &m_tlTitle );
	AddWidget( &m_tlPlayer );
	AddWidget( &m_tlGame );
	AddWidget( &m_tlControls );
	AddWidget( &m_tlVideo );
	AddWidget( &m_tlSound );
	AddWidget( &m_bPlayer );
	AddWidget( &m_bGame );
	AddWidget( &m_bControls );
	AddWidget( &m_bVideo );
	AddWidget( &m_bSound );
	AddWidget( &m_bBack );

	// null the children screens

}


/* ------------
Destructor
------------ */
OptionScreenRoot::~OptionScreenRoot()
{
	// delete children screens
}

/* ------------
KeyUpEvent
------------ */
void OptionScreenRoot::KeyUpEvent( const byte &key )
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
		if( w->GetName() == "OptionsPlayerButton" )
		{
			DescendPlayer();
		}
		else if( w->GetName() == "OptionsGameButton" )
		{
			DescendGame();
		}
		else if( w->GetName() == "OptionsControlsButton" )
		{
			DescendControls();
		}
		else if( w->GetName() == "OptionsVideoButton" )
		{
			DescendVideo();
		}
		else if( w->GetName() == "OptionsSoundButton" )
		{
			DescendSound();
		}
		else if( w->GetName() == "OptionsBackButton" )
		{
			SetCurrentScreen( (Screen*)GetParent() );
		}
	}
}

/* ------------
MouseUpEvent
------------ */
void OptionScreenRoot::MouseUpEvent( const byte &button, const int &x, const int &y )
{
		// pass to the screen
	Screen::MouseUpEvent( button, x, y );
		
	// Check to see if we are in a widget
	if( m_bPlayer.Contains( x, y ) )
	{
		DescendPlayer();
	}
	else if( m_bGame.Contains( x, y ) )
	{
		DescendGame();
	}
	else if( m_bControls.Contains( x, y ) )
	{
		DescendControls();
	}
	else if( m_bVideo.Contains( x, y ) )
	{
		DescendVideo();
	}
	else if( m_bSound.Contains( x, y ) )
	{
		DescendSound();
	}
	else if( m_bBack.Contains( x, y ) )
	{
		SetCurrentScreen( (Screen*)GetParent() );
	}
}

/* ------------
Draw
------------ */
void OptionScreenRoot::Draw()
{
	m_tlTitle.Draw();
	m_tlPlayer.Draw();
	m_tlGame.Draw();
	m_tlControls.Draw();
	m_tlVideo.Draw();
	m_tlSound.Draw();
	m_bPlayer.Draw();
	m_bGame.Draw();
	m_bControls.Draw();
	m_bVideo.Draw();
	m_bSound.Draw();
	m_bBack.Draw();
}

/* ------------
DescendPlayer
------------ */
void OptionScreenRoot::DescendPlayer()
{
}

/* ------------
DescendGame
------------ */
void OptionScreenRoot::DescendGame()
{
}

/* ------------
DescendControls
------------ */
void OptionScreenRoot::DescendControls()
{
}

/* ------------
DescendVideo
------------ */
void OptionScreenRoot::DescendVideo()
{
}

/* ------------
DescendSound
------------ */
void OptionScreenRoot::DescendSound()
{
}
