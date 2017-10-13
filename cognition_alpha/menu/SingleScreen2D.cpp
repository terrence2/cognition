// INCLUDES
////////////
#include "CogMenuGlobal.h"
#include "SingleScreen2D.h"
#include <stdio.h>
#include <gl/gl.h>

// DEFINITIONS
///////////////
#define BOARD_TOP_PAD 40
#define BUTTON_SIZE 32
#define RESTART_BUTTON_SIZE 48
#define TL_TIMER_TEXT_HEIGHT 28
#define TL_TIMER_DEC_OFFSET 48
#define UP_PIXMAP24 "game2DbuttonUp24"
#define UP_PIXMAP8 "game2Dbutton8"
#define DOWN_PIXMAP24 "game2DbuttonDown24"
#define DOWN_PIXMAP8 "game2Dbutton8"
#define QUESTION_PIXMAP24 "game2DbuttonUpQuestion24"
#define QUESTION_PIXMAP8 "game2Dbutton8"
#define FLAG_PIXMAP24 "game2DbuttonUpFlag24"
#define FLAG_PIXMAP8 "game2Dbutton8"
#define MINE_PIXMAP24 "game2DbuttonDisabledMine24"
#define MINE_PIXMAP8 "game2Dbutton8"
#define PM0_PIXMAP24 "game2DbuttonDisabled0_24"
#define PM0_PIXMAP8 "game2Dbutton8"
#define PM1_PIXMAP24 "game2DbuttonDisabled1_24"
#define PM1_PIXMAP8 "game2Dbutton8"
#define PM2_PIXMAP24 "game2DbuttonDisabled2_24"
#define PM2_PIXMAP8 "game2Dbutton8"
#define PM3_PIXMAP24 "game2DbuttonDisabled3_24"
#define PM3_PIXMAP8 "game2Dbutton8"
#define PM4_PIXMAP24 "game2DbuttonDisabled4_24"
#define PM4_PIXMAP8 "game2Dbutton8"
#define PM5_PIXMAP24 "game2DbuttonDisabled5_24"
#define PM5_PIXMAP8 "game2Dbutton8"
#define PM6_PIXMAP24 "game2DbuttonDisabled6_24"
#define PM6_PIXMAP8 "game2Dbutton8"
#define PM7_PIXMAP24 "game2DbuttonDisabled7_24"
#define PM7_PIXMAP8 "game2Dbutton8"
#define PM8_PIXMAP24 "game2DbuttonDisabled8_24"
#define PM8_PIXMAP8 "game2Dbutton8"
#define RESTART_DEAD24 "game2DrestartDead24"
#define RESTART_DEAD8 "game2Drestart8"
#define RESTART_DOWN24 "game2DrestartDown24"
#define RESTART_DOWN8 "game2Drestart8"
#define RESTART_UP24 "game2DrestartUp24"
#define RESTART_UP8 "game2Drestart8"
#define RESTART_WINNER24 "game2DrestartWinner24"
#define RESTART_WINNER8 "game2Drestart8"
#define CHECKED_MINE24 "game2DbuttonDisabledMineCheck24"
#define CHECKED_MINE8 "game2Dbutton8"
#define CHECKED_FLAG24 "game2DbuttonUpFlagChecked24"
#define CHECKED_FLAG8 "game2Dbutton8"
#define INVALID_FLAG24 "game2DbuttonUpInvalid24"
#define INVALID_FLAG8 "game2Dbutton8"
#define NUM_PIXMAPS 21

// Helpers
#define VALID_SQUARE( a, b ) (((a) >= 0) && ((a) < m_SquaresX) && ((b) >= 0) && ((b) < m_SquaresY))

// game types
#define GAME_TYPE_WITHCORNERS 2
#define GAME_TYPE_EDGES 1

// NAMESPACE
//////////////
using namespace CW;
	
