// Cognition
// dr_transparent.c
// Created  by Terrence Cole 9/12/03 3:30PM PST

// Includes
/////////////
#include "cog_global.h"

// Definitions
////////////////
#define MAX_TRANS_OBJS 1024
#define TO_TYPE_NULL 0
#define TO_TYPE_MESH 1
#define TO_TYPE_QUAD 2
#define TO_TYPE_SPRITE 3

// Local Structures
/////////////////////
typedef struct
{
	double rsqrd; // extra precision for particle systems
	int type; // the draw method
	shader_t *sh; // entity, quad
	colormap_t *cm; // sprite
	float fMapAng; // sprite
	entity_t *ent; // entity
	mesh_t *mesh; // entity
	vec3 v[4]; // quad, sprite
	vec2 tc[4]; // quad
	color clr; // quad, sprite, entity
	vec3 norm; // quad
} trans_struct_t;

// Global Prototypes
//////////////////////
/*
int trans_Initialize(void);
void trans_Terminate(void);
void d_DrawTransparent();
void trans_Quad( vec3 v1, vec3 v2, vec3 v3, vec3 v4, vec3 vNorm, 
					vec2 tc1, vec2 tc2, vec2 tc3, vec2 tc4, 
					uint8_t r, uint8_t g, uint8_t b, uint8_t a, shader_t *sh );
void trans_Mesh( entity_t *ent, mesh_t *mesh, uint8_t r, uint8_t g, uint8_t b, uint8_t a, shader_t *sh );
void trans_Sprite( vec3 vPos, float fSize, float fAng, color clr, colormap_t *cm );
*/

// Local Prototypes
/////////////////////
static void trans_quick_sort( int lo, int hi );

// Local Variables
////////////////////
static trans_struct_t transobjs[MAX_TRANS_OBJS];
static trans_struct_t *tPtrs[MAX_TRANS_OBJS];
static int32_t iNumTransObjs = 0;
static varlatch vlUsePrettyParticles;
const static vec2 spr_tc0 = { 0.0f, 0.0f };
const static vec2 spr_tc1 = { 1.0f, 0.0f };
const static vec2 spr_tc2 = { 1.0f, 1.0f };
const static vec2 spr_tc3 = { 0.0f, 1.0f };

// *********** FUNCTIONALITY ***********
/* ------------
trans_Initialize
------------ */
int trans_Initialize(void)
{
	vlUsePrettyParticles = var_GetVarLatch( "cl_use_pretty_particles" );
	return 1;
}

/* ------------
trans_Terminate
------------ */
void trans_Terminate(void)
{
}

/* ------------
trans_Quad
// v in GLOBAL(!) coordinates
------------ */
void trans_Quad( vec3 v1, vec3 v2, vec3 v3, vec3 v4, vec3 vNorm, 
					vec2 tc1, vec2 tc2, vec2 tc3, vec2 tc4, 
					uint8_t r, uint8_t g, uint8_t b, uint8_t a, shader_t *sh )
{
	vec3 vTmp;
	camera_t *cam = d_GetCamera();

	m3f_VecSubtract( vTmp, cam->vPos, v1 );
	transobjs[iNumTransObjs].rsqrd = vTmp[0] * vTmp[0] + vTmp[1] * vTmp[1] + vTmp[2] * vTmp[2];

	transobjs[iNumTransObjs].type = TO_TYPE_QUAD;
	memcpy( transobjs[iNumTransObjs].v[0], v1, sizeof(vec3) );
	memcpy( transobjs[iNumTransObjs].v[1], v2, sizeof(vec3) );
	memcpy( transobjs[iNumTransObjs].v[2], v3, sizeof(vec3) );
	memcpy( transobjs[iNumTransObjs].v[3], v4, sizeof(vec3) );
	memcpy( transobjs[iNumTransObjs].tc[0], tc1, sizeof(vec2) );
	memcpy( transobjs[iNumTransObjs].tc[1], tc2, sizeof(vec2) );
	memcpy( transobjs[iNumTransObjs].tc[2], tc3, sizeof(vec2) );
	memcpy( transobjs[iNumTransObjs].tc[3], tc4, sizeof(vec2) );
	transobjs[iNumTransObjs].clr[0] = r;
	transobjs[iNumTransObjs].clr[1] = g;
	transobjs[iNumTransObjs].clr[2] = b;
	transobjs[iNumTransObjs].clr[3] = a;
	memcpy( transobjs[iNumTransObjs].norm, vNorm, sizeof(vec3) );
	transobjs[iNumTransObjs].sh = sh;
	
	tPtrs[iNumTransObjs] = &transobjs[iNumTransObjs];
	iNumTransObjs++;
}

