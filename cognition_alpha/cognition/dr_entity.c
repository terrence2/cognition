// Cognition
// draw_entity.c
// by Terrence Cole 12/12/01

// Includes
/////////////
#include "cog_global.h"
#include "cog_opengl.h"

// Definitions
////////////////
#define MAX_TRANS_ENTS 64

// Structures
///////////////

// Global Prototypes
//////////////////////
/*
void d_DrawEnts();
// per frame drawing of all ents, sorted, culled, etc.

void d_DrawMesh( mesh_t *mesh, moo_data_t *moo );

void d_DrawEntSingle( entity_t *ent );
// draws a single entity; out of the loop; no depth sort or transparency hashing
// this could cause very stupid looking artifacts if not used correctly

*/

// Local Prototypes
/////////////////////
static void d_DrawOpaque( entity_t *ents );
// draws all ents, skipping transparent meshes, out of frustum, and irrelevant meshes

// drawing abstraction
// static void d_DrawSceneBase( entity_t *ent_head, int mapType, byte bDrawTransparency );
static void d_DrawMeshFrame( mesh_t *mesh, int frame );

// presort / pre-draw functionality
static void d_ent_FindPVS();
static byte d_SphereInFrustum( float *pos, float radius );

// Local Variables
////////////////////

// *********** FUNCTIONALITY ***********
/* ------------
d_DrawEntsBase
// rule:  only solid ents in the world:  outside of the world, anything goes
// 1) Find ents with transparency
// 3) Draw ALL ents, sans transparent parts
------------ */
void d_DrawEnts()
{
	// compute the pvs / split and sort transparents
	d_ent_FindPVS();

	// draw the opaque's
	d_DrawOpaque( *(state.ent_list) );
}

/* ------------
d_DrawEntSingle
// draws a single entity; out of the loop; no depth sort or transparency hashing
// this could cause very stupid looking artifacts if not used correctly and carefully
------------ */
/*
void d_DrawEntSingle( entity_t *ent )
{
	uint32_t a;
	uint32_t iPass, iRemaining;
	shader_t *shTmp;
	moo_data_t *mooTmp;

	// sanity checks (segfaults are bad)
	if( ent == NULL || ent->mod == NULL || ent->mod->moo == NULL ) return;

	// set mooTmp
	mooTmp = ent->mod->moo;

	// set the modelview for this entity
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glTranslatef( ent->position[X_AXIS], ent->position[Y_AXIS], ent->position[Z_AXIS] );
	glRotatef( ent->rotation[PITCH], 1.0f, 0.0f, 0.0f );
	glRotatef( ent->rotation[YAW], 0.0f, 1.0f, 0.0f );

	// iterate the meshes
	for( a = 0 ; a < mooTmp->num_meshes ; a++ )
	{
		// set the shader
		shTmp = ent->mod->shaders[a];

		// draw the mesh
		iPass = 0;
		do
		{
			iRemaining = d_SetMapState1( shTmp, iPass);
			d_DrawMeshBase( &(mooTmp->meshes[a]), mooTmp );
			iRemaining--;
			iPass++;
		} while( iRemaining );
		d_UnsetMapState1( shTmp );
	}

	// unload this transform
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();
}
*/