/* ------------
Constructor
------------ */
SingleScreen2D::SingleScreen2D( Widget *newParent )
	: Screen( "SingleScreen2D", newParent )
{
	color blue = { 0, 0, 255, 255 };

	// clear the current board
	m_SquaresX= 0;
	m_SquaresY= 0;
	m_PosX = 0;
	m_PosY = 0;
	m_BoardHeight = 0;
	m_BoardWidth = 0;
	m_bBoard = NULL;
	m_states = NULL;
	m_StartTime = -1.0;
	m_Paused = true;
	m_GameReady = false;
	m_GameOver = true;

	// setup the new parameters
	colormap_params_t cmParams;
	cmParams.bMipmap = 1; // (look pretty at all resolutions / scales)
	cmParams.bNoRescale = 0; // allow the engine to save texture memory
	cmParams.iEnv = GL_MODULATE; // standard blending
	cmParams.iFilter = ei()->cm_GetGlobalFilter(); // user level effects choice 
	cmParams.iScaleBias = 0; // no default biasing on rescale
	cmParams.iWrap = GL_CLAMP; // these are generally being used as labels

	// precache our colormaps so we don't thrash in game
	m_cmPrecache = new colormap_t*[NUM_PIXMAPS];
	m_cmPrecache[0] = ei()->cm_LoadFromFiles( ((string)UP_PIXMAP24 + "_WITH_" + (string)UP_PIXMAP8).c_str(), UP_PIXMAP24, UP_PIXMAP8, &cmParams );
	m_cmPrecache[1] = ei()->cm_LoadFromFiles( ((string)DOWN_PIXMAP24 + "_WITH_" + (string)DOWN_PIXMAP8).c_str(), DOWN_PIXMAP24, DOWN_PIXMAP8, &cmParams );
	m_cmPrecache[2] = ei()->cm_LoadFromFiles( ((string)QUESTION_PIXMAP24 + "_WITH_" + (string)QUESTION_PIXMAP8).c_str(), QUESTION_PIXMAP24, QUESTION_PIXMAP8, &cmParams );
	m_cmPrecache[3] = ei()->cm_LoadFromFiles( ((string)FLAG_PIXMAP24 + "_WITH_" + (string)FLAG_PIXMAP8).c_str(), FLAG_PIXMAP24, FLAG_PIXMAP8, &cmParams );
	m_cmPrecache[4] = ei()->cm_LoadFromFiles( ((string)MINE_PIXMAP24 + "_WITH_" + (string)MINE_PIXMAP8).c_str(), MINE_PIXMAP24, MINE_PIXMAP8, &cmParams );
	m_cmPrecache[5] = ei()->cm_LoadFromFiles( ((string)PM0_PIXMAP24 + "_WITH_" + (string)PM0_PIXMAP8).c_str(), PM0_PIXMAP24, PM0_PIXMAP8, &cmParams );
	m_cmPrecache[6] = ei()->cm_LoadFromFiles( ((string)PM1_PIXMAP24 + "_WITH_" + (string)PM1_PIXMAP8).c_str(), PM1_PIXMAP24, PM1_PIXMAP8, &cmParams );
	m_cmPrecache[7] = ei()->cm_LoadFromFiles( ((string)PM2_PIXMAP24 + "_WITH_" + (string)PM2_PIXMAP8).c_str(), PM2_PIXMAP24, PM2_PIXMAP8, &cmParams );
	m_cmPrecache[8] = ei()->cm_LoadFromFiles( ((string)PM3_PIXMAP24 + "_WITH_" + (string)PM3_PIXMAP8).c_str(), PM3_PIXMAP24, PM3_PIXMAP8, &cmParams );
	m_cmPrecache[9] = ei()->cm_LoadFromFiles( ((string)PM4_PIXMAP24 + "_WITH_" + (string)PM4_PIXMAP8).c_str(), PM4_PIXMAP24, PM4_PIXMAP8, &cmParams );
	m_cmPrecache[10] = ei()->cm_LoadFromFiles(((string)PM5_PIXMAP24 + "_WITH_" + (string)PM5_PIXMAP8).c_str(), PM5_PIXMAP24, PM5_PIXMAP8, &cmParams );
	m_cmPrecache[11] = ei()->cm_LoadFromFiles( ((string)PM6_PIXMAP24 + "_WITH_" + (string)PM6_PIXMAP8).c_str(), PM6_PIXMAP24, PM6_PIXMAP8, &cmParams );
	m_cmPrecache[12] = ei()->cm_LoadFromFiles( ((string)PM7_PIXMAP24 + "_WITH_" + (string)PM7_PIXMAP8).c_str(), PM7_PIXMAP24, PM7_PIXMAP8, &cmParams );
	m_cmPrecache[13] = ei()->cm_LoadFromFiles( ((string)PM8_PIXMAP24 + "_WITH_" + (string)PM8_PIXMAP8).c_str(), PM8_PIXMAP24, PM8_PIXMAP8, &cmParams );
	m_cmPrecache[14] = ei()->cm_LoadFromFiles( ((string)RESTART_DEAD24 + "_WITH_" + (string)RESTART_DEAD8).c_str(), RESTART_DEAD24, RESTART_DEAD8, &cmParams );
	m_cmPrecache[15] = ei()->cm_LoadFromFiles( ((string)RESTART_DOWN24 + "_WITH_" + (string)RESTART_DOWN8).c_str(), RESTART_DOWN24, RESTART_DOWN8, &cmParams );
	m_cmPrecache[16] = ei()->cm_LoadFromFiles( ((string)RESTART_UP24 + "_WITH_" + (string)RESTART_UP8).c_str(), RESTART_UP24, RESTART_UP8, &cmParams );
	m_cmPrecache[17] = ei()->cm_LoadFromFiles( ((string)RESTART_WINNER24 + "_WITH_" + (string)RESTART_WINNER8).c_str(), RESTART_WINNER24, RESTART_WINNER8, &cmParams );
	m_cmPrecache[18] = ei()->cm_LoadFromFiles( ((string)CHECKED_MINE24 + "_WITH_" + (string)CHECKED_MINE8).c_str(), CHECKED_MINE24, CHECKED_MINE8, &cmParams );
	m_cmPrecache[19] = ei()->cm_LoadFromFiles( ((string)CHECKED_FLAG24 + "_WITH_" + (string)CHECKED_FLAG8).c_str(), CHECKED_FLAG24, CHECKED_FLAG8, &cmParams );
	m_cmPrecache[20] = ei()->cm_LoadFromFiles( ((string)INVALID_FLAG24 + "_WITH_" + (string)INVALID_FLAG8).c_str(), INVALID_FLAG24,INVALID_FLAG8, &cmParams );

	// latch the persistant variables
	vlGameType = ei()->var_GetVarLatch( "2d_game_type" );
	vlHeight = ei()->var_GetVarLatch( "2d_height" );
	vlWidth = ei()->var_GetVarLatch( "2d_width" );
	vlUseQuestion = ei()->var_GetVarLatch( "2d_use_questionmarks" );
	vlNumMines = ei()->var_GetVarLatch( "2d_mine_count" );

	// clear the current state
	for( int a = 0 ; a < 9 ; a++ ) m_bDowned[a] = NULL;
	m_LeftDown = m_RightDown = m_MiddleDown = false;

	// create the board
	CreateBoard();
	m_Paused = true;

	// Create interface Widgets
	m_tlTitle.Create( "Single Player 2D", "SinglePlayer2DTitle", this, 15, 700 ); 
	m_tlTitle.SetTextSize( 32.0f );
	m_tlTitle.SetColor( blue );
	m_bBack.Create( "Back", "SingleScreen2DBackButton", this, 850, 32, 150, 32 );

	// Add these interaction widgets
	AddWidget( &m_bBack );
	AddWidget( &m_tlTitle );

}

