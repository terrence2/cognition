// Cognition
// sal_display_win.c
// Created  by Terrence Cole

// Includes
/////////////
#include "cog_global.h"

// Definitions
////////////////

// Local Structures
/////////////////////

// Global Prototypes
//////////////////////
/*
int dis_Initialize(void);
void dis_Terminate(void);
int dis_SetMode( int width, int height, int bitdepth, uint32_t maxrefreshrate );
void dis_Resized( int w, int h );
void dis_OpenWindow( display_t *param );
int dis_CloseWindow( display_t *window );
*/

// Local Prototypes
/////////////////////

// Local Variables
////////////////////

// *********** FUNCTIONALITY ***********
/* ------------
dis_Initialize - does nothing now
------------ */
int dis_Initialize(void)
{
	return 1;
}

/* ------------
dis_Terminate - does nothing at this point
------------ */
void dis_Terminate(void)
{
}

/* ------------
dis_SetMode - changes the display settings of the primary device to the given
			- zero on fail, non-zero on success
------------ */
int dis_SetMode( int width, int height, int bitdepth, uint32_t maxrefreshrate )
{	
	DEVMODE dm;
	BOOL ret = 1;
	int mode = 0;
	unsigned int high_refresh = 0;
	int high_mode = 0;

	while( ret != 0 )
	{
		memset( &dm, 0, sizeof(DEVMODE) );
		dm.dmSize = sizeof(DEVMODE);
		dm.dmDriverExtra = 0;

		ret = EnumDisplaySettings( NULL, mode, &dm );
		if( (ret != 0) && (dm.dmBitsPerPel == (unsigned)bitdepth) && 
						  (dm.dmPelsHeight == (unsigned)height) && 
						  (dm.dmPelsWidth == (unsigned)width) )
		{
			// this mode fits our requirements
			if( dm.dmDisplayFrequency > high_refresh && dm.dmDisplayFrequency <= maxrefreshrate ) 
			{
				high_refresh = dm.dmDisplayFrequency;
				high_mode = mode;
			}
		}
		
		mode++;
	}

	// see if we found a mode
	if( high_refresh < 1 ) return 0;

	// get the highest mode
	memset( &dm, 0, sizeof(DEVMODE) );
	dm.dmSize = sizeof(DEVMODE);
	dm.dmDriverExtra = 0;
	ret = EnumDisplaySettings( NULL, high_mode, &dm );
	if( !ret ) return 0;

	ChangeDisplaySettings( &dm, 0 );
	return ret;
}

/* ------------
dis_Resized - handles system resize events
------------ */
void dis_Resized( int w, int h )
{
}

/* ------------
dis_OpenWindow - opens a window with the given properties, sets parameters to valid values
			   - needs width, height, x, y, WinProc, WindowClass style and CreateWindow style
					and the Owner Instance, an Icon, a Cursor
------------ */
void dis_OpenWindow( display_t *param )
{
	WNDCLASS wc;
	RECT r;

	con_Print( "Creating Window: \"%s\"", param->winClassName );

	wc.lpszClassName = param->winClassName;
	wc.lpfnWndProc = param->WinProc;
	wc.style = param->winClassStyle;
	wc.hInstance = param->winOwnerInstance;
	wc.hIcon = param->hIcon;
	wc.hCursor = param->hCursor;
	wc.hbrBackground = param->hBackBrush;
	wc.lpszMenuName = NULL;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	
	RegisterClass( &wc );

	// compute the actual window constraints
	r.top = param->y;
	r.left = param->x;
	r.bottom = param->y + param->h;
	r.right = param->x + param->w;
	AdjustWindowRectEx( &r, param->winStyle, FALSE, param->winStyleEx );

	// create the window
	param->hWnd = CreateWindowEx( 
		param->winStyleEx,
		param->winClassName,
		"Cognition",  // window title
		param->winStyle,
		r.left,
		r.top,
		r.right - r.left,
		r.bottom - r.top,
		NULL,
		NULL,
		param->winOwnerInstance,
		NULL
	);

	if( param->hWnd == NULL  ) 
	{
		con_Print( "<RED>Display System Error:  CreateWindowEx Failed with error:  %d", GetLastError() );
		return;
	}

	ShowWindow( param->hWnd, param->nCmdShow );

	SetForegroundWindow( param->hWnd );

	BringWindowToTop( param->hWnd );

}

/* ------------
dis_CloseWindow - destroys window
------------ */
int dis_CloseWindow( display_t *window )
{
	// remove the window
	ShowWindow( window->hWnd, SW_HIDE );
	
	// close the window
	DestroyWindow(window->hWnd);
	window->hWnd = NULL;

	// unload the class
	UnregisterClass( window->winClassName, sys_handles.cInstance );

	return 1;
}