/* ------------
d_DrawTransparent
// iterates the list of ents with transparent parts, drawing the transparent meshes
// n.b. the ents that made it to this list are implicitly in the view frustum, not hidden, and have models/moos, etc
------------ */
/*
static void d_DrawTransparent( entity_t **ents, uint32_t iNumEnts )
{
	uint32_t a, b;
	uint32_t iPass, iRemaining;
	entity_t *entTmp;
	moo_data_t *mooTmp;
	shader_t *shTmp;
	
	for( a = 0 ; a < iNumEnts ; a++ )
	{
		// get stuff we'll be using in inner loops
		entTmp = ents[a];
		mooTmp = entTmp->mod->moo;

		// set the modelview for this entity
		glMatrixMode( GL_MODELVIEW );
		glPushMatrix();
		glTranslatef( entTmp->position[X_AXIS], entTmp->position[Y_AXIS], entTmp->position[Z_AXIS] );
		glRotatef( entTmp->rotation[PITCH], 1.0f, 0.0f, 0.0f );
		glRotatef( entTmp->rotation[YAW], 0.0f, 1.0f, 0.0f );
		
		for( b = 0 ; b < mooTmp->num_meshes ; b++ )
		{
			// the current shader
			shTmp = entTmp->mod->shaders[b];

			// only draw transparent meshes
			if( shTmp->bHasTransparency )
			{
				// do the drawing
				iPass = 0;
				do
				{
					iRemaining = d_SetMapState1( shTmp, iPass );
					d_DrawMeshBase( &(mooTmp->meshes[b]), mooTmp );
					iPass++;
					iRemaining--;
				} while( iRemaining );
				d_UnsetMapState1( entTmp->mod->shaders[b] );
			}
		}

		// unpush the modelview
		glMatrixMode( GL_MODELVIEW );
		glPopMatrix();
	}
}
*/

/* ------------
d_DrawOpaque
// draws all ents, skipping transparent meshes, out of frustum, and irrelevant meshes
------------ */
static void d_DrawOpaque( entity_t *ents )
{
	uint32_t a, iPass, iRemaining;
	moo_data_t *mooTmp;
	shader_t *shTmp;
	
	// iterate the main list, ignore out of frustum and transparent
	while( ents != NULL )
	{
		// entity drawing is only relevant for models (esp. those in the frustum)
		if( (ents->mod != NULL) && (ents->mod->moo != NULL) && (ents->bInFrustum) && !(ents->bHintHidden) )
		{
			// set the modelview for this entity
			glMatrixMode( GL_MODELVIEW );
			glPushMatrix();
			glTranslatef( ents->vPos[X_AXIS], ents->vPos[Y_AXIS], ents->vPos[Z_AXIS] );
			glRotatef( ents->vRot[PITCH], 1.0f, 0.0f, 0.0f );
			glRotatef( ents->vRot[YAW], 0.0f, 1.0f, 0.0f );

			// draw all meshes, sans transparent in each ent
			// FIXME: would it be worth the time to selection sort these meshes by shader b4 we get here?
			mooTmp = ents->mod->moo;
			for( a = 0 ; a < mooTmp->num_meshes ; a++ )
			{
				// get the shader
				shTmp = ents->mod->shaders[a];

				// transparent meshes should already be drawn
				if( !shTmp->bHasTransparency )
				{
					// do the drawing
					iPass = 0;
					do
					{
						iRemaining = d_SetMapState1( shTmp, iPass );
						d_DrawMesh( &(mooTmp->meshes[a]), mooTmp );
						iRemaining--;
						iPass++;
					} while( iRemaining );
					d_UnsetMapState1( shTmp );
				}
			}

			// unpush the modelview
			glMatrixMode( GL_MODELVIEW );
			glPopMatrix();
		}
	
		// next
		ents = ents->next;
	}
}

/* ------------
d_DrawEntList - Iterates the entire entity list and draws all models in the list
------------ */
/*
static void d_DrawSceneBase( entity_t *ent_head, int mapType, byte bDrawTransparency )
{
	int a;

	// split rendering of transparent sections so that we can render back to front on transparent objects
	if( !bDrawTransparency )
	{
		// iterate the main list, ignore out of frustum and transparent
		while( ent_head != NULL )
		{
			// entity drawing is only relevant for models (esp. those in the frustum
			if( (ent_head->mod != NULL) && 
				(ent_head->mod->moo != NULL) && (ent_head->bInFrustum) && !(ent_head->bHintHidden) )
			{
				d_DrawEntityBase( ent_head, mapType, bDrawTransparency );
			}

			// next
			ent_head = ent_head->next;
		}
	}
	else  // the transparent meshes, back to front
	{
		for( a = 0 ; a < num_trans_ents ; a++ )
		{
			d_DrawEntityBase( trans_ents[a], mapType, bDrawTransparency );
		}
	}
	
}
*/

