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
// collision.c
// Created 10-4-02 @ 1118 by Terrence Cole

// Includes
/////////////
#include "cog_global.h"

// Global Definitions
///////////////////////
/*
#define COL_PRIORITY_NONE 1
#define COL_PRIORITY_LOW 2
#define COL_PRIORITY_MEDIUM 3
#define COL_PRIORITY_HIGH 4
*/

// Local Structures
/////////////////////

// Global Prototypes
//////////////////////
/*
byte col_CheckEnts( float *hit, entity_t *ent1, entity_t *ent2, byte priority );
void col_Respond( entity_t *ent, vec3 hit, vec3 vNorm );
*/

// Local Prototypes
/////////////////////
static byte col_ee_CheckSphereTree( float *hit, entity_t *Ent1, mesh_t *Mesh1, entity_t *Ent2, mesh_t *Mesh2, byte priority, stNode_t *stNode1, stNode_t *stNode2 );
static byte col_ee_CheckFaces( float *hit, entity_t *ent1, stNode_t *node1,  entity_t *ent2, mesh_t *mesh2, stNode_t *node2 );
static void col_ee_DoLowPriority( entity_t *ent1, vec3 vLoc1, entity_t *ent2, vec3 vLoc2, stNode_t *stNode2, float overlap, vec3 hit );
static void col_ee_DoMediumPriority( vec3 hit, vec3 vLoc1, entity_t *Ent2, stNode_t *stNode2, vec3 vLoc2, float overlap );
static byte col_ee_st_DoRoot( vec3 hit, entity_t *Ent1, mesh_t *Mesh1, stNode_t *stNode1, entity_t *Ent2, mesh_t *Mesh2, stNode_t *stNode2, byte priority );
static void col_TransformToGlobal( float *out, const float *v1, const float *pos, const float *rot );

// Local Variables
////////////////////
static vec3 vZero = {0.0f,0.0f,0.0f};

// *********** FUNCTIONALITY ***********
/* ------------
col_Respond - general purpose collision response between an ent and an infinitly massive Point
------------ */
void col_Respond( entity_t *ent, vec3 hit, vec3 vNorm )
{
	vec3 vL, vTmp;

	// R = 2(N dot L)N - L
	
	// get the hit position
	m3f_VecScale( vL, -1.0f, ent->vVel );

	// get R
	m3f_VecScale( vTmp, 2 * m3f_VecDot( vNorm, vL ), vNorm );
	m3f_VecSubtract( ent->vVel, vTmp, vL );
}

//col_CheckPlane

/* ------------
col_CheckEnts - returns > 0 and sets hit to the point of impact if the ents collided; return 0 in all other cases
			- performs object occlusion between the two ents, leaves response to the caller's discretion
------------ */
byte col_CheckEnts( float *hit, entity_t *ent1, entity_t *ent2, byte priority )
{
	uint32_t a, b;
	byte bHit = 0;
	stNode_t *stNode1, *stNode2;
	vec3 vLoc1, vLoc2;
	float overlap;

	// assert some (limited) preconditions
	if( priority == COL_PRIORITY_NONE ) return 0;  // allow stub functionality

	// slightly dangerous speed hack
	/*
	if( ent1 == NULL )
	if( ent1->mod == NULL )
	if( ent1->mod->moo == NULL )
	if( ent2 == NULL )
	if( ent2->mod == NULL )
	if( ent2->mod->moo == NULL )
	if( hit == NULL )
	*/

	// compare each mesh against each other mesh
	for( a = 0 ; a < ent1->mod->moo->num_meshes ; a++ )
	{
		stNode1 = ent1->mod->moo->meshes[a].frames[ent1->mod->moo->cur_frame].stRoot;
		col_TransformToGlobal( vLoc1, stNode1->loc, ent1->vPos, ent1->vRot );

		for( b = 0 ; b < ent2->mod->moo->num_meshes ; b++ )
		{
			stNode2 = ent2->mod->moo->meshes[b].frames[ent2->mod->moo->cur_frame].stRoot;
			col_TransformToGlobal( vLoc2, stNode2->loc, ent2->vPos, ent2->vRot );

			// Technique:  descend the sphere trees of ent1 and ent2 simultaniously looking for 2 or more leaf spheres that touch
			overlap = (m3f_VecDistance( vLoc1, vLoc2 ) - (stNode1->radius + stNode2->radius));
			if( overlap < 0 )
			{
				if( priority == COL_PRIORITY_LOW )
				{
					col_ee_DoLowPriority( ent1, vLoc1, ent2, vLoc2, stNode2, overlap, hit );
					bHit = 1;
				}
				else 
				{
					if( col_ee_CheckSphereTree( hit, ent1, &ent1->mod->moo->meshes[a], ent2, &ent2->mod->moo->meshes[b], priority, stNode1, stNode2 ) )
					{
						bHit = 1;
					}
				}

			}
		}
	}

	if( bHit > 0 ) return 1;
	return 0;
}

