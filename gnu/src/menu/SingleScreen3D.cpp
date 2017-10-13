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
#include "SingleScreen3D.h"

// DEFINITIONS
///////////////
#define BOARD_MIN_WIDTH 2
#define BOARD_MAX_WIDTH 12
#define BOARD_MIN_HEIGHT 2
#define BOARD_MAX_HEIGHT 12
#define BOARD_MIN_DEPTH 2
#define BOARD_MAX_DEPTH 12
#define BOARD_MIN_MINES 1
#define BOARD_MAX_MINES 20
#define MINE_COUNT_SIZE_RATIO 0.4
	
// NAMESPACE
//////////////
using namespace CW;
	
/* ------------
Constructor
------------ */
SingleScreen3D::SingleScreen3D( Widget *newParent )
	: Screen( "Single3DScreen", newParent )
{
	const color blue = { 0, 0, 255, 255 };
		
	// latch the persistant variables
	m_vlGameType = ei()->var_GetVarLatch( "3d_game_type" );
	m_vlHeight = ei()->var_GetVarLatch( "3d_height" );
	m_vlWidth = ei()->var_GetVarLatch( "3d_width" );
	m_vlDepth = ei()->var_GetVarLatch( "3d_depth" );
	m_vlUseQuestion = ei()->var_GetVarLatch( "3d_use_questionmarks" );
	m_vlNumMines = ei()->var_GetVarLatch( "3d_mine_count" );

	// Create interface Widgets
	m_tlTitle.Create( "3D SinglePlayer Game", "3DSingleTitle", this, 70, 650 ); 
	m_tlTitle.SetTextSize( 64.0f );
	m_tlTitle.SetColor( blue );
	m_bBack.Create( "Back", "3DSingleBackButton", this, 850, 32, 150, 32 );
	m_bLaunch.Create( "Launch", "3DSingleLaunchButton", this, 80, 20, 188, 40 );
	m_bEnd.Create( "End Current Game", "3DSingleEndButton", this, 280, 20, 188, 40 );
	m_bEnd.Disable();

	// Questions Checkbox
	m_ckUseQs.Create( "Use Question Marks", "3DOptionsUseQsCheck", this, 700, 500 );
	if( (int)ei()->var_GetFloat( m_vlUseQuestion ) ) { m_ckUseQs.SetChecked(); }
	else { m_ckUseQs.SetUnchecked(); }

	// Game Type
	m_rbGame3.Create( "26 Adjacent - Mines Associate By Face, Edge, and Corner Adjacency", "3DSingleRadioGame3", this, 80, 600 );
	m_rbGame2.Create( "18 Adjacent - Mines Associate By Face and Edge Adjacency", "3DSingleRadioGame2", this, 80, 550 );
	m_rbGame1.Create( "6 Adjacent - Mines Associate By Face Adjacency", "3DSingleRadioGame1", this, 80, 500 );
	m_rbgGameType.Create( "3DSingleGameType", this );
	m_rbgGameType.AddButton( m_rbGame3 );
	m_rbgGameType.AddButton( m_rbGame2 );
	m_rbgGameType.AddButton( m_rbGame1 );
	if( (int)ei()->var_GetFloat( m_vlGameType ) == 3 ) m_rbGame3.SetChecked();
	else if( (int)ei()->var_GetFloat( m_vlGameType ) == 2 ) m_rbGame2.SetChecked();
	else if( (int)ei()->var_GetFloat( m_vlGameType ) == 1 ) m_rbGame1.SetChecked();

	// Width Slider
	m_tlWidth.Create( "Width", "3DSingleWidthLabel", this, 80, 450 );
	m_tlWidth.SetTextSize( 24.0f );
	m_sliderWidth.Create( "3DSingleWidthSlider", this, 80, 400, 850 );
	m_sliderWidth.SetDelta( 1.0 );
	m_sliderWidth.SetMinimum( BOARD_MIN_WIDTH );
	m_sliderWidth.SetMaximum( BOARD_MAX_WIDTH );
	m_sliderWidth.SetInteger( true );
	m_sliderWidth.SetValue( (int)ei()->var_GetFloat(m_vlWidth) );

	// Height Slider
	m_tlHeight.Create( "Height", "3DSingleHeightLabel", this, 80, 350 );
	m_tlHeight.SetTextSize( 24.0f );
	m_sliderHeight.Create( "3DSingleHeightSlider", this, 80, 300, 850 );
	m_sliderHeight.SetDelta( 1.0 );
	m_sliderHeight.SetMinimum( BOARD_MIN_HEIGHT );
	m_sliderHeight.SetMaximum( BOARD_MAX_HEIGHT );
	m_sliderHeight.SetInteger( true );
	m_sliderHeight.SetValue( (int)ei()->var_GetFloat(m_vlHeight) );

	// Depth Slider
	m_tlDepth.Create( "Depth", "3DSingleDepthLabel", this, 80, 250 );
	m_tlDepth.SetTextSize( 24.0f );
	m_sliderDepth.Create( "3DSingleDepthSlider", this, 80, 200, 850 );
	m_sliderDepth.SetDelta( 1.0 );
	m_sliderDepth.SetMinimum( BOARD_MIN_DEPTH );
	m_sliderDepth.SetMaximum( BOARD_MAX_DEPTH );
	m_sliderDepth.SetInteger( true );
	m_sliderDepth.SetValue( (int)ei()->var_GetFloat(m_vlDepth) );
	
	// Mines Slider
	m_tlMines.Create( "Mines", "3DSingleMinesLabel", this, 80, 150);
	m_tlMines.SetTextSize( 24.0f );
	m_sliderMines.Create( "3DSingleMineSlider", this, 80, 100, 850 );
	m_sliderMines.SetDelta( 1.0 );
	m_sliderMines.SetMinimum( 1.0 );
	UpdateMaxMines();
	m_sliderMines.SetInteger( true );
	m_sliderMines.SetValue( ei()->var_GetFloat(m_vlNumMines) );

	// Add to screen
	AddWidget( &m_tlTitle );
	AddWidget( &m_ckUseQs );
	AddWidget( &m_rbGame3 );
	AddWidget( &m_rbGame2 );
	AddWidget( &m_rbGame1 );
	AddWidget( &m_rbgGameType );
	AddWidget( &m_tlWidth );
	AddWidget( &m_sliderWidth );
	AddWidget( &m_tlHeight );
	AddWidget( &m_sliderHeight );
	AddWidget( &m_tlDepth );
	AddWidget( &m_sliderDepth );
	AddWidget( &m_tlMines );
	AddWidget( &m_sliderMines );
	AddWidget( &m_bLaunch );
	AddWidget( &m_bEnd );
	AddWidget( &m_bBack );
}