/* ------------
Destructor
------------ */
SingleScreen2D::~SingleScreen2D()
{
	// free the board
	DeleteBoard();

	// free the precache list
	for( int a = 0 ; a < NUM_PIXMAPS ; a++ )
	{
		ei()->cm_Unload( m_cmPrecache[a] );
	}
	SAFE_DELETEv( m_cmPrecache );
}

/* ------------
KeyUpEvent
------------ */
void SingleScreen2D::KeyUpEvent( const byte &key )
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
		if( w->GetName() == "SinglePlayer2DBackButton" )
		{
			SetCurrentScreen( (Screen*)GetParent() );
		}
	}
}

/* ------------
MouseDownEvent
------------ */
void SingleScreen2D::MouseDownEvent( const byte &button, const int &x, const int &y )
{
	// pass to the screen
	Screen::MouseDownEvent( button, x, y );

	// mark button states
	if( button == M_LEFT ) m_LeftDown = true;
	else if( button == M_MIDDLE ) m_MiddleDown = true;
	else if( button == M_RIGHT ) m_RightDown = true;

	// if we are in a game give the scared face on down
	if( !m_bBack.Contains( x, y ) && !m_GameOver && !m_GameReady )
	{
		m_bRestart.SetDown();
	}

	// if we are ready to start and the board contains the button
	if( m_GameReady && !m_GameOver )
	{
		if( (x > m_PosX) && (x < (m_BoardWidth + m_PosX)) && 
			(y > m_PosY) && (y < (m_BoardHeight + m_PosY)) )
		{
			DoStartGame();
		}
	}

	// update screen
	MouseMoveEvent( x, y );

	// dispatch board events
	if( (m_MiddleDown) || (m_RightDown && m_LeftDown) )
	{
		// Search In
	}
	else if( m_LeftDown )
	{
		// Choose In
	}
	else if( m_RightDown && !m_LeftDown )
	{
		// Toggle State
		int sqX, sqY;
		bool valid = GetBoardSquare( x, y, sqX, sqY );
		if( valid && m_states[sqX][sqY].covered )
		{
			// current state flagged -> questioned or null
			if( m_states[sqX][sqY].flagged )
			{
				m_states[sqX][sqY].flagged = false;
				m_MinesRemaining++;
				UpdateMineCount();
				if( (int)ei()->var_GetFloat( vlUseQuestion ) )
				{
					m_states[sqX][sqY].questioned = true;
					m_bBoard[sqX][sqY].SetUpPixmap( QUESTION_PIXMAP24, QUESTION_PIXMAP8 );
				}
				else
				{
					m_bBoard[sqX][sqY].SetUpPixmap( UP_PIXMAP24, UP_PIXMAP8 );
				}
			}
			// questioned -> NULL
			else if( m_states[sqX][sqY].questioned )
			{
				m_states[sqX][sqY].questioned = false;
				m_bBoard[sqX][sqY].SetUpPixmap( UP_PIXMAP24, UP_PIXMAP8 );
			}
			// NULL -> Flagged
			else // !FLAGGED && !QUESTIONED
			{
				m_states[sqX][sqY].flagged = true;
				m_MinesRemaining--;
				UpdateMineCount();
				m_bBoard[sqX][sqY].SetUpPixmap( FLAG_PIXMAP24, FLAG_PIXMAP8 );
			}
		}
	}
}

