// Cognition
// draw.c
// by Terrence Cole 12/12/01

// Includes
/////////////
#include "cog_global.h"
#include "cog_opengl.h"

// Definitions
////////////////
// #define BLUR_INTERVAL 33
#define BLUR_INTERVAL 0
// in milliseconds, 1/30 of a second

// Structures
///////////////

// Global Prototypes
//////////////////////
/*
int d_Initialize(void);
void d_BeginDraw(void);
void d_EndDraw(void);
void d_Draw(void);
void d_Terminate(void);

int d_SetMapState1( shader_t *shader, int pass );
void d_UnsetMapState1( shader_t *shader );
*/

// Local Prototypes
/////////////////////

// Local Variables
////////////////////

// *********** FUNCTIONALITY ***********
/* ------------
d_Initialiize - setup objects we'll need for rendering, 1 on succes, 0 on fail
------------ */
int d_Initialize(void)
{
	// load and set a default skybox
	if( !d_CamInitialize() ) return 0;
	if( !d_LoadSkybox(NULL) ) return 0;

	return 1;
}

/* ------------
d_Terminate 
------------ */
void d_Terminate(void)
{
	// unload the default skybox
	d_UnloadSkybox();
}

/* ------------
d_BeginDraw 
------------ */
void d_BeginDraw(void)
{
	// since we render a skybox, we don't need to clear the color buffer
	glClear( GL_DEPTH_BUFFER_BIT );

	// do the camera rotation and other maintianance (eg frustum)
	d_CameraIn();
}

/* ------------
d_EndDraw 
------------ */
void d_EndDraw(void)
{
	// end the view transform
	d_CameraOut();

	// this should resolve to a page flip on most hardware
	gl_FlipBuffers();
}

/* ------------
d_Draw
------------ */
void d_Draw(void)
{
	// the very pre-loading draw
	if( state.bInInit )
	{
		// just draw the console since we don't have media yet
		d_DrawConsoleFullscreen();
		return;
	}

	// the normal loading screen
	else if( state.bInLoading )
	{
		d_DrawConsoleFullscreen();
		return;
	}

	// draw a skybox
	d_DrawSkybox();

	// draw the particle systems
	ps_Draw();

	// activate and position lights
	l_Draw();  // located in world space

	// draw the entity list
	d_DrawEnts();

	// draw the world cubes
	d_DrawWorld();

	// sorted transparencies
	d_DrawTransparent();

	// draw foreground systems
	d_EnterOrthoMode();

		// client game related stuff
		if( ci ) ci->cl_Draw2D( &state );

		// the menu interface
		if( mi && state.bDrawMenu ) mi->menu_Draw( &state );

		// the mouse
		ms_Draw();

		// the console
		d_DrawConsole();

	d_LeaveOrthoMode();
}

