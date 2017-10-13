// Cognition
// cl_background.c
// Created 3/25/03 by Terrence Cole

// Includes
/////////////
#include "cl_global.h"
#include <stdio.h>
#include <math.h>

// Definitions
////////////////
#define BG_PART_RANGES 3
#define BG_CL_MAX_PARTICLE_NAMES 10

// indexes
#define BG_SIZE_2_5 0
#define BG_SIZE_5 1
#define BG_SIZE_10 2
#define BG_SIZE_25 3
#define BG_SIZE_50 4
#define BG_SIZE_100 5
#define BG_SIZE_200 6
#define BG_SIZE_400 7
#define BG_SIZE_1000 8
#define BG_NUM_SIZES 9

// counts
#define BG_STR_COUNT_2_5 8
#define BG_STR_COUNT_5 8
#define BG_STR_COUNT_10 8
#define BG_STR_COUNT_25 6
#define BG_STR_COUNT_50 6
#define BG_STR_COUNT_100 6
#define BG_STR_COUNT_200 6
#define BG_STR_COUNT_400 6
#define BG_STR_COUNT_1000 6
#define BG_NUM_STR_COUNTS 6

// in most systems there exist twice as many particles
// of half the size
/*
#define BG_SCALE_RAW_2_5 256
#define BG_SCALE_RAW_5 128
#define BG_SCALE_RAW_10 64
#define BG_SCALE_RAW_25 32
#define BG_SCALE_RAW_50 16
#define BG_SCALE_RAW_100 8
#define BG_SCALE_RAW_200 4
#define BG_SCALE_RAW_400 2
#define BG_SCALE_RAW_1000 1
#define BG_SCALE_UNITY 511
*/
#define BG_SCALE_RAW_2_5 0
#define BG_SCALE_RAW_5 0
#define BG_SCALE_RAW_10 100
#define BG_SCALE_RAW_25 200
#define BG_SCALE_RAW_50 0
#define BG_SCALE_RAW_100 0
#define BG_SCALE_RAW_200 0
#define BG_SCALE_RAW_400 0
#define BG_SCALE_RAW_1000 1
#define BG_SCALE_UNITY 301


// Local Structures
/////////////////////

char objTable[BG_NUM_SIZES][8][128] =
{
	// 2_5
	{
		"comet_2_5_blue.def",
		"comet_2_5_dark_green.def",
		"comet_2_5_green.def",
		"comet_2_5_light_blue.def",
		"comet_2_5_orange.def",
		"comet_2_5_purple.def",
		"comet_2_5_red.def",
		"comet_2_5_yellow.def",
	},
	// 5
	{
		"comet_5_blue.def",
		"comet_5_dark_green.def",
		"comet_5_green.def",
		"comet_5_light_blue.def",
		"comet_5_orange.def",
		"comet_5_purple.def",
		"comet_5_red.def",
		"comet_5_yellow.def",
	},
	// 10
	{
		"comet_10_blue.def",
		"comet_10_dark_green.def",
		"comet_10_green.def",
		"comet_10_light_blue.def",
		"comet_10_orange.def",
		"comet_10_purple.def",
		"comet_10_red.def",
		"comet_10_yellow.def",
	},
	// 25
	{
		"comet_25_frozen_white.def",
		"comet_25_gas_purple.def",
		"comet_25_ice_trans.def",
		"comet_25_molten_orange.def",
		"comet_25_molten_red.def",
		"comet_25_rock_purple.def",
		"",
		"",
	},
	// 50
	{
		"comet_50_frozen_white.def",
		"comet_50_gas_purple.def",
		"comet_50_ice_trans.def",
		"comet_50_molten_orange.def",
		"comet_50_molten_red.def",
		"comet_50_rock_purple.def",
		"",
		"",
	},
	// 100
	{
		"comet_100_frozen_white.def",
		"comet_100_gas_purple.def",
		"comet_100_ice_trans.def",
		"comet_100_molten_orange.def",
		"comet_100_molten_red.def",
		"comet_100_rock_purple.def",
		"",
		"",
	},
	// 200
	{
		"comet_200_frozen_white.def",
		"comet_200_gas_purple.def",
		"comet_200_ice_trans.def",
		"comet_200_molten_orange.def",
		"comet_200_molten_red.def",
		"comet_200_rock_purple.def",
		"",
		"",
	},
	// 400
	{
		"comet_400_frozen_white.def",
		"comet_400_gas_purple.def",
		"comet_400_ice_trans.def",
		"comet_400_molten_orange.def",
		"comet_400_molten_red.def",
		"comet_400_rock_purple.def",
		"",
		"",
	},
	// 1000
	{
		"comet_1000_frozen_white.def",
		"comet_1000_gas_purple.def",
		"comet_1000_ice_trans.def",
		"comet_1000_molten_orange.def",
		"comet_1000_molten_red.def",
		"comet_1000_rock_purple.def",
		"",
		"",
	}
};

// Global Prototypes
//////////////////////
/*
int bg_cl_Initialize(void);
void bg_cl_Terminate(void);
void bg_cl_Animate();
*/

// Local Prototypes
/////////////////////
static int bg_cl_Init_GetLight();
static int bg_cl_Init_GetEnts();
static void bg_cl_OccludeObject( entity_t *ent, aabb_t *bbox );

// Local Variables
////////////////////
static entity_t **bgArray = NULL;
static int bgNumEntities = 0;
static light_t *bgLight1 = NULL;
static aabb_t bgBox;

// *********** FUNCTIONALITY ***********
/* ------------
bg_cl_Initialize - allocates storage for background entities, get background entities, spawns all necessary structures to display them
			- and other draw related necessities not performed by the player
			- nb. camera and skybox are already up, just need a light or two
------------ */
int bg_cl_Initialize(void)
{
	// assert
	assert( bgArray == NULL );
	if( bgArray != NULL )
	{
		ei->con_Print( "Client Background Assert Failed in Init!:  bgArray is NOT NULL" );
		ei->eng_Stop( "170001" );
		return 0;
	}
	assert( plState.world != NULL );
	if( plState.world == NULL )
	{
		ei->con_Print( "Client Background Assert Failed in Init!:  plState.world is NULL" );
		ei->eng_Stop( "170008" );
		return 0;
	}

	ei->con_Print( "\n<BLUE>Initializing</BLUE> Client Background..." );

	// create a bounding box that extends the view depth out from each side
	memcpy( &bgBox, &(plState.world->wrldBnds), sizeof(aabb_t) );
	bgBox.lobounds[0] -= CL_FAR_CLIP_PLANE;
	bgBox.lobounds[1] -= CL_FAR_CLIP_PLANE;
	bgBox.lobounds[2] -= CL_FAR_CLIP_PLANE;
	bgBox.hibounds[0] += CL_FAR_CLIP_PLANE;
	bgBox.hibounds[1] += CL_FAR_CLIP_PLANE;
	bgBox.hibounds[2] += CL_FAR_CLIP_PLANE;
	
	// get the lighting
	if( !bg_cl_Init_GetLight() ) return 0;
	
	// get the ents
	if( !bg_cl_Init_GetEnts() ) return 0;

	return 1;
}