/* ------------
MouseUpEvent
------------ */
void SingleScreen2D::MouseUpEvent( const byte &button, const int &x, const int &y )
{
	// pass to the screen
	Screen::MouseUpEvent( button, x, y );

	// mark button states
	if( button == M_LEFT ) m_LeftDown = false;
	else if( button == M_MIDDLE ) m_MiddleDown = false;
	else if( button == M_RIGHT ) m_RightDown = false;
	
	// Check to see if we are in a widget
	if( m_bBack.Contains( x, y ) )
	{
		SetCurrentScreen( (Screen*)GetParent() );
	}
	else if( m_bRestart.Contains( x, y ) )
	{
		m_bRestart.SetUp();
		RestartGame();
		return;
	}
	else
	{
		m_bRestart.SetUp();
	}
	
	// update the screen
	MouseMoveEvent( x, y );

	// get the position
	int sqX, sqY;
	bool valid = GetBoardSquare( x, y, sqX, sqY );

	// make sure we're in the world
	if( !valid ) return;
	
	// Uncover single action
	if( button == M_LEFT && !m_RightDown )
	{
		UncoverSquare( sqX, sqY );
		CheckGameEnd();
	}
	// Uncover Multi Action
	else if( (button ==  M_MIDDLE) || 
			(button == M_LEFT && m_RightDown) || 
			(button == M_RIGHT && m_LeftDown) )
	{
		if( m_CurGame == GAME_TYPE_EDGES )
		{
			int tx, ty;

			// uncover each of the four possible buttons that could be down
			// but only if the mine count == to the flag count and we are uncovered
			int flagcnt = 0;
			for( int a = 0 ; a < 4  ; a++ )
			{
				switch( a )
				{
				case 0:	tx = sqX - 1; ty = sqY;	break;
				case 1:	tx = sqX + 1;	ty = sqY;	break;
				case 2:	tx = sqX; 	ty = sqY - 1;	break;
				case 3:	tx = sqX; 	ty = sqY + 1;	break;
				default:	break;
				}
				if( VALID_SQUARE( tx, ty ) && m_states[tx][ty].flagged ) flagcnt++;
			}

			if( (flagcnt == m_states[sqX][sqY].cnt) && !m_states[sqX][sqY].covered )
			{
				for( int a = 0 ; a < 4  ; a++ )
				{
					switch( a )
					{
					case 0:	tx = sqX - 1; ty = sqY;	break;
					case 1:	tx = sqX + 1;	ty = sqY;	break;
					case 2:	tx = sqX; 	ty = sqY - 1;	break;
					case 3:	tx = sqX; 	ty = sqY + 1;	break;
					default:	break;
					}

					UncoverSquare( tx, ty );
				}
			}	

		}
		else
		{
			int tx, ty;

			// uncover each of the nine possible buttons that could be down
			// but only if the mine count == to the flag count and we are uncovered
			int flagcnt = 0;
			for( int a = 0 ; a < 8  ; a++ )
			{
				switch( a )
				{
				case 0:	tx = sqX - 1; ty = sqY - 1;	break;
				case 1:	tx = sqX; 	ty = sqY - 1;	break;
				case 2:	tx = sqX + 1; ty = sqY - 1;	break;
				case 3:	tx = sqX - 1; ty = sqY;	break;
				case 4:	tx = sqX + 1; ty = sqY;	break;
				case 5:	tx = sqX - 1; ty = sqY + 1;	break;
				case 6:	tx = sqX; 	ty = sqY + 1;	break;
				case 7:	tx = sqX + 1; ty = sqY + 1;break;
				default:	break;
				}
				if( VALID_SQUARE( tx, ty ) && m_states[tx][ty].flagged ) flagcnt++;
			}

			if( (flagcnt == m_states[sqX][sqY].cnt) && !m_states[sqX][sqY].covered )
			{
				for( int a = 0 ; a < 8  ; a++ )
				{
					switch( a )
					{
					case 0:	tx = sqX - 1; ty = sqY - 1;	break;
					case 1:	tx = sqX; ty = sqY - 1;	break;
					case 2:	tx = sqX + 1; ty = sqY - 1;	break;
					case 3:	tx = sqX - 1; ty = sqY;	break;
					case 4:	tx = sqX + 1; ty = sqY;	break;
					case 5:	tx = sqX - 1; ty = sqY + 1;	break;
					case 6:	tx = sqX; ty = sqY + 1;	break;
					case 7:	tx = sqX + 1; ty = sqY + 1;break;
					default:	break;
					}

					UncoverSquare( tx, ty );
				}
			}
		}
		CheckGameEnd();
	}
}

/* ------------
MouseMoveEvent
------------ */
void SingleScreen2D::MouseMoveEvent( const int &x, const int &y )
{
	// allow the superclass a slice of time
	Screen::MouseMoveEvent( x, y );

	// get the current square
	int sqX, sqY;
	bool valid = GetBoardSquare( x, y, sqX, sqY );
	
	// undown all downed buttons
	for( int a = 0 ; a < 9 ; a++ )
	{
		if( m_bDowned[a] ) m_bDowned[a]->SetUp();
		m_bDowned[a] = 0;
	}
	int cnt = 0;
	
	// process the search mode
	if( (m_MiddleDown) || (m_RightDown && m_LeftDown) )
	{
		if( m_CurGame == GAME_TYPE_EDGES )
		{
			int tx, ty;
			
			// select each of the nine possible buttons that could be down
			for( int a = 0 ; a < 4  ; a++ )
			{
				switch( a )
				{
				case 0:	tx = sqX - 1; ty = sqY;	break;
				case 1:	tx = sqX + 1;	ty = sqY;	break;
				case 2:	tx = sqX; 	ty = sqY - 1;	break;
				case 3:	tx = sqX; 	ty = sqY + 1;	break;
				default:	break;
				}

				if( VALID_SQUARE(tx,ty) && m_states[tx][ty].covered && 
					!m_states[tx][ty].flagged && !m_states[tx][ty].questioned )
				{
					m_bBoard[tx][ty].SetDown();
					m_bDowned[cnt] = &m_bBoard[tx][ty];
					cnt++;
				}
			}
		}
		else
		{
			int tx, ty;
			
			// select each of the nine possible buttons that could be down
			for( int a = 0 ; a < 9  ; a++ )
			{
				switch( a )
				{
				case 0:	tx = sqX - 1; ty = sqY - 1;	break;
				case 1:	tx = sqX; 	ty = sqY - 1;	break;
				case 2:	tx = sqX + 1; ty = sqY - 1;	break;
				case 3:	tx = sqX - 1; ty = sqY;	break;
				case 4:	tx = sqX; 	ty = sqY;	break;
				case 5:	tx = sqX + 1; ty = sqY;	break;
				case 6:	tx = sqX - 1; ty = sqY + 1;	break;
				case 7:	tx = sqX; 	ty = sqY + 1;	break;
				case 8:	tx = sqX + 1; ty = sqY + 1;break;
				default:	break;
				}

				if( VALID_SQUARE(tx,ty) && m_states[tx][ty].covered && 
					!m_states[tx][ty].flagged && !m_states[tx][ty].questioned )
				{
					m_bBoard[tx][ty].SetDown();
					m_bDowned[cnt] = &m_bBoard[tx][ty];
					cnt++;
				}
			}
		}
	}

	// process choose mode
	else if( m_LeftDown )
	{
		if( valid && m_states[sqX][sqY].covered && 
			!m_states[sqX][sqY].flagged && !m_states[sqX][sqY].questioned )
		{
			m_bBoard[sqX][sqY].SetDown();
			m_bDowned[cnt] = &m_bBoard[sqX][sqY];
			cnt++;
		}
	}
}

