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
// game_world.c
// Created 1-31-03 by Terrence Cole

// Includes
/////////////
#include "cog_global.h"

// Definitions
////////////////


// Global Structs
////////////////
/*
*/

// Global Prototypes
//////////////////////
/*
int wrld_Initialize(void);
void wrld_Terminate(void);

world_t *wrld_Create( int *size, int dimension, int numMines );
// allocates an array of data for the world board.	puts it in state and marks all as changed

void wrld_Delete( world_t *world );
// there exists only one world in the state

int wrld_ClearSquare( world_t *world, int x, int y, int z );
// clears one square and returns a value reflecting the cleared square

byte wrld_RayIntersect( world_t *world, vec3 vStart, vec3 vDir, int *vCube );
// traces a ray from a spot into the world to the first non-hiddne square; returns in vCube the non-hidden square
*/

// Local Prototypes
//////////////////
static world_t *wrld_Create_Shared( int *size, int numMines );
static world_t *wrld_CreateN6( int *size, int numMines );
static world_t *wrld_CreateN18( int *size, int numMines );
static world_t *wrld_CreateN26( int *size, int numMines );

static void wrld_DeleteNX( world_t *world );

static void wrld_LinkTail( world_t *wrld );
static void wrld_Unlink( world_t *wrld );

// Local Variables
////////////////
static world_t *wrldHead = NULL;
static world_t *wrldTail = NULL;

// *********** FUNCTIONALITY ***********
/* ------------
wrld_Initialize - grab a linked list to track all created worlds
------------ */
int wrld_Initialize(void)
{
	return 1;
}

/* ------------
wrld_Terminate - get rid of all worlds and free the list
------------ */
void wrld_Terminate(void)
{
	world_t *tmpWrld;

	while( wrldHead != NULL )
	{
		tmpWrld = wrldHead->next;
		wrld_Delete( wrldHead );
		wrldHead = tmpWrld;
	}

	wrldHead = wrldTail = NULL;
}

/* ------------
wrld_Create
// dimension == neighborhood
------------ */
world_t *wrld_Create( int *size, int dimension, int numMines )
{
	world_t *newWorld = NULL;
	
	// assert
	if( size == NULL ) return NULL;

	con_Print( "<BLUE>Creating</BLUE> the Game World." );
	eng_LoadingFrame();
	
	// based on neighborhood	
	switch( dimension )
	{
	case 6:
		newWorld = wrld_CreateN6( size, numMines );
		break;
	case 18:
		newWorld = wrld_CreateN18( size, numMines );
		break;
	case 26:
		newWorld = wrld_CreateN26( size, numMines );
		break;
	default:
		con_Print( "<RED>World Create Failed:  The indicated dimension is not supported." );
		return NULL;
	}

	if( newWorld == NULL )
	{
		con_Print( "<RED>Create World 3D Failed.  Cannot create world." );
		return NULL;
	}
	wrld_LinkTail( newWorld );

	con_Print( "<BLUE>Success</BLUE> World Creation is Complete." );
	eng_LoadingFrame();
	
	return newWorld;
}

/* ------------
wrld_Delete
------------ */
void wrld_Delete( world_t *wrld )
{
	world_t *tmp;
	
	// assert
	if( wrld == NULL ) return;

	tmp = wrldHead;
	while( tmp != NULL )
	{
		if( tmp == wrld )
		{
			// delete all(?) references to world
			wrld_Unlink( wrld );
			wrld_DeleteNX( wrld ); 
			return;
		}
		tmp = tmp->next;
	}

	// not found on list
	return;
}

