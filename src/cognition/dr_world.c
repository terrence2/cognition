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
// dr_world.c
// Created 2-14-3 by Terrence Cole

// Includes
/////////////
#include "cog_global.h"
#include "cog_opengl.h"

// Definitions
////////////////

// Global Prototypes
//////////////////////
/*
void d_SetWorld( world_t *world );
aabb_t *d_GetWorldBounds();
void d_DrawWorld( void );
void wrld_SetColor( char *color );
*/

// Local Prototypes
//////////////////
static void d_Wrld_DrawCubes( void );
static void d_Wrld_DrawFlags( void );
static void d_DrawWorldEnds( void );

// Global Variables
////////////////////
/*
*/

// Local Variables
/////////////////
static world_t *curWorld = NULL;
static aabb_t nullWrldBnds = { { 10.0f, 10.0f, 10.0f }, { -10.0f, -10.0f, -10.0f } };

// *********** FUNCTIONALITY ***********
/* ------------
dr_SetWorld - sets the currently drawn world - called by the client on init to tell us what to draw
------------ */
void d_SetWorld( world_t *world )
{
	curWorld = world;
}

/* ------------
d_GetWorldBounds
------------ */
aabb_t *d_GetWorldBounds()
{
	if( curWorld == NULL )
	{
		return &nullWrldBnds;
	}
	else
	{
		return &(curWorld->wrldBnds);
	}
}

/* ------------
dr_WorldBase - the base functionality draw for the world - works entirely in OGL 1.3, no extensions
------------ */
void d_DrawWorld( void )
{
	int iPass;
	int iRemaining;

	// assert that we have something to draw
	if( curWorld == NULL ) return;

	// for the world
	iPass = 0;
	do
	{
		iRemaining = d_SetMapState1( curWorld->shFace, iPass );
		glColor4ub( 255, 255, 255, 255 );
		d_Wrld_DrawCubes();
		iPass++;
		iRemaining--;
	} while( iRemaining );
	d_UnsetMapState1( curWorld->shFace );

	// preset our GL state to 2nd pass
	glDepthMask( GL_FALSE );
	glDepthFunc( GL_EQUAL );
	glBlendFunc( GL_ZERO, GL_SRC_COLOR );

	// for the flags
	iPass = 0;
	do
	{
		iRemaining = d_SetMapState1( curWorld->shFlag, iPass );
		d_Wrld_DrawFlags();
		iPass++;
		iRemaining--;
	} while( iRemaining );
	d_UnsetMapState1( curWorld->shFlag );

	// reset to first pass
	glDepthMask( GL_TRUE );
	glDepthFunc( GL_LESS );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	// draw the internal, transparent ents
	d_DrawWorldEnds();
}