/* ------------
d_DrawEntityBase - draws the bDrawTransparency meshes from one entity in 'BASE' mode 
------------ */
/*
void d_DrawEntityBase( entity_t *ent, int mapType, byte bDrawTransparency )
{
	int a, b;
	colormap_t *map;
	shader_t *sh;

	// assert
	if( ent == NULL ) return;
	
	// look at all meshes to see what to draw
	for( a = 0 ; a < ent->mod->moo->num_meshes ; a++ )
	{
		map = NULL;
		sh = ent->mod->shaders[a];
		
		// only draw transparent meshes in the bDrawTransparency drawing pass
		if(  sh->bHasTransparency != bDrawTransparency )
		{
			// this is bad practice, but it keeps the draw loop from looking UGLY
			continue;
		}
		
		// the map we pass to the draw mesh fcn will be based on mapType
		switch( mapType )
		{
			case MAP_FLAG_COLOR:
				map = ent->mod->shaders[a]->ColorMap;
				break;
			case MAP_FLAG_SUBCOLOR:
				map = ent->mod->shaders[a]->subColorMap;
				break;
			case MAP_FLAG_ENV:
				map = ent->mod->shaders[a]->EnvMap;
				break;
			case MAP_FLAG_BUMP: // no bump mapping in OGL 1.2 mode
			default:
				return;
		}
			
		// we don't draw without a map
		if( map != NULL )
		{
			// set lighting states
			if( !ent->mod->shaders[a]->bApplyLights )
			{
				glDisable( GL_LIGHTING );
			}

			// set the texture states
			glMatrixMode( GL_TEXTURE );
			glPushMatrix();
			glLoadIdentity();

			// setup the transforms
			if( mapType != MAP_FLAG_ENV )
			{
				for( b = 0 ; b < sh->bNumControllers ; b++ )
				{
					switch( sh->controller[b].bFunct )
					{
					float fTmp;
					case SHADER_FUNCT_LINEAR:
						fTmp = (sh->controller[b].fRate * (float)state.frame_time) + sh->controller[b].fT0;
						(sh->controller[b].bAxis) ? glTranslatef( 0.0f, fTmp, 0.0f ) : glTranslatef( fTmp, 0.0f, 0.0f ); 
						break;
					case SHADER_FUNCT_SINE:
						// fAmp * sine( fRate ) + fT0
						fTmp = (sh->controller[b].fAmp * SIN( sh->controller[b].fRate * (float)state.frame_time )) + sh->controller[b].fT0;
						(sh->controller[b].bAxis) ? glTranslatef( 0.0f, fTmp, 0.0f ) : glTranslatef( fTmp, 0.0f, 0.0f ); 
						break;
					case SHADER_FUNCT_NONE:
					default:
						break;
					}
				}
			}

				glMatrixMode( GL_MODELVIEW );
				glPushMatrix();
	
					// set the transform for this mesh
					glTranslatef( self->vPos[X_AXIS], self->vPos[Y_AXIS], self->vPos[Z_AXIS] );
					glRotatef( ent->rotation[PITCH], 1.0f, 0.0f, 0.0f );
					glRotatef( ent->rotation[YAW], 0.0f, 1.0f, 0.0f );
	
					// set the texture states for this pass
					SAFE_BIND( map );
				
					// setup material states
	
						// if the mesh is non-animating we can draw with arrays, else
						// we have to perform per-vertex processing
						if( ent->mod->moo->num_frames > 1 )
						{
							d_DrawMeshBase( &(ent->mod->moo->meshes[a]), ent->mod->moo );
						}
						else
						{
							d_DrawMeshFrameBase( &(ent->mod->moo->meshes[a]), 0 );
						}
				// pop the model view additions for this mesh
				glPopMatrix();

			// pop the texturing
			glMatrixMode( GL_TEXTURE );
			glPopMatrix();

			// reset lighting states
			if( !ent->mod->shaders[a]->bApplyLights )
			{
				glEnable( GL_LIGHTING );
			}
		}
	}
}
*/

