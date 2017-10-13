// Cognition
// startlog.c
// Created 2-25-02 @ 1034 by Terrence Cole

// Includes
/////////////
#include "cog_global.h"

// Definitions
////////////////
#define SL_DEF_WIDTH 640
#define SL_DEF_HEIGHT 480

#define SL_LINE_LENGTH 1024
#define SL_NUM_TABS 5

#define SL_BM_NAME "logo_small.bmp"
#define SL_BM_WIDTH 640
#define SL_BM_HEIGHT 180

#define SL_NUM_HISTORY_LINES 100

// Structures
///////////////
typedef struct
{
	int top;
	char text[SL_LINE_LENGTH];
} startlog_line_t;

typedef struct
{
	// internal state tracking
	int slActive;
	int numLines;  // onscreen visible
	int lnNum;
	int slTextHeight;

	// windows device
	display_t window;
	HFONT slFont, slOldFont;
	HDC mDC;
	HBITMAP hBM;

	// structural tracking
	startlog_line_t slHistory[SL_NUM_HISTORY_LINES];
	RECT slCltRect;
	RECT slTextRect;
	RECT slBMRect;

	int slTabs[SL_NUM_TABS];
} startlog_state_t;

// Global Prototypes
//////////////////////
/*
int sl_Initialize(void);
void sl_Terminate(void);
void sl_Print( char *text );
*/

// Local Prototypes
/////////////////////
int sl_OpenWindow();
static int sl_Update();
static void sl_PaintText();
static void sl_PaintLogo();

// Local Variables
////////////////////
static startlog_state_t slstate;

// *********** FUNCTIONALITY ***********
/* ------------
sl_Init
------------ */
int sl_Initialize(void)
{
	char *path;

	con_Print( "\nInitializing Screen Log..." );
	if( slstate.slActive > 0 ) sl_Terminate();

	// open the console window
	con_Print( "\tOpening Log Window..." );
	if( !sl_OpenWindow() )
	{
		sl_Terminate();
		return 0;
	}

	// get our static device objects
	con_Print( "\tGetting Static Device Objects..." );
	slstate.window.glDC = GetDC( slstate.window.hWnd );
	
	slstate.slFont = GetStockObject(ANSI_VAR_FONT);
	slstate.slOldFont = SelectObject( slstate.window.glDC, slstate.slFont );

	path = fs_GetMediaPath( SL_BM_NAME, "startlog" );
	con_Print( "\tLoading Header Bitmap \"%s\"", path );
	slstate.hBM = LoadImage( NULL, path, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION | LR_DEFAULTSIZE );
	slstate.mDC = CreateCompatibleDC( slstate.window.glDC );
	SelectObject( slstate.mDC, slstate.hBM );

	// clear the history
	memset( &slstate.slHistory, 0, sizeof(startlog_line_t) * SL_NUM_HISTORY_LINES );
	slstate.lnNum = 0;

	// test our object selection
	if( !slstate.slOldFont )	
	{
		sl_Terminate();
		return 0;
	}
 
	if( !sl_Update() )
	{
		sl_Terminate();
		return 0;
	}

	// update the string
	sl_PaintText();
	sl_PaintLogo();

	con_Print( "\tScreen Log Initialized Successfully." );
	slstate.slActive = 1;

	return 1; 
}

/* ------------
sl_Terminate
------------ */
void sl_Terminate(void)
{
	slstate.slActive = 0;

	con_Print( "\n<RED>Terminating</RED> Screen Log..." );

	// release our device objects
	if( slstate.window.glDC && slstate.slOldFont )
	{
		con_Print( "\tReplacing Device Context Font..." );
		SelectObject( slstate.window.glDC, slstate.slOldFont );	
	}
	if( slstate.window.hWnd && slstate.window.glDC )
	{
		con_Print( "\tReleasing Log Device Context..." );
		ReleaseDC( slstate.window.hWnd, slstate.window.glDC );
	}
	if( slstate.window.hWnd )
	{
		con_Print( "\tDestroying Log Window..." );
		dis_CloseWindow( &slstate.window );
	}
	if( slstate.hBM )
	{
		con_Print( "\tDeleting Header Bitmap..." );
		DeleteObject(slstate.hBM);
	}
	if( slstate.mDC )
	{
		con_Print( "\tDeleting Device Context..." );
		DeleteDC(slstate.mDC);
	}

	con_Print( "\tScreen Log Terminated Successfully." );
}

/* ------------
sl_WinProc
------------ */
LONG WINAPI sl_WinProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch(uMsg)
	{
	case WM_SIZE:
		sl_Update();
		return 0;
	case WM_PAINT:
		if( RectVisible(slstate.window.glDC, &slstate.slBMRect) )
			sl_PaintLogo();
		if( RectVisible(slstate.window.glDC, &slstate.slTextRect) )
			sl_PaintText();
		return 0;
	default:
		return DefWindowProc( hWnd, uMsg, wParam, lParam );
	}
}