/* ------------
d_Wrld_DrawCubes
// STRATEGY:
// we need to draw planes in 3 orientations, going in, in 2 directions toward us from outside
// we only draw planes on cubes that go (out from us) from empty to solid and not solid to empty
------------ */
static void d_Wrld_DrawCubes( void )
{
	int a, b, c;
	float Xaxis, Yaxis, Zaxis;
	camera_t *camTmp;
	int size[3];
	aabb_t wrldBnds;
	wrld_cube_t ***cubes = curWorld->cubes;

	memcpy( size, curWorld->size, sizeof(int) * 3 );
	memcpy( &wrldBnds, &(curWorld->wrldBnds), sizeof(aabb_t) );

	// get the current camera
	camTmp = d_GetCamera();

	// starting at the low X and moving towards current X, considering n to n+1
	a = 0;
	Xaxis = wrldBnds.lobounds[X_AXIS] + WORLD_CUBE_SIZE;
	while( (Xaxis < camTmp->vPos[X_AXIS]) && (a < size[0]) )
	{
		// check for each cube under the other two axis
		for( b = 0 ; b < size[1] ; b++ )
		{
			for( c = 0 ; c < size[2] ; c++ )
			{
				// we're going from low to high, toward our view point, so draw cubes that go from solid to hollow
				if( (cubes[a][b][c].bHidden) && (((a + 1) >= size[0]) || !(cubes[a+1][b][c].bHidden)) )
				{
					glBegin( GL_QUADS );
						glNormal3f( 1.0f, 0.0f, 0.0f );
						
						glTexCoord2f( 0.0f, 0.0f );
						glVertex3fv( cubes[a][b][c].verts[1] );
	
						glTexCoord2f( 0.0f, 1.0f );
						glVertex3fv( cubes[a][b][c].verts[2] );
	
						glTexCoord2f( 1.0f, 1.0f );
						glVertex3fv( cubes[a][b][c].verts[6] );
	
						glTexCoord2f( 1.0f, 0.0f );
						glVertex3fv( cubes[a][b][c].verts[5] );
	
					glEnd();
				}
			}
		}
		
		a++;
		Xaxis += WORLD_CUBE_SIZE;
	}
	
	// starting at the high X and moving towards current X, considering a to a-1
	a = size[0] - 1;
	Xaxis = wrldBnds.hibounds[X_AXIS] - WORLD_CUBE_SIZE;
	while( (Xaxis > camTmp->vPos[X_AXIS]) && (a >= 0) )
	{
		// check for each cube under the other two axis
		for( b = 0 ; b < size[1] ; b++ )
		{
			for( c = 0 ; c < size[2] ; c++ )
			{
				// we're going from low to high, toward our view point, so draw cubes that go from solid to hollow
				if( (cubes[a][b][c].bHidden) && (((a - 1) < 0 ) || !(cubes[a-1][b][c].bHidden)) )
				{
					glBegin( GL_QUADS );
						glNormal3f( -1.0f, 0.0f, 0.0f );
					
						glTexCoord2f( 0.0f, 0.0f );
						glVertex3fv( cubes[a][b][c].verts[4] );
	
						glTexCoord2f( 0.0f, 1.0f );
						glVertex3fv( cubes[a][b][c].verts[7] );
	
						glTexCoord2f( 1.0f, 1.0f );
						glVertex3fv( cubes[a][b][c].verts[3] );
	
						glTexCoord2f( 1.0f, 0.0f );
						glVertex3fv( cubes[a][b][c].verts[0] );
					glEnd();
				}
			}
		}
		
		a--;
		Xaxis -= WORLD_CUBE_SIZE;
	}

	// starting at the low Y and moving towards current Y, considering n to n+1
	b = 0;
	Yaxis = wrldBnds.lobounds[Y_AXIS] + WORLD_CUBE_SIZE;
	while( (Yaxis < camTmp->vPos[Y_AXIS]) && (b < size[1]) )
	{
		// check for each cube under the other two axis
		for( a = 0 ; a < size[0] ; a++ )
		{
			for( c = 0 ; c < size[2] ; c++ )
			{
				// we're going from low to high, toward our view point, so draw cubes that go from solid to hollow
				if( (cubes[a][b][c].bHidden) && (((b + 1) >= size[1]) || !(cubes[a][b+1][c].bHidden)) )
				{

					glBegin( GL_QUADS );
						glNormal3f( 0.0f, 1.0f, 0.0f );
						
						glTexCoord2f( 0.0f, 0.0f );
						glVertex3fv( cubes[a][b][c].verts[2] );

						glTexCoord2f( 0.0f, 1.0f );
						glVertex3fv( cubes[a][b][c].verts[3] );

						glTexCoord2f( 1.0f, 1.0f );
						glVertex3fv( cubes[a][b][c].verts[7] );

						glTexCoord2f( 1.0f, 0.0f );
						glVertex3fv( cubes[a][b][c].verts[6] );

					glEnd();
				}
			}
		}
		
		b++;
		Yaxis += WORLD_CUBE_SIZE;
	}

	// starting at the high Y and moving towards current Y, considering b to b-1
	b = size[1] - 1;
	Yaxis = wrldBnds.hibounds[Y_AXIS] - WORLD_CUBE_SIZE;
	while( (Yaxis > camTmp->vPos[Y_AXIS]) && (b >= 0) )
	{
		// check for each cube under the other two axis
		for( a = 0 ; a < size[0] ; a++ )
		{
			for( c = 0 ; c < size[2] ; c++ )
			{
				// we're going from low to high, toward our view point, so draw cubes that go from solid to hollow
				if( (cubes[a][b][c].bHidden) && (((b - 1) < 0 ) || !(cubes[a][b-1][c].bHidden)) )
				{
					glBegin( GL_QUADS );
						glNormal3f( 0.0f, -1.0f, 0.0f );
					
						glTexCoord2f( 0.0f, 0.0f );
						glVertex3fv( cubes[a][b][c].verts[0] );

						glTexCoord2f( 0.0f, 1.0f );
						glVertex3fv( cubes[a][b][c].verts[1] );

						glTexCoord2f( 1.0f, 1.0f );
						glVertex3fv( cubes[a][b][c].verts[5] );

						glTexCoord2f( 1.0f, 0.0f );
						glVertex3fv( cubes[a][b][c].verts[4] );
					glEnd();
				}
			}
		}
		
		b--;
		Yaxis -= WORLD_CUBE_SIZE;
	}

	// starting at the low Z and moving towards current Z, considering n to n+1
	c = 0;
	Zaxis = wrldBnds.lobounds[Z_AXIS] + WORLD_CUBE_SIZE;
	while( (Zaxis < camTmp->vPos[Z_AXIS]) && (c < size[2]) )
	{
		// check for each cube under the other two axis
		for( a = 0 ; a < size[0] ; a++ )
		{
			for( b = 0 ; b < size[1] ; b++ )
			{
				// we're going from low to high, toward our view point, so draw cubes that go from solid to hollow
				if( (cubes[a][b][c].bHidden) && (((c + 1) >= size[2]) || !(cubes[a][b][c+1].bHidden)) )
				{
					glBegin( GL_QUADS );
						glNormal3f( 0.0f, 0.0f, 1.0f );
						
						glTexCoord2f( 0.0f, 0.0f );
						glVertex3fv( cubes[a][b][c].verts[5] );

						glTexCoord2f( 0.0f, 1.0f );
						glVertex3fv( cubes[a][b][c].verts[6] );

						glTexCoord2f( 1.0f, 1.0f );
						glVertex3fv( cubes[a][b][c].verts[7] );

						glTexCoord2f( 1.0f, 0.0f );
						glVertex3fv( cubes[a][b][c].verts[4] );

					glEnd();
				}
			}
		}
		
		c++;
		Zaxis += WORLD_CUBE_SIZE;
	}

	// starting at the high Y and moving towards current Y, considering b to b-1
	c = size[2] - 1;
	Zaxis = wrldBnds.hibounds[Z_AXIS] - WORLD_CUBE_SIZE;
	while( (Zaxis > camTmp->vPos[Z_AXIS]) && (c >= 0) )
	{
		// check for each cube under the other two axis
		for( a = 0 ; a < size[0] ; a++ )
		{
			for( b = 0 ; b < size[1] ; b++ )
			{
				// we're going from low to high, toward our view point, so draw cubes that go from solid to hollow
				if( (cubes[a][b][c].bHidden) && (((c - 1) < 0 ) || !(cubes[a][b][c-1].bHidden)) )
				{
					glBegin( GL_QUADS );
						glNormal3f( 0.0f, 0.0f, -1.0f );
					
						glTexCoord2f( 0.0f, 0.0f );
						glVertex3fv( cubes[a][b][c].verts[0] );

						glTexCoord2f( 0.0f, 1.0f );
						glVertex3fv( cubes[a][b][c].verts[3] );

						glTexCoord2f( 1.0f, 1.0f );
						glVertex3fv( cubes[a][b][c].verts[2] );

						glTexCoord2f( 1.0f, 0.0f );
						glVertex3fv( cubes[a][b][c].verts[1] );
					glEnd();
				}
			}
		}
		
		c--;
		Zaxis -= WORLD_CUBE_SIZE;
	}
}