/* ------------
col_CheckSphereTree - finds the collision of spheres below node 1 and 2, returns NULL if it no collision
------------ */
static byte col_ee_CheckSphereTree( float *hit, entity_t *Ent1, mesh_t *Mesh1, entity_t *Ent2, mesh_t *Mesh2, byte priority, stNode_t *stNode1, stNode_t *stNode2 )
{
	int a, b;
	vec3 vLoc1, vLoc2;
	byte bHit = 0;

	// if we have descended both sphere trees all the way, test the final two leaves for a collision
	if( !stNode1->numChildren && !stNode2->numChildren )
	{
		if( col_ee_st_DoRoot( hit, Ent1, Mesh1, stNode1, Ent2, Mesh2, stNode2, priority ) > 0 ) return 1;
		return 0;
	}

	// the case may arise where we reach a leaf for one node before the other
	else if( !stNode1->numChildren && stNode2->numChildren )
	{
		col_TransformToGlobal( vLoc1, stNode1->loc, Ent1->vPos, Ent1->vRot );
		for( a = 0 ; a < stNode2->numChildren ; a++ )
		{
			col_TransformToGlobal( vLoc2, stNode2->children[a]->loc, Ent2->vPos, Ent2->vRot );
			if( m3f_VecDistance( vLoc1, vLoc2 ) < (stNode1->radius + stNode2->children[a]->radius) )
			{
				if( col_ee_CheckSphereTree( hit, Ent1, Mesh1, Ent2, Mesh2, priority, stNode1, stNode2->children[a] ) ) 
					bHit = 1;
			}
		}

		if( bHit > 0 ) return 1;
		return 0;
	}

	else if( stNode1->numChildren && !stNode2->numChildren )
	{
		col_TransformToGlobal( vLoc2, stNode2->loc, Ent2->vPos, Ent2->vRot );
		for( a = 0 ; a < stNode1->numChildren ; a++ )
		{
			col_TransformToGlobal( vLoc1, stNode1->children[a]->loc, Ent1->vPos, Ent1->vRot );
			if( m3f_VecDistance( vLoc1, vLoc2 ) <	(stNode1->children[a]->radius + stNode2->radius) )
			{
				if( col_ee_CheckSphereTree( hit, Ent1, Mesh1, Ent2, Mesh2, priority, stNode1->children[a], stNode2 ) ) 
					bHit = 1;
			}
		}

		if( bHit > 0 ) return 1;
		return 0;
	}
	
	else
	{
		// if both nodes have sub-spheres compute the collision against those
		for( a = 0 ; a < stNode1->numChildren ; a++ )
		{
			col_TransformToGlobal( vLoc1, stNode1->children[a]->loc, Ent1->vPos, Ent1->vRot );
			for( b = 0 ; b < stNode2->numChildren ; b++ )
			{
				col_TransformToGlobal( vLoc2, stNode2->children[b]->loc, Ent2->vPos, Ent2->vRot );

				// check each lower bounding sphere in 1 against each in 2 
				if( m3f_VecDistance(vLoc1, vLoc2) <	(stNode1->children[a]->radius + stNode2->children[b]->radius) )
				{
					if( col_ee_CheckSphereTree( hit, Ent1, Mesh1, Ent2, Mesh2, priority, stNode1->children[a], stNode2->children[b] ) ) 
						bHit = 1;
				}
			}
		}

		if( bHit > 0 ) return 1;
		return 0;
	}
}