/* ------------
Destructor
------------ */
SingleScreen3D::~SingleScreen3D()
{
}

/* ------------
KeyUpEvent
------------ */
void SingleScreen3D::KeyUpEvent( const byte &key )
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
		if( w->GetName() == "3DSingleLaunchButton" )
		{
			StartGame();
		}
	}
}

/* ------------
MouseUpEvent
------------ */
void SingleScreen3D::MouseUpEvent( const byte &button, const int &x, const int &y )
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
	else if( m_bLaunch.Contains( x, y ) )
	{
		StartGame();
	}
	else if( m_bEnd.Contains( x, y ) )
	{
		EndGame();
	}
}

/* ------------
Frame
------------ */
void SingleScreen3D::Frame( exec_state_t *state )
{
	if( state->bGameType != GAME_TYPE_NOGAME ) m_bEnd.Enable();
	else m_bEnd.Disable();
}
	
/* ------------
Draw
------------ */
void SingleScreen3D::Draw()
{
	m_tlTitle.Draw();
	m_bBack.Draw();
	m_bLaunch.Draw();
	m_ckUseQs.Draw();
	m_rbGame3.Draw();
	m_rbGame2.Draw();
	m_rbGame1.Draw();
	m_tlWidth.Draw();
	m_sliderWidth.Draw();
	m_tlHeight.Draw();
	m_sliderHeight.Draw();
	m_tlDepth.Draw();
	m_sliderDepth.Draw();
	m_tlMines.Draw();
	m_sliderMines.Draw();
	m_bEnd.Draw();
}