/* ------------
wrld_CreateN6
------------ */
static world_t *wrld_Create_Shared( int *size, int numMines )
{
	int a, b, c, d;
	byte bDone;
	world_t *tWrld;
	float loX, loY, loZ;
	float hiX, hiY, hiZ;
	unsigned long int ulCurOff, ulMaxOff, ulOffset;

	// tell us about it
	con_Print( "<BLUE>Entering</BLUE> Shared World Creation Algorithm." );
	eng_LoadingFrame();

	// assert
	if( size == NULL )
	{
		con_Print( "<RED>World Create 3D Failed:  Size pointer is NULL." );
		return 0;
	}
	for( a = 0 ; a < 3 ; a++ )
	{
		if( size[a] < 2 )
		{
			con_Print( "<RED>World Create 3D Failed:  Size in axis %d is less than 2", a );
			return 0;
		}
	}
	if( (size[0] * size[1] * size[2]) < numMines )
	{
		con_Print( "<RED>World Create 3D Failed:  More mines requested than will fit in the game world." );
		con_Print( "<RED>\tDimensions: [ %d, %d, %d ], Size:  %d,  Mines:  %d", size[0], size[1], size[2], size[0] * size[1] * size[2], numMines );
		return 0;
	}

	con_Print( "<BLUE>Allocating</BLUE> the Game World." );
	eng_LoadingFrame();

	// allocate a world
	tWrld = mem_alloc( sizeof(world_t) );
	if( tWrld == NULL )
	{
		con_Print( "<RED>World Create 3D Failed:  Unable to allocate memory for the world." ); 
		return 0;
	}
	memset( tWrld, 0, sizeof(world_t) );
	
	// allocate x pointers
	tWrld->cubes = mem_alloc( sizeof(wrld_cube_t**) * size[0] );
	if( tWrld->cubes == NULL )
	{
		con_Print( "<RED>World Create 3D Failed:  Memory allocation failed for X axis." );
		return 0;
	}
	memset( tWrld->cubes, 0, sizeof(wrld_cube_t**) * size[0] );

	// allocate y pointers
	for( a = 0 ; a < size[0] ; a++ )
	{
		tWrld->cubes[a] = mem_alloc( sizeof(wrld_cube_t*) * size[1] );
		if( tWrld->cubes[a] == NULL )
		{
			con_Print( "<RED>World Create 3D Failed:  Memory allocate failed for Y axis at %d.", a );
			for( a = 0 ; a < size[0] ; a++ ) SAFE_RELEASE( tWrld->cubes[a] );
			SAFE_RELEASE( tWrld->cubes );
			SAFE_RELEASE( tWrld );
			return 0;
		}
		memset( tWrld->cubes[a], 0, sizeof(wrld_cube_t*) * size[1] );
	}

	// allocate z axis
	for( a = 0 ; a < size[0] ; a++ )
	{
		for( b = 0 ; b < size[1] ; b++ )
		{
			tWrld->cubes[a][b] = mem_alloc( sizeof(wrld_cube_t) * size[2] );
			if( tWrld->cubes[a][b] == NULL )
			{
				con_Print( "<RED>World Create 3D Failed:  Unable to get memory for world cubes." );
				for( a = 0 ; a < size[0] ; a++ )
				{
					for( b = 0 ; b < size[1] ; b++ )
					{
						SAFE_RELEASE( tWrld->cubes[a][b] );
					}
				}
				for( a = 0 ; a < size[0] ; a++ ) SAFE_RELEASE( tWrld->cubes[a] );
				SAFE_RELEASE( tWrld->cubes );
				SAFE_RELEASE( tWrld );
				return 0;
			}
			memset( tWrld->cubes[a][b], 0, sizeof(wrld_cube_t) * size[2] );
		}
	}

	con_Print( "<BLUE>Loading</BLUE> Additional World Media." );
	eng_LoadingFrame();
	
	// set other world states
	tWrld->size[0] = size[0];
	tWrld->size[1] = size[1];
	tWrld->size[2] = size[2];
	tWrld->cubeSize = WORLD_CUBE_SIZE;
	tWrld->wrldBnds.hibounds[X_AXIS] = (tWrld->cubeSize * size[0]) / 2.0f;
	tWrld->wrldBnds.lobounds[X_AXIS] = -(tWrld->wrldBnds.hibounds[X_AXIS]);
	tWrld->wrldBnds.hibounds[Y_AXIS] = (tWrld->cubeSize * size[1]) / 2.0f;
	tWrld->wrldBnds.lobounds[Y_AXIS] = -(tWrld->wrldBnds.hibounds[Y_AXIS]);
	tWrld->wrldBnds.hibounds[Z_AXIS] = (tWrld->cubeSize * size[2]) / 2.0f;
	tWrld->wrldBnds.lobounds[Z_AXIS] = -(tWrld->wrldBnds.hibounds[Z_AXIS]);
	tWrld->numCubes = size[0] * size[1] * size[2];
	tWrld->numMines = numMines;
	tWrld->shFace = sh_LoadShader( "cube_face.sh" );
	tWrld->shFlag = sh_LoadShader( "cube_flag.sh" );
	tWrld->shEnd = sh_LoadShader( "world_border.sh" );

	// assert for our shaders
	if( (tWrld->shFace == NULL) || (tWrld->shEnd == NULL ) || (tWrld->shFlag == NULL) )
	{
		con_Print( "<RED>World Load Failed:  Could not load a required shader." );
		for( a = 0 ; a < size[0] ; a++ )
		{
			for( b = 0 ; b < size[1] ; b++ )
			{
				SAFE_RELEASE( tWrld->cubes[a][b] );
			}
		}
		for( a = 0 ; a < size[0] ; a++ ) SAFE_RELEASE( tWrld->cubes[a] );
		SAFE_RELEASE( tWrld->cubes );
		SAFE_RELEASE( tWrld );
		return NULL;
	}

	// color is pulled from the var system
	tWrld->world_color_r = var_GetVarLatch( "world_color_r" );
	tWrld->world_color_g = var_GetVarLatch( "world_color_g" );
	tWrld->world_color_b = var_GetVarLatch( "world_color_b" );
	if( (tWrld->world_color_r == NULL) || (tWrld->world_color_g == NULL) || (tWrld->world_color_b == NULL) )
	{
		con_Print( "<RED>World Load Failed:  Could not latch a required variable." );
		for( a = 0 ; a < size[0] ; a++ )
		{
			for( b = 0 ; b < size[1] ; b++ )
			{
				SAFE_RELEASE( tWrld->cubes[a][b] );
			}
		}
		for( a = 0 ; a < size[0] ; a++ ) SAFE_RELEASE( tWrld->cubes[a] );
		SAFE_RELEASE( tWrld->cubes );
		SAFE_RELEASE( tWrld );
		return NULL;
	}

	con_Print( "<BLUE>Precomputing</BLUE> World Geometry." );
	eng_LoadingFrame();
	
	// get planes
	memcpy( tWrld->planes[0].V, tWrld->wrldBnds.hibounds, sizeof(vec3) );
	memcpy( tWrld->planes[1].V, tWrld->wrldBnds.hibounds, sizeof(vec3) );
	memcpy( tWrld->planes[2].V, tWrld->wrldBnds.hibounds, sizeof(vec3) );
	memcpy( tWrld->planes[3].V, tWrld->wrldBnds.lobounds, sizeof(vec3) );
	memcpy( tWrld->planes[4].V, tWrld->wrldBnds.lobounds, sizeof(vec3) );
	memcpy( tWrld->planes[5].V, tWrld->wrldBnds.lobounds, sizeof(vec3) );
	tWrld->planes[0].norm[0] = 1.0f;
	tWrld->planes[0].norm[1] = 0.0f;
	tWrld->planes[0].norm[2] = 0.0f;
	tWrld->planes[1].norm[0] = 0.0f;
	tWrld->planes[1].norm[1] = 1.0f;
	tWrld->planes[1].norm[2] = 0.0f;
	tWrld->planes[2].norm[0] = 0.0f;
	tWrld->planes[2].norm[1] = 0.0f;
	tWrld->planes[2].norm[2] = 1.0f;
	tWrld->planes[3].norm[0] = -1.0f;
	tWrld->planes[3].norm[1] = 0.0f;
	tWrld->planes[3].norm[2] = 0.0f;
	tWrld->planes[4].norm[0] = 0.0f;
	tWrld->planes[4].norm[1] = -1.0f;
	tWrld->planes[4].norm[2] = 0.0f;
	tWrld->planes[5].norm[0] = 0.0f;
	tWrld->planes[5].norm[1] = 0.0f;
	tWrld->planes[5].norm[2] = -1.0f;
	tWrld->planes[0].D = -m3f_VecDot( tWrld->planes[0].V, tWrld->planes[0].norm );
	tWrld->planes[1].D = -m3f_VecDot( tWrld->planes[1].V, tWrld->planes[1].norm );
	tWrld->planes[2].D = -m3f_VecDot( tWrld->planes[2].V, tWrld->planes[2].norm );
	tWrld->planes[3].D = -m3f_VecDot( tWrld->planes[3].V, tWrld->planes[3].norm );
	tWrld->planes[4].D = -m3f_VecDot( tWrld->planes[4].V, tWrld->planes[4].norm );
	tWrld->planes[5].D = -m3f_VecDot( tWrld->planes[5].V, tWrld->planes[5].norm );

	// construct face quads
	loX = tWrld->wrldBnds.lobounds[X_AXIS];
	loY = tWrld->wrldBnds.lobounds[Y_AXIS];
	loZ = tWrld->wrldBnds.lobounds[Z_AXIS];
	hiX = tWrld->wrldBnds.hibounds[X_AXIS];
	hiY = tWrld->wrldBnds.hibounds[Y_AXIS];
	hiZ = tWrld->wrldBnds.hibounds[Z_AXIS];
	
	// 6 5 1 2
	tWrld->quads[0][0][X_AXIS] = hiX + 1.0f;
	tWrld->quads[0][0][Y_AXIS] = hiY + 1.0f;
	tWrld->quads[0][0][Z_AXIS] = hiZ + 1.0f;
	tWrld->quads[0][1][X_AXIS] = hiX + 1.0f;
	tWrld->quads[0][1][Y_AXIS] = loY - 1.0f;
	tWrld->quads[0][1][Z_AXIS] = hiZ + 1.0f;
	tWrld->quads[0][2][X_AXIS] = hiX + 1.0f;
	tWrld->quads[0][2][Y_AXIS] = loY - 1.0f;
	tWrld->quads[0][2][Z_AXIS] = loZ - 1.0f;
	tWrld->quads[0][3][X_AXIS] = hiX + 1.0f;
	tWrld->quads[0][3][Y_AXIS] = hiY + 1.0f;
	tWrld->quads[0][3][Z_AXIS] = loZ - 1.0f;
	// 3 7 6 2 
	tWrld->quads[1][0][X_AXIS] = loX - 1.0f;
	tWrld->quads[1][0][Y_AXIS] = hiY + 1.0f;
	tWrld->quads[1][0][Z_AXIS] = loZ - 1.0f;
	tWrld->quads[1][1][X_AXIS] = loX - 1.0f;
	tWrld->quads[1][1][Y_AXIS] = hiY + 1.0f;
	tWrld->quads[1][1][Z_AXIS] = hiZ + 1.0f;
	tWrld->quads[1][2][X_AXIS] = hiX + 1.0f;
	tWrld->quads[1][2][Y_AXIS] = hiY + 1.0f;
	tWrld->quads[1][2][Z_AXIS] = hiZ + 1.0f;
	tWrld->quads[1][3][X_AXIS] = hiX + 1.0f;
	tWrld->quads[1][3][Y_AXIS] = hiY + 1.0f;
	tWrld->quads[1][3][Z_AXIS] = loZ - 1.0f;
	// 7 4 5 6 
	tWrld->quads[2][0][X_AXIS] = loX - 1.0f;
	tWrld->quads[2][0][Y_AXIS] = hiY + 1.0f;
	tWrld->quads[2][0][Z_AXIS] = hiZ + 1.0f;
	tWrld->quads[2][1][X_AXIS] = loX - 1.0f;
	tWrld->quads[2][1][Y_AXIS] = loY - 1.0f;
	tWrld->quads[2][1][Z_AXIS] = hiZ + 1.0f;
	tWrld->quads[2][2][X_AXIS] = hiX + 1.0f;
	tWrld->quads[2][2][Y_AXIS] = loY - 1.0f;
	tWrld->quads[2][2][Z_AXIS] = hiZ + 1.0f;
	tWrld->quads[2][3][X_AXIS] = hiX + 1.0f;
	tWrld->quads[2][3][Y_AXIS] = hiY + 1.0f;
	tWrld->quads[2][3][Z_AXIS] = hiZ + 1.0f;
	// 3 0 4 7 
	tWrld->quads[3][0][X_AXIS] = loX - 1.0f;
	tWrld->quads[3][0][Y_AXIS] = hiY + 1.0f;
	tWrld->quads[3][0][Z_AXIS] = loZ - 1.0f;
	tWrld->quads[3][1][X_AXIS] = loX - 1.0f;
	tWrld->quads[3][1][Y_AXIS] = loY - 1.0f;
	tWrld->quads[3][1][Z_AXIS] = loZ - 1.0f;
	tWrld->quads[3][2][X_AXIS] = loX - 1.0f;
	tWrld->quads[3][2][Y_AXIS] = loY - 1.0f;
	tWrld->quads[3][2][Z_AXIS] = hiZ + 1.0f;
	tWrld->quads[3][3][X_AXIS] = loX - 1.0f;
	tWrld->quads[3][3][Y_AXIS] = hiY + 1.0f;
	tWrld->quads[3][3][Z_AXIS] = hiZ + 1.0f;
	// 1 5 4 0
	tWrld->quads[4][0][X_AXIS] = hiX + 1.0f;
	tWrld->quads[4][0][Y_AXIS] = loY - 1.0f;
	tWrld->quads[4][0][Z_AXIS] = loZ - 1.0f;
	tWrld->quads[4][1][X_AXIS] = hiX + 1.0f;
	tWrld->quads[4][1][Y_AXIS] = loY - 1.0f;
	tWrld->quads[4][1][Z_AXIS] = hiZ + 1.0f;
	tWrld->quads[4][2][X_AXIS] = loX - 1.0f;
	tWrld->quads[4][2][Y_AXIS] = loY - 1.0f;
	tWrld->quads[4][2][Z_AXIS] = hiZ + 1.0f;
	tWrld->quads[4][3][X_AXIS] = loX - 1.0f;
	tWrld->quads[4][3][Y_AXIS] = loY - 1.0f;
	tWrld->quads[4][3][Z_AXIS] = loZ - 1.0f;
	// 2 1 0 3
	tWrld->quads[5][0][X_AXIS] = hiX + 1.0f;
	tWrld->quads[5][0][Y_AXIS] = hiY + 1.0f;
	tWrld->quads[5][0][Z_AXIS] = loZ - 1.0f;
	tWrld->quads[5][1][X_AXIS] = hiX + 1.0f;
	tWrld->quads[5][1][Y_AXIS] = loY - 1.0f;
	tWrld->quads[5][1][Z_AXIS] = loZ - 1.0f;
	tWrld->quads[5][2][X_AXIS] = loX - 1.0f;
	tWrld->quads[5][2][Y_AXIS] = loY - 1.0f;
	tWrld->quads[5][2][Z_AXIS] = loZ - 1.0f;	
	tWrld->quads[5][3][X_AXIS] = loX - 1.0f;
	tWrld->quads[5][3][Y_AXIS] = hiY + 1.0f;
	tWrld->quads[5][3][Z_AXIS] = loZ - 1.0f;

	// set default parameters
	for( a = 0 ; a < size[0] ; a++ )
	{
		for( b = 0 ; b < size[1] ; b++ )
		{
			for( c = 0 ; c < size[2] ; c++ )
			{
				tWrld->cubes[a][b][c].bHidden = 1;
				tWrld->cubes[a][b][c].bFlagged = 0;
				tWrld->cubes[a][b][c].numAround = 0;
			}
		}
	}

	con_Print( "<BLUE>Filling</BLUE> the World Cubes with Creamy Nuggat." );
	eng_LoadingFrame();
	
	// assign vertex positions
	for( a = 0 ; a < size[0] ; a++ )
	{
		loX = tWrld->wrldBnds.lobounds[X_AXIS] + (a * WORLD_CUBE_SIZE);
		hiX = tWrld->wrldBnds.lobounds[X_AXIS] + ((a + 1) * WORLD_CUBE_SIZE);

		for( b = 0 ; b < size[1] ; b++ )
		{
			loY = tWrld->wrldBnds.lobounds[Y_AXIS] + (b * WORLD_CUBE_SIZE);
			hiY = tWrld->wrldBnds.lobounds[Y_AXIS] + ((b + 1) * WORLD_CUBE_SIZE);
		
			for( c = 0 ; c < size[2] ; c++ )
			{
				loZ = tWrld->wrldBnds.lobounds[Z_AXIS] + (c * WORLD_CUBE_SIZE);
				hiZ = tWrld->wrldBnds.lobounds[Z_AXIS] + ((c + 1) * WORLD_CUBE_SIZE);

				// v0				
				tWrld->cubes[a][b][c].verts[0][X_AXIS] = loX;
				tWrld->cubes[a][b][c].verts[0][Y_AXIS] = loY;
				tWrld->cubes[a][b][c].verts[0][Z_AXIS] = loZ;

				// v1
				tWrld->cubes[a][b][c].verts[1][X_AXIS] = hiX;
				tWrld->cubes[a][b][c].verts[1][Y_AXIS] = loY;
				tWrld->cubes[a][b][c].verts[1][Z_AXIS] = loZ;

				// v2
				tWrld->cubes[a][b][c].verts[2][X_AXIS] = hiX;
				tWrld->cubes[a][b][c].verts[2][Y_AXIS] = hiY;
				tWrld->cubes[a][b][c].verts[2][Z_AXIS] = loZ;

				// v3
				tWrld->cubes[a][b][c].verts[3][X_AXIS] = loX;
				tWrld->cubes[a][b][c].verts[3][Y_AXIS] = hiY;
				tWrld->cubes[a][b][c].verts[3][Z_AXIS] = loZ;

				// v4
				tWrld->cubes[a][b][c].verts[4][X_AXIS] = loX;
				tWrld->cubes[a][b][c].verts[4][Y_AXIS] = loY;
				tWrld->cubes[a][b][c].verts[4][Z_AXIS] = hiZ;

				// v5
				tWrld->cubes[a][b][c].verts[5][X_AXIS] = hiX;
				tWrld->cubes[a][b][c].verts[5][Y_AXIS] = loY;
				tWrld->cubes[a][b][c].verts[5][Z_AXIS] = hiZ;

				// v6
				tWrld->cubes[a][b][c].verts[6][X_AXIS] = hiX;
				tWrld->cubes[a][b][c].verts[6][Y_AXIS] = hiY;
				tWrld->cubes[a][b][c].verts[6][Z_AXIS] = hiZ;

				// v7
				tWrld->cubes[a][b][c].verts[7][X_AXIS] = loX;
				tWrld->cubes[a][b][c].verts[7][Y_AXIS] = hiY;
				tWrld->cubes[a][b][c].verts[7][Z_AXIS] = hiZ;
			}
		}
	}

	con_Print( "<BLUE>Assigning</BLUE> Mines Randomly to Creamy Nuggat Filling." );
	eng_LoadingFrame();
	
	// assign mines
	for( d = 0 ; d < numMines ; d++ )
	{
		bDone = 0;
		ulCurOff = 0;
		ulMaxOff = tWrld->numCubes - d;
		ulOffset = rng_Long( 0, ulMaxOff );

		a = 0;
		while( !bDone && (a < tWrld->size[0]) )
		{
			b = 0;
			while( !bDone && (b < tWrld->size[1]) )
			{
				c = 0;
				while( !bDone && (c < tWrld->size[2]) )
				{
					// advance only on non-mine squares
					if( tWrld->cubes[a][b][c].numAround != WRLD_CUBE_MINE )
					{
						ulCurOff++;
					}
					
					// if this is the offset to which we are seeking, set the mine and start over
					if( ulCurOff == ulOffset )
					{
						tWrld->cubes[a][b][c].numAround = WRLD_CUBE_MINE;
						bDone = 1;
					}
					
					// increment in Z
					c++;
				}
				
				// increment in Y
				b++;
			}

			// increment on X
			a++;
		}
	}

	con_Print( "<BLUE>Finished</BLUE> Shared Init." );
	eng_LoadingFrame();
	
	return tWrld;
}