/* ------------
trans_Mesh
------------ */
void trans_Mesh( entity_t *ent, mesh_t *mesh, uint8_t r, uint8_t g, uint8_t b, uint8_t a, shader_t *sh )
{
	vec3 vLoc;
	vec3 vTmp;
	camera_t *cam = d_GetCamera();
	
	assert( ent != NULL );
	assert( ent->mod != NULL );
	assert( ent->mod->moo != NULL );
	assert( mesh != NULL );

	if( iNumTransObjs >= MAX_TRANS_OBJS )
	{
		con_Print( "<RED>Transparent Drawing System Error:  Out of Object Tracking Space" );
		return;
	}

	// FIXME:  Make the moo's store mesh centers explicitly
	// effectivly, the sphere tree center for the current frame IS the mesh center, but it'd be nice to have it more convenient
	m3f_TransformToGlobal( vLoc, mesh->frames[ent->mod->moo->cur_frame].stRoot->loc, ent->vPos, ent->vRot );
	
	// compute rsqrd
	m3f_VecSubtract( vTmp, vLoc, cam->vPos );
	transobjs[iNumTransObjs].rsqrd = vTmp[0] * vTmp[0] + vTmp[1] * vTmp[1] + vTmp[2] * vTmp[2];

	// store stuff for draw
	transobjs[iNumTransObjs].type = TO_TYPE_MESH;
	transobjs[iNumTransObjs].sh = sh;
	transobjs[iNumTransObjs].ent = ent;
	transobjs[iNumTransObjs].mesh = mesh;
	transobjs[iNumTransObjs].clr[0] = r;
	transobjs[iNumTransObjs].clr[1] = g;
	transobjs[iNumTransObjs].clr[2] = b;
	transobjs[iNumTransObjs].clr[3] = a;
	
	tPtrs[iNumTransObjs] = &transobjs[iNumTransObjs];
	iNumTransObjs++;
}

