#ifndef _SINGLESCREEN2D_H_
#define _SINGLESCREEN2D_H_

// INCLUDES
/////////////
#include "CogMenuGlobal.h"
#include "Screen.h"
#include "TextLabel.h"
#include "Button.h"

// NAMESPACE
//////////////
using namespace CW;

// SINGLESCREEN2D
///////////////////
class SingleScreen2D : public Screen
{
public:
	SingleScreen2D( Widget *newParent );
	~SingleScreen2D();

	// Event Handlers
	void KeyUpEvent( const byte &key );
	void MouseDownEvent( const byte &button, const int &x, const int &y );
	void MouseUpEvent( const byte &button, const int &x, const int &y );
	void MouseMoveEvent( const int &x, const int &y );

	// Drawing Handler
	void Draw();

	// Frame Handler / Timing
	void Frame( exec_state_t *state );

private:
	bool GetBoardSquare( const int &x, const int &y, int &sqX, int &sqY );
	void UpdateMineCount();
	void UncoverSquare( const int &sqX, const int &sqY );
	void CheckGameEnd();
	void ShowLoseGame();
	void ShowWinGame();
	void DoStartGame();
	void DoEndGame();
	void RestartGame();

	bool CreateBoard();
	bool CreateBoard_Allocate();
	bool CreateBoard_Layout();
	bool CreateBoard_Buttons();
	bool CreateBoard_Mines();
	bool CreateBoard_Number();

	void DeleteBoard();
	
private:
	// Interface
	TextLabel m_tlTitle;
	Button m_bBack;

	// fixed Game widgets
	Button m_bRestart;
	TextLabel m_tlTime;
	TextLabel m_tlMinesRemaining;
	int m_MinesRemaining;
	double m_StartTime;
	bool m_Paused;
	bool m_GameReady;
	bool m_GameOver;

	typedef struct
	{
		bool mined;
		bool covered;
		bool flagged;
		bool questioned;
		bool recursed;
		int cnt;
	} sqState_t;
	
	// the current game board
	int m_SquaresX; // in number of squares
	int m_SquaresY; // in number of squares
	int m_CurGame; // the game type
	int m_PosX; // in screen coord
	int m_PosY; // in screen coord
	int m_BoardWidth; // in screen coord
	int m_BoardHeight; // in screen coord
	Button **m_bBoard;
	sqState_t **m_states;
	colormap_t **m_cmPrecache;

	// transient state
	bool m_LeftDown;
	bool m_RightDown;
	bool m_MiddleDown;
	Button* m_bDowned[9];

	// option states
	varlatch vlUseQuestion; // { "2d_use_questionmarks", "0", 0, 0, 0, NULL },
		// whether or not to use checkmarks on right button press
	varlatch vlWidth; //	{ "2d_width", "10", 10, 0, 0, NULL },
		// the grid width
	varlatch vlHeight; //	{ "2d_height", "10", 10, 0, 0, NULL },
		// the grid height
	varlatch vlGameType; //	{ "2d_game_type", "0", 0, 0, NULL },
		// 0 for edge and corner, 1 for edges only
	varlatch vlNumMines; 

};

#endif // _SINGLESCREEN2D_H_