/* ------------
d_DrawMesh 
// this draws a general mesh, interpolating frame animations
// assumes transform is already set
------------ */
void d_DrawMesh( mesh_t *mesh, moo_data_t *moo )
{
	vec3 v1;
	vec3 next;
	uint32_t index;
	uint32_t iLen;
	uint32_t next_frame;
	uint32_t a, b, c;

	// contract out to a quicker function if we can
	if( moo->num_frames <= 1 )
	{
		d_DrawMeshFrame( mesh, 0 );
		return;
	}

	// find the next frame		
	next_frame = moo->cur_frame + 1;
	while( next_frame >= moo->num_frames ) next_frame -= moo->num_frames;

	// draw each list
	for( a = 0 ; a < mesh->num_dlists ; a++ )
	{
		if( mesh->dlists[a].length > 0 )
		{
			iLen = mesh->dlists[a].length;
			
			// setup to draw the list
			glBegin( GL_TRIANGLE_STRIP );
			
				for( b = 0 ; b < iLen ; b++ )
				{
					// get stuff we'll be using frequently as the dereference is expensive
					index = mesh->dlists[a].indicies[b];
					memcpy( v1, mesh->frames[moo->cur_frame].verts[ index ], sizeof(vec3) );
					memcpy( next, mesh->frames[next_frame].verts[ index ], sizeof(vec3) );
					
					v1[X_AXIS] += moo->sub_frame * (next[X_AXIS] - v1[X_AXIS]);
					v1[Y_AXIS] += moo->sub_frame * (next[Y_AXIS] - v1[Y_AXIS]);
					v1[Z_AXIS] += moo->sub_frame * (next[Z_AXIS] - v1[Z_AXIS]);

					glTexCoord2fv( mesh->mverts[ index ] );
					// FIXME: interpolate this too
					glNormal3fv( mesh->frames[0].vNorms[ index ] );
					glVertex3fv( v1 );
				}
				
			glEnd();
		}
		else
		{
			// draw spare tris
			glBegin( GL_TRIANGLES );
			
				iLen = ABS(mesh->dlists[a].length);
				for( b = 0 ; b < iLen ; b++ )
				{
					// each index specifies a triangle
					index =  mesh->dlists[a].indicies[b] * 3; // n.b. this is only the base

					// FIXME: do a manual unroll of this for braindead compilers
					for( c = 0 ; c < 3 ; c++ )
					{
						
						memcpy( v1, mesh->frames[moo->cur_frame].verts[ mesh->tris[ index + c ] ], sizeof(vec3) );
						memcpy( next, mesh->frames[next_frame].verts[ mesh->tris[ index + c ] ], sizeof(vec3) );

						v1[X_AXIS] += moo->sub_frame * (next[X_AXIS] - v1[X_AXIS]);
						v1[Y_AXIS] += moo->sub_frame * (next[Y_AXIS] - v1[Y_AXIS]);
						v1[Z_AXIS] += moo->sub_frame * (next[Z_AXIS] - v1[Z_AXIS]);

						glTexCoord2fv( mesh->mverts[ mesh->mtris[ index + c ] ] );
						glNormal3fv( mesh->frames[moo->cur_frame].vNorms[ mesh->tris[ index + c ] ] );
						glVertex3fv( v1 );
					}
					

				}
			glEnd();
		}
	}

	return;
}