/* ------------
d_SetTexState
// called prior to a draw to set the texture matrix properly
// returns the number of remaining passes needed to draw the surface or group of surfaces with the given shader
// 1 pass rendering isn't EVER going to minimize our state changes, so let GL work out the details for us
------------ */
int d_SetMapState1( shader_t *shader, int pass )
{
	int a;
	int iNeededPasses;

	// what is this?
	if( pass < 0 || pass > 2 ) return 0;

	// setup for the common case
	glDisable( GL_TEXTURE_GEN_S );
	glDisable( GL_TEXTURE_GEN_T );

	// we are only concerned with texture operations here
	glMatrixMode( GL_TEXTURE );
	glLoadIdentity();

	// shut off lighting if we don't want it for this surface
	if( !shader->bApplyLights )
	{
		glDisable( GL_LIGHTING );
	}

	// if we are set to use the material, set the proper states
	if( shader->bUseMaterial )
	{
		glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, shader->material.ambient );
		glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, shader->material.diffuse );
		glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, shader->material.specular );
		glMateriali( GL_FRONT_AND_BACK, GL_SHININESS, shader->material.shininess );
	}

	// if we have a transform, go ahead and apply it;
	// if this turns into an env map pass, it will setup the TU for auto texgen
	// also:  we can apply whatever xform we want to the base tex matrix 
	// as we'll be resetting it here, for env pass and in UnsetMapState
	for( a = 0 ; a < shader->bNumControllers ; a++ )
	{
		float fTmp = 0.0f;
		switch( shader->controller[a].bFunct )
		{
		case SHADER_FUNCT_LINEAR:
			fTmp = (shader->controller[a].fRate * (float)state.frame_time) + shader->controller[a].fT0;
			break;
		case SHADER_FUNCT_SINE:
			// fAmp * sine( fRate ) + fT0
			fTmp = (shader->controller[a].fAmp * SIN( shader->controller[a].fRate * (float)state.frame_time )) + shader->controller[a].fT0;
			break;
		case SHADER_FUNCT_NONE:
		default:
			break;
		}

		// mux in the new transform, or 0 if none occured
		if( shader->controller[a].bAxis )
		{
			glTranslatef( 0.0f, fTmp, 0.0f );
		}
		else
		{
			glTranslatef( fTmp, 0.0f, 0.0f );
		}
	}

	// This may be a bit excessive and is necessarily highly confusing
	// It is:  a list of ALL possible draw pass / shader types

	// Common Case #1:  1, 2, or 3 pass, chosen later
	if( pass == 0 && shader->ColorMap != NULL )
	{
		// bind it
		glBindTexture( GL_TEXTURE_2D, shader->ColorMap->gl_name );

		// figure out how many passes we are going to need
		iNeededPasses = 1;
		if( shader->subColorMap != NULL ) iNeededPasses++;
		if( shader->EnvMap != NULL ) iNeededPasses++;
		return iNeededPasses;
	}

	// Common Case #2:  2nd of 2 or 3 passes, running on subColor 
	else if( pass == 1 && shader->subColorMap != NULL )
	{
		// bind it
		glBindTexture( GL_TEXTURE_2D, shader->subColorMap->gl_name );

		// set for second pass blending
		glDepthMask( GL_FALSE );
		glDepthFunc( GL_EQUAL );
		glBlendFunc( GL_ZERO, GL_SRC_COLOR );
	
		// count remaining passes
		iNeededPasses = 1;
		if( shader->EnvMap != NULL ) iNeededPasses++;
		return iNeededPasses;
	}

	// Common Case #3:  2nd of 2 passes, running EnvMap
	else if( pass == 1 && shader->subColorMap == NULL  && shader->EnvMap != NULL )
	{
		// set our texture units for auto env mapping
		glTexGenf( GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );
		glTexGenf( GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );
		glEnable( GL_TEXTURE_GEN_S );
		glEnable( GL_TEXTURE_GEN_T );
		
		// bind it
		glBindTexture( GL_TEXTURE_2D, shader->EnvMap->gl_name );

		// set for second pass blending
		glDepthMask( GL_FALSE );
		glDepthFunc( GL_EQUAL );
		glBlendFunc( GL_ZERO, GL_SRC_COLOR );

		// that was it
		return 1;
	}

	// Common Case #4: 3rd of 3 passes
	else if( pass == 2 && shader->EnvMap != NULL )
	{
		// set our texture units for auto env mapping
		glTexGenf( GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );
		glTexGenf( GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );
		glEnable( GL_TEXTURE_GEN_S );
		glEnable( GL_TEXTURE_GEN_T );
		
		// bind it
		glBindTexture( GL_TEXTURE_2D, shader->EnvMap->gl_name );

		// should already be here, but set for second pass blending anyway
		glDepthMask( GL_FALSE );
		glDepthFunc( GL_EQUAL );
		glBlendFunc( GL_ZERO, GL_SRC_COLOR );

		//we're done
		return 1;
	}

	// 1 pass: only env mapping
	else if( pass == 0 && shader->ColorMap == NULL && shader->EnvMap != NULL )
	{
		// set our texture units for auto env mapping
		glTexGenf( GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );
		glTexGenf( GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );
		glEnable( GL_TEXTURE_GEN_S );
		glEnable( GL_TEXTURE_GEN_T );

		// bind the texture
		glBindTexture( GL_TEXTURE_2D, shader->EnvMap->gl_name );

		// we only need one pass
		return 1;
	}

	// 1 pass:  material only
	else if( pass == 0 && shader->ColorMap == NULL && shader->EnvMap == NULL ) return 1;

	// these all return, so the else is here:
	// this is my oversight committee
	assert(0);
	con_Print( "<RED>Warning:  in d_SetMapState1, arrived at end return!" );
	return 0;
}

/* ------------
d_UnsetTexState
// called after every group of surfaces is drawn; undoes SetTexState
------------ */
void d_UnsetMapState1( shader_t *shader )
{
	// FIXME:  we may or may not need the first two items
	// do we draw anything that doesn't take this setup?  yes (for now).  keep them

	// reset these to the common case
	glDisable( GL_TEXTURE_GEN_S );
	glDisable( GL_TEXTURE_GEN_T );

	// reset anything untoward we may have done
	glMatrixMode( GL_TEXTURE );
	glLoadIdentity();

	// if we had a material, reset it to the default
	if( shader->bUseMaterial )
	{
		glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, gl_state.matDefault.ambient );
		glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, gl_state.matDefault.diffuse );
		glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, gl_state.matDefault.specular );
		glMateriali( GL_FRONT_AND_BACK, GL_SHININESS, gl_state.matDefault.shininess );
	}

	// restore lighting
	if( !shader->bApplyLights )
	{
		glEnable( GL_LIGHTING );
	}

	// reset all states to first pass-style
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glDepthMask( GL_TRUE );
	glDepthFunc( GL_LESS );
}


/*
					for( c = 0 ; c < gl_state.num_cm_units ; c++ )
					{
						gl_state.glMultiTexCoord2fARB( GL_TEXTURE0_ARB + c, 
													mesh->mverts[ mesh->mtris[ mesh->dlists[a].indicies[b] * 3 + 0 ] ][0],
													mesh->mverts[ mesh->mtris[ mesh->dlists[a].indicies[b] * 3 + 0 ] ][1] );
					}
*/


