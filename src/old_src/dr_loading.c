// Cognition
// dr_loading.c
// Created  by Terrence Cole
// 1st revision 3/14/03

// Includes
/////////////
#include "cog_global.h"
#include "sv_console.h"

// Definitions
////////////////

// Local Structures
/////////////////////

// Global Prototypes
//////////////////////
/*
int d_LoadLoadingScreen(void);
void d_UnloadLoadingScreen(void);
void d_DrawLoadingBase();
*/

// Local Prototypes
/////////////////////

// Local Variables
////////////////////
// static entity_t *entMine = NULL;
static shader_t *shLoadScreen = NULL;
//static camera_t camLoading;

// *********** FUNCTIONALITY ***********
/* ------------
d_LoadLoadingScreen - gets the shader, model, and other media for the loading screen
------------ */
int d_LoadLoadingScreen(void)
{
//	vec3 vPos;
//	vec3 vAng;
/*
	// assert
	if( entMine != NULL )
	{
		con_Print( "<RED>Draw LoadLoadingScreen Assert Failed:  entMine is not NULL!" );
		eng_Stop( "150010" );
		return 0;
	}
*/	if( shLoadScreen != NULL )
	{
		con_Print( "<RED>Draw LoadLoadingScreen Assert Failed:	shLoadScreen is not NULL!" );
		eng_Stop( "150011" );
		return 0;
	}
/*
	// position and load the entity
	vPos[0] = 45.0f;
	vPos[1] = 20.0f;
	vPos[2] = -70.0f;
	vAng[0] = 5.0f;
	vAng[1] = 10.0f;
	entMine = ent_Spawn( "LoadingMine", 0, "mine.def", vPos, vAng, (float*)vUnity, NULL, NULL, 0 );

	// test the load
	if( entMine == NULL )
	{
		con_Print( "<RED>Draw LoadLoadingScreen Error:  Entity Load for entMine Failed!." );
		return 0;
	}

	// set the mine hidden
	entMine->bHintHidden = 1;
*/
	// get the shader
	shLoadScreen = sh_LoadShader( "loading_screen.sh" );
	if( shLoadScreen == NULL )
	{
		con_Print( "<RED>Draw LoadLoadingScreen Error:  Shader load for shLoadScreen Failed!" );
		return 0;
	}
/*
	// get a camera
	memset( &camLoading, 0, sizeof(camera_t) );
	camLoading.ffar = 100.0f;
	camLoading.fnear = 1.0f;
	camLoading.type = CAMERA_PROJECTION;
	camLoading.ffov = 45.0f;
	camLoading.vPos[0] = camLoading.vPos[1] = camLoading.vPos[2] = 0.0f;
	camLoading.vAng[0] = camLoading.vAng[1] = 0.0f;
*/
	return 1;
}

/* ------------
d_UnloadLoadingScreen - unloads the loading screen media
------------ */
void d_UnloadLoadingScreen(void)
{
//	if( entMine != NULL ) ent_Remove( entMine );
	if( shLoadScreen != NULL ) sh_UnloadShader( shLoadScreen );
}

/* ------------
d_DrawLoadingBase - draws the loading screen
------------ */
void d_DrawLoadingBase()
{
	// ent draw
//	static float vLightDir[4] = { 0.1f, 0.1f, 1.0f, 0.0f };
//	static float vWhite[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
//	light_t *lTmp;

	// text draw
	int index;
	int a;
	float y;
	int iNumLines;

	// assert (that we have media from which to draw)
	if( shLoadScreen == NULL )
	{
		con_Print( "Draw DrawLoadingBase Assert Failed:  shLoadScreen is NULL" );
		eng_Stop( "150012" );
		return;
	}
	if( shLoadScreen->ColorMap == NULL )
	{
		con_Print( "Draw DrawLoadingBase Assert Failed:  shLoadScreen->ColorMap is NULL" );
		eng_Stop( "150013" );
		return;
	}
/*
	if( entMine == NULL )
	{
		con_Print( "Draw DrawLoadingBase Assert Failed:  entMine is NULL" );
		eng_Stop( "150014" );
		return;
	}
*/
	// get a clean slate, first
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();

	// setup states
	glDisable( GL_LIGHTING );
	glDepthMask( GL_FALSE );
	glDisable( GL_DEPTH_TEST );

		// draw the background in ortho
		glOrtho( 0, SCR_WIDTH, 0, SCR_HEIGHT, -10, 10 );

		// draw the colormap
		SAFE_BIND( shLoadScreen->ColorMap );
		glColor4ub( 255, 255, 255, 255 );

		glBegin( GL_QUADS );
			glNormal3f( 0.0f, 0.0f, 1.0f );

			glTexCoord2f( 0.0f, 1.0f );
			glVertex2f( 0.0f, 		SCR_HEIGHT );
				
			glTexCoord2f( 0.0f, 0.0f );
			glVertex2f( 0.0f, 		0.0f );
				
			glTexCoord2f( 1.0f, 0.0f );
			glVertex2f( SCR_WIDTH, 	0.0f );

			glTexCoord2f( 1.0f, 1.0f );
			glVertex2f( SCR_WIDTH, 	SCR_HEIGHT );
		glEnd();

		// draw console history to the bottom of the screen
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		glColor4ub( 255, 255, 255, 255 );
		
		// draw visible console lines
		y = 0.0f;
		index = console.lastLine;
		iNumLines = (int)(300.0f / console.cs_height);
		for( a = iNumLines ; a > 0 ; a-- )
		{
			d_TextLineWhite( console.lines[index], 12.0f, 2.0f, y );
			y += (console.cs_height);
		
			index--;
			while( index < 0 ) index += NUM_CON_LINES;
			if( index >= NUM_CON_LINES ) index = NUM_CON_LINES - 1;  // we can only go down so don't wrap around
		}

	// reset states
	glEnable( GL_DEPTH_TEST );
	glDepthMask( GL_TRUE );
	glEnable( GL_LIGHTING );
/*
		// set a nice projection
		glLoadIdentity();
		d_SetCamera( &camLoading );

		// get a light
		lTmp = l_Spawn( LIGHT_DIRECTIONAL, vLightDir, vWhite, vWhite, vWhite, 0, 0, vLightDir, 0, 0, 0 );
		l_Draw();

		// draw the mine
		entMine->bHintHidden = 0;
		d_DrawEntityBase( entMine, MAP_FLAG_COLOR, 0 );
		entMine->bHintHidden = 1;
		entMine->rotation[1] += 5.0f;
		while( entMine->rotation[1] >= 360.0f ) entMine->rotation[1] -= 360.0f;

		// get rid of the light
		l_Kill( lTmp );
*/
	// reset the slate
	// glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();
}