/* ------------
d_Wrld_DrawFlags - 
------------ */
static void d_Wrld_DrawFlags( void )
{
	int a, b, c;
	float Xaxis, Yaxis, Zaxis;
	camera_t *camTmp;
	int size[3];
	aabb_t wrldBnds;
	wrld_cube_t ***cubes = curWorld->cubes;

	memcpy( size, curWorld->size, sizeof(int) * 3 );
	memcpy( &wrldBnds, &(curWorld->wrldBnds), sizeof(aabb_t) );
	// get the current camera
	camTmp = d_GetCamera();

	// starting at the low X and moving towards current X, considering n to n+1
	a = 0;
	Xaxis = wrldBnds.lobounds[X_AXIS] + WORLD_CUBE_SIZE;
	while( (Xaxis < camTmp->vPos[X_AXIS]) && (a < size[0]) )
	{
		// check for each cube under the other two axis
		for( b = 0 ; b < size[1] ; b++ )
		{
			for( c = 0 ; c < size[2] ; c++ )
			{
				// we're going from low to high, toward our view point, so draw cubes that go from solid to hollow
				if( (cubes[a][b][c].bHidden) && (cubes[a][b][c].bFlagged) && (((a + 1) >= size[0]) || !(cubes[a+1][b][c].bHidden)) )
				{
					glBegin( GL_QUADS );
						glNormal3f( 1.0f, 0.0f, 0.0f );
						
						glTexCoord2f( 0.0f, 0.0f );
						glVertex3fv( cubes[a][b][c].verts[1] );
	
						glTexCoord2f( 0.0f, 1.0f );
						glVertex3fv( cubes[a][b][c].verts[2] );
	
						glTexCoord2f( 1.0f, 1.0f );
						glVertex3fv( cubes[a][b][c].verts[6] );
	
						glTexCoord2f( 1.0f, 0.0f );
						glVertex3fv( cubes[a][b][c].verts[5] );
	
					glEnd();
				}
			}
		}
		
		a++;
		Xaxis += WORLD_CUBE_SIZE;
	}
	
	// starting at the high X and moving towards current X, considering a to a-1
	a = size[0] - 1;
	Xaxis = wrldBnds.hibounds[X_AXIS] - WORLD_CUBE_SIZE;
	while( (Xaxis > camTmp->vPos[X_AXIS]) && (a >= 0) )
	{
		// check for each cube under the other two axis
		for( b = 0 ; b < size[1] ; b++ )
		{
			for( c = 0 ; c < size[2] ; c++ )
			{
				// we're going from low to high, toward our view point, so draw cubes that go from solid to hollow
				if( (cubes[a][b][c].bHidden)  && (cubes[a][b][c].bFlagged) && (((a - 1) < 0) || !(cubes[a-1][b][c].bHidden)) )
				{
					glBegin( GL_QUADS );
						glNormal3f( -1.0f, 0.0f, 0.0f );
					
						glTexCoord2f( 0.0f, 0.0f );
						glVertex3fv( cubes[a][b][c].verts[4] );
	
						glTexCoord2f( 0.0f, 1.0f );
						glVertex3fv( cubes[a][b][c].verts[7] );
	
						glTexCoord2f( 1.0f, 1.0f );
						glVertex3fv( cubes[a][b][c].verts[3] );
	
						glTexCoord2f( 1.0f, 0.0f );
						glVertex3fv( cubes[a][b][c].verts[0] );
					glEnd();
				}
			}
		}
		
		a--;
		Xaxis -= WORLD_CUBE_SIZE;
	}

	// starting at the low Y and moving towards current Y, considering n to n+1
	b = 0;
	Yaxis = wrldBnds.lobounds[Y_AXIS] + WORLD_CUBE_SIZE;
	while( (Yaxis < camTmp->vPos[Y_AXIS]) && (b < size[1]) )
	{
		// check for each cube under the other two axis
		for( a = 0 ; a < size[0] ; a++ )
		{
			for( c = 0 ; c < size[2] ; c++ )
			{
				// we're going from low to high, toward our view point, so draw cubes that go from solid to hollow
				if( (cubes[a][b][c].bHidden) && (cubes[a][b][c].bFlagged) && (((b + 1) >= size[1]) || !(cubes[a][b+1][c].bHidden)) )
				{

					glBegin( GL_QUADS );
						glNormal3f( 0.0f, 1.0f, 0.0f );
						
						glTexCoord2f( 0.0f, 0.0f );
						glVertex3fv( cubes[a][b][c].verts[2] );

						glTexCoord2f( 0.0f, 1.0f );
						glVertex3fv( cubes[a][b][c].verts[3] );

						glTexCoord2f( 1.0f, 1.0f );
						glVertex3fv( cubes[a][b][c].verts[7] );

						glTexCoord2f( 1.0f, 0.0f );
						glVertex3fv( cubes[a][b][c].verts[6] );

					glEnd();
				}
			}
		}
		
		b++;
		Yaxis += WORLD_CUBE_SIZE;
	}

	// starting at the high Y and moving towards current Y, considering b to b-1
	b = size[1] - 1;
	Yaxis = wrldBnds.hibounds[Y_AXIS] - WORLD_CUBE_SIZE;
	while( (Yaxis > camTmp->vPos[Y_AXIS]) && (b >= 0) )
	{
		// check for each cube under the other two axis
		for( a = 0 ; a < size[0] ; a++ )
		{
			for( c = 0 ; c < size[2] ; c++ )
			{
				// we're going from low to high, toward our view point, so draw cubes that go from solid to hollow
				if( (cubes[a][b][c].bHidden) && (cubes[a][b][c].bFlagged) && (((b - 1) < 0) || !(cubes[a][b-1][c].bHidden)) )
				{
					glBegin( GL_QUADS );
						glNormal3f( 0.0f, -1.0f, 0.0f );
					
						glTexCoord2f( 0.0f, 0.0f );
						glVertex3fv( cubes[a][b][c].verts[0] );

						glTexCoord2f( 0.0f, 1.0f );
						glVertex3fv( cubes[a][b][c].verts[1] );

						glTexCoord2f( 1.0f, 1.0f );
						glVertex3fv( cubes[a][b][c].verts[5] );

						glTexCoord2f( 1.0f, 0.0f );
						glVertex3fv( cubes[a][b][c].verts[4] );
					glEnd();
				}
			}
		}
		
		b--;
		Yaxis -= WORLD_CUBE_SIZE;
	}

	// starting at the low Z and moving towards current Z, considering n to n+1
	c = 0;
	Zaxis = wrldBnds.lobounds[Z_AXIS] + WORLD_CUBE_SIZE;
	while( (Zaxis < camTmp->vPos[Z_AXIS]) && (c < size[2]) )
	{
		// check for each cube under the other two axis
		for( a = 0 ; a < size[0] ; a++ )
		{
			for( b = 0 ; b < size[1] ; b++ )
			{
				// we're going from low to high, toward our view point, so draw cubes that go from solid to hollow
				if( (cubes[a][b][c].bHidden) && (cubes[a][b][c].bFlagged) && (((c + 1) >= size[2]) || !(cubes[a][b][c+1].bHidden)) )
				{
					glBegin( GL_QUADS );
						glNormal3f( 0.0f, 0.0f, 1.0f );
						
						glTexCoord2f( 0.0f, 0.0f );
						glVertex3fv( cubes[a][b][c].verts[5] );

						glTexCoord2f( 0.0f, 1.0f );
						glVertex3fv( cubes[a][b][c].verts[6] );

						glTexCoord2f( 1.0f, 1.0f );
						glVertex3fv( cubes[a][b][c].verts[7] );

						glTexCoord2f( 1.0f, 0.0f );
						glVertex3fv( cubes[a][b][c].verts[4] );

					glEnd();
				}
			}
		}
		
		c++;
		Zaxis += WORLD_CUBE_SIZE;
	}

	// starting at the high Y and moving towards current Y, considering b to b-1
	c = size[2] - 1;
	Zaxis = wrldBnds.hibounds[Z_AXIS] - WORLD_CUBE_SIZE;
	while( (Zaxis > camTmp->vPos[Z_AXIS]) && (c >= 0) )
	{
		// check for each cube under the other two axis
		for( a = 0 ; a < size[0] ; a++ )
		{
			for( b = 0 ; b < size[1] ; b++ )
			{
				// we're going from low to high, toward our view point, so draw cubes that go from solid to hollow
				if( (cubes[a][b][c].bHidden) && (cubes[a][b][c].bFlagged) && (((c - 1) < 0) || !(cubes[a][b][c-1].bHidden)) )
				{
					glBegin( GL_QUADS );
						glNormal3f( 0.0f, 0.0f, -1.0f );
					
						glTexCoord2f( 0.0f, 0.0f );
						glVertex3fv( cubes[a][b][c].verts[0] );

						glTexCoord2f( 0.0f, 1.0f );
						glVertex3fv( cubes[a][b][c].verts[3] );

						glTexCoord2f( 1.0f, 1.0f );
						glVertex3fv( cubes[a][b][c].verts[2] );

						glTexCoord2f( 1.0f, 0.0f );
						glVertex3fv( cubes[a][b][c].verts[1] );
					glEnd();
				}
			}
		}
		
		c--;
		Zaxis -= WORLD_CUBE_SIZE;
	}
}

