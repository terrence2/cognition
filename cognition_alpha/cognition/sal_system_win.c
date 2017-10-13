// Cognition
// system.c
// Created 2-25-02 @ 1023 by Terrence Cole

// Includes
/////////////
#include "cog_global.h"
#include "sal_input_win_win32.h"

// Definitions
////////////////
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL                   0x020A
#endif // WM_MOUSEWHEEL

// Structures
///////////////

// Global Prototypes
//////////////////////
/*
void sys_ProcessSystemMessages();
void sys_PrintSystemInformation();
void sys_Exit( int errorcode );
*/

// Local Prototypes
/////////////////////
static int sys_PumpMessage( HWND win );


// Local Variables
////////////////////

// *********** FUNCTIONALITY ***********

/* ------------
sys_CheckMessages
------------ */
void sys_ProcessSystemMessages()
{
	MSG msg;
	HWND glWnd = gl_GetWindow()->hWnd;

	if( glWnd == NULL ) return;

	memset( &msg, 0, sizeof(MSG) );
	while( PeekMessage( &msg, glWnd, 0, 0, PM_NOREMOVE ) ) 
	{
		if( !sys_PumpMessage( glWnd ) ) {
			sys_Exit(0);
		}
	}
}

/* ------------
sys_PumpMessage
------------ */
static int sys_PumpMessage( HWND win )
{
	MSG msg;

	if( !GetMessage( &msg, win, 0, 0 ) )
	{
		return 0;
	}

	DispatchMessage( &msg );

	return 1;
}

/* ---------------
sys_WinProc
--------------- */
LONG WINAPI sys_WinProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	LONG lDef = 1;

	switch(uMsg)
	{
////////////  KEYBOARD EVENTS /////////////
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		in_w32_Event( in_w32_MapKey( lParam ), 1 );
		return 0;

	case WM_KEYUP:
	case WM_SYSKEYUP:
		in_w32_Event( in_w32_MapKey( lParam ), 0 );
		return 0;

////////////  MOUSE MOVEMENT /////////////
	case WM_LBUTTONDOWN:
	case WM_NCLBUTTONDOWN:
		in_w32_Event( M_LEFT, 1 );
		return 0;
	case WM_RBUTTONDOWN:
	case WM_NCRBUTTONDOWN:
		in_w32_Event( M_RIGHT, 1 );
		return 0;
	case WM_MBUTTONDOWN:
	case WM_NCMBUTTONDOWN:
		in_w32_Event( M_MIDDLE, 1 );
		return 0;

	case WM_LBUTTONUP:
	case WM_NCLBUTTONUP:
		in_w32_Event( M_LEFT, 0 );
		return 0;
	case WM_RBUTTONUP:
	case WM_NCRBUTTONUP:
		in_w32_Event( M_RIGHT, 0 );
		return 0;
	case WM_MBUTTONUP:
	case WM_NCMBUTTONUP:
		in_w32_Event( M_MIDDLE, 0 );
		return 0;

	case WM_MOUSEMOVE:
	case WM_NCMOUSEMOVE:
		in_w32_Event( M_XAXIS, ((short)LOWORD(lParam)) );
		in_w32_Event( M_YAXIS, ((short)HIWORD(lParam)) );
		return 0;

	case WM_MOUSEWHEEL:
		in_w32_Event( M_WHEEL, ((short)HIWORD(wParam)) ); // minus == Mouse wheel Down (120 units for a single move)
		return 0;

	case WM_CAPTURECHANGED:
		sys_handles.cMouseCap = NULL;
		return 0;

////////////  SYSTEM EVENTS /////////////
	case WM_DESTROY:
		return 0;

	case WM_PAINT:
		// screen flips, etc get handled by opengl
		ValidateRect( hWnd, NULL );
		return 0;

	case WM_SIZE:
		dis_Resized( LOWORD(lParam), HIWORD(lParam) );
		return 0;

	case WM_SYSCOMMAND:
		// disable the screensaver
		if( wParam == SC_SCREENSAVE ) return 0;

	default:
		lDef = DefWindowProc( hWnd, uMsg, wParam, lParam );
	}
	return lDef;
}

/* ------------
WinMain
------------ */
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	// don't support win16
	if( hPrevInstance != NULL )
		return 0;
		
	// load data to the sys_handles
	sys_handles.cInstance = hInstance;
	sys_handles.cWndProc = sys_WinProc;
	sys_handles.cmdLine = lpCmdLine;
	sys_handles.nCmdShow = nCmdShow;

	// do the game
	eng_Start();

	return 0;
}

/* ------------
sys_Exit
------------ */
void sys_Exit( int errorcode )
{
	if( errorcode != 0 )
	{
		MessageBox( NULL, "An error occured during execution:  Exiting.\nPlease check runlog.txt for details.", "Program Error", MB_OK | MB_ICONWARNING );
	}

	exit(errorcode);
}

/* ------------
sys_ReportSystemInformation
------------ */
void sys_PrintSystemInformation()
{
	OSVERSIONINFO versioninfo;
	char osname[14] = "";
	unsigned long build;

	con_Print( "\nRetrieving Operating System Information..." );

	versioninfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if( !GetVersionEx( &versioninfo ) ) return;

	if( versioninfo.dwPlatformId == VER_PLATFORM_WIN32s ) { // Win32s on Windows 3.1. 
		con_Print( "\tDetected Win32s for Windows" );
		con_Print( "This program requires a version of Windows 95 or greater to run.  The program will now exit." );
		return;
	}
	else if( versioninfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ) { 
		// Win32 on Windows 95 or Windows 98.
		// For Windows 95, dwMinorVersion is zero. 
		// For Windows 98, dwMinorVersion is greater than zero. 
		if( versioninfo.dwMinorVersion <= 0 ) {
			tcstrncpy( osname, "Windows 95\0", 11 );
		}
		else if( versioninfo.dwMinorVersion == 10 ) {
			tcstrncpy( osname, "Windows 98\0", 11 );
		}
		else if( versioninfo.dwMinorVersion == 90 ) {
			tcstrncpy( osname, "Windows ME\0", 11 );
		}
		else {
			tcstrncpy( osname, "Unrecognized OS\0", 16 );
			return;
		}
		build = versioninfo.dwBuildNumber >> 16;
	}
	else if( versioninfo.dwPlatformId == VER_PLATFORM_WIN32_NT ) {
		if( (versioninfo.dwMinorVersion == 0) && (versioninfo.dwMajorVersion == 5) )
		{
			tcstrncpy( osname, "Windows 2000\0", 13 );
		}
		else if( (versioninfo.dwMinorVersion == 0) && (versioninfo.dwMajorVersion == 4) )
		{
			tcstrncpy( osname, "Windows NT 4.0\0", 15 );
		}
		else if( versioninfo.dwMinorVersion == 51 )
		{
			tcstrncpy( osname, "Windows NT 3.51\0", 16 );
		}
		else if( versioninfo.dwMinorVersion == 1 )
		{
			tcstrncpy( osname, "Windows XP\0", 11 );
		}
		else
		{
			tcstrncpy( osname, "Unrecognized OS\0", 16 );
		}
		build = versioninfo.dwBuildNumber;
	}
	else {
		con_Print( "\tPlatform ID not recognized." );
		return;
	}

	con_Print( "\tOperating System: %s - %s", osname, versioninfo.szCSDVersion );
	con_Print( "\tVersion: %d.%d", versioninfo.dwMajorVersion, versioninfo.dwMinorVersion );
	con_Print( "\tBuild: %d\n", versioninfo.dwBuildNumber );
}

