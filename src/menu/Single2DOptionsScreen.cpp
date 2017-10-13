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
#include "Single2DOptionsScreen.h"

// DEFINITIONS
///////////////
#define BOARD_MIN_WIDTH 6
#define BOARD_MAX_WIDTH 30
#define BOARD_MIN_HEIGHT 3
#define BOARD_MAX_HEIGHT 17
#define MINE_COUNT_SIZE_RATIO 0.4

// NAMESPACE
//////////////
using namespace CW;
	
/* ------------
Constructor
------------ */
Single2DOptionsScreen::Single2DOptionsScreen( Widget *newParent )
	: Screen( "2DGameOptions", newParent )
{
	const color blue = { 0, 0, 255, 255 };

	// latch the persistant variables
	vlGameType = ei()->var_GetVarLatch( "2d_game_type" );
	vlHeight = ei()->var_GetVarLatch( "2d_height" );
	vlWidth = ei()->var_GetVarLatch( "2d_width" );
	vlUseQuestion = ei()->var_GetVarLatch( "2d_use_questionmarks" );
	vlNumMines = ei()->var_GetVarLatch( "2d_mine_count" );
	
	// Create interface Widgets
	m_tlTitle.Create( "2D Game Options", "2DOptionsTitle", this, 70, 610 ); 
	m_tlTitle.SetTextSize( 64.0f );
	m_tlTitle.SetColor( blue );
	m_bBack.Create( "Back", "2DOptionsBackButton", this, 850, 32, 150, 32 );
	m_bPlay.Create( "Play", "2DOptionsPlayButton", this, 150, 48, 175, 48 );

	// Questions Checkbox
	m_ckUseQs.Create( "Use Question Marks", "2DOptionsUseQsCheck", this, 80, 550 );
	if( (int)ei()->var_GetFloat( vlUseQuestion ) ) {	m_ckUseQs.SetChecked(); }
	else { m_ckUseQs.SetUnchecked(); }

	// Game Type
	m_rbGame2.Create( "8 - Adjacent Game Type", "2DOptionsRadioGame2", this, 450, 550 );
	m_rbGame1.Create( "4 - Adjacent Game Type", "2DOptionsRadioGame1", this, 450, 500 );
	m_rbgGameType.Create( "2DOptionsGameType", this );
	m_rbgGameType.AddButton( m_rbGame2 );
	m_rbgGameType.AddButton( m_rbGame1 );
	if( (int)ei()->var_GetFloat( vlGameType ) == 2 ) m_rbGame2.SetChecked();
	else if( (int)ei()->var_GetFloat( vlGameType ) == 1 ) m_rbGame1.SetChecked();
	
	// Width Slider
	m_tlWidth.Create( "Width", "2DOptionsWidthLabel", this, 80, 450 );
	m_tlWidth.SetTextSize( 24.0f );
	m_sliderWidth.Create( "2DOptionsWidthSlider", this, 80, 400, 850 );
	m_sliderWidth.SetDelta( 1.0 );
	m_sliderWidth.SetMinimum( BOARD_MIN_WIDTH );
	m_sliderWidth.SetMaximum( BOARD_MAX_WIDTH );
	m_sliderWidth.SetInteger( true );
	m_sliderWidth.SetValue( (int)ei()->var_GetFloat(vlWidth) );

	// Height Slider
	m_tlHeight.Create( "Height", "2DOptionsHeightLabel", this, 80, 350 );
	m_tlHeight.SetTextSize( 24.0f );
	m_sliderHeight.Create( "2DOptionsHeightSlider", this, 80, 300, 850 );
	m_sliderHeight.SetDelta( 1.0 );
	m_sliderHeight.SetMinimum( BOARD_MIN_HEIGHT );
	m_sliderHeight.SetMaximum( BOARD_MAX_HEIGHT );
	m_sliderHeight.SetInteger( true );
	m_sliderHeight.SetValue( (int)ei()->var_GetFloat(vlHeight) );
	
	// Mines Slider
	m_tlMines.Create( "Mines", "2DOptionsMinesLabel", this, 80, 250);
	m_tlMines.SetTextSize( 24.0f );
	m_sliderMines.Create( "2DOptionsMineSlider", this, 80, 200, 850 );
	m_sliderMines.SetDelta( 1.0 );
	m_sliderMines.SetMinimum( 1.0 );
	UpdateMaxMines();
	m_sliderMines.SetInteger( true );
	m_sliderMines.SetValue( ei()->var_GetFloat(vlNumMines) );

	// Add to screen
	AddWidget( &m_tlTitle );
	AddWidget( &m_ckUseQs );
	AddWidget( &m_rbGame2 );
	AddWidget( &m_rbGame1 );
	AddWidget( &m_rbgGameType );
	AddWidget( &m_tlWidth );
	AddWidget( &m_sliderWidth );
	AddWidget( &m_tlHeight );
	AddWidget( &m_sliderHeight );
	AddWidget( &m_tlMines );
	AddWidget( &m_sliderMines );
	AddWidget( &m_bBack );
	AddWidget( &m_bPlay );

	// NULL the children screens
	m_Screen2D = NULL;
}

