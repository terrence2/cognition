// Cognition
// .c
// Created  by Terrence Cole

// Includes
/////////////
#include "cog_global.h"
#include "sal_input_win_di.h"
#include "sal_input_win_win32.h"

// Definitions
////////////////
#define IN_MODE_NONE 0
#define IN_MODE_DINPUT 1
#define IN_MODE_WIN32 2

// Local Structures
/////////////////////

// Global Prototypes
//////////////////////
/*
int in_Initialize(void);
void in_Terminate(void);
void in_ProcessEvents();
void in_HideMouse();
void in_UnhideMouse();
*/


// Local Prototypes
/////////////////////

// Local Variables
////////////////////
static varlatch vlMode = NULL;
static int iMode = IN_MODE_NONE;

// *********** FUNCTIONALITY ***********
/* ------------
in_Initialize - opens the requested input device 
			  - export work to another module based on the value of input_mode
------------ */
int in_Initialize(void)
{
	double tStart, tEnd;
	
	con_Print( "\n<BLUE>Initializing</BLUE> Input." );
	eng_LoadingFrame();
	tStart = t_GetSaneTime();

	// clear the axises
	memset( state.mAxis, 0, sizeof(short) * 2 );
	memset( state.mAxisPrev, 0, sizeof(short) * 2 );
	memset( state.mAxisDelta, 0, sizeof(short) * 2 );
	state.mWheel = 0;

	// get the mode for dispatching functionality
	vlMode = var_GetVarLatch( "input_mode" );
	if( vlMode == NULL )
	{
		con_Print( "Input Init:  Get Var Latch for \"input_mode\" failed." );
		return 0;
	}

	// check the mode flag
	if( tcstricmp( var_GetString(vlMode), "auto" ) || tcstricmp( var_GetString(vlMode), "direct_input" ) )
	{
		if( !in_di_Initialize() )
		{
			con_Print( "\tDirect Input Initialization Failed.  Falling back to Win32 input support." );

			if( !in_w32_Initialize() )
			{
				con_Print( "Input Init:  Could not open an input device." );
				return 0;
			}
			else
			{
				con_Print( "\tUsing Win32 Input Mode." );
				iMode = IN_MODE_WIN32;
			}
		}
		else
		{
			con_Print( "\tUsing DirectInput." );
			iMode = IN_MODE_DINPUT;
		}
	}
	else
	{
		if( !in_w32_Initialize() )
		{
			con_Print( "\tWin32 Input Initialization Failed.  Falling back to DirectInput input support." );

			if( !in_di_Initialize() )
			{
				con_Print( "Input Init:  Could not open an input device." );
				return 0;
			}
			else
			{
				con_Print( "\tUsing DirectInput." );
				iMode = IN_MODE_DINPUT;
			}
		}
		else
		{
			con_Print( "\tUsing Win32 Input Mode." );
			iMode = IN_MODE_WIN32;
		}
	}

	tEnd = t_GetSaneTime();
	con_Print( "\tDone:  %.3f sec", tEnd - tStart );
	eng_LoadingFrame();

	return 1;
}

/* ------------
in_Terminate - closes input devices
------------ */
void in_Terminate(void)
{
	// if iMode is not set, the system is not started
	if( iMode == IN_MODE_NONE ) return;

	con_Print( "\nTerminating Input System." );

	switch( iMode )
	{
	case IN_MODE_DINPUT:
		con_Print( "\tClosing Direct Input." );
		in_di_Terminate();
		return;
	case IN_MODE_WIN32:
	default:
		con_Print( "\tClosing Win32 Input." );
		in_w32_Terminate();
		return;
	}
}

/* ------------
in_ProcessEvents - dispatches input events to the bind dereferencer
------------ */
void in_ProcessEvents()
{
	switch( iMode )
	{
	case IN_MODE_DINPUT:
		in_di_ProcessEvents();
		return;
	case IN_MODE_WIN32:
	default:
		// events under Win32 are routed from the WinProc
		// this function does by frame maintenence
		in_w32_ProcessEvents();
		return;
	}
}

/* ------------
in_HideMouse - hides the mouse
------------ */
void in_HideMouse()
{
	while( ShowCursor( FALSE ) >= 0 ) ;
}

/* ------------
in_UnhideMouse - makes the mouse visible
------------ */
void in_UnhideMouse()
{
	while( ShowCursor( TRUE ) < 0 ) ;
}