/* ------------
trans_Mesh
------------ */
void trans_Sprite( vec3 vPos, float fSize, float fAng, color clr, colormap_t *cm )
{
	vec3 vNorm;
	vec3 vRt, vUp;
	camera_t *cam = d_GetCamera();

	// runtime overshoot
	if( iNumTransObjs >= MAX_TRANS_OBJS )
	{
		con_Print( "<RED>Transparent Drawing System Error:  Out of Object Tracking Space" );
		return;
	}

	// assert
	assert( vPos );
	assert( clr );
	assert( cm );

	// get the apparent distance
	m3f_VecSubtract( vNorm, cam->vPos, vPos );

	// set basic props
	transobjs[iNumTransObjs].rsqrd = vNorm[0] * vNorm[0] + vNorm[1] * vNorm[1] + vNorm[2] * vNorm[2];
	transobjs[iNumTransObjs].type = TO_TYPE_SPRITE;

	// set color
	transobjs[iNumTransObjs].clr[0] = clr[0];
	transobjs[iNumTransObjs].clr[1] = clr[1];
	transobjs[iNumTransObjs].clr[2] = clr[2];
	transobjs[iNumTransObjs].clr[3] = clr[3];

	// set the map and angle
	transobjs[iNumTransObjs].cm = cm;
	transobjs[iNumTransObjs].fMapAng = fAng;

	// get right and up
	if( (int)var_GetFloat( vlUsePrettyParticles ) ) // low detail
	{
		// unitize the normal
		m3f_VecUnitize( vNorm );

		// get vRt
		m3f_VecCross( vRt, vNorm, cam->vUp );
		m3f_VecUnitize( vRt );

		// get vUp
		m3f_VecCross( vUp, vNorm, vRt );

		// scale to size
		m3f_VecScale( vRt, fSize, vRt );
		m3f_VecScale( vUp, fSize, vUp );
	}
	else
	{
		// just get from camera
		m3f_VecScale( vRt, fSize, cam->vRt );
		m3f_VecScale( vUp, fSize, cam->vUp );
	}

	// get geometry
	transobjs[iNumTransObjs].v[0][X_AXIS] = vPos[X_AXIS] - vRt[X_AXIS] - vUp[X_AXIS];
	transobjs[iNumTransObjs].v[0][Y_AXIS] = vPos[Y_AXIS] - vRt[Y_AXIS] - vUp[Y_AXIS];
	transobjs[iNumTransObjs].v[0][Z_AXIS] = vPos[Z_AXIS] - vRt[Z_AXIS] - vUp[Z_AXIS];
	transobjs[iNumTransObjs].v[1][X_AXIS] = vPos[X_AXIS] + vRt[X_AXIS] - vUp[X_AXIS];
	transobjs[iNumTransObjs].v[1][Y_AXIS] = vPos[Y_AXIS] + vRt[Y_AXIS] - vUp[Y_AXIS];
	transobjs[iNumTransObjs].v[1][Z_AXIS] = vPos[Z_AXIS] + vRt[Z_AXIS] - vUp[Z_AXIS];
	transobjs[iNumTransObjs].v[2][X_AXIS] = vPos[X_AXIS] + vRt[X_AXIS] + vUp[X_AXIS];
	transobjs[iNumTransObjs].v[2][Y_AXIS] = vPos[Y_AXIS] + vRt[Y_AXIS] + vUp[Y_AXIS];
	transobjs[iNumTransObjs].v[2][Z_AXIS] = vPos[Z_AXIS] + vRt[Z_AXIS] + vUp[Z_AXIS];
	transobjs[iNumTransObjs].v[3][X_AXIS] = vPos[X_AXIS] - vRt[X_AXIS] + vUp[X_AXIS];
	transobjs[iNumTransObjs].v[3][Y_AXIS] = vPos[Y_AXIS] - vRt[Y_AXIS] + vUp[Y_AXIS];
	transobjs[iNumTransObjs].v[3][Z_AXIS] = vPos[Z_AXIS] - vRt[Z_AXIS] + vUp[Z_AXIS];
	
	tPtrs[iNumTransObjs] = &transobjs[iNumTransObjs];
	iNumTransObjs++;
}

