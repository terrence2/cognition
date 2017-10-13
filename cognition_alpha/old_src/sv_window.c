// Cognition
// sv_window.c
// Created  by Terrence Cole 9/9/03

// Includes
/////////////
#include "cog_global.h"
#include "sv_window.h"

// Definitions
////////////////

// Local Structures
/////////////////////


// Global Prototypes
//////////////////////
/*
int win_Initialize();
void win_Terminate();
void win_Update();
void win_KeyEvent( uint8_t key, uint8_t key_state );
window_t win_Create( window_t parent, int x, int y, int w, int h, int flags, void *data );
void win_Destroy( window_t winHandle );
window_t win_GetRoot();
int win_SetParent( window_t window, window_t newParent );
int win_GetPosition( window_t window, int *x, int *y );
int win_GetSize( window_t window, int *w, int *h );
void win_PrintWindows( char *null );

void win_SetBordered( window_t window, int bIsBordered );
void win_SetFilled( window_t window, int bIsFilled );
void win_SetHidden( window_t window, int bIsHidden );
void win_SetTopmost( window_t window, int bIsTopmost );
void win_SetPosition( window_t window, int iX, int iY );
void win_SetSize( window_t window, int iW, int iH );
void win_SetColor( window_t window, uint8_t vColor[4] );
void win_SetDrawHandler( window_t window, void (*drawhandler)( void *data ) );
void win_SetEventHandler( window_t window, void(*eventhandler)( void *data, uint8_t key, uint8_t key_state ) );
void win_SetMouseInHandler( window_t window, void(*mouseinhandler)( void *data, int x, int y ) );
void win_SetMouseOutHandler( window_t window, void(*mouseouthandler)( void *data, int x, int y ) );
void win_SetFocus( window_t window );
*/

// Local Prototypes
/////////////////////
static void win_DefaultKeyEventHandler( void *data, uint8_t key, uint8_t key_state );
static void win_DefaultMouseInHandler( void *data, int x, int y );
static void win_DefaultMouseOutHandler( void *data, int x, int y );
static int win_IsValidHandle( window_t win );
static void win_UnlinkChild( win_struct_t *parent, win_struct_t *child );
static void win_AddChild( win_struct_t *parent, win_struct_t *child );
static void win_ChildKeyEvent( win_struct_t *parent, win_struct_t *child, uint8_t key, uint8_t key_state );
static void win_ChildMouseEvent( win_struct_t *parent, win_struct_t *child, uint8_t key, uint8_t key_state, int x, int y );

// Local Variables
////////////////////
static uint8_t wDefaultColor[4] = { 0, 0, 255, 255 };
static win_struct_t *winFocused = &windows[0];


// *********** FUNCTIONALITY ***********
/* ------------
win_Initialize
------------ */
int win_Initialize()
{
	colormap_params_t cmParams;

	con_Print( "\n<BLUE>Initializing</BLUE> window system..." );

	// clear everything
	memset( windows, 0, sizeof(win_struct_t) * WIN_MAX_WINDOWS );

	// setup the root
	winRoot = 0;
	wsRoot = &windows[winRoot];
	wsRoot->bInUse = 1;
	wsRoot->draw = win_DefaultDrawHandler;
	wsRoot->keyevent = win_DefaultKeyEventHandler;
	wsRoot->mousein = win_DefaultMouseInHandler;
	wsRoot->mouseout = win_DefaultMouseOutHandler;
	wsRoot->bBorder = 0;
	wsRoot->bFilled = 0;
	wsRoot->bHidden = 0;
	wsRoot->bTopmost = 0;
	wsRoot->color[0] = 0;
	wsRoot->color[1] = 0;
	wsRoot->color[2] = 255;
	wsRoot->color[3] = 255;
	win_SetPosition( winRoot, 0, 0 );
	win_SetSize( winRoot, 1024, 768 );
	wsRoot->parent = NULL;
	wsRoot->iNumChildren = 0;	

	// grab our various colormaps
	cmParams.iFilter = GL_LINEAR;
	cmParams.iWrap = GL_CLAMP;
	cmParams.iEnv = GL_MODULATE;
	cmParams.bMipmap = 1;
	cmParams.bNoRescale = 0;
	cmParams.iScaleBias = 0;
	cmBorder = cm_LoadFromFiles( "winBorder", "winBorder24", "winBorder8", &cmParams);
	cmFiller = cm_LoadFromFiles( "winFiller", "winFiller24", "winFiller8", &cmParams );
	if( cmBorder == NULL || cmFiller == NULL )
	{
		con_Print( "\t<RED>Failed.  Could not load Colormaps." );
		return 0;
	}

	// set the focus to root
	win_SetFocus( win_GetRoot() );
	
	con_Print( "\t<BLUE>Success.</BLUE>" );	
	return 1;
}