/* ------------
d_DrawWorldEndsBase - draws the bounding box for the world
------------ */
static void d_DrawWorldEnds( void )
{
	int a;
	vec3 vTmp;
	vec2 tc0 = { 0.0f, 0.0f }, tc1 = { 0.0f, 1.0f }, tc2 = { 1.0f, 1.0f }, tc3 = { 1.0f, 0.0f };

	for( a = 0 ; a < 6 ; a++ )
	{
		trans_Quad( curWorld->quads[a][0], 
					curWorld->quads[a][1],
					curWorld->quads[a][2],
					curWorld->quads[a][3],
					curWorld->planes[a].norm,
					tc0, tc1, tc2, tc3, 
					(uint8_t)var_GetFloat( curWorld->world_color_r ),
					(uint8_t)var_GetFloat( curWorld->world_color_g ),
					(uint8_t)var_GetFloat( curWorld->world_color_b ),
					128,
					curWorld->shEnd );
		m3f_VecScale( vTmp, -1.0f, curWorld->planes[a].norm );
		trans_Quad( curWorld->quads[a][3], 
					curWorld->quads[a][2],
					curWorld->quads[a][1],
					curWorld->quads[a][0],
					vTmp,
					tc0, tc1, tc2, tc3, 
					(uint8_t)var_GetFloat( curWorld->world_color_r ),
					(uint8_t)var_GetFloat( curWorld->world_color_g ),
					(uint8_t)var_GetFloat( curWorld->world_color_b ),
					128,
					curWorld->shEnd );
	}
}

