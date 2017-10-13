// Cognition
// sv_mouse.c
// Created  by Terrence Cole 10/9/03

// Includes
/////////////
#include "cog_global.h"

// Definitions
////////////////
#define M_X_AXIS 1
#define M_Y_AXIS 0
#define CURSOR_SIZE 32

// Local Structures
/////////////////////

// Global Prototypes
//////////////////////
/*
int ms_Initialize();
void ms_Terminate();
void ms_Update();
int ms_GetPosition( int *x, int *y );
void ms_Show();
void ms_Hide();
int ms_MouseIsVisible();
int ms_MouseIsHidden();
void ms_Draw();
*/

// Local Prototypes
/////////////////////

// Local Variables
////////////////////
static int bIsHidden = 1;
static colormap_t *cmCursor = NULL;
static int position[2] = { SCR_WIDTH / 2, SCR_HEIGHT / 2 };
static varlatch vlMenuSensitivity = NULL;
static color msColor = { 255, 255, 255, 255 };

// *********** FUNCTIONALITY ***********
/* ------------
m_Initialize
// Depends:  Windowing, PixmapWidget
------------ */
int ms_Initialize()
{
	double tStart, tEnd;
	colormap_params_t cmParams;
	
	con_Print( "\n<BLUE>Initializing</BLUE> Mouse System..." );
	eng_LoadingFrame();
	tStart = t_GetSaneTime();

	// get cursor picture
	cmParams.bMipmap = 1; // (look pretty at all resolutions / scales)
	cmParams.bNoRescale = 0; // allow the engine to save texture memory
	cmParams.iEnv = GL_MODULATE; // standard blending
	cmParams.iFilter = cm_GetGlobalFilter(); // user level effects choice 
	cmParams.iScaleBias = 0; // no default biasing on rescale
	cmParams.iWrap = GL_CLAMP; 
	cmCursor = cm_LoadFromFiles( "cursor", "cursor24", "cursor8", &cmParams );
	
	// latch the sensitivity
	vlMenuSensitivity = var_GetVarLatch( "menu_sensitivity" );
	if( vlMenuSensitivity == NULL )
	{
		con_Print( "<RED>Mouse System Initialization Failed:  Could Not Latch to Sensitivity" );
		return 0;
	}

	tEnd = t_GetSaneTime();
	con_Print( "\tDone:  %.3f sec", tEnd - tStart );
	eng_LoadingFrame();

	return 1;
}

/* ------------
m_Terminate
------------ */
void ms_Terminate()
{
	con_Print( "\n<RED>Terminating</RED> Mouse System..." );

	if( cmCursor )
	{
		cm_Unload( cmCursor );
	}

	con_Print( "\tDone." );
}

/* ------------
m_Update
// get and set new position of cursor
------------ */
void ms_Update()
{
	// no updates if hidden
	if( bIsHidden ) return;

	// no updates if not initialized
	if( !cmCursor ) return;

	// update off deltas
	position[X_AXIS] -= state.mAxisDelta[M_X_AXIS] * (int)var_GetFloat( vlMenuSensitivity );
	position[Y_AXIS] += state.mAxisDelta[M_Y_AXIS] * (int)var_GetFloat( vlMenuSensitivity );

	// lock to screen constraints
	if( position[X_AXIS] > SCR_WIDTH ) position[X_AXIS] = SCR_WIDTH;
	if( position[X_AXIS] < 0 ) position[X_AXIS] = 0;
	if( position[Y_AXIS] > SCR_HEIGHT - CURSOR_SIZE ) position[Y_AXIS] = SCR_HEIGHT - CURSOR_SIZE;
	if( position[Y_AXIS] < -CURSOR_SIZE ) position[Y_AXIS] = -CURSOR_SIZE;
}

/* ------------
m_GetPosition
// sets x and y, returns bIsHidden
------------ */
int ms_GetPosition( int *x, int *y )
{
	if( x ) *x = position[X_AXIS];
	if( y ) *y = position[Y_AXIS] + CURSOR_SIZE;
	return bIsHidden;
}

/* ------------
ms_Show
------------ */
void ms_Show()
{
	bIsHidden = 0;
}

/* ------------
ms_Hide
------------ */
void ms_Hide()
{
	bIsHidden = 1;
}

/* ------------
ms_MouseIsVisible
------------ */
int ms_MouseIsVisible()
{
	return !bIsHidden;
}

/* ------------
ms_MouseIsHidden
------------ */
int ms_MouseIsHidden()
{
	return bIsHidden;
}

/* ------------
ms_Draw
------------ */
void ms_Draw()
{
	if( bIsHidden ) return;
	d_MenuRect( (float)position[0], (float)(position[0] + CURSOR_SIZE), 
				(float)position[1], (float)(position[1] + CURSOR_SIZE),
				msColor, cmCursor );
}