/* ------------
win_Terminate
------------ */
void win_Terminate()
{
	if( cmBorder ) cm_Unload( cmBorder );
	if( cmFiller ) cm_Unload( cmFiller );
}

/* ------------
win_Update
// look for mouseovers
// called every frame
------------ */
void win_Update()
{
	int x = 0;
	int y = 0;
	int a;
	win_struct_t *win;

	// get mouse position	
	ms_GetPosition( &x, &y );

	// check each window
	for( a = 0 ; a < WIN_MAX_WINDOWS ; a++ )
	{
		win = &windows[a];
		if( win->bInUse && !win->bHidden )
		{
			if( (x > win->left) && (x < win->right) &&
				(y > win->bottom) && (y < win->top) )
			{
				if( !win->bMouseOverLastFrame )
				{
					win->bMouseOverLastFrame = 1;
					if( win->mousein ) win->mousein( win->data, x, y );
				}
			}
			else
			{
				if( win->bMouseOverLastFrame )
				{
					win->bMouseOverLastFrame = 0;
					if( win->mouseout ) win->mouseout( win->data, x, y );
				}
			}
		}
	}
}

/* ------------
win_KeyEvent
// sends keyboard events to the focused window and up the tree to parents
// mouse input sets the focus and sends an event to the windows at the cursor, parent to childmost
------------ */
void win_KeyEvent( uint8_t key, uint8_t key_state )
{
	int x, y;

	// check the event type
	if( key >= M_EVENTS_LOW && key <= M_EVENTS_HIGH )
	{
		ms_GetPosition( &x, &y );
		win_ChildMouseEvent( NULL, &(windows[win_GetRoot()]), key, key_state, x, y );
	}
	else
	{
		win_ChildKeyEvent( NULL, winFocused, key, key_state );
	}
}

/* ------------
win_ChildKeyEvent
// calls up the tree to the root, passing key events
------------ */
static void win_ChildKeyEvent( win_struct_t *parent, win_struct_t *child, uint8_t key, uint8_t key_state )
{
	if( !child ) return;
	if( !child->bInUse ) return;

	if( child->keyevent ) child->keyevent( child->data, key, key_state );
	win_ChildKeyEvent( NULL, child->parent, key, key_state );
}

/* ------------
win_ChildMouseEvent
// cascades mouse events down the tree
------------ */
static void win_ChildMouseEvent( win_struct_t *parent, win_struct_t *child, uint8_t key, uint8_t key_state, int x, int y )
{
	int cnt, a;

	assert(child);
	if( !child ) return;
 
	// do not descend if this does not apply to us
	if( (x > child->left) && (x < child->right) && (y > child->bottom) && (y < child->top) )
	{
		// call this
		if( child->keyevent ) child->keyevent( child->data, key, key_state );
		// descend
		for( cnt = 0, a = 0 ; a < WIN_MAX_CHILDREN && cnt < child->iNumChildren ; a++ )
		{
			if( child->children[a] && child->children[a]->bInUse )
			{
				win_ChildMouseEvent( child, child->children[a], key, key_state, x, y );
				cnt++;
			}
		}

		// if this is the childmost window, set the focus
		winFocused = child;
	}
}