/* ------------
Destructor
------------ */
Single2DOptionsScreen::~Single2DOptionsScreen()
{
	SAFE_DELETE( m_Screen2D );
}

/* ------------
KeyUpEvent
------------ */
void Single2DOptionsScreen::KeyUpEvent( const byte &key )
{	
	// pass it to the parent
	Screen::KeyUpEvent( key );

	// Do the update
	UpdateVars();

	// do custom processing
	if( key == K_ESCAPE )
	{
		SetCurrentScreen( (Screen*)GetParent() );
	}
	else if( key == K_ENTER || key == KP_ENTER || key == K_SPACE )
	{
		// Get the focused Item
		Widget *w = GetFocusedItem();
		if( w->GetName() == "2DOptionsPlayButton" )
		{
			DescendPlay();
		}
	}
}

/* ------------
MouseUpEvent
------------ */
void Single2DOptionsScreen::MouseUpEvent( const byte &button, const int &x, const int &y )
{
	// pass to the screen
	Screen::MouseUpEvent( button, x, y );

	// update values
	UpdateVars();

	// widget handlers
	if( m_bBack.Contains( x, y ) )
	{
		SetCurrentScreen( (Screen*)GetParent() );
	}
	else if( m_bPlay.Contains( x, y ) )
	{
		DescendPlay();
	}
}

/* ------------
Draw
------------ */
void Single2DOptionsScreen::Draw()
{
	m_tlTitle.Draw();
	m_ckUseQs.Draw();
	m_rbGame1.Draw();
	m_rbGame2.Draw();
	m_bPlay.Draw();
	m_bBack.Draw();
	m_tlWidth.Draw();
	m_sliderWidth.Draw();
	m_tlHeight.Draw();
	m_sliderHeight.Draw();
	m_tlMines.Draw();
	m_sliderMines.Draw();
}

/* ------------
UpdateVars
------------ */
void Single2DOptionsScreen::UpdateVars()
{
	UpdateUseQuestions();
	UpdateGameType();
	UpdateWidth();
	UpdateHeight();
	UpdateMineCount();
	UpdateMaxMines();
}
	
/* ------------
UpdateUseQuestions
------------ */
void Single2DOptionsScreen::UpdateUseQuestions()
{
	if( m_ckUseQs.IsChecked() )
	{
		ei()->var_UpdateFloat( vlUseQuestion, 1.0 );
	}
	else
	{
		ei()->var_UpdateFloat( vlUseQuestion, 0.0 );
	}
}

/* ------------
UpdateGameType
------------ */
void Single2DOptionsScreen::UpdateGameType()
{
	if( m_rbGame2.IsChecked() )
	{
		ei()->var_UpdateFloat( vlGameType, 2.0 );
	}
	else if( m_rbGame1.IsChecked() )
	{
		ei()->var_UpdateFloat( vlGameType, 1.0 );
	}
}

/* ------------
UpdateWidth
------------ */
void Single2DOptionsScreen::UpdateWidth()
{
	double dWidth = m_sliderWidth.GetValue();
	if( (int)dWidth > BOARD_MAX_WIDTH ) m_sliderWidth.SetValue( BOARD_MAX_WIDTH );
	if( (int)dWidth < BOARD_MIN_WIDTH ) m_sliderWidth.SetValue( BOARD_MIN_WIDTH );
	ei()->var_UpdateFloat( vlWidth, m_sliderWidth.GetValue() );
}

/* ------------
UpdateHeight
------------ */
void Single2DOptionsScreen::UpdateHeight()
{
	double dHeight = m_sliderHeight.GetValue();
	if( (int)dHeight > BOARD_MAX_HEIGHT ) m_sliderHeight.SetValue( BOARD_MAX_HEIGHT );
	if( (int)dHeight < BOARD_MIN_HEIGHT ) m_sliderHeight.SetValue( BOARD_MIN_HEIGHT );
	ei()->var_UpdateFloat( vlHeight, m_sliderHeight.GetValue() );
}

/* ------------
UpdateMineCount
------------ */
void Single2DOptionsScreen::UpdateMineCount()
{
	ei()->var_UpdateFloat( vlNumMines, m_sliderMines.GetValue() );
}

/* ------------
UpdateMaxMines
------------ */
void Single2DOptionsScreen::UpdateMaxMines()
{
	double dSize = (double)((int)m_sliderWidth.GetValue() * (int)m_sliderHeight.GetValue());
	m_sliderMines.SetMaximum( (int)(dSize * MINE_COUNT_SIZE_RATIO) );
}

/* ------------
DescendPlay
------------ */
void Single2DOptionsScreen::DescendPlay()
{
	// we always want to start a new game anyway
	if( m_Screen2D )
	{
		SAFE_DELETE( m_Screen2D );
	}

	// recreate it	
	m_Screen2D = new SingleScreen2D( this );
}