/* ------------
wrld_SetColor
// color can be a string that contains the name of a color or 3 tokens for r, g, b in range [0,255]
------------ */
void wrld_SetColor( char *color )
{
	int toToken, tokenLen;
	varlatch wcr, wcg, wcb;

	if( curWorld == NULL )
	{
		wcr = var_GetVarLatch( "world_color_r" );
		wcg = var_GetVarLatch( "world_color_g" );
		wcb = var_GetVarLatch( "world_color_b" );
	}
	else
	{
		wcr = curWorld->world_color_r;
		wcg = curWorld->world_color_g;
		wcb = curWorld->world_color_b;
	}
	
	// print if called w/out args
	if( color == NULL ||  tcstrlen(color) <= 0 ) 
	{
		con_Print( "Current Color:  ( %d, %d, %d )", (uint8_t)var_GetFloat( wcr ),
												(uint8_t)var_GetFloat( wcg ),
												(uint8_t)var_GetFloat( wcb ) );
		return;
	}

	if( par_GetNextToken( color, &toToken, &tokenLen ) )
	{
		color += toToken;
		if( tcstrincmp( color, "help", 4 ) )
		{
			con_Print( "Usage:  world_color [color|r g b]" );
			con_Print( "\tSpecify colors as a name or three numbers" );
			con_Print( "\tValid Names are: black, white, gray, red," );
			con_Print( "\t\t\t\torange, yellow, green, blue, violet," );
			con_Print( "\t\t\t\taqua, magenta, and cornflowerblue" );
			con_Print( "\tValid Color Triplets are in the range [0,255]" );
		}
		else if( tcstrincmp( color, "black", 5 ) )
		{
			var_UpdateFloat( wcr, 0.0f );
			var_UpdateFloat( wcg, 0.0f );
			var_UpdateFloat( wcb, 0.0f );
		}
		else if( tcstrincmp( color, "white", 5 ) )
		{
			var_UpdateFloat( wcr, 255.0f );
			var_UpdateFloat( wcg, 255.0f );
			var_UpdateFloat( wcb, 255.0f );
		}
		else if( tcstrincmp( color, "gray", 4 ) )
		{
			var_UpdateFloat( wcr, 192.0f );
			var_UpdateFloat( wcg, 192.0f );
			var_UpdateFloat( wcb, 192.0f );
		}
		else if( tcstrincmp( color, "red", 3 ) )
		{
			var_UpdateFloat( wcr, 255.0f );
			var_UpdateFloat( wcg, 0.0f );
			var_UpdateFloat( wcb, 0.0f );
		}
		else if( tcstrincmp( color, "orange", 6 ) )
		{
			var_UpdateFloat( wcr, 255.0f );
			var_UpdateFloat( wcg, 165.0f );
			var_UpdateFloat( wcb, 0.0f );
		}
		else if( tcstrincmp( color, "yellow", 6 ) )
		{
			var_UpdateFloat( wcr, 255.0f );
			var_UpdateFloat( wcg, 255.0f );
			var_UpdateFloat( wcb, 0.0f );
		}
		else if( tcstrincmp( color, "green", 5 ) )
		{
			var_UpdateFloat( wcr, 0.0f );
			var_UpdateFloat( wcg, 255.0f );
			var_UpdateFloat( wcb, 0.0f );
		}
		else if( tcstrincmp( color, "blue", 4 ) )
		{
			var_UpdateFloat( wcr, 0.0f );
			var_UpdateFloat( wcg, 0.0f );
			var_UpdateFloat( wcb, 255.0f );
		}
		else if( tcstrincmp( color, "violet", 6 ) )
		{
			var_UpdateFloat( wcr, 238.0f );
			var_UpdateFloat( wcg, 130.0f );
			var_UpdateFloat( wcb, 238.0f );
		}
		else if( tcstrincmp( color, "aqua", 4 ) )
		{
			var_UpdateFloat( wcr, 0.0f );
			var_UpdateFloat( wcg, 255.0f );
			var_UpdateFloat( wcb, 255.0f );
		}
		else if( tcstrincmp( color, "magenta", 7 ) )
		{
			var_UpdateFloat( wcr, 255.0f );
			var_UpdateFloat( wcg, 0.0f );
			var_UpdateFloat( wcb, 255.0f );
		}
		else if( tcstrincmp( color, "cornflowerblue", 14 ) )
		{
			var_UpdateFloat( wcr, 100.0f );
			var_UpdateFloat( wcg, 149.0f );
			var_UpdateFloat( wcb, 237.0f );
		}
		else
		{
			uint8_t r, g, b;
			r = (uint8_t)tcatoi( color );
			color += tokenLen;
			if( par_GetNextToken( color, &toToken, &tokenLen ) )
			{
				color += toToken;
				g = (uint8_t)tcatoi( color );
				color += tokenLen;
				if( par_GetNextToken( color, &toToken, &tokenLen ) )
				{
					color += toToken;
					b = (uint8_t)tcatoi( color );
				}
				else
				{
					b = 0;
				}
			}
			else
			{
				g = b = 0;
			}

			var_UpdateFloat( wcr, (float)r );
			var_UpdateFloat( wcg, (float)g );
			var_UpdateFloat( wcb, (float)b );
		}
	}
}