/* ------------
win_Create
// data can be anything to get passed to the draw function
------------ */
window_t win_Create( window_t parent, int x, int y, int w, int h, int flags, void *data )
{
	int a;
	int bIsDone;
	window_t wNew;
	win_struct_t *tmp;

	con_Print( "\n<BLUE>Creating</BLUE> Window ( %d, %d, %d, %d )...", x, y, w, h );
	
	// find the first available window
	a = 0;
	wNew = NULL_WINDOW;
	bIsDone = 0;
	while( a < WIN_MAX_WINDOWS && !bIsDone )
	{
		if( !windows[a].bInUse ) { wNew = a; bIsDone = 1; }
		a++;
	} 

	// test
	if( wNew == NULL_WINDOW )
	{
		con_Print( "<RED>Window System Warning:  Window List is Full.  Create Failed." );
		return NULL_WINDOW;
	}

	// setup the new window
	tmp = &windows[wNew];
	tmp->bInUse = 1;

	// flags
	if( flags & WIN_FLAG_HIDDEN ) tmp->bHidden = 1;
	else tmp->bHidden = 0;
	if( flags & WIN_FLAG_BORDERED ) tmp->bBorder = 1;
	else tmp->bBorder = 0;
	if( flags & WIN_FLAG_FILLED ) tmp->bFilled = 1;
	else tmp->bFilled = 0;
	if( flags & WIN_FLAG_TOPMOST ) tmp->bTopmost = 1;
	else tmp->bTopmost = 0;

	// links
	tmp->iNumChildren = 0;
	memset( tmp->children, 0, sizeof(win_struct_t*) * WIN_MAX_CHILDREN );

	// parameters
	win_SetPosition( wNew, x, y );
	win_SetSize( wNew, w, h );
	win_SetColor( wNew, wDefaultColor );
	tmp->data = data;
	
	// parent
	if( win_IsValidHandle(parent) )
	{
		tmp->parent = &windows[parent]; 
	}
	else
	{
		tmp->parent = &windows[win_GetRoot()];
	}

	// add this window to the parent's children
	win_AddChild( tmp->parent, tmp );

	// setup default callbacks
	tmp->keyevent = win_DefaultKeyEventHandler;
	tmp->mousein = win_DefaultMouseInHandler;
	tmp->mouseout = win_DefaultMouseOutHandler;
	tmp->draw = win_DefaultDrawHandler;

	con_Print( "\t<GREEN>Success!" );

	return wNew;
}

/* ------------
win_Destroy
------------ */
void win_Destroy( window_t winHandle )
{
	win_struct_t *win;

	assert( win_IsValidHandle(winHandle) );
	if( !win_IsValidHandle(winHandle) )
	{
		con_Print( "<RED>Window System Error:  Destroy called on invalid handle." );
		return;
	}

	// get a quick reference
	win = &windows[winHandle];

	// quick disable
	win->bInUse = 0;

	// softly assert the parent
	if( win->parent == NULL ) return;

	// unlink the parent's reference to this
	win_UnlinkChild( win->parent, win );
}

// ACCESSORS
/* ------------
win_GetRoot
------------ */
window_t win_GetRoot()
{
	return winRoot;
}

/* ------------
win_SetParent
------------ */
int win_SetParent( window_t window, window_t newParent )
{
	// assert
	if( !win_IsValidHandle( window ) )
	{
		con_Print( "<RED>Win_SetParent Failed:  window is invalid." );
		return 0;
	}

	// unlink the previous parent
	win_UnlinkChild( windows[window].parent, &windows[window] );

	// restructure
	if( win_IsValidHandle( newParent ) )
	{
		win_AddChild( &windows[newParent], &windows[window] ); 
	}
	else
	{
		win_AddChild( NULL, &windows[window] );
	}

	return 1;
}