/* ------------
Draw
------------ */
void SingleScreen2D::Draw()
{
	// Do the Drawing for the board
	for( int x = 0 ; x < m_SquaresX ; x++ )
	{
		for( int y = 0 ; y < m_SquaresY ; y++ )
		{
			m_bBoard[x][y].Draw();
		}
	}

	// Draw the board interface widgets
	m_bRestart.Draw();
	m_tlTime.Draw();
	m_tlMinesRemaining.Draw();

	// Do the drawing for the widges
	m_bBack.Draw();
	m_tlTitle.Draw();
}

/* ------------
Frame
------------ */
void SingleScreen2D::Frame( exec_state_t *state )
{
	// check for non-started / stopped time
	if( m_Paused ) return;

	// get the elapsed time
	double elapsed = ei()->t_GetSaneTime() - m_StartTime;
	char buffer[1024];
	snprintf( buffer, 1024, "%.2f ", elapsed );

	// print the elapsed time
	m_tlTime.SetText( buffer );

	// compute the pre decimal size
	snprintf( buffer, 1024, "%d", (int)elapsed );
	
	// get the text length
	int len = (int)ei()->d_GetTextWidth( buffer, TL_TIMER_TEXT_HEIGHT );
	m_tlTime.SetPosition( m_PosX + m_BoardWidth - len - TL_TIMER_DEC_OFFSET, m_PosY + m_BoardHeight );

}

/* ------------
GetBoardSquare
------------ */
bool SingleScreen2D::GetBoardSquare( const int &x, const int &y, int &sqX, int &sqY )
{	
	// compute the actual position
	float fx = (float)(x - m_PosX) / BUTTON_SIZE;
	float fy = (float)(y - m_PosY) / BUTTON_SIZE;

	// map these into actual coordinates
	sqX = (int)fx;
	sqY = (int)fy;
	if( fx < 0.0f ) sqX -= 1;
	if( fy < 0.0f ) sqY -= 1;

	// check validity
	return (sqX >= 0) && (sqX < m_SquaresX) && (sqY >= 0) && (sqY < m_SquaresY);
}

/* ------------
UpdateMineCount
------------ */
void SingleScreen2D::UpdateMineCount()
{
	char buffer[1024];
	snprintf( buffer, 1024, "%d", m_MinesRemaining );
	m_tlMinesRemaining.SetText( buffer );
}

/* ------------
UncoverSquare
------------ */
void SingleScreen2D::UncoverSquare( const int &sqX, const int &sqY )
{
	// ensure we're in the world
	if( !VALID_SQUARE(sqX,sqY) ) return;

	// ensure that the we are not flagged
	if( m_states[sqX][sqY].flagged ) return;

	// ensure that we're not QuestionMarked
	if( m_states[sqX][sqY].questioned ) return;

	// ensure that we havn't already been here
	if( m_states[sqX][sqY].recursed ) return;

	// uncover it
	m_states[sqX][sqY].covered = false;
	m_bBoard[sqX][sqY].Disable();

	// expand zeros if this is a zero square
	if( !m_states[sqX][sqY].cnt )
	{
		// Set the zero recursed flag so we don't have to come back here
		m_states[sqX][sqY].recursed = true;

		if( m_CurGame == GAME_TYPE_EDGES )
		{
			UncoverSquare( sqX - 1, 	sqY );
			UncoverSquare( sqX + 1, 	sqY );
			UncoverSquare( sqX, 		sqY - 1 );
			UncoverSquare( sqX, 		sqY + 1 );
		}
		else
		{
			UncoverSquare( sqX - 1, 	sqY - 1 );
			UncoverSquare( sqX, 		sqY - 1 );
			UncoverSquare( sqX + 1, 	sqY -1 );
			UncoverSquare( sqX - 1, 	sqY );
			UncoverSquare( sqX + 1, 	sqY );
			UncoverSquare( sqX - 1, 	sqY + 1 );
			UncoverSquare( sqX, 		sqY + 1 );
			UncoverSquare( sqX + 1, 	sqY + 1 );
		}
	}
}

