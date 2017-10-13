// Cognition
// client
// cl_foreground.c
// Created 3/20/03 by Terrence Cole

// Includes
/////////////
#include "cl_global.h"
#include "gl/gl.h"
#include <stdio.h> // sprintf

// Definitions
////////////////
#define CL_HUD_IMG24 "hudTop24"
#define CL_HUD_IMG8 "hudTop8"
#define CL_CROSSHAIR_IMG24 "crosshair0-24"
#define CL_CROSSHAIR_IMG8 "crosshair0-8"

#define CL_THREE_IMG24 ""
#define CL_THREE_IMG8 ""
#define CL_TWO_IMG24 ""
#define CL_TWO_IMG8 ""
#define CL_ONE_IMG24 ""
#define CL_ONE_IMG8 ""

#define FGO_MINETEXT_CENTERX 75
#define FGO_TEXT_BOTTOMY 78

#define FGO_TEXT_FONT 65.0f
#define FGO_TEXT_TAIL_SIZE 8.0f

#define FGO_TIMEDEC_CENTERX 956
#define FGO_TIMEBOTTOM_CENTERY 78
	
// Local Structures
/////////////////////

// Global Prototypes
//////////////////////
/*
int fgo_cl_Initialize(void);
void fgo_cl_Terminate(void);
void fgo_cl_Reload( char *null );
void fgo_cl_Frame();
void fgo_cl_Draw();
*/

// Local Prototypes
/////////////////////
static void fgo_cl_DrawHUD();
static void fgo_cl_DrawLaunchState();
static void fgo_cl_DrawLounge();

// Local Variables
////////////////////
static colormap_t *cmHUD = NULL;
static colormap_t *cmCrosshair = NULL;
static colormap_t *cmLoungeScreen = NULL;
static colormap_t *cmLaunchScreen = NULL;
static colormap_t *cmNumber3 = NULL;
static colormap_t *cmNumber2 = NULL;
static colormap_t *cmNumber1 = NULL;
static colormap_t *cmGo = NULL;

// *********** FUNCTIONALITY ***********
/* ------------
fgo_cl_Initialize - attempts the load of all required foreground objects, returns 0 if a load fails, 1 on success
------------ */
int fgo_cl_Initialize(void)
{
	char buffer[1024];
	
	// setup the new parameters
	colormap_params_t cmParams;
	cmParams.bMipmap = 1; // (look pretty at all resolutions / scales)
	cmParams.bNoRescale = 0; // allow the engine to save texture memory
	cmParams.iEnv = GL_MODULATE; // standard blending
	cmParams.iFilter = ei->cm_GetGlobalFilter(); // user level effects choice 
	cmParams.iScaleBias = 0; // no default biasing on rescale
	cmParams.iWrap = GL_CLAMP; // these are generally being used as labels
	
	// load the hud image
	cmHUD = ei->cm_LoadFromFiles( "clientHUD", CL_HUD_IMG24, CL_HUD_IMG8, &cmParams );
	if( !cmHUD )
	{
		ei->con_Print( "<RED>Client Foreground Load Failed:  HUD top colormap load failed." );
		return 0;
	}

	// load the cursor image
	cmCrosshair = ei->cm_LoadFromFiles( "clientCrosshair", CL_CROSSHAIR_IMG24, CL_CROSSHAIR_IMG8, &cmParams );
	if( !cmCrosshair )
	{
		ei->con_Print( "<RED>Client Foreground Load Failed:  HUD crosshair colormap load failed." );
		return 0;
	}

	// load backgrounds for loading and lounging
	snprintf( buffer, 1024, "background%d", ei->rng_Long( 0, CON_NUM_BACKGROUNDS ) );
	cmLoungeScreen = ei->cm_LoadConstAlpha( "lounge_bg", buffer, IMG_ALPHA_SOLID, &cmParams );
	if( cmLoungeScreen == NULL )
	{
		ei->con_Print( "<RED>Client Foreground Init Failed:  Load Lounge Background Screen failed." );
		return 0;
	}
	snprintf( buffer, 1024, "background%d", ei->rng_Long( 0, CON_NUM_BACKGROUNDS ) );
	cmLaunchScreen = ei->cm_LoadConstAlpha( "launch_bg", buffer, IMG_ALPHA_SOLID, &cmParams );
	if( cmLaunchScreen == NULL )
	{
		ei->con_Print( "<RED>Client Foreground Init Failed:  Load Launch Background Screen failed." );
		return 0;
	}

	// load the background overlays for the lounge and load screens
	cmNumber3 =  ei->cm_LoadFromFiles( "LoungeThree", CL_THREE_IMG24, CL_THREE_IMG8, &cmParams );
	cmNumber2 =  ei->cm_LoadFromFiles( "LoungeTwo", CL_TWO_IMG24, CL_TWO_IMG8, &cmParams );
	cmNumber1 =  ei->cm_LoadFromFiles( "LoungeOne", CL_ONE_IMG24, CL_ONE_IMG8, &cmParams );
	
	return 1;
}