/* ------------
wrld_CreateN6
------------ */
static world_t *wrld_CreateN6( int *size, int numMines )
{
	world_t *tWrld;
	int a, b, c;

	// do shared init
	tWrld = wrld_Create_Shared( size, numMines );
	if( !tWrld )
	{
		con_Print( "<RED>World Shared Create Failed." );
		return NULL;
	}

	con_Print( "<BLUE>Counting</BLUE> Mines." );
	eng_LoadingFrame();
	
	// compute the number of surrounding mines
	for( a = 0 ; a < size[0] ; a++ )
	{
		for( b = 0 ; b < size[1] ; b++ )
		{
			for( c = 0 ; c < size[2] ; c++ )
			{
				// if this is a mine square, add one to all of the non-mine squares around it
				if( tWrld->cubes[a][b][c].numAround == WRLD_CUBE_MINE )
				{
					// the low ends
					if( (a > 0) && (tWrld->cubes[a - 1][b][c].numAround != WRLD_CUBE_MINE) ) tWrld->cubes[a - 1][b][c].numAround++;
					if( (b > 0) && (tWrld->cubes[a][b - 1][c].numAround != WRLD_CUBE_MINE) ) tWrld->cubes[a][b - 1][c].numAround++;
					if( (c > 0) && (tWrld->cubes[a][b][c - 1].numAround != WRLD_CUBE_MINE) ) tWrld->cubes[a][b][c - 1].numAround++;

					// the high ends
					if( ((a + 1) < size[0]) && (tWrld->cubes[a + 1][b][c].numAround != WRLD_CUBE_MINE) ) tWrld->cubes[a + 1][b][c].numAround++;
					if( ((b + 1) < size[1]) && (tWrld->cubes[a][b + 1][c].numAround != WRLD_CUBE_MINE) ) tWrld->cubes[a][b + 1][c].numAround++;
					if( ((c + 1) < size[2]) && (tWrld->cubes[a][b][c + 1].numAround != WRLD_CUBE_MINE) ) tWrld->cubes[a][b][c + 1].numAround++;
				}
			}
		}
	}

	// done successfully
	return tWrld;
}