/* ------------
bg_cl_Init_GetLight
------------ */
static int bg_cl_Init_GetLight()
{
	vec4 vLtPos1 = { 0.7f, 0.7f, 1.0f, 0.0f }; 
	vec4 vLtAmb = { 0.25f, 0.25f, 0.22f, 1.0f };
	vec4 vLtDif1 = { 1.0f, 1.0f, 0.93f, 1.0f };
	vec4 vLtSpec = { 1.0f, 1.0f, 1.0f, 1.0f };
	vec3 vLtSptDir = { 0.0f, 0.0f, 0.0f };

	// spawn a light for the scene
	bgLight1 = ei->l_Spawn( LIGHT_DIRECTIONAL, vLtPos1, vLtAmb, vLtDif1, vLtSpec, 0, 45, vLtSptDir, 1.0f, 0.0f, 0.0f );
	if( bgLight1 == NULL )
	{
		ei->con_Print( "Client Background Error on Init:  Light Spawn returned a NULL light." );
		return 0;
	}
	return 1;
}

/* ------------
bg_cl_Init_GetEnts
------------ */
static int bg_cl_Init_GetEnts()
{
	float fBGComplexity;
	int a, b, cnt;
	int sizeCnt[BG_NUM_SIZES];
	int strCnt[BG_NUM_SIZES];
	int sizeTotal;
	
	// get the expected scene complexity
	fBGComplexity = ei->var_GetFloatFromName( "cl_background_complexity" );

	// get the count of each object size range
	sizeCnt[BG_SIZE_2_5] = (int)(((float)BG_SCALE_RAW_2_5 / (float)BG_SCALE_UNITY) * fBGComplexity);
	sizeCnt[BG_SIZE_5] = (int)(((float)BG_SCALE_RAW_5 / (float)BG_SCALE_UNITY) * fBGComplexity);
	sizeCnt[BG_SIZE_10] = (int)(((float)BG_SCALE_RAW_10 / (float)BG_SCALE_UNITY) * fBGComplexity);
	sizeCnt[BG_SIZE_25] = (int)(((float)BG_SCALE_RAW_25 / (float)BG_SCALE_UNITY) * fBGComplexity);
	sizeCnt[BG_SIZE_50] = (int)(((float)BG_SCALE_RAW_50 / (float)BG_SCALE_UNITY) * fBGComplexity);
	sizeCnt[BG_SIZE_100] = (int)(((float)BG_SCALE_RAW_100 / (float)BG_SCALE_UNITY) * fBGComplexity);
	sizeCnt[BG_SIZE_200] = (int)(((float)BG_SCALE_RAW_200 / (float)BG_SCALE_UNITY) * fBGComplexity);
	sizeCnt[BG_SIZE_400] = (int)(((float)BG_SCALE_RAW_400 / (float)BG_SCALE_UNITY) * fBGComplexity);
	sizeCnt[BG_SIZE_1000] = (int)(((float)BG_SCALE_RAW_1000 / (float)BG_SCALE_UNITY) * fBGComplexity);

	// accumulate to sizeTotal
	sizeTotal = 0;
	for( a = 0 ; a < BG_NUM_SIZES ; a++ ) {	sizeTotal += sizeCnt[a]; }
	bgNumEntities = sizeTotal;
	ei->con_Print( "\tCreating %d Particles...", bgNumEntities );

	// fill in the string count
	strCnt[BG_SIZE_2_5] = BG_STR_COUNT_2_5;
	strCnt[BG_SIZE_5] = BG_STR_COUNT_5;
	strCnt[BG_SIZE_10] = BG_STR_COUNT_10;
	strCnt[BG_SIZE_25] = BG_STR_COUNT_25;
	strCnt[BG_SIZE_50] = BG_STR_COUNT_50;
	strCnt[BG_SIZE_100] = BG_STR_COUNT_100;
	strCnt[BG_SIZE_200] = BG_STR_COUNT_200;
	strCnt[BG_SIZE_400] = BG_STR_COUNT_400;
	strCnt[BG_SIZE_1000] = BG_STR_COUNT_1000;

	// allocate the number of entities
	bgArray = (entity_t**)ei->mem_alloc( sizeof(entity_t*) * bgNumEntities );
	if( bgArray == NULL )
	{
		ei->con_Print( "Client Background Error on Init:  Memory allocate failed for %d entity pointers.", bgNumEntities );
		return 0;
	}
	memset( bgArray, 0, sizeof(entity_t*) * bgNumEntities );

	// allocate entities
	cnt = 0;
	for( a = 0 ; a < BG_NUM_SIZES ; a++ )
	{
		for( b = 0 ; b < sizeCnt[a] ; b++ )
		{
			vec3 vPos;
			vec3 vAng;
			byte byAxis;
			char cNameBuffer[1024];
			int iModelRandom;

			// get a name
			snprintf( cNameBuffer, 1024, "clBackground%d", cnt );

			// get a random position
			vPos[0] = ei->rng_Float( bgBox.lobounds[0], bgBox.hibounds[0] ); 
			vPos[1] = ei->rng_Float( bgBox.lobounds[1], bgBox.hibounds[1] ); 
			vPos[2] = ei->rng_Float( bgBox.lobounds[2], bgBox.hibounds[2] ); 

			// lock the position outside of the world bounds
			byAxis = ei->rng_Byte( 0, 3 );
			vPos[byAxis] += plState.world->wrldBnds.hibounds[byAxis] - plState.world->wrldBnds.lobounds[byAxis];

			// set a random rotation
			vAng[0] = ei->rng_Float( 0.0f, 360.0f );
			vAng[1] = ei->rng_Float( 0.0f, 360.0f );

			// get a random string
			iModelRandom = ei->rng_Long( 0, strCnt[a] );

			ei->con_Print( "Loading Model %d from %s", cnt, objTable[a][iModelRandom] );
			
			// spawn the ent
			bgArray[cnt] = ei->ent_Spawn( cNameBuffer, 0, objTable[a][iModelRandom], vPos, vAng, (float*)vUnity, NULL, NULL, 0 );
			if( bgArray[cnt] == NULL )
			{
				ei->con_Print( "Client Background Error on Init: Entity Load Failed for Entity %d / %d", cnt, bgNumEntities );
				return 0;
			}

			// set a random rotation rate
			bgArray[cnt]->ang_velocity[0] = ei->rng_Float( 0.0f, 200.0f );
			bgArray[cnt]->ang_velocity[1] = ei->rng_Float( 0.0f, 200.0f );

			// set a meandering direction for movement
			bgArray[cnt]->velocity[0] = ei->rng_Float( -50.0f, 50.0f );
			bgArray[cnt]->velocity[1] = ei->rng_Float( -50.0f, 50.0f );
			bgArray[cnt]->velocity[2] = ei->rng_Float( -20.0f, 400.0f );

			cnt++;
		}
	}

	return 1;
}
	