/* ------------
col_CheckFaces - returns true if mesh1 protruded into mesh2 and was offset away
------------ */
static byte col_ee_CheckFaces( float *hit, entity_t *ent1, stNode_t *node1, 
									entity_t *ent2, mesh_t *mesh2, stNode_t *node2 )
{
	int a, b;
	int cnt, flag = 0;
	byte bHit = 0;
	float dist, minDist;
	vec3 vPt, vNorm, v0;

	for( a = 0 ; a < node1->numInPoints ; a++ )
	{
		col_TransformToGlobal( vPt, node1->inPoints[a], ent1->vPos, ent1->vRot );

		// is the above point behind all faces of the opposing geometry
		cnt = 0;
		minDist = 9999999.f;
		for( b = 0 ; b < node2->numInFaces ; b++ )
		{
			// dv = -( n * v0 )
			// dp = -( n * pt )

			col_TransformToGlobal( vNorm, mesh2->frames[ent2->mod->moo->cur_frame].tNorms[ node2->inFaces[b] ], vZero, ent2->vRot );
			col_TransformToGlobal( v0, mesh2->frames[ent2->mod->moo->cur_frame].verts[ mesh2->tris[ node2->inFaces[b] * 3 + 0 ] ], ent2->vPos, ent2->vRot );
		
			// transform the normal of v0 to world space
			if( (dist = m3f_VecDot( vNorm, v0 ) - m3f_VecDot( vNorm, vPt )) >= 0 )  // if this point is behind the plane 
			{
				if( dist < minDist )
				{
					minDist = dist;
					flag = b;
				}
				cnt++;
			}
		}

		// check to see if we're behind all planes
		if( cnt >= node2->numInFaces )
		{
			// we need to transform out of the object, but only on one plane
			// earlier we flagged the 'closest' plane as flag, use this face.
			// we also need to transform along the face normal to be sure we 
			// exit on the face and don't just end up behind another or too far out
			// this simplification will also result in occasional visual glitches as object appear
			// to collide from too far away, but a recusive solid hull test is a BAD IDEA
			float t;

			col_TransformToGlobal( vNorm, mesh2->frames[ent2->mod->moo->cur_frame].tNorms[ node2->inFaces[flag] ], vZero, ent2->vRot );
			col_TransformToGlobal( v0, mesh2->frames[ent2->mod->moo->cur_frame].verts[ mesh2->tris[ node2->inFaces[flag] * 3 + 0 ] ], ent2->vPos, ent2->vRot );

			t = m3f_VecDot( vNorm, v0 ) / m3f_VecDot( vNorm, vPt );

			ent1->vPos[X_AXIS] += vNorm[X_AXIS] * t;
			ent1->vPos[Y_AXIS] += vNorm[Y_AXIS] * t;
			ent1->vPos[Z_AXIS] += vNorm[Z_AXIS] * t;

			hit[X_AXIS] = vPt[X_AXIS] + vNorm[X_AXIS] * t;
			hit[Y_AXIS] = vPt[Y_AXIS] + vNorm[Y_AXIS] * t;
			hit[Z_AXIS] = vPt[Z_AXIS] + vNorm[Z_AXIS] * t;

			bHit = 1;
		}
	}

	if( bHit > 0 ) return 1;
	return 0;
}