/* ------------
wrld_CreateN18
------------ */
static world_t *wrld_CreateN18( int *size, int numMines )
{
	world_t *tWrld;
	int a, b, c, d;
	int i = 0, j = 0, k = 0;

	// do shared init
	tWrld = wrld_Create_Shared( size, numMines );
	if( !tWrld )
	{
		con_Print( "<RED>World Shared Create Failed." );
		return NULL;
	}

	con_Print( "<BLUE>Counting</BLUE> Mines." );
	eng_LoadingFrame();
	
	// compute the number of surrounding mines
	for( a = 0 ; a < size[0] ; a++ )
	{
		for( b = 0 ; b < size[1] ; b++ )
		{
			for( c = 0 ; c < size[2] ; c++ )
			{
				// if this is a mine square, add one to all of the non-mine squares around it
				if( tWrld->cubes[a][b][c].numAround == WRLD_CUBE_MINE )
				{
					// split up the search
					for( d = 0 ; d < 18 ; d++ )
					{
						switch( d )
						{
						// FACES
						case 0:
							i = a - 1;	j = b; 		k = c;
							break;
						case 1:
							i = a; 		j = b - 1;	k = c;
							break;
						case 2:
							i = a; 		j = b; 		k = c - 1;
							break;
						case 3:
							i = a + 1; 	j = b; 		k = c;
							break;
						case 4:
							i = a; 		j = b + 1;	k = c;
							break;
						case 5:
							i = a; 		j = b; 		k = c + 1;
							break;
						// EDGES
						// X/Z plane
						case 6:
							i = a - 1;	j = b; 		k = c - 1;
							break;
						case 7:
							i = a + 1;	j = b; 		k = c - 1;
							break;
						case 8:
							i = a - 1;	j = b; 		k = c + 1;
							break;
						case 9:
							i = a + 1; 	j = b; 		k = c + 1;
							break;
						// X/Y plane
						case 10:
							i = a - 1;	j = b - 1;	k = c;
							break;
						case 11:
							i = a + 1;	j = b - 1;	k = c;
							break;
						case 12:
							i = a - 1;	j = b + 1;	k = c;
							break;
						case 13:
							i = a + 1;	j = b + 1;	k = c;
							break;
						// Y/Z plane
						case 14:
							i = a; 		j = b - 1;	k = c - 1;
							break;
						case 15:
							i = a; 		j = b + 1;	k = c - 1;
							break;
						case 16:
							i = a; 		j = b - 1;	k = c + 1;
							break;
						case 17:
							i = a; 		j = b + 1;	k = c + 1;
							break;
						default:
							break;
						}

						// unified assertion
						if( i >= 0 && j >= 0 && k >= 0 && 
							i < tWrld->size[0] && j < tWrld->size[1] && k < tWrld->size[2] &&
							tWrld->cubes[i][j][k].numAround != WRLD_CUBE_MINE )
						{
							tWrld->cubes[i][j][k].numAround++;
						}	
						
					}
				}
			}
		}
	}

	// done successfully
	return tWrld;
}