/* ------------
win_GetPosition
------------ */
int win_GetPosition( window_t window, int *x, int *y )
{
	if( !win_IsValidHandle( window ) )
	{
		return 0;
	}
	if( x ) *x = windows[window].x;
	if( y ) *y = windows[window].y;
	return 1;
}

/* ------------
win_GetSize
------------ */
int win_GetSize( window_t window, int *w, int *h )
{
	if( !w || !h )
	{
		con_Print( "Win_GetPosition Failed:  w or h is NULL" );
		return 0;
	}
	if( !win_IsValidHandle( window ) )
	{
		*w = *h = 0;
		return 0;
	}
	*w = windows[window].w;
	*h = windows[window].h;
	return 1;
}

/* ------------
win_PrintWindows
// Prints the window list
------------ */
void win_PrintWindows( char *null )
{
	int a;

	con_Print( "(X, Y, W, H), Mouse Over, Num Children" );
	
	for( a = 0 ; a < WIN_MAX_WINDOWS ; a++ )
	{
		if( windows[a].bInUse )
		{
			con_Print( "( %d, %d, %d, %d ), %d, %d",  windows[a].x, windows[a].y, 
												   windows[a].w, windows[a].h, 
												   windows[a].bMouseOverLastFrame, 
												   windows[a].iNumChildren );
		}
	}
}

/* ------------
win_SetBordered
------------ */
void win_SetBordered( window_t window, int bIsBordered )
{
	if( !win_IsValidHandle( window ) ) return;
	windows[window].bBorder = bIsBordered;
}

/* ------------
win_SetFilled
------------ */
void win_SetFilled( window_t window, int bIsFilled )
{
	if( !win_IsValidHandle( window ) ) return;
	windows[window].bFilled = bIsFilled;
}

/* ------------
win_SetHidden
------------ */
void win_SetHidden( window_t window, int bIsHidden )
{
	if( !win_IsValidHandle( window ) ) return;
	windows[window].bHidden = bIsHidden;
}

/* ------------
win_SetTopmost
------------ */
void win_SetTopmost( window_t window, int bIsTopmost )
{
	if( !win_IsValidHandle( window ) ) return;
	windows[window].bTopmost = bIsTopmost;
}

/* ------------
win_SetPosition
------------ */
void win_SetPosition( window_t window, int iX, int iY )
{
	if( !win_IsValidHandle( window ) ) return;

	// set extents
	windows[window].x = windows[window].left = iX;
	windows[window].y = windows[window].bottom = iY;
	windows[window].right = iX + windows[window].w;
	windows[window].top = iY + windows[window].h;

	// compute corners
	windows[window].corners[0][X_AXIS] = (float)windows[window].left;
	windows[window].corners[0][Y_AXIS] = (float)windows[window].bottom;
	windows[window].corners[1][X_AXIS] = (float)windows[window].right;
	windows[window].corners[1][Y_AXIS] = (float)windows[window].bottom;
	windows[window].corners[2][X_AXIS] = (float)windows[window].right;
	windows[window].corners[2][Y_AXIS] = (float)windows[window].top;
	windows[window].corners[3][X_AXIS] = (float)windows[window].left;
	windows[window].corners[3][Y_AXIS] = (float)windows[window].top;
}

/* ------------
win_SetSize
------------ */
void win_SetSize( window_t window, int iW, int iH )
{
	if( !win_IsValidHandle( window ) ) return;

	// set extents
	windows[window].w = iW;
	windows[window].h = iH;
	windows[window].right = windows[window].left + iW;
	windows[window].top = windows[window].bottom + iH;

	// compute corners
	windows[window].corners[0][X_AXIS] = (float)windows[window].left;
	windows[window].corners[0][Y_AXIS] = (float)windows[window].bottom;
	windows[window].corners[1][X_AXIS] = (float)windows[window].right;
	windows[window].corners[1][Y_AXIS] = (float)windows[window].bottom;
	windows[window].corners[2][X_AXIS] = (float)windows[window].right;
	windows[window].corners[2][Y_AXIS] = (float)windows[window].top;
	windows[window].corners[3][X_AXIS] = (float)windows[window].left;
	windows[window].corners[3][Y_AXIS] = (float)windows[window].top;
}