/* ------------
col_ee_DoLowPriority - if a low priority hit was found, this is called to do 1st order collision and find the hit location
------------ */
static void col_ee_DoLowPriority( entity_t *ent1, vec3 vLoc1, entity_t *ent2, vec3 vLoc2, stNode_t *stNode2, float overlap, vec3 hit )
{
	vec3 vTo1, vTmp; // a tmp vector

	// get the direction to sphere 1
	m3f_VecSubtract( vTo1, vLoc1, vLoc2 );
	m3f_VecUnitize( vTo1 );

	// scale the direction by the overlap to get the offset for ent2
	m3f_VecScale( vTmp, overlap, vTo1 );  // remember that overlap is negative
	m3f_VecAdd( ent2->vPos, ent2->vPos, vTmp );
	m3f_VecAdd( vLoc2, vLoc2, vTmp );

	// scale the direction to get to the collision point
	m3f_VecScale( vTmp, stNode2->radius, vTo1 );
	m3f_VecAdd( hit, vLoc2, vTmp ); 
}

/* ------------
col_ee_DoMediumPriority - if a medium priority hit was found, this is called to do 1st order collision and find the hit location
------------ */
static void col_ee_DoMediumPriority( vec3 hit, vec3 vLoc1, entity_t *Ent2, stNode_t *stNode2, vec3 vLoc2, float overlap )
{
	vec3 vTo1;
	vec3 vTmp;
	
	// get the direction to sphere 1 from sphere 2
	m3f_VecSubtract( vTo1, vLoc1, vLoc2 );
	m3f_VecUnitize( vTo1 );
	
	// offset entity 2 to be outside of sphere 1
	m3f_VecScale( vTmp, (overlap * 2), vTo1 );
	m3f_VecAdd( Ent2->vPos, Ent2->vPos, vTmp );
	m3f_VecAdd( vLoc2, vLoc2, vTmp );
	
	// get the hit from the direction
	m3f_VecScale( vTmp, stNode2->radius, vTo1 );
	m3f_VecAdd( hit, vLoc2, vTmp );
}

/* ------------
col_ee_st_DoRoot - called in CheckSphereTree when the root is reached, returns 1 on collision, 2 on no collision, handles low order collision
------------ */
static byte col_ee_st_DoRoot( vec3 hit, entity_t *Ent1, mesh_t *Mesh1, stNode_t *stNode1, entity_t *Ent2, mesh_t *Mesh2, stNode_t *stNode2, byte priority )
{
	float overlap;
	vec3 vLoc1, vLoc2;
	byte bHit = 0;
	
	col_TransformToGlobal( vLoc1, stNode1->loc, Ent1->vPos, Ent1->vRot );
	col_TransformToGlobal( vLoc2, stNode2->loc, Ent2->vPos, Ent2->vRot );
	
	overlap = m3f_VecDistance( vLoc1, vLoc2 ) - (stNode1->radius + stNode2->radius);
	if( overlap > 0 ) return 0;
	else
	{
		// at medium priority, we are done at the leaf spheres
		if( priority == COL_PRIORITY_MEDIUM )
		{
			col_ee_DoMediumPriority( hit, vLoc1, Ent2, stNode2, vLoc2, overlap );
			return 1;
		}
	
		// otherwise attempt to find an exact collision
		if( col_ee_CheckFaces( hit, Ent1, stNode1,	Ent2, Mesh2, stNode2 ) )
		{
			bHit = 1;
		}
	
		if( bHit > 0 ) return 1;
		return 0;
	}
}

/* ------------
col_TransformToGlobal - transforms v1 with pos and rot and stores it to out
					  - all vectors except for out should be initialized
------------ */
static void col_TransformToGlobal( float *out, const float *v1, const float *pos, const float *rot )
{
	// Mn = MryMrpMt
	// ...I love my TI-89

	float sinx = SINd(rot[X_AXIS]);
	float siny = SINd(rot[Y_AXIS]);
	float cosx = COSd(rot[X_AXIS]);
	float cosy = COSd(rot[Y_AXIS]);

	out[X_AXIS] = v1[X_AXIS]*cosy + v1[Z_AXIS]*siny + pos[X_AXIS];
	out[Y_AXIS] = v1[Y_AXIS]*cosx + sinx * (v1[X_AXIS]*siny - v1[Z_AXIS]*cosy) + pos[Y_AXIS];
	out[Z_AXIS] = cosx * (v1[Z_AXIS]*cosy - v1[X_AXIS]*siny) + v1[Y_AXIS]*sinx + pos[Z_AXIS];
}