/* ------------
wrld_CreateN26
------------ */
static world_t *wrld_CreateN26( int *size, int numMines )
{
	world_t *tWrld;
	int a, b, c, d;
	int i = 0, j = 0, k = 0;

	// do shared init
	tWrld = wrld_Create_Shared( size, numMines );
	if( !tWrld )
	{
		con_Print( "<RED>World Shared Create Failed." );
		return NULL;
	}

	con_Print( "<BLUE>Counting</BLUE> Mines." );
	eng_LoadingFrame();
	
	// compute the number of surrounding mines
	for( a = 0 ; a < size[0] ; a++ )
	{
		for( b = 0 ; b < size[1] ; b++ )
		{
			for( c = 0 ; c < size[2] ; c++ )
			{
				// if this is a mine square, add one to all of the non-mine squares around it
				if( tWrld->cubes[a][b][c].numAround == WRLD_CUBE_MINE )
				{
					// split up the search
					for( d = 0 ; d < 26 ; d++ )
					{
						switch( d )
						{
						// FACES
						case 0:
							i = a - 1;	j = b; 		k = c;
							break;
						case 1:
							i = a; 		j = b - 1;	k = c;
							break;
						case 2:
							i = a; 		j = b; 		k = c - 1;
							break;
						case 3:
							i = a + 1; 	j = b; 		k = c;
							break;
						case 4:
							i = a; 		j = b + 1;	k = c;
							break;
						case 5:
							i = a; 		j = b; 		k = c + 1;
							break;
						// EDGES
						// X/Z plane
						case 6:
							i = a - 1;	j = b; 		k = c - 1;
							break;
						case 7:
							i = a + 1;	j = b; 		k = c - 1;
							break;
						case 8:
							i = a - 1;	j = b; 		k = c + 1;
							break;
						case 9:
							i = a + 1; 	j = b; 		k = c + 1;
							break;
						// X/Y plane
						case 10:
							i = a - 1;	j = b - 1;	k = c;
							break;
						case 11:
							i = a + 1;	j = b - 1;	k = c;
							break;
						case 12:
							i = a - 1;	j = b + 1;	k = c;
							break;
						case 13:
							i = a + 1;	j = b + 1;	k = c;
							break;
						// Y/Z plane
						case 14:
							i = a; 		j = b - 1;	k = c - 1;
							break;
						case 15:
							i = a; 		j = b + 1;	k = c - 1;
							break;
						case 16:
							i = a; 		j = b - 1;	k = c + 1;
							break;
						case 17:
							i = a; 		j = b + 1;	k = c + 1;
							break;
						// CORNERS
						// top
						case 18:
							i = a - 1;	j = b + 1;	k = c - 1;
							break;
						case 19:
							i = a + 1;	j = b + 1;	k = c - 1;
							break;
						case 20:
							i = a - 1;	j = b + 1;	k = c + 1;
							break;
						case 21:
							i = a + 1; 	j = b + 1;	k = c + 1;
							break;
						// bottom
						case 22:
							i = a - 1;	j = b - 1;	k = c - 1;
							break;
						case 23:
							i = a + 1;	j = b - 1;	k = c - 1;
							break;
						case 24:
							i = a - 1;	j = b - 1;	k = c + 1;
							break;
						case 25:
							i = a + 1;	j = b - 1;	k = c + 1;
							break;
						default:
							break;
						}

						// unified assertion
						if( i >= 0 && j >= 0 && k >= 0 && 
							i < tWrld->size[0] && j < tWrld->size[1] && k < tWrld->size[2] &&
							tWrld->cubes[i][j][k].numAround != WRLD_CUBE_MINE )
						{
							tWrld->cubes[i][j][k].numAround++;
						}	
						
					}
				}
			}
		}
	}

	// done successfully
	return tWrld;
}