/* ------------
win_SetColor
------------ */
void win_SetColor( window_t window, uint8_t vColor[4] )
{
	if( !win_IsValidHandle( window ) ) return;
	memcpy( windows[window].color, vColor, sizeof(uint8_t) * 4 );
}

/* ------------
win_SetDrawHandler
------------ */
void win_SetDrawHandler( window_t window, void (*drawhandler)( void *data ) )
{
	if( !win_IsValidHandle( window ) ) return;
	if( drawhandler != NULL ) windows[window].draw = drawhandler;
}

/* ------------
win_SetEventHandler
------------ */
void win_SetEventHandler( window_t window, void(*eventhandler)( void *data, uint8_t key, uint8_t key_state ) )
{
	if( !win_IsValidHandle( window ) ) return;
	if( eventhandler != NULL ) windows[window].keyevent = eventhandler;
}

/* ------------
win_SetMouseInHandler
------------ */
void win_SetMouseInHandler( window_t window, void(*mouseinhandler)( void *data, int x, int y ) )
{
	if( !win_IsValidHandle( window ) ) return;
	if( mouseinhandler != NULL ) windows[window].mousein = mouseinhandler;
}

/* ------------
win_SetMouseOutHandler
------------ */
void win_SetMouseOutHandler( window_t window, void(*mouseouthandler)( void *data, int x, int y ) )
{
	if( !win_IsValidHandle( window ) ) return;
	if( mouseouthandler != NULL ) windows[window].mouseout = mouseouthandler;
}

/* ------------
win_SetFocus
// set window to receive keyboard input key input
------------ */
void win_SetFocus( window_t window )
{
	if( !win_IsValidHandle(window) ) return;
	winFocused = &windows[window];
}

/* ------------
win_DefaultKeyEventHandler
------------ */
static void win_DefaultKeyEventHandler( void *data, uint8_t key, uint8_t key_state )
{
}

/* ------------
win_DefaultMouseInHandler
------------ */
static void win_DefaultMouseInHandler( void *data, int x, int y )
{
}

/* ------------
win_DefaultMouseOutHandler
------------ */
static void win_DefaultMouseOutHandler( void *data, int x, int y )
{
}

/* ------------
win_IsValidHandle
------------ */
static int win_IsValidHandle( window_t win )
{
	if( win < 0 ) return 0; // inclusive of NULL_WINDOW
	if( win >= WIN_MAX_WINDOWS ) return 0;
	if( !windows[win].bInUse ) return 0;
	return 1;
}

/* ------------
win_UnlinkChild
------------ */
static void win_UnlinkChild( win_struct_t *parent, win_struct_t *child )
{
	int a;

	assert( parent );
	assert( child );

	// iteration of parent's children
	a = 0;
	while( a < WIN_MAX_CHILDREN )
	{
		if( parent->children[a] == child )
		{
			// unlinking
			parent->children[a] = NULL;
			parent->iNumChildren--;
			child->parent = NULL;
			return;
		}
		a++;
	}
}

/* ------------
win_AddChild
------------ */
static void win_AddChild( win_struct_t *parent, win_struct_t *child )
{
	int a;

	assert( child );

	// can use NULL for root
	if( !parent )
	{
		parent = &windows[win_GetRoot()];
	}

	if( parent->iNumChildren >= WIN_MAX_CHILDREN )
	{
		con_Print( "<RED>Warning!  win_AddChild: parent with too many children." );
		return;
	}

	// look for a free spot
	a = 0;
	while( a < WIN_MAX_CHILDREN )
	{
		if( parent->children[a] == NULL )
		{
			parent->children[a] = child;
			parent->iNumChildren++;
			child->parent = parent;
			return;
		}

		a++;
	}

	con_Print( "<RED>Warning!  win_AddChild: PARENT WITH MISMATCHED CHILD LIST / COUNT." );
	assert(1);
}