/* ------------
bg_cl_Terminate - step out of whatever Initialize did.
------------ */
void bg_cl_Terminate(void)
{
	int a;
	
	// kill the scene light
	if( bgLight1 != NULL ) ei->l_Kill( bgLight1 );

	// kill the entities
	if( bgArray != NULL )
	{
		for( a = 0 ; a < bgNumEntities ; a++ )
		{
			if( bgArray[a] != NULL )
			{
				ei->ent_Remove( bgArray[a] );
			}
		}
		SAFE_RELEASE( bgArray );
	}
}

/* ------------
bg_cl_Animate - handle stately animation of the background
------------ */
void bg_cl_Animate()
{
	int a, b;
	byte prio;
	float dist;
	vec3 hit;
	float elapsed_sec = plState.tDelta  / 1000.0f;
	
	// assert
	if( bgArray == NULL )
	{
		ei->con_Print( "Client Background Assert Failed in Animate!:  bgArray is NULL" );
		ei->eng_Stop( "170003" );
		return;
	}
	if( bgLight1 == NULL )
	{
		ei->con_Print( "Client Background Assert Failed in Animate!:  bgLight1 is NULL" );
		ei->eng_Stop( "170004" );
		return;
	}
	if( plState.world == NULL )
	{
		ei->con_Print( "Client Background Assert Failed in Animate:  plState.world is NULL" );
		ei->eng_Stop( "170005" );
		return;
	}

	// iterate the particles
	for( a = 0 ; a < bgNumEntities ; a++ )
	{
		if( bgArray[a] != NULL )
		{
			// use the acceleration to change the velocity
			// v2 = v1 + a * t
			bgArray[a]->velocity[X_AXIS] += bgArray[a]->accel[X_AXIS] * elapsed_sec;
			bgArray[a]->velocity[Y_AXIS] += bgArray[a]->accel[Y_AXIS] * elapsed_sec;
			bgArray[a]->velocity[Z_AXIS] += bgArray[a]->accel[Z_AXIS] * elapsed_sec;

			// use the velocity to calculate a change in position
			bgArray[a]->position[X_AXIS] += bgArray[a]->velocity[X_AXIS] * elapsed_sec;
			bgArray[a]->position[Y_AXIS] += bgArray[a]->velocity[Y_AXIS] * elapsed_sec;
			bgArray[a]->position[Z_AXIS] += bgArray[a]->velocity[Z_AXIS] * elapsed_sec;

			// use the w to calculate a new rotation
			bgArray[a]->rotation[PITCH] += bgArray[a]->ang_velocity[PITCH] * elapsed_sec;
			bgArray[a]->rotation[YAW] += bgArray[a]->ang_velocity[YAW] * elapsed_sec;

			// cap the rotation on X
			while( bgArray[a]->rotation[PITCH] >= 360.0 ) bgArray[a]->rotation[PITCH] -= 360.0f;
			while( bgArray[a]->rotation[PITCH] < 0.0f ) bgArray[a]->rotation[PITCH] += 360.0f;

			// cap the rotation on Y
			while( bgArray[a]->rotation[YAW] >= 360.0 ) bgArray[a]->rotation[YAW] -= 360.0f;
			while( bgArray[a]->rotation[YAW] < 0.0f ) bgArray[a]->rotation[YAW] += 360.0f;

			// do the keyframe animation
			if( bgArray[a]->mod->moo->num_frames > 1 )
			{
				// frame sub part delta is the ( time elapsed / time per frame ) => time elapsed * frames per millisecond
				bgArray[a]->mod->moo->sub_frame += (float)((double)(plState.tDelta) * bgArray[a]->mod->moo->frames_per_millisecond * .01);
				while( bgArray[a]->mod->moo->sub_frame > 1.00f ) 
				{
					bgArray[a]->mod->moo->sub_frame -= 1.00f;
					bgArray[a]->mod->moo->cur_frame += 1;
				}
				while( bgArray[a]->mod->moo->cur_frame >= bgArray[a]->mod->moo->num_frames )
				{
					bgArray[a]->mod->moo->cur_frame -= bgArray[a]->mod->moo->num_frames;
				}
			}

			// if we are in the world, get us out, NOW
			if( m_PointInBox( bgArray[a]->position, &(plState.world->wrldBnds) ) )
			{
				bgArray[a]->position[Z_AXIS] = bgBox.hibounds[Z_AXIS];
			}

			// keep background objects out of the world
			bg_cl_OccludeObject( bgArray[a], &(plState.world->wrldBnds) );

			// check to see if any of the particles leave, if so, warp them to the other side
			if( bgArray[a]->position[X_AXIS] < bgBox.lobounds[X_AXIS] ) bgArray[a]->position[X_AXIS] += (bgBox.hibounds[X_AXIS] - bgBox.lobounds[X_AXIS]);
			if( bgArray[a]->position[Y_AXIS] < bgBox.lobounds[Y_AXIS] ) bgArray[a]->position[Y_AXIS] += (bgBox.hibounds[Y_AXIS] - bgBox.lobounds[Y_AXIS]);
			if( bgArray[a]->position[Z_AXIS] < bgBox.lobounds[Z_AXIS] ) bgArray[a]->position[Z_AXIS] += (bgBox.hibounds[Z_AXIS] - bgBox.lobounds[Z_AXIS]);
			if( bgArray[a]->position[X_AXIS] > bgBox.hibounds[X_AXIS] ) bgArray[a]->position[X_AXIS] -= (bgBox.hibounds[X_AXIS] - bgBox.lobounds[X_AXIS]);
			if( bgArray[a]->position[Y_AXIS] > bgBox.hibounds[Y_AXIS] ) bgArray[a]->position[Y_AXIS] -= (bgBox.hibounds[Y_AXIS] - bgBox.lobounds[Y_AXIS]);
			if( bgArray[a]->position[Z_AXIS] > bgBox.hibounds[Z_AXIS] ) bgArray[a]->position[Z_AXIS] -= (bgBox.hibounds[Z_AXIS] - bgBox.lobounds[Z_AXIS]);

			// the complexity of a hit test should be directly proportional to a viewer's perception, use distance
			dist = m3f_VecDistance( plState.self->position, bgArray[a]->position );
			if( dist < 400.0f ) prio = COL_PRIORITY_HIGH;
			else if( dist < 800.0f ) prio = COL_PRIORITY_MEDIUM;
			else if( dist < plState.world->wrldBnds.hibounds[0] ) prio = COL_PRIORITY_LOW;
			else prio = COL_PRIORITY_NONE;

			// N^2 collision isn't bad since a cubic element, R, requires fewer computations with distance
			for( b = 0 ; b < bgNumEntities ; b++ )
			{
				if( bgArray[b] != NULL && b != a )
				{
					ei->col_CheckEnts( hit, bgArray[a], bgArray[b], prio );
				}
			}
		}
	}
}