/* ------------
CheckGameEnd
------------ */
void SingleScreen2D::CheckGameEnd()
{
	// Dead if:
	// there exists an uncovered mined square

	// Winner if:
	// the covered square count == the number of mines
	int cnt = 0;
	
	// check all squares
	for( int x = 0 ; x < m_SquaresX ; x++ )
	{
		for( int y = 0 ; y < m_SquaresY ; y++ )
		{
			// check for dead state
			if( !m_states[x][y].covered && m_states[x][y].mined )
			{
				DoEndGame();
				ShowLoseGame();
				return;
			}

			// increment mine and covered square counts
			if( m_states[x][y].covered ) cnt++;
			if( m_states[x][y].mined ) cnt--;
		}
	}

	// check for win
	if( !cnt )
	{
		DoEndGame();
		ShowWinGame();
	}
}

/* ------------
ShowLoseGame
------------ */
void SingleScreen2D::ShowLoseGame()
{
	// check all squares
	for( int x = 0 ; x < m_SquaresX ; x++ )
	{
		for( int y = 0 ; y < m_SquaresY ; y++ )
		{
			// if mined
			if( m_states[x][y].mined )
			{
				// if mined and flagged 
				if( m_states[x][y].flagged )
				{
					// show a green checked flag
					m_bBoard[x][y].SetUpPixmap( CHECKED_FLAG24, CHECKED_FLAG8 );
				}
				// if mined and unflagged uncover so the user can see it
				else
				{
					m_states[x][y].covered = false;
					m_bBoard[x][y].Disable();
				}
			}
			// unmined
			else
			{
				// wrongly placed flags
				if( m_states[x][y].flagged )
				{
					m_bBoard[x][y].SetUpPixmap( INVALID_FLAG24, INVALID_FLAG8 );
				}
				// uncover it so the user can see it
				else
				{
					m_states[x][y].covered = false;
					m_bBoard[x][y].Disable();
				}
			}
			
		}
	}

	// Set the dead face
	m_bRestart.SetUpPixmap( RESTART_DEAD24, RESTART_DEAD8 );
}

/* ------------
ShowWinGame
------------ */
void SingleScreen2D::ShowWinGame()
{
	// check all squares
	for( int x = 0 ; x < m_SquaresX ; x++ )
	{
		for( int y = 0 ; y < m_SquaresY ; y++ )
		{
			// if mined
			if( m_states[x][y].mined )
			{
				// if mined and flagged 
				if( m_states[x][y].flagged )
				{
					// show a green checked flag
					m_bBoard[x][y].SetUpPixmap( CHECKED_FLAG24, CHECKED_FLAG8 );
				}
				// if mined and unflagged check it as a success
				else
				{
					m_states[x][y].covered = false;
					m_bBoard[x][y].SetDisabledPixmap( CHECKED_MINE24, CHECKED_MINE8 );
					m_bBoard[x][y].Disable();
				}
			}
			// unmined
			else
			{
				// wrongly placed flags
				if( m_states[x][y].flagged )
				{
					// we shouldn't get here on win
					m_bBoard[x][y].SetUpPixmap( INVALID_FLAG24, INVALID_FLAG8 );
				}
				// uncover it so the user can see it
				else
				{
					m_states[x][y].covered = false;
					m_bBoard[x][y].Disable();
				}
			}
			
		}
	}

	// set the winner face
	m_bRestart.SetUpPixmap( RESTART_WINNER24, RESTART_WINNER8 );
}

/* ------------
DoStartGame
------------ */
void SingleScreen2D::DoStartGame()
{
	// set the state
	m_GameReady = false;
	m_GameOver = false;
	
	// grab a start time
	m_StartTime = ei()->t_GetSaneTime();
	m_Paused = false;
}

/* ------------
DoEndGame
------------ */
void SingleScreen2D::DoEndGame()
{
	// get the current time
	double m_TotalTime = ei()->t_GetSaneTime() - m_StartTime;
	m_Paused = true;

	// set the game state
	m_GameOver = true;
	m_GameReady = false;

	// see if we have a better time than a previous record
}
	
/* ------------
RestartGame
------------ */
void SingleScreen2D::RestartGame()
{
	CreateBoard();
}

/* ------------
CreateBoard
------------ */
bool SingleScreen2D::CreateBoard()
{
	// free the old board
	DeleteBoard();

	// get the size
	m_SquaresX = (int)ei()->var_GetFloat( vlWidth );
	m_SquaresY = (int)ei()->var_GetFloat( vlHeight );

	// Get the type
	m_CurGame = (int)ei()->var_GetFloat( vlGameType );
	
	// sanity check
	if( m_SquaresX < 0 || m_SquaresY < 0 )
	{
		ei()->con_Print( "<RED>Create Game 2D Sanity Check: dimension < 0" );
		return false;
	}
	if( m_CurGame != GAME_TYPE_EDGES && m_CurGame != GAME_TYPE_WITHCORNERS )
	{
		ei()->con_Print( "<RED>Create Game 2D Sanity Check:  invalid Game Type" );
		return false;
	}

	// FIXME: fix max height
	
	// allocate space for it
	if( !CreateBoard_Allocate() )
	{
		ei()->con_Print( "<RED>Create Game 2D Allocation Failed." );
		return false;
	}
	
	// position it
	if( !CreateBoard_Layout() )
	{
		ei()->con_Print( "<RED>Create Game 2D Layout Failed." );
		return false;
	}

	// initialize the buttons
	CreateBoard_Buttons();

	// assign mines
	CreateBoard_Mines();

	// set-up numbers
	CreateBoard_Number();

	// show the count for the first time
	UpdateMineCount();

	// set up timing
	m_Paused = true;
	m_GameReady = true;
	m_GameOver = false;

	return true;
}