/* ------------
fgo_cl_Terminate - unload the working foreground set
------------ */
void fgo_cl_Terminate(void)
{
	ei->con_Print( "\nUnloading the client foreground objects..." );
}

/* ------------
fgo_cl_Reload - reloads the hud from file
------------ */
void fgo_cl_Reload( char *null )
{
	fgo_cl_Terminate();
	fgo_cl_Initialize();
}

/* ------------
fgo_cl_Frame - makes any computations, updates, etc. necessary for this frame of animation
------------ */
void fgo_cl_Frame()
{
	// this can get called before we are fully inited
	if( plState.self == NULL ) return;

	// no error condition on this operation
	plState.iMinesRemaining = wrld_CalculateMinesRemaining();

	// this has error conditions, but the error is okay for display for now
	plState.iMinesUnder = wrld_GetMinesUnderPlayer( plState.self->vPos );

	// the time computation is notably simpler
	plState.fGameTime = (float)(plState.tCurrent - plState.cl_join_time) / 1000.0f;
}

/* ------------
fgo_cl_Draw
------------ */
void fgo_cl_Draw()
{
	if( plState.game_state == SER_STATE_LOUNGE )
	{
		fgo_cl_DrawLounge();
	}
	else if( plState.game_state == SER_STATE_LAUNCH )
	{
		fgo_cl_DrawLaunchState();
	}
	else if( plState.game_state == SER_STATE_PLAYING )
	{
		fgo_cl_DrawHUD();
	}
}

/* ------------
fgo_cl_DrawHUD
------------ */
static void fgo_cl_DrawHUD()
{
	static color white = { 255, 255, 255, 255 };
	static color black = { 0, 0, 0, 255 };
	static color red = { 255, 0, 0, 255 };
	byte *clr;
	char buffer[1024];
	char buffer2[1024];
	float x, y, w, h;
	
	// draw the hud image
	ei->d_MenuRect( 0, SCR_WIDTH, SCR_HEIGHT - 128, SCR_HEIGHT, white, cmHUD );

	// common
	h = ei->d_GetTextHeight( FGO_TEXT_FONT );

	// center and draw the mines remaining
	snprintf( buffer, 1024, "%d", plState.iMinesRemaining );
	w = ei->d_GetTextWidth( buffer, FGO_TEXT_FONT );
	x = FGO_MINETEXT_CENTERX - (w / 2.0f);
	y = SCR_HEIGHT - FGO_TEXT_BOTTOMY;
	if( plState.iMinesRemaining >= 0 ) clr = white;
	else clr = red;
	ei->d_TextLineColor( buffer, FGO_TEXT_FONT, clr, x, y );

	// center and draw the time elapsed
	snprintf( buffer, 1024, "%.1f", plState.fGameTime );
	// get a measurement string
	snprintf( buffer2, 1024, "%d", (int)plState.fGameTime );
	w = ei->d_GetTextWidth( buffer2, FGO_TEXT_FONT );
	x = FGO_TIMEDEC_CENTERX - w;
	y = SCR_HEIGHT - FGO_TIMEBOTTOM_CENTERY;
	ei->d_TextLineColor( buffer, FGO_TEXT_FONT, white, x, y );

	// draw the crosshairs
	ei->d_MenuRect( 496, 528, 368, 400, white, cmCrosshair );
}

/* ------------
fgo_cl_DrawLaunchState
------------ */
static void fgo_cl_DrawLaunchState()
{
	static color white = { 255, 255, 255, 255 };
	static color start = { 255, 0, 0, 255 };
	static color end = { 0, 255, 0, 255 };


	// clear the color buffer with a pretty picture
	ei->d_MenuRect( 0, SCR_WIDTH, 0, SCR_HEIGHT, white, cmLaunchScreen );
	
	if( plState.launch_count == 4 && 
		(plState.launch_time - plState.tCurrent) < 4000 )
	{
		plState.launch_count = 3;
		
		//ei->d_MenuRect( clr, cmNumber3 );
	}
	else if( plState.launch_count == 3 && 
		(plState.launch_time - plState.tCurrent) < 3000 )
	{
		plState.launch_count = 2;

	}
	else if( plState.launch_count == 2 && 
		(plState.launch_time - plState.tCurrent) < 2000 )
	{
		plState.launch_count = 1;

	}
	else if( plState.launch_count == 1 && 
		(plState.launch_time - plState.tCurrent) < 1000 )
	{
		plState.launch_count = 0;

	}
}

/* ------------
fgo_cl_DrawLounge
------------ */
static void fgo_cl_DrawLounge()
{
	static color white = { 255, 255, 255, 255 };
	
	// draw the crosshairs
	ei->d_MenuRect( 0, SCR_WIDTH, 0, SCR_HEIGHT, white, cmLoungeScreen );
}