/* ------------
sl_OpenWindow
------------ */
int sl_OpenWindow()
{
	RECT dtRect;

	// size and position the window
	GetClientRect( GetDesktopWindow(), &dtRect );
	slstate.window.x = (dtRect.right - SL_DEF_WIDTH) / 2;
	slstate.window.y = (dtRect.bottom - SL_DEF_HEIGHT) / 2;
	slstate.window.w = SL_DEF_WIDTH;
	slstate.window.h = SL_DEF_HEIGHT;

	// setup the window
	slstate.window.hBackBrush = CreateSolidBrush( RGB( 255, 255, 255 ) );
	slstate.window.hCursor = NULL;
	slstate.window.hIcon = NULL;
	tcstrcpy( slstate.window.winClassName, "LogView" ); 
	slstate.window.winClassStyle = CS_OWNDC;
	slstate.window.winOwnerInstance = sys_handles.cInstance;
	slstate.window.WinProc = sl_WinProc;
	slstate.window.winStyle = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME;
	slstate.window.winStyleEx = 0;
	slstate.window.nCmdShow = sys_handles.nCmdShow;

	dis_OpenWindow( &slstate.window );
	if( slstate.window.hWnd == NULL )
	{
		con_Print( "Startlog Open Window:  Display, Open Window failed." );
		return 0;
	}
	
	return 1;
}

/* ------------
sl_Update
------------ */
static int sl_Update()
{
	TEXTMETRIC tm;
	int a;
	int error;
	char buffer[1024];

	// setup formatting
	if( !GetClientRect( slstate.window.hWnd, &slstate.slCltRect ) )
	{
		error = GetLastError();
		sprintf( buffer, "Error: %d", error );
		MessageBox( NULL, buffer, "Error", MB_OK | MB_ICONWARNING );
		return 0;
	}
	GetTextMetrics( slstate.window.glDC, &tm );

	// adjust for the size of the bitmap
	memcpy( &slstate.slTextRect, &slstate.slCltRect, sizeof(RECT) );
	memcpy( &slstate.slBMRect, &slstate.slCltRect, sizeof(RECT) );

	slstate.slTextRect.top += SL_BM_HEIGHT + 1;
	slstate.slBMRect.bottom = SL_BM_HEIGHT;

	// recalculate the number of visible lines
	slstate.numLines = ( (slstate.slTextRect.bottom - slstate.slTextRect.top) / tm.tmHeight);
	slstate.slTextHeight = tm.tmHeight + 2;

	// get the "logical" position of the ordered lines
	for( a = 0 ; a < SL_NUM_HISTORY_LINES ; a++ )
	{
		slstate.slHistory[a].top = slstate.slCltRect.bottom - ((a + 1) * slstate.slTextHeight);
	}

	// tabs
	memset( slstate.slTabs, 0, sizeof(slstate.slTabs) );
	for( a=0 ; a<SL_NUM_TABS ; a++ )
	{
		slstate.slTabs[a] = (5*a*tm.tmAveCharWidth);
	}

	return 1;
}

/* ------------
sl_Print
------------ */
void sl_Print( char *text )
{
	int a;
	int index;
	int len;

	if( slstate.slActive < 1 ) return;

	// advance to the next line
	slstate.lnNum++;
	if( slstate.lnNum > SL_NUM_HISTORY_LINES ) slstate.lnNum = 0;
	index = slstate.lnNum;  // we need to use local memory because this function will recurse on new-lines

	// zero
	memset( slstate.slHistory[index].text, 0, sizeof(char) * SL_LINE_LENGTH );
	
	// get and cap the length
	len = tcstrlen(text);
	if( len > SL_LINE_LENGTH ) len = SL_LINE_LENGTH;

	// copy catching new-lines
	for( a = 0 ; a < len ; a++ )
	{
		// catch special escape characters
		if( text[a] == '\n' )
		{
			slstate.slHistory[index].text[a] = '\0';
			len = a; // stop now on this line

			// this needs to break to the next line, so recurse
			sl_Print( (text + a + 1) );
		}
		else
		{
			slstate.slHistory[index].text[a] = text[a];
		}
	}
	
	InvalidateRect( slstate.window.hWnd, &slstate.slTextRect, TRUE );
}

/* ------------
sl_Paint
------------ */
static void sl_PaintText()
{
	int a;
	int yOff;
	int index; // so that we can wrap around on NUM_HIST_LINES w/out restarting the counter

	// start w/ bottom at lnNum
	yOff = slstate.slHistory[0].top;
	index = slstate.lnNum;

	for( a = 0 ; a < slstate.numLines ; a++ )
	{
		

		TabbedTextOut( slstate.window.glDC, 2, yOff, slstate.slHistory[a].text, 
						tcstrlen( slstate.slHistory[a].text ), SL_NUM_TABS, slstate.slTabs, 0 );
		yOff -= slstate.slTextHeight;
		index++;
		if( index > SL_NUM_HISTORY_LINES ) index = 0;
	}

	ValidateRect( slstate.window.hWnd, &slstate.slTextRect );
}

/* ------------
sl_PaintLogo
------------ */
static void sl_PaintLogo()
{
	if( IsIconic(slstate.window.hWnd) )
	{
		ValidateRect( slstate.window.hWnd, &slstate.slBMRect );
		return;
	}

	StretchBlt( slstate.window.glDC, 0, 0, slstate.slBMRect.right, slstate.slBMRect.bottom, slstate.mDC, 0, 0, SL_BM_WIDTH, SL_BM_HEIGHT, SRCCOPY );
	ValidateRect( slstate.window.hWnd, &slstate.slBMRect );
//	BitBlt( slstate.window.glDC, 0, 0, SL_BM_WIDTH, SL_BM_HEIGHT, slstate.mDC, 0, 0, SRCCOPY );
}