/* ------------
bg_cl_OcculudeObject - clips ent out of bbox, responds to collisions between bbox and ent
					- ent is represented as ONLY it's bounding sphere
------------ */
static void bg_cl_OccludeObject( entity_t *ent, aabb_t *bbox )
{
	// strategy:  the overlap region is where the sphere is less than one radius away from a plane side
	//			the sphere must be inside or in the overlap region FOR EACH FACE
	//			the collision handles different based on the number of overlap regions [0-3]

	aabb_t bBnds;
	byte axis[3];
	byte cnt = 0;
	vec3 vTmp;

	memcpy( &bBnds, bbox, sizeof(aabb_t) );
	bBnds.lobounds[X_AXIS] -= ent->radius;
	bBnds.lobounds[Y_AXIS] -= ent->radius;
	bBnds.lobounds[Z_AXIS] -= ent->radius;
	bBnds.hibounds[X_AXIS] += ent->radius;
	bBnds.hibounds[Y_AXIS] += ent->radius;
	bBnds.hibounds[Z_AXIS] += ent->radius;
	memset( axis, 0, sizeof(byte) * 3 );

/*
	Strategy: Clip on descending levels of object primitive: face, edge, corner
*/

	/// FACES ///

	// Z-axis planes
	if( (ent->position[X_AXIS] >= bbox->lobounds[X_AXIS]) &&
		(ent->position[X_AXIS] <= bbox->hibounds[X_AXIS]) &&
		(ent->position[Y_AXIS] >= bbox->lobounds[Y_AXIS]) &&
		(ent->position[Y_AXIS] <= bbox->hibounds[Y_AXIS]) )
	{
		// Z on 0+ and greater
		if( (ent->position[Z_AXIS] > 0.0f) && (ent->position[Z_AXIS] < bBnds.hibounds[Z_AXIS]) ) 
		{
			vTmp[X_AXIS] = ent->position[X_AXIS];
			vTmp[Y_AXIS] = ent->position[Y_AXIS];
			vTmp[Z_AXIS] = bbox->hibounds[Z_AXIS];
			ei->col_Respond( ent, vTmp );
		}

		// Z on 0 and less
		else if( (ent->position[Z_AXIS] <= 0.0f) && (ent->position[Z_AXIS] > bBnds.lobounds[Z_AXIS]) ) 
		{
			vTmp[X_AXIS] = ent->position[X_AXIS];
			vTmp[Y_AXIS] = ent->position[Y_AXIS];
			vTmp[Z_AXIS] = bbox->lobounds[Z_AXIS];
			ei->col_Respond( ent, vTmp );
		}
	}

	// Y-axis planes
	else if( (ent->position[X_AXIS] >= bbox->lobounds[X_AXIS]) &&
		(ent->position[X_AXIS] <= bbox->hibounds[X_AXIS]) &&
		(ent->position[Z_AXIS] >= bbox->lobounds[Z_AXIS]) &&
		(ent->position[Z_AXIS] <= bbox->hibounds[Z_AXIS]) )
	{
		// Y on 0 and less
		if( (ent->position[Y_AXIS] <= 0.0f) && (ent->position[Y_AXIS] > bBnds.lobounds[Y_AXIS]) ) 
		{
			vTmp[X_AXIS] = ent->position[X_AXIS];
			vTmp[Y_AXIS] = bbox->lobounds[Y_AXIS];
			vTmp[Z_AXIS] = ent->position[Z_AXIS];
			ei->col_Respond( ent, vTmp );
		}

		// Y on 0+ and greater
		else if( (ent->position[Y_AXIS] > 0.0f) && (ent->position[Y_AXIS] < bBnds.hibounds[Y_AXIS]) ) 
		{
			vTmp[X_AXIS] = ent->position[X_AXIS];
			vTmp[Y_AXIS] = bbox->hibounds[Y_AXIS];
			vTmp[Z_AXIS] = ent->position[Z_AXIS];
			ei->col_Respond( ent, vTmp );
		}
	}

	// X-axis planes
	else 	if( (ent->position[Z_AXIS] >= bbox->lobounds[Z_AXIS]) &&
			(ent->position[Z_AXIS] <= bbox->hibounds[Z_AXIS]) &&
			(ent->position[Y_AXIS] >= bbox->lobounds[Y_AXIS]) &&
			(ent->position[Y_AXIS] <= bbox->hibounds[Y_AXIS]) )
	{
		// X on 0 and less
		if( (ent->position[X_AXIS] <= 0.0f) && (ent->position[X_AXIS] > bBnds.lobounds[X_AXIS]) ) 
		{
			vTmp[X_AXIS] = bbox->lobounds[X_AXIS];
			vTmp[Y_AXIS] = ent->position[Y_AXIS];
			vTmp[Z_AXIS] = ent->position[Z_AXIS];
			ei->col_Respond( ent, vTmp );
		}
	
		// X on 0+ and greater
		else if( (ent->position[X_AXIS] > 0.0f) && (ent->position[X_AXIS] < bBnds.hibounds[X_AXIS]) ) 
		{
			vTmp[X_AXIS] = bbox->hibounds[X_AXIS];
			vTmp[Y_AXIS] = ent->position[Y_AXIS];
			vTmp[Z_AXIS] = ent->position[Z_AXIS];
			ei->col_Respond( ent, vTmp );
		}
	}

	// all edge and corner cases
	else
	{
		vec2 v2Delta;
		vec3 vOverflow;
		float t;
		float fDist;

		/// EDGES ///

		// on the X-Z plane
		if( (ent->position[Y_AXIS] > bbox->lobounds[Y_AXIS]) && (ent->position[Y_AXIS] < bbox->hibounds[Y_AXIS]) )
		{
			// this gives us 4 corners to compare in x-z space with
			v2Delta[0] = ent->position[X_AXIS] - bbox->lobounds[X_AXIS];
			v2Delta[1] = ent->position[Z_AXIS] - bbox->lobounds[Z_AXIS];
			fDist = (float)sqrt( v2Delta[0] * v2Delta[0] + v2Delta[1] * v2Delta[1] );
			if( fDist < ent->radius )
			{
				// collision occured, do occlusion
				t = fDist - ent->radius;
				memcpy( vTmp, ent->velocity, sizeof(vec3) );
				m3f_VecUnitize( vTmp );
				m3f_VecScale( vOverflow, t, vTmp );
				m3f_VecAdd( ent->position, ent->position, vOverflow );

				// do response
				vTmp[X_AXIS] = bbox->lobounds[X_AXIS];
				vTmp[Y_AXIS] = ent->position[Y_AXIS];
				vTmp[Z_AXIS] = bbox->lobounds[Z_AXIS];
				ei->col_Respond( ent, vTmp );
				return;
			}

			v2Delta[0] = ent->position[X_AXIS] - bbox->hibounds[X_AXIS];
			v2Delta[1] = ent->position[Z_AXIS] - bbox->hibounds[Z_AXIS];
			fDist = (float)sqrt( v2Delta[0] * v2Delta[0] + v2Delta[1] * v2Delta[1] );
			if( fDist < ent->radius )
			{
				// collision occured, do occlusion
				t = fDist - ent->radius;
				memcpy( vTmp, ent->velocity, sizeof(vec3) );
				m3f_VecUnitize( vTmp );
				m3f_VecScale( vOverflow, t, vTmp );
				m3f_VecAdd( ent->position, ent->position, vOverflow );

				// do response
				vTmp[X_AXIS] = bbox->hibounds[X_AXIS];
				vTmp[Y_AXIS] = ent->position[Y_AXIS];
				vTmp[Z_AXIS] = bbox->hibounds[Z_AXIS];
				ei->col_Respond( ent, vTmp );
				return;
			}

			v2Delta[0] = ent->position[X_AXIS] - bbox->hibounds[X_AXIS];
			v2Delta[1] = ent->position[Z_AXIS] - bbox->lobounds[Z_AXIS];
			fDist = (float)sqrt( v2Delta[0] * v2Delta[0] + v2Delta[1] * v2Delta[1] );
			if( fDist < ent->radius )
			{
				// collision occured, do occlusion
				t = fDist - ent->radius;
				memcpy( vTmp, ent->velocity, sizeof(vec3) );
				m3f_VecUnitize( vTmp );
				m3f_VecScale( vOverflow, t, vTmp );
				m3f_VecAdd( ent->position, ent->position, vOverflow );

				// do response
				vTmp[X_AXIS] = bbox->hibounds[X_AXIS];
				vTmp[Y_AXIS] = ent->position[Y_AXIS];
				vTmp[Z_AXIS] = bbox->lobounds[Z_AXIS];
				ei->col_Respond( ent, vTmp );
				return;
			}

			v2Delta[0] = ent->position[X_AXIS] - bbox->lobounds[X_AXIS];
			v2Delta[1] = ent->position[Z_AXIS] - bbox->hibounds[Z_AXIS];
			fDist = (float)sqrt( v2Delta[0] * v2Delta[0] + v2Delta[1] * v2Delta[1] );
			if( fDist < ent->radius )
			{
				// collision occured, do occlusion
				t = fDist - ent->radius;
				memcpy( vTmp, ent->velocity, sizeof(vec3) );
				m3f_VecUnitize( vTmp );
				m3f_VecScale( vOverflow, t, vTmp );
				m3f_VecAdd( ent->position, ent->position, vOverflow );

				// do response
				vTmp[X_AXIS] = bbox->lobounds[X_AXIS];
				vTmp[Y_AXIS] = ent->position[Y_AXIS];
				vTmp[Z_AXIS] = bbox->hibounds[Z_AXIS];
				ei->col_Respond( ent, vTmp );
				return;
			}
		}

		// Y-Z axis
		else if( (ent->position[X_AXIS] > bbox->lobounds[X_AXIS]) && (ent->position[X_AXIS] < bbox->hibounds[X_AXIS]) )
		{
			// this gives us 4 corners to compare in y-z space with
			v2Delta[0] = ent->position[Y_AXIS] - bbox->lobounds[Y_AXIS];
			v2Delta[1] = ent->position[Z_AXIS] - bbox->lobounds[Z_AXIS];
			fDist = (float)sqrt( v2Delta[0] * v2Delta[0] + v2Delta[1] * v2Delta[1] );
			if( fDist < ent->radius )
			{
				// collision occured, do occlusion
				t = fDist - ent->radius;
				memcpy( vTmp, ent->velocity, sizeof(vec3) );
				m3f_VecUnitize( vTmp );
				m3f_VecScale( vOverflow, t, vTmp );
				m3f_VecAdd( ent->position, ent->position, vOverflow );

				// do response
				vTmp[X_AXIS] = ent->position[X_AXIS];
				vTmp[Y_AXIS] = bbox->lobounds[Y_AXIS];
				vTmp[Z_AXIS] = bbox->lobounds[Z_AXIS];
				ei->col_Respond( ent, vTmp );
				return;
			}

			v2Delta[0] = ent->position[Y_AXIS] - bbox->hibounds[Y_AXIS];
			v2Delta[1] = ent->position[Z_AXIS] - bbox->hibounds[Z_AXIS];
			fDist = (float)sqrt( v2Delta[0] * v2Delta[0] + v2Delta[1] * v2Delta[1] );
			if( fDist < ent->radius )
			{
				// collision occured, do occlusion
				t = fDist - ent->radius;
				memcpy( vTmp, ent->velocity, sizeof(vec3) );
				m3f_VecUnitize( vTmp );
				m3f_VecScale( vOverflow, t, vTmp );
				m3f_VecAdd( ent->position, ent->position, vOverflow );

				// do response
				vTmp[X_AXIS] = ent->position[X_AXIS];
				vTmp[Y_AXIS] = bbox->hibounds[Y_AXIS];
				vTmp[Z_AXIS] = bbox->hibounds[Z_AXIS];
				ei->col_Respond( ent, vTmp );
				return;
			}

			v2Delta[0] = ent->position[Y_AXIS] - bbox->hibounds[Y_AXIS];
			v2Delta[1] = ent->position[Z_AXIS] - bbox->lobounds[Z_AXIS];
			fDist = (float)sqrt( v2Delta[0] * v2Delta[0] + v2Delta[1] * v2Delta[1] );
			if( fDist < ent->radius )
			{
				// collision occured, do occlusion
				t = fDist - ent->radius;
				memcpy( vTmp, ent->velocity, sizeof(vec3) );
				m3f_VecUnitize( vTmp );
				m3f_VecScale( vOverflow, t, vTmp );
				m3f_VecAdd( ent->position, ent->position, vOverflow );

				// do response
				vTmp[X_AXIS] = ent->position[X_AXIS];
				vTmp[Y_AXIS] = bbox->hibounds[Y_AXIS];
				vTmp[Z_AXIS] = bbox->lobounds[Z_AXIS];
				ei->col_Respond( ent, vTmp );
				return;
			}

			v2Delta[0] = ent->position[Y_AXIS] - bbox->lobounds[Y_AXIS];
			v2Delta[1] = ent->position[Z_AXIS] - bbox->hibounds[Z_AXIS];
			fDist = (float)sqrt( v2Delta[0] * v2Delta[0] + v2Delta[1] * v2Delta[1] );
			if( fDist < ent->radius )
			{
				// collision occured, do occlusion
				t = fDist - ent->radius;
				memcpy( vTmp, ent->velocity, sizeof(vec3) );
				m3f_VecUnitize( vTmp );
				m3f_VecScale( vOverflow, t, vTmp );
				m3f_VecAdd( ent->position, ent->position, vOverflow );

				// do response
				vTmp[X_AXIS] = ent->position[X_AXIS];
				vTmp[Y_AXIS] = bbox->lobounds[Y_AXIS];
				vTmp[Z_AXIS] = bbox->hibounds[Z_AXIS];
				ei->col_Respond( ent, vTmp );
				return;
			}
		}

		// X-Y axis
		else if( (ent->position[Z_AXIS] > bbox->lobounds[Z_AXIS]) && (ent->position[Z_AXIS] < bbox->hibounds[Z_AXIS]) )
		{
			// this gives us 4 corners to compare in x-y space with
			v2Delta[0] = ent->position[X_AXIS] - bbox->lobounds[X_AXIS];
			v2Delta[1] = ent->position[Y_AXIS] - bbox->lobounds[Y_AXIS];
			fDist = (float)sqrt( v2Delta[0] * v2Delta[0] + v2Delta[1] * v2Delta[1] );
			if( fDist < ent->radius )
			{
				// collision occured, do occlusion
				t = fDist - ent->radius;
				memcpy( vTmp, ent->velocity, sizeof(vec3) );
				m3f_VecUnitize( vTmp );
				m3f_VecScale( vOverflow, t, vTmp );
				m3f_VecAdd( ent->position, ent->position, vOverflow );

				// do response
				vTmp[X_AXIS] = bbox->lobounds[X_AXIS];
				vTmp[Y_AXIS] = bbox->lobounds[Y_AXIS];
				vTmp[Z_AXIS] = ent->position[Z_AXIS];
				ei->col_Respond( ent, vTmp );
				return;
			}

			v2Delta[0] = ent->position[X_AXIS] - bbox->hibounds[X_AXIS];
			v2Delta[1] = ent->position[Y_AXIS] - bbox->hibounds[Y_AXIS];
			fDist = (float)sqrt( v2Delta[0] * v2Delta[0] + v2Delta[1] * v2Delta[1] );
			if( fDist < ent->radius )
			{
				// collision occured, do occlusion
				t = fDist - ent->radius;
				memcpy( vTmp, ent->velocity, sizeof(vec3) );
				m3f_VecUnitize( vTmp );
				m3f_VecScale( vOverflow, t, vTmp );
				m3f_VecAdd( ent->position, ent->position, vOverflow );

				// do response
				vTmp[X_AXIS] = bbox->hibounds[X_AXIS];
				vTmp[Y_AXIS] = bbox->hibounds[Y_AXIS];
				vTmp[Z_AXIS] = ent->position[Z_AXIS];
				ei->col_Respond( ent, vTmp );
				return;
			}

			v2Delta[0] = ent->position[X_AXIS] - bbox->hibounds[X_AXIS];
			v2Delta[1] = ent->position[Y_AXIS] - bbox->lobounds[Y_AXIS];
			fDist = (float)sqrt( v2Delta[0] * v2Delta[0] + v2Delta[1] * v2Delta[1] );
			if( fDist < ent->radius )
			{
				// collision occured, do occlusion
				t = fDist - ent->radius;
				memcpy( vTmp, ent->velocity, sizeof(vec3) );
				m3f_VecUnitize( vTmp );
				m3f_VecScale( vOverflow, t, vTmp );
				m3f_VecAdd( ent->position, ent->position, vOverflow );

				// do response
				vTmp[X_AXIS] = bbox->hibounds[X_AXIS];
				vTmp[Y_AXIS] = bbox->lobounds[Y_AXIS];
				vTmp[Z_AXIS] = ent->position[Z_AXIS];
				ei->col_Respond( ent, vTmp );
				return;
			}

			v2Delta[0] = ent->position[X_AXIS] - bbox->lobounds[X_AXIS];
			v2Delta[1] = ent->position[Y_AXIS] - bbox->hibounds[Y_AXIS];
			fDist = (float)sqrt( v2Delta[0] * v2Delta[0] + v2Delta[1] * v2Delta[1] );
			if( fDist < ent->radius )
			{
				// collision occured, do occlusion
				t = fDist - ent->radius;
				memcpy( vTmp, ent->velocity, sizeof(vec3) );
				m3f_VecUnitize( vTmp );
				m3f_VecScale( vOverflow, t, vTmp );
				m3f_VecAdd( ent->position, ent->position, vOverflow );

				// do response
				vTmp[X_AXIS] = bbox->lobounds[X_AXIS];
				vTmp[Y_AXIS] = bbox->hibounds[Y_AXIS];
				vTmp[Z_AXIS] = ent->position[Z_AXIS];
				ei->col_Respond( ent, vTmp );
				return;
			}
		}

		else
		{
			vec3 v3Delta;
			
			/// CORNERS ///

			// 0 = 000
			if( 	(ent->position[X_AXIS] > bbox->lobounds[X_AXIS]) && 
				(ent->position[Y_AXIS] > bbox->lobounds[Y_AXIS]) && 
				(ent->position[Z_AXIS] > bbox->lobounds[Z_AXIS]) )
			{
				v3Delta[X_AXIS] = ent->position[X_AXIS] - bbox->lobounds[X_AXIS];
				v3Delta[Y_AXIS] = ent->position[Y_AXIS] - bbox->lobounds[Y_AXIS];
				v3Delta[Z_AXIS] = ent->position[Z_AXIS] - bbox->lobounds[Z_AXIS];
				fDist = (float)sqrt( v3Delta[0] * v3Delta[0] + v3Delta[1] * v3Delta[1] + v3Delta[2] * v3Delta[2] );
				if( fDist < ent->radius )
				{
					// collision occured, occlude object
					t = fDist - ent->radius;
					memcpy( vTmp, ent->velocity, sizeof(vec3) );
					m3f_VecUnitize( vTmp );
					m3f_VecScale( vOverflow, t, vTmp );
					m3f_VecAdd( ent->position, ent->position, vOverflow );

					// do the response
					ei->col_Respond( ent, bbox->lobounds );
				}
			}

			// 1 = 001
			else if( 	(ent->position[X_AXIS] > bbox->lobounds[X_AXIS]) && 
					(ent->position[Y_AXIS] > bbox->lobounds[Y_AXIS]) && 
					(ent->position[Z_AXIS] > bbox->hibounds[Z_AXIS]) )
			{
				v3Delta[X_AXIS] = ent->position[X_AXIS] - bbox->lobounds[X_AXIS];
				v3Delta[Y_AXIS] = ent->position[Y_AXIS] - bbox->lobounds[Y_AXIS];
				v3Delta[Z_AXIS] = ent->position[Z_AXIS] - bbox->hibounds[Z_AXIS];
				fDist = (float)sqrt( v3Delta[0] * v3Delta[0] + v3Delta[1] * v3Delta[1] + v3Delta[2] * v3Delta[2] );
				if( fDist < ent->radius )
				{
					// collision occured, occlude object
					t = fDist - ent->radius;
					memcpy( vTmp, ent->velocity, sizeof(vec3) );
					m3f_VecUnitize( vTmp );
					m3f_VecScale( vOverflow, t, vTmp );
					m3f_VecAdd( ent->position, ent->position, vOverflow );

					// do the response
					vTmp[X_AXIS] = bbox->lobounds[X_AXIS];
					vTmp[Y_AXIS] = bbox->lobounds[Y_AXIS];
					vTmp[Z_AXIS] = bbox->hibounds[Z_AXIS];
					ei->col_Respond( ent, vTmp );
				}
			}

			// 2 = 010
			else if( 	(ent->position[X_AXIS] > bbox->lobounds[X_AXIS]) && 
					(ent->position[Y_AXIS] > bbox->hibounds[Y_AXIS]) && 
					(ent->position[Z_AXIS] > bbox->lobounds[Z_AXIS]) )
			{
				v3Delta[X_AXIS] = ent->position[X_AXIS] - bbox->lobounds[X_AXIS];
				v3Delta[Y_AXIS] = ent->position[Y_AXIS] - bbox->hibounds[Y_AXIS];
				v3Delta[Z_AXIS] = ent->position[Z_AXIS] - bbox->lobounds[Z_AXIS];
				fDist = (float)sqrt( v3Delta[0] * v3Delta[0] + v3Delta[1] * v3Delta[1] + v3Delta[2] * v3Delta[2] );
				if( fDist < ent->radius )
				{
					// collision occured, occlude object
					t = fDist - ent->radius;
					memcpy( vTmp, ent->velocity, sizeof(vec3) );
					m3f_VecUnitize( vTmp );
					m3f_VecScale( vOverflow, t, vTmp );
					m3f_VecAdd( ent->position, ent->position, vOverflow );

					// do the response
					vTmp[X_AXIS] = bbox->lobounds[X_AXIS];
					vTmp[Y_AXIS] = bbox->hibounds[Y_AXIS];
					vTmp[Z_AXIS] = bbox->lobounds[Z_AXIS];
					ei->col_Respond( ent, vTmp );
				}
			}

			// 3 = 011
			else if( 	(ent->position[X_AXIS] > bbox->lobounds[X_AXIS]) && 
					(ent->position[Y_AXIS] > bbox->hibounds[Y_AXIS]) && 
					(ent->position[Z_AXIS] > bbox->hibounds[Z_AXIS]) )
			{
				v3Delta[X_AXIS] = ent->position[X_AXIS] - bbox->lobounds[X_AXIS];
				v3Delta[Y_AXIS] = ent->position[Y_AXIS] - bbox->hibounds[Y_AXIS];
				v3Delta[Z_AXIS] = ent->position[Z_AXIS] - bbox->hibounds[Z_AXIS];
				fDist = (float)sqrt( v3Delta[0] * v3Delta[0] + v3Delta[1] * v3Delta[1] + v3Delta[2] * v3Delta[2] );
				if( fDist < ent->radius )
				{
					// collision occured, occlude object
					t = fDist - ent->radius;
					memcpy( vTmp, ent->velocity, sizeof(vec3) );
					m3f_VecUnitize( vTmp );
					m3f_VecScale( vOverflow, t, vTmp );
					m3f_VecAdd( ent->position, ent->position, vOverflow );

					// do the response
					vTmp[X_AXIS] = bbox->lobounds[X_AXIS];
					vTmp[Y_AXIS] = bbox->hibounds[Y_AXIS];
					vTmp[Z_AXIS] = bbox->hibounds[Z_AXIS];
					ei->col_Respond( ent, vTmp );
				}
			}

			// 4 = 100
			else if(	(ent->position[X_AXIS] > bbox->hibounds[X_AXIS]) && 
					(ent->position[Y_AXIS] > bbox->lobounds[Y_AXIS]) && 
					(ent->position[Z_AXIS] > bbox->lobounds[Z_AXIS]) )
			{
				v3Delta[X_AXIS] = ent->position[X_AXIS] - bbox->hibounds[X_AXIS];
				v3Delta[Y_AXIS] = ent->position[Y_AXIS] - bbox->lobounds[Y_AXIS];
				v3Delta[Z_AXIS] = ent->position[Z_AXIS] - bbox->lobounds[Z_AXIS];
				fDist = (float)sqrt( v3Delta[0] * v3Delta[0] + v3Delta[1] * v3Delta[1] + v3Delta[2] * v3Delta[2] );
				if( fDist < ent->radius )
				{
					// collision occured, occlude object
					t = fDist - ent->radius;
					memcpy( vTmp, ent->velocity, sizeof(vec3) );
					m3f_VecUnitize( vTmp );
					m3f_VecScale( vOverflow, t, vTmp );
					m3f_VecAdd( ent->position, ent->position, vOverflow );

					// do the response
					vTmp[X_AXIS] = bbox->hibounds[X_AXIS];
					vTmp[Y_AXIS] = bbox->lobounds[Y_AXIS];
					vTmp[Z_AXIS] = bbox->lobounds[Z_AXIS];
					ei->col_Respond( ent, vTmp );
				}
			}

			// 5 = 101
			else if(	(ent->position[X_AXIS] > bbox->hibounds[X_AXIS]) && 
					(ent->position[Y_AXIS] > bbox->lobounds[Y_AXIS]) && 
					(ent->position[Z_AXIS] > bbox->hibounds[Z_AXIS]) )
			{
				v3Delta[X_AXIS] = ent->position[X_AXIS] - bbox->hibounds[X_AXIS];
				v3Delta[Y_AXIS] = ent->position[Y_AXIS] - bbox->lobounds[Y_AXIS];
				v3Delta[Z_AXIS] = ent->position[Z_AXIS] - bbox->hibounds[Z_AXIS];
				fDist = (float)sqrt( v3Delta[0] * v3Delta[0] + v3Delta[1] * v3Delta[1] + v3Delta[2] * v3Delta[2] );
				if( fDist < ent->radius )
				{
					// collision occured, occlude object
					t = fDist - ent->radius;
					memcpy( vTmp, ent->velocity, sizeof(vec3) );
					m3f_VecUnitize( vTmp );
					m3f_VecScale( vOverflow, t, vTmp );
					m3f_VecAdd( ent->position, ent->position, vOverflow );

					// do the response
					vTmp[X_AXIS] = bbox->hibounds[X_AXIS];
					vTmp[Y_AXIS] = bbox->lobounds[Y_AXIS];
					vTmp[Z_AXIS] = bbox->hibounds[Z_AXIS];
					ei->col_Respond( ent, vTmp );
				}
			}

			// 6 = 110
			else if(	(ent->position[X_AXIS] > bbox->hibounds[X_AXIS]) && 
					(ent->position[Y_AXIS] > bbox->hibounds[Y_AXIS]) && 
					(ent->position[Z_AXIS] > bbox->lobounds[Z_AXIS]) )
			{
				v3Delta[X_AXIS] = ent->position[X_AXIS] - bbox->hibounds[X_AXIS];
				v3Delta[Y_AXIS] = ent->position[Y_AXIS] - bbox->hibounds[Y_AXIS];
				v3Delta[Z_AXIS] = ent->position[Z_AXIS] - bbox->lobounds[Z_AXIS];
				fDist = (float)sqrt( v3Delta[0] * v3Delta[0] + v3Delta[1] * v3Delta[1] + v3Delta[2] * v3Delta[2] );
				if( fDist < ent->radius )
				{
					// collision occured, occlude object
					t = fDist - ent->radius;
					memcpy( vTmp, ent->velocity, sizeof(vec3) );
					m3f_VecUnitize( vTmp );
					m3f_VecScale( vOverflow, t, vTmp );
					m3f_VecAdd( ent->position, ent->position, vOverflow );

					// do the response
					vTmp[X_AXIS] = bbox->hibounds[X_AXIS];
					vTmp[Y_AXIS] = bbox->hibounds[Y_AXIS];
					vTmp[Z_AXIS] = bbox->lobounds[Z_AXIS];
					ei->col_Respond( ent, vTmp );
				}
			}

			// 7 = 111
			else if(	(ent->position[X_AXIS] > bbox->hibounds[X_AXIS]) && 
					(ent->position[Y_AXIS] > bbox->hibounds[Y_AXIS]) && 
					(ent->position[Z_AXIS] > bbox->hibounds[Z_AXIS]) )
			{
				v3Delta[X_AXIS] = ent->position[X_AXIS] - bbox->hibounds[X_AXIS];
				v3Delta[Y_AXIS] = ent->position[Y_AXIS] - bbox->hibounds[Y_AXIS];
				v3Delta[Z_AXIS] = ent->position[Z_AXIS] - bbox->hibounds[Z_AXIS];
				fDist = (float)sqrt( v3Delta[0] * v3Delta[0] + v3Delta[1] * v3Delta[1] + v3Delta[2] * v3Delta[2] );
				if( fDist < ent->radius )
				{
					// collision occured, occlude object
					t = fDist - ent->radius;
					memcpy( vTmp, ent->velocity, sizeof(vec3) );
					m3f_VecUnitize( vTmp );
					m3f_VecScale( vOverflow, t, vTmp );
					m3f_VecAdd( ent->position, ent->position, vOverflow );

					// do the response
					ei->col_Respond( ent, bbox->hibounds );
				}
			}			
		}
	}
}