/* ------------
CreateBoard_Number
------------ */
bool SingleScreen2D::CreateBoard_Number()
{
	int tx, ty;

	// iterate every square on the board
	for( int x = 0 ; x < m_SquaresX ; x++ )
	{
		for( int y = 0 ; y < m_SquaresY ; y++ )
		{
			// The number to place in the square. 
			// This is based on the mines around the square
			// which is based on game type.
			int cnt = 0;

			if( m_CurGame == GAME_TYPE_EDGES )
			{
				// count all relevant squares
				for( int e = 0 ; e < 4 ; e++ )
				{
					switch( e )
					{
					case 0: tx = x - 1; ty = y; break;
					case 1: tx = x; ty = y + 1; break;
					case 2: tx = x + 1; ty = y; break;
					case 3: tx = x; ty = y - 1; break;
					default: break;
					}
					if( VALID_SQUARE(tx,ty) && m_states[tx][ty].mined ) cnt++;
				}
			}
			else
			{
				// count all relevant squares
				for( int e = 0 ; e < 9 ; e++ )
				{
					switch( e )
					{
					case 0:	tx = x - 1; ty = y - 1;	break;
					case 1:	tx = x; ty = y - 1;	break;
					case 2:	tx = x + 1; ty = y - 1;	break;
					case 3:	tx = x - 1; ty = y;	break;
					case 4:	tx = x; ty = y;		break;
					case 5:	tx = x + 1; ty = y;	break;
					case 6:	tx = x - 1; ty = y + 1;	break;
					case 7:	tx = x; ty = y + 1;	break;
					case 8:	tx = x + 1; ty = y + 1;break;
					default: break;
					}
					if( VALID_SQUARE(tx,ty) && m_states[tx][ty].mined ) cnt++;
				}
			}
			
			// set the pixmap
			switch( cnt )
			{
				case 0: m_bBoard[x][y].SetDisabledPixmap( PM0_PIXMAP24, PM0_PIXMAP8 ); break;
				case 1: m_bBoard[x][y].SetDisabledPixmap( PM1_PIXMAP24, PM1_PIXMAP8 ); break;
				case 2: m_bBoard[x][y].SetDisabledPixmap( PM2_PIXMAP24, PM2_PIXMAP8 ); break;
				case 3: m_bBoard[x][y].SetDisabledPixmap( PM3_PIXMAP24, PM3_PIXMAP8 ); break;
				case 4: m_bBoard[x][y].SetDisabledPixmap( PM4_PIXMAP24, PM4_PIXMAP8 ); break;
				case 5: m_bBoard[x][y].SetDisabledPixmap( PM5_PIXMAP24, PM5_PIXMAP8 ); break;
				case 6: m_bBoard[x][y].SetDisabledPixmap( PM6_PIXMAP24, PM6_PIXMAP8 ); break;
				case 7: m_bBoard[x][y].SetDisabledPixmap( PM7_PIXMAP24, PM7_PIXMAP8 ); break;
				case 8: m_bBoard[x][y].SetDisabledPixmap( PM8_PIXMAP24, PM8_PIXMAP8 ); break;
				default: break;
			}
			
			// mask in the mine count
			m_states[x][y].cnt = cnt;

			// squares with mines override the number picture
			if( m_states[x][y].mined ) m_bBoard[x][y].SetDisabledPixmap( MINE_PIXMAP24, MINE_PIXMAP8 );
		}
	}
	return true;
}

/* ------------
CreateBoard_Mines
// Mine the Field
------------ */
bool SingleScreen2D::CreateBoard_Mines()
{
	int freespace = m_SquaresX * m_SquaresY;
	int numMines = (int)ei()->var_GetFloat( vlNumMines );
	int cnt = 0;
	int x, y;
	bool bDone;

	// panic condition
	if( numMines > freespace )
	{
		ei()->var_UpdateFloat( vlNumMines, 1.0f );
		numMines = 1;
	}
	
	// add each mine
	while( cnt < numMines )
	{
		// get a random offset into the freespace
		int offset = ei()->rng_Long( 0, freespace - cnt );

		// prepare to iterate
		bDone = false;
		x = 0;

		// iterate through freespace until we reach the offset
		while( !bDone && (x < m_SquaresX) )
		{
			// next row
			y = 0;
			while( !bDone && (y < m_SquaresY) )
			{
				if( m_states[x][y].mined )
				{
					// skip already mined squares
				}
				else
				{
					// have we reached the random mineable position?
					if( !offset )
					{
						// add the mine
						m_states[x][y].mined = true;

						// set the underside
						m_bBoard[x][y].SetDisabledPixmap( MINE_PIXMAP24, MINE_PIXMAP8 );

						// increment the added mines
						cnt++;
						
						// mark us as done
						bDone = true;
					}

					// continue
					offset--;
				}

				// next square
				y++;
			}

			// next column
			x++;
		}
	}
	return true;
}