/* ------------
d_DrawMeshFrame
------------ */
static void d_DrawMeshFrame( mesh_t *mesh, int frame )
{
	uint32_t a, b;
	uint32_t iLen;
	
	// setup array pointers / iterators
	glNormalPointer( GL_FLOAT, 0, mesh->frames[frame].vNorms );
	glTexCoordPointer( 2, GL_FLOAT, 0, mesh->mverts );
	glVertexPointer( 3, GL_FLOAT, 0, mesh->frames[frame].verts );

	// all the drawing data is contained in vbo's
	for( a = 0 ; a < mesh->num_dlists ; a++ )
	{
		// strips can go straight through the array call
		if( mesh->dlists[a].length > 0 )
		{
			glDrawElements( GL_TRIANGLE_STRIP, mesh->dlists[a].length, GL_UNSIGNED_INT, mesh->dlists[a].indicies );
		}

		// spare triangles have to be rendered individually to dereference the triangle indicies
		else
		{
			iLen = ABS(mesh->dlists[a].length);
			glBegin( GL_TRIANGLES );
			
				for( b = 0 ; b < iLen ; b++ )
				{
					// vert 1
					glTexCoord2fv( mesh->mverts[ mesh->mtris[ mesh->dlists[a].indicies[b] * 3 + 0 ] ] );
					glNormal3fv( mesh->frames[frame].vNorms[ mesh->tris[ mesh->dlists[a].indicies[b] * 3 + 0 ] ] );
					glVertex3fv( mesh->frames[frame].verts[ mesh->tris[ mesh->dlists[a].indicies[b] * 3 + 0 ] ] );

					// vert 2
					glTexCoord2fv( mesh->mverts[ mesh->mtris[ mesh->dlists[a].indicies[b] * 3 + 1 ] ] );
					glNormal3fv( mesh->frames[frame].vNorms[ mesh->tris[ mesh->dlists[a].indicies[b] * 3 + 1 ] ] );
					glVertex3fv( mesh->frames[frame].verts[ mesh->tris[ mesh->dlists[a].indicies[b] * 3 + 1 ] ] );

					// vert 3
					glTexCoord2fv( mesh->mverts[ mesh->mtris[ mesh->dlists[a].indicies[b] * 3 + 2 ] ] );
					glNormal3fv( mesh->frames[frame].vNorms[ mesh->tris[ mesh->dlists[a].indicies[b] * 3 + 2 ] ] );
					glVertex3fv( mesh->frames[frame].verts[ mesh->tris[ mesh->dlists[a].indicies[b] * 3 + 2 ] ] );
				}

			glEnd();
		}
	}
}


/* ------------
d_ent_FindPVS - markes all entities in the current frustum for drawing, sorts transparent entities
------------ */
static void d_ent_FindPVS()
{
	int a;
	entity_t *ent = *(state.ent_list);
	camera_t *camTmp;
	
	// get the current camera
	camTmp = d_GetCamera();

	while( (ent != NULL) )
	{
		// is the entity drawable?
		if( !(ent->bHintHidden) && (ent->mod != NULL) && (ent->mod->moo != NULL) )
		{
			// is the ent in the pvs?
			ent->bInFrustum = d_SphereInFrustum( ent->vPos, (ent->fRadius * 1.01f) );

			if( ent->bInFrustum )
			{
				// if we are in the frustum are we transparent anywhere?
				a = 0;
				while( (a < ent->mod->num_shaders) )
				{
					if( ent->mod->shaders[a]->bHasTransparency )
					{
						trans_Mesh( ent, &(ent->mod->moo->meshes[a]), 255, 255, 255, 255, ent->mod->shaders[a] );
					}
					a++;
				}
			}
		}
	
		// next
		ent = ent->next;
	}
}

/* ------------
d_SphereInFrustum
------------ */
static byte d_SphereInFrustum( float *pos, float radius )
{
	float d;
	camera_t *camTmp;

	// get the current camera transform
	camTmp = d_GetCamera();

	// other tests are more detailed  // negative distances are OUTSIDE of the plane
	d = (m3f_VecDot( camTmp->pTop.norm, pos ) + camTmp->pTop.D) + radius;
	if( d < 0 ) return 0;
	
	d = (m3f_VecDot( camTmp->pBottom.norm, pos ) + camTmp->pBottom.D) + radius;
	if( d < 0 ) return 0;
	
	d = (m3f_VecDot( camTmp->pLeft.norm, pos ) + camTmp->pLeft.D) + radius;
	if( d < 0 ) return 0;
	
	d = (m3f_VecDot( camTmp->pRight.norm, pos ) + camTmp->pRight.D) + radius;
	if( d < 0 ) return 0;

	return 1;
}