/* ------------
wrld_Delete3D
------------ */
static void wrld_DeleteNX( world_t *world )
{
	int a, b, c;

	// assert
	if( world == NULL ) return;

	// free the ents
	for( a = 0 ; a < world->size[0] ; a++ )
	{
		for( b = 0 ; b < world->size[1] ; b++ )
		{
			for( c = 0 ; c < world->size[2] ; c++ )
			{
				if( world->cubes[a][b][c].entCenter != NULL )
				{
					ei->ent_Remove( world->cubes[a][b][c].entCenter );
				}
			}
		}
	}

	// free all columns and rows
	for( a = 0 ; a < world->size[0] ; a++ )
	{
		for( b = 0 ; b < world->size[1] ; b++ )
		{
			SAFE_RELEASE( world->cubes[a][b] );
		}

		SAFE_RELEASE( world->cubes[a] );
	}

	sh_UnloadShader( world->shFace );
	sh_UnloadShader( world->shFlag );
	sh_UnloadShader( world->shEnd );
	SAFE_RELEASE( world->cubes );
	SAFE_RELEASE( world );
}

/* ------------
wrld_ClearSquare - unhides the square indicated
				- returns 1 if the square contains a mine
				- returns 2 if the square is flagged
				- returns 3 if the square is a number
				- returns 4 if the square was 0 and was recursed
				- returns 0 if a fatal error occured
------------ */
int wrld_ClearSquare( world_t *world, int x, int y, int z )
{
	// assert
	if( world == NULL ) return 0;
	if( (x < 0) || (x >= world->size[0]) ) return 0;
	if( (y < 0) || (y >= world->size[1]) ) return 0;
	if( (z < 0) || (z >= world->size[2]) ) return 0;
	if( !(world->cubes[x][y][z].bHidden) ) return 0; 
	
	// 3 cases:  hit mine, hit a marked square, hit > 0, hit 0
	// if we hit a zero we need to recurse

	// check for the mine
	if(world->cubes[x][y][z].numAround == WRLD_CUBE_MINE )
	{
		world->cubes[x][y][z].bHidden = 1;
		return 1;
	}

	// check for the flagged state
	if( world->cubes[x][y][z].bFlagged )
	{
		return 2;
	}

	// check for the number case
	if( world->cubes[x][y][z].numAround > 0 )
	{
		world->cubes[x][y][z].bHidden = 0 ;
		// new draw is in world cubes
//		if( world->cubes[x][y][z].entCenter != NULL ) world->cubes[x][y][z].entCenter->bHintHidden = 0;
		return 3;
	}

	// check for the recursive case
	if( world->cubes[x][y][z].numAround == 0 )
	{
		world->cubes[x][y][z].bHidden = 0;
		wrld_ClearSquare( world, x - 1, y, z );
		wrld_ClearSquare( world, x + 1, y, z );
		wrld_ClearSquare( world, x, y - 1, z );
		wrld_ClearSquare( world, x, y + 1, z );
		wrld_ClearSquare( world, x, y, z - 1 );
		wrld_ClearSquare( world, x, y, z + 1 );
		return 4;
	}

	return 0;
}