/* ------------
trans_Draw
------------ */
void d_DrawTransparent()
{
	int32_t a, bDone;
	int iPass, iRemaining;

	// states
	glDepthMask( GL_FALSE );

	// sort
	trans_quick_sort( 0, iNumTransObjs - 1 );

	// draw
	a = iNumTransObjs - 1;
	bDone = 0;
	while( a >= 0 && !bDone )
	{
		if( tPtrs[a] == NULL ) 
		{
			iNumTransObjs = 0;
			bDone = 1;
		}
		else if( tPtrs[a]->type == TO_TYPE_NULL )
		{
		}
		else if( tPtrs[a]->type == TO_TYPE_MESH )
		{
			// transform to global
			glMatrixMode( GL_MODELVIEW );
			glPushMatrix();
			
				glTranslatef( tPtrs[a]->ent->vPos[X_AXIS], tPtrs[a]->ent->vPos[Y_AXIS], tPtrs[a]->ent->vPos[Z_AXIS] );
				glRotatef( tPtrs[a]->ent->vRot[PITCH], 1.0f, 0.0f, 0.0f );
				glRotatef( tPtrs[a]->ent->vRot[YAW], 0.0f, 1.0f, 0.0f );

				// do the drawing
				iPass = 0;
				do {
					iRemaining = d_SetMapState1( tPtrs[a]->sh, iPass );
					// force this
					glDepthMask( GL_FALSE );
					glColor4ubv( tPtrs[a]->clr );
					d_DrawMesh( tPtrs[a]->mesh, tPtrs[a]->ent->mod->moo );
					iRemaining--;
					iPass++;
				} while( iRemaining );
				d_UnsetMapState1( tPtrs[a]->sh );

			glMatrixMode( GL_MODELVIEW );
			glPopMatrix();
		}
		else if( tPtrs[a]->type == TO_TYPE_QUAD )
		{
			// this data *should* be pre transformed
			iPass = 0;
			do {
				iRemaining = d_SetMapState1( tPtrs[a]->sh, iPass );
				glDepthMask( GL_FALSE );
				glColor4ubv( tPtrs[a]->clr );
				glBegin( GL_QUADS );
					glNormal3fv( tPtrs[a]->norm );
					
					glTexCoord2fv( tPtrs[a]->tc[0] );
					glVertex3fv( tPtrs[a]->v[0] );
					
					glTexCoord2fv( tPtrs[a]->tc[1] );
					glVertex3fv( tPtrs[a]->v[1] );
					
					glTexCoord2fv( tPtrs[a]->tc[2] );
					glVertex3fv( tPtrs[a]->v[2] );
					
					glTexCoord2fv( tPtrs[a]->tc[3] );
					glVertex3fv( tPtrs[a]->v[3] );
				glEnd();
				
				iRemaining--;
				iPass++;
			} while( iRemaining );
			d_UnsetMapState1( tPtrs[a]->sh );
		}
		else if( tPtrs[a]->type == TO_TYPE_SPRITE )
		{
			// isolate the texture matrix
			glMatrixMode( GL_TEXTURE );
			glPushMatrix();
			glLoadIdentity();

			// rotate
			glTranslatef( 0.5, 0.5, 0.0 );
			glRotatef( tPtrs[a]->fMapAng, 0.0f, 0.0f, 1.0f );
			glTranslatef( -0.5, -0.5, 0.0 );

			// color
			glColor4ubv( tPtrs[a]->clr );
			SAFE_BIND( tPtrs[a]->cm );

			// internally lit
			glDisable( GL_LIGHTING );

			// do the drawing
			glBegin( GL_QUADS );
				glTexCoord2fv(spr_tc0);
				glVertex3fv( tPtrs[a]->v[0] );
				glTexCoord2fv(spr_tc1);
				glVertex3fv( tPtrs[a]->v[1] );
				glTexCoord2fv(spr_tc2);
				glVertex3fv( tPtrs[a]->v[2] );
				glTexCoord2fv(spr_tc3);
				glVertex3fv( tPtrs[a]->v[3] );
			glEnd();

			// restore states
			glEnable( GL_LIGHTING );
			glPopMatrix();
		}
		else
		{
		}

		a--;
	}

	glDepthMask( GL_TRUE );

	// reset
	iNumTransObjs = 0;
}

static void trans_quick_sort( int lo, int hi )
{
	int i, p;
	
	// breakpoint
	if( lo >= hi ) return;

	// partition and sort
	for( i = lo, p = lo - 1 ; i <= hi ; i++ )
	{
		if( tPtrs[i]->rsqrd <= tPtrs[hi]->rsqrd && i != ++p ) 
		{
			trans_struct_t *tmp = tPtrs[i];
			tPtrs[i] = tPtrs[p];
			tPtrs[p] = tmp;
		}
	}
	
	// recurse
	trans_quick_sort( lo, p - 1 );
	trans_quick_sort( p + 1, hi );
}