/* ------------
CreateBoard_Buttons
// The board Buttons
------------ */
bool SingleScreen2D::CreateBoard_Buttons()
{
	char buffer[1024];

	// init buttons
	for( int x = 0 ; x < m_SquaresX ; x++ )
	{
		for( int y = 0 ; y < m_SquaresY; y++ )
		{
			// get a name for this square
			snprintf( buffer, 1024, "button( %d, %d )", x, y );

			// Create each button
			m_bBoard[x][y].Create( "", buffer, this, m_PosX + (x * BUTTON_SIZE), 
						m_PosY + (y * BUTTON_SIZE), BUTTON_SIZE, BUTTON_SIZE );

			// set a better picture
			m_bBoard[x][y].SetUpPixmap( UP_PIXMAP24, UP_PIXMAP8 );
			m_bBoard[x][y].SetDownPixmap( DOWN_PIXMAP24, DOWN_PIXMAP8 );

			// make unfocusable
			m_bBoard[x][y].SetUnfocusable();

			// make it not click
			m_bBoard[x][y].DisableSounds();
		}
	}
	
	return true;
}

/* ------------
CreateBoard_Layout
// positioning and widgets
------------ */
bool SingleScreen2D::CreateBoard_Layout()
{
	const color red = { 255, 0, 0, 255 };
	
	// center the board on the screen
	m_BoardWidth = BUTTON_SIZE * m_SquaresX;
	m_BoardHeight = BUTTON_SIZE * m_SquaresY;
	m_PosX = (SCR_WIDTH - m_BoardWidth) / 2;
	m_PosY = (SCR_HEIGHT - (m_BoardHeight + RESTART_BUTTON_SIZE + BOARD_TOP_PAD)) / 2;

	// center the interface widget
	int offx = (SCR_WIDTH - RESTART_BUTTON_SIZE) / 2;
	int offy = m_PosY + m_BoardHeight;

	// the restart button
	m_bRestart.Create( "", "SingleScreen2DGameRestartLabel", this, 
						offx, offy, RESTART_BUTTON_SIZE, RESTART_BUTTON_SIZE );
	m_bRestart.SetDownPixmap( RESTART_DOWN24, RESTART_DOWN8 );
	m_bRestart.SetUpPixmap( RESTART_UP24, RESTART_UP8 );

	// the mines counter
	m_tlMinesRemaining.Create( "", "SingleScreen2DGameMinesLabel", this, 
							m_PosX, offy );
	m_tlMinesRemaining.SetTextSize( 28.0f );
	m_MinesRemaining = (int)ei()->var_GetFloat( vlNumMines );

	// the time counter
	int len = (int)ei()->d_GetTextWidth( "0", TL_TIMER_TEXT_HEIGHT );
	m_tlTime.Create( "0.00", "SingleScreen2DGameTimeLabel", this, 
							m_PosX + m_BoardWidth - len - TL_TIMER_DEC_OFFSET, offy );
	m_tlTime.SetTextSize( TL_TIMER_TEXT_HEIGHT );
	m_tlTime.SetColor( red );

	return true;
}

/* ------------
CreateBoard_Allocate
// gets free store memory for the board and board state
------------ */
bool SingleScreen2D::CreateBoard_Allocate()
{
	// allocate columns
	m_bBoard = (Button**)new Button*[m_SquaresX];
	m_states = (sqState_t**)new sqState_t*[m_SquaresX];

	// check allocations
	if( !m_bBoard )
	{
		ei()->con_Print( "<RED>Allocation Failed for Board Button Arrays." );
		return false;
	}
	if( !m_states )
	{
		ei()->con_Print( "<RED>Allocation Failed for Board State Arrays." );
		return false;
	}

	// allocate rows
	for( int a = 0 ; a < m_SquaresX ; a++ )
	{
		m_bBoard[a] = (Button*)new Button[m_SquaresY];
		m_states[a] = (sqState_t*)new sqState_t[m_SquaresY];

		if( !m_bBoard[a] )
		{
			ei()->con_Print( "<RED>Allocation Failed for Board Button Column %d.", a );
			return false;
		}
		if( !m_states[a] )
		{
			ei()->con_Print( "<RED>Allocation Failed for Board State Column %d.", a );
			return false;
		}

	}

	// clear the state memory
	for( int x = 0 ; x < m_SquaresX ; x++ )
	{
		for( int y = 0; y < m_SquaresY ; y++ )
		{
			// cover it
			m_states[x][y].mined = false;
			m_states[x][y].covered = true;
			m_states[x][y].flagged = false;
			m_states[x][y].questioned = false;
			m_states[x][y].recursed = false;
			m_states[x][y].cnt = 0;
		}
	}
	
	return true;
}
	
/* ------------
DeleteBoard
------------ */
void SingleScreen2D::DeleteBoard()
{
	// return mem to free store
	for( int a = 0 ; a < m_SquaresX ; a++ )
	{
		SAFE_DELETEv( m_bBoard[a] );
		SAFE_DELETEv( m_states[a] );
	}
	SAFE_DELETE( m_bBoard );
	SAFE_DELETE( m_states );

	// zero stuff
	m_PosX = 0;
	m_PosY = 0;
	m_Right = 0;
	m_Top = 0;
	m_SquaresX = 0;
	m_SquaresY = 0;
	m_bBoard = NULL;
	m_states = NULL;
	m_StartTime = -1;
	m_Paused = true;
}