/* ------------
wrld_RayIntersect 
// vDir must be normalized
// vCube is a 3-int array to place the output;  only valid when returning non-zero
------------ */
byte wrld_RayIntersect( world_t *world, vec3 vStart, vec3 vDir, int *vCube )
{
	float fPos;
	int vCur[3];
	int vNext[3];
	int i, a;  // safe maximums for our while loops
	aabb_t cube;
	aabb_t wrldBnds;
	int vDelta[3];
	int size[3];
	byte bHaveHit, bHaveNext;
	float tNear, tFar;
	vec3 vTmp;
	int bStartedOutside;

	assert( world != NULL );
	assert( vStart != NULL );
	assert( vDir != NULL );
	assert( vCube != NULL );
	if( world == NULL )
	{
		con_Print( "World RayIntersect Error:  World is NULL." );
		return 0;
	}
	if( vStart == NULL )
	{
		con_Print( "World RayIntersect Error:  vStart is NULL." );
		return 0;
	}
	if( vDir == NULL )
	{
		con_Print( "World RayIntersect Error:  vDir is NULL." );
		return 0;
	}
	if( vCube == NULL )
	{
		con_Print( "World RayIntersect Error:  vCube is NULL." );
		return 0;
	}

	memcpy( &wrldBnds, &world->wrldBnds, sizeof(aabb_t) );
	memcpy( size, world->size, sizeof(int) * 3 );

	if( m_RayBoxIntersect( &tNear, &tFar, vStart, vDir, &wrldBnds ) )
	{
		// project to the cube, giving outselves the benefit of doubt
		// vStart += vDir * (tNear + 0.001);
		m3f_VecScale( vTmp, tNear + 0.001f, vDir );
		m3f_VecAdd( vStart, vStart, vTmp ); 
		bStartedOutside = 1;
	}
	else
	{
		bStartedOutside = 0;
	}

	// the LINE:  X = Xo + t*D
	// D:  vDir
	// Xo: vStart

	// set initials
	vCur[0] = vCur[1] = vCur[2] = 0;

	// locate in X; this is guaranteed to end, even at +/- infinity
	fPos = wrldBnds.lobounds[X_AXIS] + WORLD_CUBE_SIZE;
	while( fPos <= vStart[X_AXIS] ) { fPos += WORLD_CUBE_SIZE; vCur[X_AXIS]++; }

	// locate in Y
	fPos = wrldBnds.lobounds[Y_AXIS] + WORLD_CUBE_SIZE;
	while( fPos <= vStart[Y_AXIS] ) { fPos += WORLD_CUBE_SIZE; vCur[Y_AXIS]++; }

	// locate in Z
	fPos = wrldBnds.lobounds[Z_AXIS] + WORLD_CUBE_SIZE;
	while( fPos <= vStart[Z_AXIS] ) { fPos += WORLD_CUBE_SIZE; vCur[Z_AXIS]++; }

	// assert the world position
	if( (vCur[0] >= world->size[0]) || (vCur[1] >= size[1]) || (vCur[2] >= size[2]) ) { return 0; }

	// we may be noclipped into a solid square, or we started outside and hit a square already
	if( world->cubes[vCur[0]][vCur[1]][vCur[2]].bHidden )
	{
		if( bStartedOutside )
		{
			vCube[0] = vCur[0];
			vCube[1] = vCur[1];
			vCube[2] = vCur[2];
			return 1;
		}
		else
		{
			return 0;
		}
	}
	
	// get a direction for each dimension; these tell us the three adjacent cubes to test against each iteration
	if( vDir[X_AXIS] < 0 ) vDelta[X_AXIS] = -1;
	else vDelta[X_AXIS] = 1;
	if( vDir[Y_AXIS] < 0 ) vDelta[Y_AXIS] = -1;
	else vDelta[Y_AXIS] = 1;
	if( vDir[Z_AXIS] < 0 ) vDelta[Z_AXIS] = -1;
	else vDelta[Z_AXIS] = 1;

	// iterate until we find an empty to solid transition
	// inputs are:  a, vCur
	// outputs are vCur as vNext

	i = 0;
	bHaveHit = 0;
	while( !bHaveHit && (i < 32) )
	{
		// we need to check 3 boxes in the axis of vDir in each square
		a = 0;
		bHaveNext = 0;
		while( (a < 3) && (!bHaveNext) )
		{
			// select a box
			switch(a)
			{
			case 0:
				vNext[0] = vCur[0] + vDelta[0];
				vNext[1] = vCur[1];
				vNext[2] = vCur[2];
				break;
			case 1:
				vNext[0] = vCur[0];
				vNext[1] = vCur[1] + vDelta[1];
				vNext[2] = vCur[2];
				break;
			case 2:
				vNext[0] = vCur[0];
				vNext[1] = vCur[1];
				vNext[2] = vCur[2] + vDelta[2];
				break;
			default:
				break;
			}	

			// ensure that it is a box
			if( (vNext[0] >= 0) && (vNext[0] < size[0]) && 
				(vNext[1] >= 0) && (vNext[1] < size[1]) && 
				(vNext[2] >= 0) && (vNext[2] < size[2]) )
			{
				// get a cube to use in this iteration
				cube.lobounds[X_AXIS] = wrldBnds.lobounds[X_AXIS] + (vNext[X_AXIS] * WORLD_CUBE_SIZE);
				cube.hibounds[X_AXIS] = cube.lobounds[X_AXIS] + WORLD_CUBE_SIZE;
				cube.lobounds[Y_AXIS] = wrldBnds.lobounds[Y_AXIS] + (vNext[Y_AXIS] * WORLD_CUBE_SIZE);
				cube.hibounds[Y_AXIS] = cube.lobounds[Y_AXIS] + WORLD_CUBE_SIZE;
				cube.lobounds[Z_AXIS] = wrldBnds.lobounds[Z_AXIS] + (vNext[Z_AXIS] * WORLD_CUBE_SIZE);
				cube.hibounds[Z_AXIS] = cube.lobounds[Z_AXIS] + WORLD_CUBE_SIZE;

				// if we intersect on this face, move to this face's adjacents
				if( m_RayBoxIntersect( &tNear, &tFar, vStart, vDir, &cube ) )
				{
					bHaveNext = 1;
				}
			}
			a++;
		}

		// if we don't find a next box, don't continue
		// this should happen when all 3 of our possibles are not in the world
		if( !bHaveNext ) return 0;

		// is this a 'hit'
		if( world->cubes[vNext[0]][vNext[1]][vNext[2]].bHidden )
		{
			vCube[0] = vNext[0];
			vCube[1] = vNext[1];
			vCube[2] = vNext[2];
			return 1;
		}
		
		// if not, copy our output to our input and run again
		vCur[0] = vNext[0];
		vCur[1] = vNext[1];
		vCur[2] = vNext[2];
	}

	return 0;
}

static void wrld_LinkTail( world_t *wrld )
{
	if( wrldHead == NULL ) wrldHead = wrld;

	if( wrldTail == NULL )
	{
		wrldTail = wrld;
		wrld->next = NULL;
		wrld->prev = NULL;
		return;
	}

	wrld->prev = wrldTail;
	wrld->next = NULL;

	wrldTail->next = wrld;
	wrldTail = wrld;
}

static void wrld_Unlink( world_t *wrld )
{
	if( wrld->prev != NULL ) wrld->prev->next = wrld->next;
	if( wrld->next != NULL ) wrld->next->prev = wrld->prev;

	if( wrldTail == wrld ) wrldTail = wrld->prev;
	if( wrldHead == wrld ) wrldHead = wrld->next;

	wrld->prev = NULL;
	wrld->next = NULL;
}
