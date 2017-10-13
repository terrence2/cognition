// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

// Cognition
// dr_skybox.c
// by Terrence Cole 1/3/04

// Includes
/////////////
#include "cog_global.h"
#include "cog_opengl.h"

// Definitions
////////////////

// Structures
///////////////

// Global Prototypes
//////////////////////
/*
int d_LoadSkybox( char *name );
	// Loads the skybox

void d_UnloadSkybox(void);
	// Unloads the skybox

void d_DrawSkybox();
	// draw the skybox
*/

// Local Prototypes
/////////////////////

// Local Variables
////////////////////
static entity_t *skybox = NULL;
static char defSkybox[] = "skybox01.def";
static vec3 vZero = { 0.0f, 0.0f, 0.0f };
//static color sbSunColor = { 253, 255, 126, 255 };
//static vec3 sbSunV1 = { -10, 0, -100 };
//static vec3 sbSunV2 = { -10, 0, -100 };
//static vec3 sbSunV3 = { -10, 0, -100 };
//static vec3 sbSunV4 = { -10, 0, -100 };

// *********** FUNCTIONALITY ***********
/* ------------
d_LoadSkybox
------------ */
int d_LoadSkybox( char *name )
{
	char *userModel;
	entity_t *prevSkybox = skybox;

	// if the name is specified load it
	if( name )
	{
		// try to load the user model
		skybox = ent_Spawn( "skybox", 0, name, vZero, vZero );
	}
	// otherwise use the players set skybox
	else
	{
		// get the user variable
		userModel = var_GetStringFromName( "cl_skybox" );

		// try to load the user model
		skybox = ent_Spawn( "skybox", 0, userModel, vZero, vZero );

		// if we don't have a model, load the default
		if( skybox == NULL )
		{
			skybox = ent_Spawn( "skybox", 0, defSkybox, vZero, vZero );
		}
	}
	
	// try to revert
	if( skybox == NULL && prevSkybox != NULL )
	{
		con_Print( "<RED>Skybox Load Failed:  Using previous skybox." );
		skybox = prevSkybox;
		prevSkybox = NULL;
	}

	// unload the previous
	ent_Remove( prevSkybox );	
	
	// postcondition
	if( skybox == NULL )
	{
		con_Print( "<RED>Draw System Error:  Postcondition failed:  skybox is NULL leaving LoadSkybox" );
		eng_Stop( "150004" );
	}

	// setup skybox for rendering
	skybox->bHintHidden = 1;

	// render a sun somewhere
	// we're now rendering this with the background

	return 1;
}

/* ------------
d_UnloadDefaultSkybox - unloads the default skybox standin
------------ */
void d_UnloadSkybox(void)
{
	if( skybox ) ent_Remove( skybox );
	skybox = NULL;
}

/* ------------
d_DrawSkyboxBase - draw the current skybox
------------ */
void d_DrawSkybox()
{
	uint32_t a;
	int iRemaining, iPass;
	camera_t *camTmp;

	// assert
	if( skybox == NULL )
	{
		con_Print( "<RED>Draw System Error:  Assert Failed in DrawSkyboxBase:  skybox is NULL" );
		eng_Stop( "150005" );
		return;
	}
	
	// sanity check
	if( skybox->mod == NULL || skybox->mod->moo == NULL ) return;

	// get the current camera
	camTmp = d_GetCamera();

	// turn off the depth buffer
	glDepthMask( GL_FALSE );

	// do the skybox position muxed with our camera position
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();

	// set the skybox transform as just the rotation
	glLoadIdentity();
	glRotatef( -camTmp->vAng[PITCH], 1.0f, 0.0f, 0.0f );
	glRotatef( -camTmp->vAng[YAW], 0.0f, 1.0f, 0.0f );

	glColor4f( 255, 255, 255, 255 );

	// draw each mesh in the skybox
	for( a = 0 ; a < skybox->mod->moo->num_meshes ; a++ )
	{
		iPass = 0;
		do
		{
			iRemaining = d_SetMapState1( skybox->mod->shaders[a], iPass );
			d_DrawMesh( &(skybox->mod->moo->meshes[a]), skybox->mod->moo );
			iRemaining--;
			iPass++;
		} while( iRemaining );
		d_UnsetMapState1( skybox->mod->shaders[a] );
	}

	// paste on the sun
	
	
	// pop the transform
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();

	// turn back on the depth buffer
	glDepthMask( GL_TRUE );
}