/* ------------
UpdateVars
------------ */
void SingleScreen3D::UpdateVars()
{
	UpdateUseQuestions();
	UpdateGameType();
	UpdateWidth();
	UpdateHeight();
	UpdateDepth();
	UpdateMineCount();
	UpdateMaxMines();
}
	
/* ------------
UpdateUseQuestions
------------ */
void SingleScreen3D::UpdateUseQuestions()
{
	if( m_ckUseQs.IsChecked() )
	{
		ei()->var_UpdateFloat( m_vlUseQuestion, 1.0 );
	}
	else
	{
		ei()->var_UpdateFloat( m_vlUseQuestion, 0.0 );
	}
}

/* ------------
UpdateGameType
------------ */
void SingleScreen3D::UpdateGameType()
{
	if( m_rbGame3.IsChecked() )
	{
		ei()->var_UpdateFloat( m_vlGameType, 3.0 );
	}
	else if( m_rbGame2.IsChecked() )
	{
		ei()->var_UpdateFloat( m_vlGameType, 2.0 );
	}
	else if( m_rbGame1.IsChecked() )
	{
		ei()->var_UpdateFloat( m_vlGameType, 1.0 );
	}
}

/* ------------
UpdateWidth
------------ */
void SingleScreen3D::UpdateWidth()
{
	double dWidth = m_sliderWidth.GetValue();
	if( (int)dWidth > BOARD_MAX_WIDTH ) m_sliderWidth.SetValue( BOARD_MAX_WIDTH );
	if( (int)dWidth < BOARD_MIN_WIDTH ) m_sliderWidth.SetValue( BOARD_MIN_WIDTH );
	ei()->var_UpdateFloat( m_vlWidth, m_sliderWidth.GetValue() );
}

/* ------------
UpdateHeight
------------ */
void SingleScreen3D::UpdateHeight()
{
	double dHeight = m_sliderHeight.GetValue();
	if( (int)dHeight > BOARD_MAX_HEIGHT ) m_sliderHeight.SetValue( BOARD_MAX_HEIGHT );
	if( (int)dHeight < BOARD_MIN_HEIGHT ) m_sliderHeight.SetValue( BOARD_MIN_HEIGHT );
	ei()->var_UpdateFloat( m_vlHeight, m_sliderHeight.GetValue() );
}

/* ------------
UpdateDepth
------------ */
void SingleScreen3D::UpdateDepth()
{
	double dDepth = m_sliderDepth.GetValue();
	if( (int)dDepth > BOARD_MAX_HEIGHT ) m_sliderDepth.SetValue( BOARD_MAX_HEIGHT );
	if( (int)dDepth < BOARD_MIN_HEIGHT ) m_sliderDepth.SetValue( BOARD_MIN_HEIGHT );
	ei()->var_UpdateFloat( m_vlDepth, m_sliderDepth.GetValue() );
}

/* ------------
UpdateMineCount
------------ */
void SingleScreen3D::UpdateMineCount()
{
	ei()->var_UpdateFloat( m_vlNumMines, m_sliderMines.GetValue() );
}

/* ------------
UpdateMaxMines
------------ */
void SingleScreen3D::UpdateMaxMines()
{
	double dSize = (double)((int)m_sliderWidth.GetValue() * 
							(int)m_sliderHeight.GetValue() *
							(int)m_sliderDepth.GetValue() );
	m_sliderMines.SetMaximum( (int)(dSize * MINE_COUNT_SIZE_RATIO) );
}

/* ------------
StartGame
------------ */
void SingleScreen3D::StartGame()
{
	char type[3];
	char game[2048];

	// select the game type
	if( m_rbGame1.IsChecked() ) snprintf( type, 3, "6" );
	else if( m_rbGame2.IsChecked() ) snprintf( type, 3, "18" );
	else if( m_rbGame3.IsChecked() ) snprintf( type, 3, "26" );

	// create the game string
	snprintf( game, 2048, "%s %d %d %d %d", type, (int)m_sliderWidth.GetValue(),
										(int)m_sliderHeight.GetValue(),
										(int)m_sliderDepth.GetValue(),
										(int)m_sliderMines.GetValue() );

	ei()->game_Start( game );
}

/* ------------
EndGame
------------ */
void SingleScreen3D::EndGame()
{
	if( m_bEnd.IsEnabled() ) ei()->game_End( NULL );
}
