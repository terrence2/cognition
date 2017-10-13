// Cognition
// game_background.c
// Created 3/25/03 by Terrence Cole
// Generalized 1/3/04 by TDC

// Includes
/////////////
#include "cog_global.h"

// STRUCTURES
///////////////
typedef struct
{
	float fXDist;
	float fYDist;
	float fZDist;
} corridor_t;

// Definitions
////////////////

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
// of half the size (we'll use a good normalization instead)
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
static corridor_t objCorridor[BG_NUM_SIZES] =
{
	// A corridor defines an area of particle flow from -Z to +Z.
	// We use this concept to keep small particles close to the
	// game area where they can be seen and to give big particles
	// a wide range of movement to give a perception of great depth.
	{ 4000.0f, 4000.0f, 4000.0f },
	{ 6000.0f, 6000.0f, 6000.0f },
	{ 8000.0f, 8000.0f, 8000.0f },
	{ 10000.0f, 10000.0f, 10000.0f },
	{ 12000.0f, 12000.0f, 12000.0f },
	{ 14000.0f, 14000.0f, 14000.0f },
	{ 16000.0f, 16000.0f, 16000.0f },
	{ 18000.0f, 18000.0f, 18000.0f },
	{ 20000.0f, 20000.0f, 20000.0f }
};
static float fMaxDepth = 20000.0f;

static char objTable[BG_NUM_SIZES][8][128] =
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
int bg_Initialize(void);
void bg_Terminate(void);
void bg_Animate();
*/

// Local Prototypes
/////////////////////
static int bg_Init_GetLight();
static int bg_Init_GetEnts();
static void bg_Ani_Particle( entity_t *ent, int size );
static void bg_OccludeObject( entity_t *ent, aabb_t *bbox );
static void bg_EntHitsWorld( entity_t *ent, vec3 vLoc, vec3 vNorm );


// Local Variables
////////////////////
static entity_t **bgArray[BG_NUM_SIZES];
static int sizeCnt[BG_NUM_SIZES];
static int bgNumEntities = 0;
static light_t *bgLight1 = NULL;

// *********** FUNCTIONALITY ***********
/* ------------
bg_Initialize - allocates storage for background entities, get background entities, spawns all necessary structures to display them
			- and other draw related necessities not performed by the player
------------ */
int bg_Initialize(void)
{
	con_Print( "\n<BLUE>Initializing</BLUE> Background..." );
	eng_LoadingFrame();
	
	// get the lighting
	if( !bg_Init_GetLight() ) return 0;
	
	// get the ents
	if( !bg_Init_GetEnts() ) return 0;

	return 1;
}

/* ------------
bg_Init_GetLight
------------ */
static int bg_Init_GetLight()
{
	vec4 vLtPos1 = { 0.7f, 0.7f, 1.0f, 0.0f }; 
	vec4 vLtAmb = { 0.25f, 0.25f, 0.22f, 1.0f };
	vec4 vLtDif1 = { 1.0f, 1.0f, 0.93f, 1.0f };
	vec4 vLtSpec = { 1.0f, 1.0f, 1.0f, 1.0f };
	vec3 vLtSptDir = { 0.0f, 0.0f, 0.0f };

	// spawn a light for the scene
	bgLight1 = l_Spawn( LIGHT_DIRECTIONAL, vLtPos1, vLtAmb, vLtDif1, vLtSpec, 0, 45, vLtSptDir, 1.0f, 0.0f, 0.0f );
	if( bgLight1 == NULL )
	{
		con_Print( "Client Background Error on Init:  Light Spawn returned a NULL light." );
		return 0;
	}
	return 1;
}

/* ------------
bg_Init_GetEnts
------------ */
static int bg_Init_GetEnts()
{
	float fBGComplexity;
	int a, b;
	int strCnt[BG_NUM_SIZES];
	int sizeTotal;
	
	// get the expected scene complexity
	fBGComplexity = var_GetFloatFromName( "cl_background_complexity" );

	// get the count of each object size range
	sizeCnt[BG_SIZE_2_5] = (int)(((float)BG_SCALE_RAW_2_5 / (float)BG_SCALE_UNITY) * fBGComplexity);
	sizeCnt[BG_SIZE_5] = (int)(((float)BG_SCALE_RAW_5 / (float)BG_SCALE_UNITY) * fBGComplexity);
	sizeCnt[BG_SIZE_10] = (int)(((float)BG_SCALE_RAW_10 / (float)BG_SCALE_UNITY) * fBGComplexity);
	sizeCnt[BG_SIZE_25] = (int)(((float)BG_SCALE_RAW_25 / (float)BG_SCALE_UNITY) * fBGComplexity);
	sizeCnt[BG_SIZE_50] = (int)(((float)BG_SCALE_RAW_50 / (float)BG_SCALE_UNITY) * fBGComplexity);
	sizeCnt[BG_SIZE_100] = (int)(((float)BG_SCALE_RAW_100 / (float)BG_SCALE_UNITY) * fBGComplexity);
	sizeCnt[BG_SIZE_200] = (int)(((float)BG_SCALE_RAW_200 / (float)BG_SCALE_UNITY) * fBGComplexity);
	sizeCnt[BG_SIZE_400] = (int)(((float)BG_SCALE_RAW_400 / (float)BG_SCALE_UNITY) * fBGComplexity);
//	sizeCnt[BG_SIZE_1000] = (int)(((float)BG_SCALE_RAW_1000 / (float)BG_SCALE_UNITY) * fBGComplexity);
	sizeCnt[BG_SIZE_1000] = 0; // these look like crap

	// accumulate to sizeTotal
	sizeTotal = 0;
	for( a = 0 ; a < BG_NUM_SIZES ; a++ ) {	sizeTotal += sizeCnt[a]; }
	bgNumEntities = sizeTotal;
	con_Print( "<BLUE>Creating</BLUE> %d Particles...", bgNumEntities );
	eng_LoadingFrame();

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

	// allocate entities
	for( a = 0 ; a < BG_NUM_SIZES ; a++ )
	{
		// loading message
		con_Print( "<BLUE>Creating</BLUE> %d Size %d Particles...", sizeCnt[a], a );
		eng_LoadingFrame();

		// allocate
		bgArray[a] = (entity_t**)mem_alloc( sizeof(entity_t*) * sizeCnt[a]);
		if( bgArray[a] == NULL )
		{
			con_Print( "Client Background Error on Init:  Memory allocate failed on size %d for %d entity pointers.", a, sizeCnt[a] );
			return 0;
		}
		memset( bgArray[a], 0, sizeof(entity_t*) * sizeCnt[a] );

		// create ents
		for( b = 0 ; b < sizeCnt[a] ; b++ )
		{
			vec3 vPos;
			vec3 vAng;
			char cNameBuffer[1024];
			int iModelRandom;

			// get a name
			snprintf( cNameBuffer, 1024, "clBackground(%d,%d)", a, b  );

			// get a random position
			vPos[X_AXIS] = rng_Float( -objCorridor[a].fXDist, objCorridor[a].fXDist ); 
			vPos[Y_AXIS] = rng_Float( -objCorridor[a].fYDist, objCorridor[a].fYDist ); 
			vPos[Z_AXIS] = rng_Float( -objCorridor[a].fZDist, objCorridor[a].fZDist ); 

			// set a random rotation
			vAng[0] = rng_Float( 0.0f, 360.0f );
			vAng[1] = rng_Float( 0.0f, 360.0f );

			// get a random string
			iModelRandom = rng_Long( 0, strCnt[a] );

			// this just takes too long
			//con_Print( "\t<BLUE>Loading</BLUE> Model <YELLOW>(%d,%d)</YELLOW> from <ORANGE>\"%s\"</ORANGE>.", a, b, objTable[a][iModelRandom] );
			
			// spawn the ent
			bgArray[a][b] = ent_Spawn( cNameBuffer, 0, objTable[a][iModelRandom], vPos, vAng );
			if( bgArray[a][b] == NULL )
			{
				con_Print( "Client Background Error on Init: Entity Load Failed for Entity %d / %d", b, sizeCnt[a] );
				return 0;
			}

			// set a random rotation rate
			bgArray[a][b]->vAngVel[0] = rng_Float( 0.0f, 200.0f );
			bgArray[a][b]->vAngVel[1] = rng_Float( 0.0f, 200.0f );

			// set a meandering direction for movement
			bgArray[a][b]->vVel[0] = rng_Float( -50.0f, 50.0f );
			bgArray[a][b]->vVel[1] = rng_Float( -50.0f, 50.0f );
			bgArray[a][b]->vVel[2] = rng_Float( -20.0f, 400.0f );

			eng_LoadingFrame();
		}
	}

	return 1;
}
	
/* ------------
bg_Terminate - step out of whatever Initialize did.
------------ */
void bg_Terminate(void)
{
	int a, b;
	
	// kill the scene light
	if( bgLight1 != NULL ) l_Kill( bgLight1 );

	// kill the entities
	for( a = 0 ; a < BG_NUM_SIZES ; a++ )
	{
		if( bgArray[a] != NULL )
		{
			for( b = 0 ; b < sizeCnt[a] ; b++ )
			{
				if( bgArray[a][b] != NULL )
				{
					ent_Remove( bgArray[a][b] );
				}
			}
			SAFE_RELEASE( bgArray[a] );
		}
	}
}

/* ------------
bg_Animate - handle stately animation of the background
------------ */
void bg_Animate()
{
	int a, b;
	
	// iterate the particles
	for( a = 0 ; a < BG_NUM_SIZES ; a++ )
	{
		for( b = 0 ; b < sizeCnt[a] ; b++ )
		{
			if( bgArray[a] != NULL )
			{
				bg_Ani_Particle( bgArray[a][b], a );
			}
		}
	}
}

/* ------------
bg_Ani_Particle
------------ */
static void bg_Ani_Particle( entity_t *ent, int size )
{
	aabb_t *box;
	float dt = state.delta_time / 1000.0f;

	if( !ent ) return;
	
	// use the acceleration to change the velocity
	// v2 = v1 + a * t
	ent->vVel[X_AXIS] += ent->vAccel[X_AXIS] * dt;
	ent->vVel[Y_AXIS] += ent->vAccel[Y_AXIS] * dt;
	ent->vVel[Z_AXIS] += ent->vAccel[Z_AXIS] * dt;

	// use the vVel to calculate a change in position
	ent->vPos[X_AXIS] += ent->vVel[X_AXIS] * dt;
	ent->vPos[Y_AXIS] += ent->vVel[Y_AXIS] * dt;
	ent->vPos[Z_AXIS] += ent->vVel[Z_AXIS] * dt;

	// use the w to calculate a new vRot
	ent->vRot[PITCH] += ent->vAngVel[PITCH] * dt;
	ent->vRot[YAW] += ent->vAngVel[YAW] * dt;

	// cap the vRot on X
	while( ent->vRot[PITCH] >= 360.0 ) ent->vRot[PITCH] -= 360.0f;
	while( ent->vRot[PITCH] < 0.0f ) ent->vRot[PITCH] += 360.0f;

	// cap the vRot on Y
	while( ent->vRot[YAW] >= 360.0 ) ent->vRot[YAW] -= 360.0f;
	while( ent->vRot[YAW] < 0.0f ) ent->vRot[YAW] += 360.0f;



	// do the keyframe animation
	if( ent->mod->moo->num_frames > 1 )
	{
		// frame sub part delta is the ( time elapsed / time per frame ) => time elapsed * frames per millisecond
		ent->mod->moo->sub_frame += (float)((double)(state.delta_time) * ent->mod->moo->frames_per_millisecond * .01);
		while( ent->mod->moo->sub_frame > 1.00f ) 
		{
			ent->mod->moo->sub_frame -= 1.00f;
			ent->mod->moo->cur_frame += 1;
		}
		while( ent->mod->moo->cur_frame >= ent->mod->moo->num_frames )
		{
			ent->mod->moo->cur_frame -= ent->mod->moo->num_frames;
		}
	}

	// get the current world box
	box = d_GetWorldBounds();

	// if we are in the world, get us out, NOW
	if( m_PointInBox( ent->vPos, box ) )
	{
		// move us out now
		ent->vPos[Z_AXIS] = box->hibounds[Z_AXIS] + ent->fRadius;

		// make sure we keep going away
		if( ent->vVel[Z_AXIS] < 0 ) ent->vVel[Z_AXIS] *= -1;
	}

	// keep background objects out of the world
	bg_OccludeObject( ent, box );

	// check to see if any of the particles leave, if so, warp them to the other side
	if( ent->vPos[X_AXIS] < -objCorridor[size].fXDist ) ent->vPos[X_AXIS] += (2 * objCorridor[size].fXDist);
	if( ent->vPos[Y_AXIS] < -objCorridor[size].fYDist ) ent->vPos[Y_AXIS] += (2 * objCorridor[size].fYDist);
	if( ent->vPos[Z_AXIS] < -objCorridor[size].fZDist ) ent->vPos[Z_AXIS] += (2 * objCorridor[size].fZDist);

	if( ent->vPos[X_AXIS] > objCorridor[size].fXDist ) ent->vPos[X_AXIS] -= (2 * objCorridor[size].fXDist);
	if( ent->vPos[Y_AXIS] > objCorridor[size].fYDist ) ent->vPos[Y_AXIS] -= (2 * objCorridor[size].fYDist);
	if( ent->vPos[Z_AXIS] > objCorridor[size].fZDist ) ent->vPos[Z_AXIS] -= (2 * objCorridor[size].fZDist);
/*
	// the complexity of a hit test should be directly proportional to a viewer's perception, use distance
	dist = m3f_VecDistance( plState.self->position, ent->vPos );
	if( dist < 400.0f ) prio = COL_PRIORITY_HIGH;
	else if( dist < 800.0f ) prio = COL_PRIORITY_MEDIUM;
	else if( dist < plState.world->wrldBnds.hibounds[0] ) prio = COL_PRIORITY_LOW;
	else prio = COL_PRIORITY_NONE;

	// N^2 collision, hmmmm....
	for( b = 0 ; b < bgNumEntities ; b++ )
	{
		if( bgArray[b] != NULL && b != a )
		{
			col_CheckEnts( hit, ent, bgArray[b], prio );
		}
	}
*/
}

/* ------------
bg_OcculudeObject - clips ent out of bbox, responds to collisions between bbox and ent
					- ent is represented as ONLY it's bounding sphere
------------ */
static void bg_OccludeObject( entity_t *ent, aabb_t *bbox )
{
	// strategy:  the overlap region is where the sphere is less than one radius away from a plane side
	//			the sphere must be inside or in the overlap region FOR EACH FACE
	//			the collision handles different based on the number of overlap regions [0-3]

	aabb_t bBnds;
	byte axis[3];
	byte cnt = 0;
	vec3 vTmp, vN;

	memcpy( &bBnds, bbox, sizeof(aabb_t) );
	bBnds.lobounds[X_AXIS] -= ent->fRadius;
	bBnds.lobounds[Y_AXIS] -= ent->fRadius;
	bBnds.lobounds[Z_AXIS] -= ent->fRadius;
	bBnds.hibounds[X_AXIS] += ent->fRadius;
	bBnds.hibounds[Y_AXIS] += ent->fRadius;
	bBnds.hibounds[Z_AXIS] += ent->fRadius;
	memset( axis, 0, sizeof(byte) * 3 );

/*
	Strategy: Clip on descending levels of object primitive: face, edge, corner
*/

	/// FACES ///

	// Z-axis planes
	if( (ent->vPos[X_AXIS] >= bbox->lobounds[X_AXIS]) &&
		(ent->vPos[X_AXIS] <= bbox->hibounds[X_AXIS]) &&
		(ent->vPos[Y_AXIS] >= bbox->lobounds[Y_AXIS]) &&
		(ent->vPos[Y_AXIS] <= bbox->hibounds[Y_AXIS]) )
	{
		// Z on 0+ and greater
		if( (ent->vPos[Z_AXIS] > 0.0f) && (ent->vPos[Z_AXIS] <= bBnds.hibounds[Z_AXIS]) ) 
		{
			// occlude
			ent->vPos[Z_AXIS] = bBnds.hibounds[Z_AXIS] + 1.0f;
			// compute collision parameters
			vTmp[X_AXIS] = ent->vPos[X_AXIS];
			vTmp[Y_AXIS] = ent->vPos[Y_AXIS];
			vTmp[Z_AXIS] = bbox->hibounds[Z_AXIS];
			vN[0] = 0.0f; vN[1] = 0.0f; vN[2] = 1.0f;
			bg_EntHitsWorld( ent, vTmp, vN );
		}

		// Z on 0 and less
		else if( (ent->vPos[Z_AXIS] <= 0.0f) && (ent->vPos[Z_AXIS] >= bBnds.lobounds[Z_AXIS]) ) 
		{
			ent->vPos[Z_AXIS] = bBnds.lobounds[Z_AXIS] - 1.0f;
			vTmp[X_AXIS] = ent->vPos[X_AXIS];
			vTmp[Y_AXIS] = ent->vPos[Y_AXIS];
			vTmp[Z_AXIS] = bbox->lobounds[Z_AXIS];
			vN[0] = 0.0f; vN[1] = 0.0f; vN[2] = -1.0f;
			bg_EntHitsWorld( ent, vTmp, vN );
		}
	}

	// Y-axis planes
	else if( (ent->vPos[X_AXIS] >= bbox->lobounds[X_AXIS]) &&
		(ent->vPos[X_AXIS] <= bbox->hibounds[X_AXIS]) &&
		(ent->vPos[Z_AXIS] >= bbox->lobounds[Z_AXIS]) &&
		(ent->vPos[Z_AXIS] <= bbox->hibounds[Z_AXIS]) )
	{
		// Y on 0 and less
		if( (ent->vPos[Y_AXIS] <= 0.0f) && (ent->vPos[Y_AXIS] >= bBnds.lobounds[Y_AXIS]) ) 
		{
			ent->vPos[Y_AXIS] = bBnds.lobounds[Y_AXIS] - 1.0f;
			vTmp[X_AXIS] = ent->vPos[X_AXIS];
			vTmp[Y_AXIS] = bbox->lobounds[Y_AXIS];
			vTmp[Z_AXIS] = ent->vPos[Z_AXIS];
			vN[0] = 0.0f; vN[1] = -1.0f; vN[2] = 0.0f;
			bg_EntHitsWorld( ent, vTmp, vN );
		}

		// Y on 0+ and greater
		else if( (ent->vPos[Y_AXIS] > 0.0f) && (ent->vPos[Y_AXIS] <= bBnds.hibounds[Y_AXIS]) ) 
		{
			ent->vPos[Y_AXIS] = bBnds.hibounds[Y_AXIS] + 1.0f;
			vTmp[X_AXIS] = ent->vPos[X_AXIS];
			vTmp[Y_AXIS] = bbox->hibounds[Y_AXIS];
			vTmp[Z_AXIS] = ent->vPos[Z_AXIS];
			vN[0] = 0.0f; vN[1] = 1.0f; vN[2] = 0.0f;
			bg_EntHitsWorld( ent, vTmp, vN );
		}
	}

	// X-axis planes
	else 	if( (ent->vPos[Z_AXIS] >= bbox->lobounds[Z_AXIS]) &&
			(ent->vPos[Z_AXIS] <= bbox->hibounds[Z_AXIS]) &&
			(ent->vPos[Y_AXIS] >= bbox->lobounds[Y_AXIS]) &&
			(ent->vPos[Y_AXIS] <= bbox->hibounds[Y_AXIS]) )
	{
		// X on 0 and less
		if( (ent->vPos[X_AXIS] <= 0.0f) && (ent->vPos[X_AXIS] >= bBnds.lobounds[X_AXIS]) ) 
		{
			ent->vPos[X_AXIS] = bBnds.lobounds[X_AXIS] - 1.0f;
			vTmp[X_AXIS] = bbox->lobounds[X_AXIS];
			vTmp[Y_AXIS] = ent->vPos[Y_AXIS];
			vTmp[Z_AXIS] = ent->vPos[Z_AXIS];
			vN[0] = -1.0f; vN[1] = 0.0f; vN[2] = 0.0f;
			bg_EntHitsWorld( ent, vTmp, vN );
		}
	
		// X on 0+ and greater
		else if( (ent->vPos[X_AXIS] > 0.0f) && (ent->vPos[X_AXIS] <= bBnds.hibounds[X_AXIS]) ) 
		{
			ent->vPos[X_AXIS] = bBnds.hibounds[X_AXIS] + 1.0f;
			vTmp[X_AXIS] = bbox->hibounds[X_AXIS];
			vTmp[Y_AXIS] = ent->vPos[Y_AXIS];
			vTmp[Z_AXIS] = ent->vPos[Z_AXIS];
			vN[0] = 1.0f; vN[1] = 0.0f; vN[2] = 0.0f;
			bg_EntHitsWorld( ent, vTmp, vN );
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
		if( (ent->vPos[Y_AXIS] > bbox->lobounds[Y_AXIS]) && (ent->vPos[Y_AXIS] < bbox->hibounds[Y_AXIS]) )
		{
			// this gives us 4 corners to compare in x-z space with
			v2Delta[0] = ent->vPos[X_AXIS] - bbox->lobounds[X_AXIS];
			v2Delta[1] = ent->vPos[Z_AXIS] - bbox->lobounds[Z_AXIS];
			fDist = (float)sqrt( v2Delta[0] * v2Delta[0] + v2Delta[1] * v2Delta[1] );
			if( fDist < ent->fRadius )
			{
				// collision occured, do occlusion
				t = fDist - ent->fRadius;
				memcpy( vTmp, ent->vVel, sizeof(vec3) );
				m3f_VecUnitize( vTmp );
				m3f_VecScale( vOverflow, t, vTmp );
				m3f_VecAdd( ent->vPos, ent->vPos, vOverflow );

				// do response
				vTmp[X_AXIS] = bbox->lobounds[X_AXIS];
				vTmp[Y_AXIS] = ent->vPos[Y_AXIS];
				vTmp[Z_AXIS] = bbox->lobounds[Z_AXIS];
				vN[0] = -1.0f; vN[1] = 0.0f; vN[2] = -1.0f;
				bg_EntHitsWorld( ent, vTmp, vN );
				return;
			}

			v2Delta[0] = ent->vPos[X_AXIS] - bbox->hibounds[X_AXIS];
			v2Delta[1] = ent->vPos[Z_AXIS] - bbox->hibounds[Z_AXIS];
			fDist = (float)sqrt( v2Delta[0] * v2Delta[0] + v2Delta[1] * v2Delta[1] );
			if( fDist < ent->fRadius )
			{
				// collision occured, do occlusion
				t = fDist - ent->fRadius;
				memcpy( vTmp, ent->vVel, sizeof(vec3) );
				m3f_VecUnitize( vTmp );
				m3f_VecScale( vOverflow, t, vTmp );
				m3f_VecAdd( ent->vPos, ent->vPos, vOverflow );

				// do response
				vTmp[X_AXIS] = bbox->hibounds[X_AXIS];
				vTmp[Y_AXIS] = ent->vPos[Y_AXIS];
				vTmp[Z_AXIS] = bbox->hibounds[Z_AXIS];
				vN[0] = 1.0f; vN[1] = 0.0f; vN[2] = 1.0f;
				bg_EntHitsWorld( ent, vTmp, vN );
				return;
			}

			v2Delta[0] = ent->vPos[X_AXIS] - bbox->hibounds[X_AXIS];
			v2Delta[1] = ent->vPos[Z_AXIS] - bbox->lobounds[Z_AXIS];
			fDist = (float)sqrt( v2Delta[0] * v2Delta[0] + v2Delta[1] * v2Delta[1] );
			if( fDist < ent->fRadius )
			{
				// collision occured, do occlusion
				t = fDist - ent->fRadius;
				memcpy( vTmp, ent->vVel, sizeof(vec3) );
				m3f_VecUnitize( vTmp );
				m3f_VecScale( vOverflow, t, vTmp );
				m3f_VecAdd( ent->vPos, ent->vPos, vOverflow );

				// do response
				vTmp[X_AXIS] = bbox->hibounds[X_AXIS];
				vTmp[Y_AXIS] = ent->vPos[Y_AXIS];
				vTmp[Z_AXIS] = bbox->lobounds[Z_AXIS];
				vN[0] = 1.0f; vN[1] = 0.0f; vN[2] = -1.0f;
				bg_EntHitsWorld( ent, vTmp, vN );
				return;
			}

			v2Delta[0] = ent->vPos[X_AXIS] - bbox->lobounds[X_AXIS];
			v2Delta[1] = ent->vPos[Z_AXIS] - bbox->hibounds[Z_AXIS];
			fDist = (float)sqrt( v2Delta[0] * v2Delta[0] + v2Delta[1] * v2Delta[1] );
			if( fDist < ent->fRadius )
			{
				// collision occured, do occlusion
				t = fDist - ent->fRadius;
				memcpy( vTmp, ent->vVel, sizeof(vec3) );
				m3f_VecUnitize( vTmp );
				m3f_VecScale( vOverflow, t, vTmp );
				m3f_VecAdd( ent->vPos, ent->vPos, vOverflow );

				// do response
				vTmp[X_AXIS] = bbox->lobounds[X_AXIS];
				vTmp[Y_AXIS] = ent->vPos[Y_AXIS];
				vTmp[Z_AXIS] = bbox->hibounds[Z_AXIS];
				vN[0] = -1.0f; vN[1] = 0.0f; vN[2] = 1.0f;
				bg_EntHitsWorld( ent, vTmp, vN );
				return;
			}
		}

		// Y-Z axis
		else if( (ent->vPos[X_AXIS] > bbox->lobounds[X_AXIS]) && (ent->vPos[X_AXIS] < bbox->hibounds[X_AXIS]) )
		{
			// this gives us 4 corners to compare in y-z space with
			v2Delta[0] = ent->vPos[Y_AXIS] - bbox->lobounds[Y_AXIS];
			v2Delta[1] = ent->vPos[Z_AXIS] - bbox->lobounds[Z_AXIS];
			fDist = (float)sqrt( v2Delta[0] * v2Delta[0] + v2Delta[1] * v2Delta[1] );
			if( fDist < ent->fRadius )
			{
				// collision occured, do occlusion
				t = fDist - ent->fRadius;
				memcpy( vTmp, ent->vVel, sizeof(vec3) );
				m3f_VecUnitize( vTmp );
				m3f_VecScale( vOverflow, t, vTmp );
				m3f_VecAdd( ent->vPos, ent->vPos, vOverflow );

				// do response
				vTmp[X_AXIS] = ent->vPos[X_AXIS];
				vTmp[Y_AXIS] = bbox->lobounds[Y_AXIS];
				vTmp[Z_AXIS] = bbox->lobounds[Z_AXIS];
				vN[0] = 0.0f; vN[1] = -1.0f; vN[2] = -1.0f;
				bg_EntHitsWorld( ent, vTmp, vN );
				return;
			}

			v2Delta[0] = ent->vPos[Y_AXIS] - bbox->hibounds[Y_AXIS];
			v2Delta[1] = ent->vPos[Z_AXIS] - bbox->hibounds[Z_AXIS];
			fDist = (float)sqrt( v2Delta[0] * v2Delta[0] + v2Delta[1] * v2Delta[1] );
			if( fDist < ent->fRadius )
			{
				// collision occured, do occlusion
				t = fDist - ent->fRadius;
				memcpy( vTmp, ent->vVel, sizeof(vec3) );
				m3f_VecUnitize( vTmp );
				m3f_VecScale( vOverflow, t, vTmp );
				m3f_VecAdd( ent->vPos, ent->vPos, vOverflow );

				// do response
				vTmp[X_AXIS] = ent->vPos[X_AXIS];
				vTmp[Y_AXIS] = bbox->hibounds[Y_AXIS];
				vTmp[Z_AXIS] = bbox->hibounds[Z_AXIS];
				vN[0] = 0.0f; vN[1] = 1.0f; vN[2] = 1.0f;
				bg_EntHitsWorld( ent, vTmp, vN );
				return;
			}

			v2Delta[0] = ent->vPos[Y_AXIS] - bbox->hibounds[Y_AXIS];
			v2Delta[1] = ent->vPos[Z_AXIS] - bbox->lobounds[Z_AXIS];
			fDist = (float)sqrt( v2Delta[0] * v2Delta[0] + v2Delta[1] * v2Delta[1] );
			if( fDist < ent->fRadius )
			{
				// collision occured, do occlusion
				t = fDist - ent->fRadius;
				memcpy( vTmp, ent->vVel, sizeof(vec3) );
				m3f_VecUnitize( vTmp );
				m3f_VecScale( vOverflow, t, vTmp );
				m3f_VecAdd( ent->vPos, ent->vPos, vOverflow );

				// do response
				vTmp[X_AXIS] = ent->vPos[X_AXIS];
				vTmp[Y_AXIS] = bbox->hibounds[Y_AXIS];
				vTmp[Z_AXIS] = bbox->lobounds[Z_AXIS];
				vN[0] = 0.0f; vN[1] = 1.0f; vN[2] = -1.0f;
				bg_EntHitsWorld( ent, vTmp, vN );
				return;
			}

			v2Delta[0] = ent->vPos[Y_AXIS] - bbox->lobounds[Y_AXIS];
			v2Delta[1] = ent->vPos[Z_AXIS] - bbox->hibounds[Z_AXIS];
			fDist = (float)sqrt( v2Delta[0] * v2Delta[0] + v2Delta[1] * v2Delta[1] );
			if( fDist < ent->fRadius )
			{
				// collision occured, do occlusion
				t = fDist - ent->fRadius;
				memcpy( vTmp, ent->vVel, sizeof(vec3) );
				m3f_VecUnitize( vTmp );
				m3f_VecScale( vOverflow, t, vTmp );
				m3f_VecAdd( ent->vPos, ent->vPos, vOverflow );

				// do response
				vTmp[X_AXIS] = ent->vPos[X_AXIS];
				vTmp[Y_AXIS] = bbox->lobounds[Y_AXIS];
				vTmp[Z_AXIS] = bbox->hibounds[Z_AXIS];
				vN[0] = 0.0f; vN[1] = -1.0f; vN[2] = 1.0f;
				bg_EntHitsWorld( ent, vTmp, vN );
				return;
			}
		}

		// X-Y axis
		else if( (ent->vPos[Z_AXIS] > bbox->lobounds[Z_AXIS]) && (ent->vPos[Z_AXIS] < bbox->hibounds[Z_AXIS]) )
		{
			// this gives us 4 corners to compare in x-y space with
			v2Delta[0] = ent->vPos[X_AXIS] - bbox->lobounds[X_AXIS];
			v2Delta[1] = ent->vPos[Y_AXIS] - bbox->lobounds[Y_AXIS];
			fDist = (float)sqrt( v2Delta[0] * v2Delta[0] + v2Delta[1] * v2Delta[1] );
			if( fDist < ent->fRadius )
			{
				// collision occured, do occlusion
				t = fDist - ent->fRadius;
				memcpy( vTmp, ent->vVel, sizeof(vec3) );
				m3f_VecUnitize( vTmp );
				m3f_VecScale( vOverflow, t, vTmp );
				m3f_VecAdd( ent->vPos, ent->vPos, vOverflow );

				// do response
				vTmp[X_AXIS] = bbox->lobounds[X_AXIS];
				vTmp[Y_AXIS] = bbox->lobounds[Y_AXIS];
				vTmp[Z_AXIS] = ent->vPos[Z_AXIS];
				vN[0] = -1.0f; vN[1] = -1.0f; vN[2] = 0.0f;
				bg_EntHitsWorld( ent, vTmp, vN );
				return;
			}

			v2Delta[0] = ent->vPos[X_AXIS] - bbox->hibounds[X_AXIS];
			v2Delta[1] = ent->vPos[Y_AXIS] - bbox->hibounds[Y_AXIS];
			fDist = (float)sqrt( v2Delta[0] * v2Delta[0] + v2Delta[1] * v2Delta[1] );
			if( fDist < ent->fRadius )
			{
				// collision occured, do occlusion
				t = fDist - ent->fRadius;
				memcpy( vTmp, ent->vVel, sizeof(vec3) );
				m3f_VecUnitize( vTmp );
				m3f_VecScale( vOverflow, t, vTmp );
				m3f_VecAdd( ent->vPos, ent->vPos, vOverflow );

				// do response
				vTmp[X_AXIS] = bbox->hibounds[X_AXIS];
				vTmp[Y_AXIS] = bbox->hibounds[Y_AXIS];
				vTmp[Z_AXIS] = ent->vPos[Z_AXIS];
				vN[0] = 1.0f; vN[1] = 1.0f; vN[2] = 0.0f;
				bg_EntHitsWorld( ent, vTmp, vN );
				return;
			}

			v2Delta[0] = ent->vPos[X_AXIS] - bbox->hibounds[X_AXIS];
			v2Delta[1] = ent->vPos[Y_AXIS] - bbox->lobounds[Y_AXIS];
			fDist = (float)sqrt( v2Delta[0] * v2Delta[0] + v2Delta[1] * v2Delta[1] );
			if( fDist < ent->fRadius )
			{
				// collision occured, do occlusion
				t = fDist - ent->fRadius;
				memcpy( vTmp, ent->vVel, sizeof(vec3) );
				m3f_VecUnitize( vTmp );
				m3f_VecScale( vOverflow, t, vTmp );
				m3f_VecAdd( ent->vPos, ent->vPos, vOverflow );

				// do response
				vTmp[X_AXIS] = bbox->hibounds[X_AXIS];
				vTmp[Y_AXIS] = bbox->lobounds[Y_AXIS];
				vTmp[Z_AXIS] = ent->vPos[Z_AXIS];
				vN[0] = 1.0f; vN[1] = -1.0f; vN[2] = 0.0f;
				bg_EntHitsWorld( ent, vTmp, vN );
				return;
			}

			v2Delta[0] = ent->vPos[X_AXIS] - bbox->lobounds[X_AXIS];
			v2Delta[1] = ent->vPos[Y_AXIS] - bbox->hibounds[Y_AXIS];
			fDist = (float)sqrt( v2Delta[0] * v2Delta[0] + v2Delta[1] * v2Delta[1] );
			if( fDist < ent->fRadius )
			{
				// collision occured, do occlusion
				t = fDist - ent->fRadius;
				memcpy( vTmp, ent->vVel, sizeof(vec3) );
				m3f_VecUnitize( vTmp );
				m3f_VecScale( vOverflow, t, vTmp );
				m3f_VecAdd( ent->vPos, ent->vPos, vOverflow );

				// do response
				vTmp[X_AXIS] = bbox->lobounds[X_AXIS];
				vTmp[Y_AXIS] = bbox->hibounds[Y_AXIS];
				vTmp[Z_AXIS] = ent->vPos[Z_AXIS];
				vN[0] = -1.0f; vN[1] = 1.0f; vN[2] = 0.0f;
				bg_EntHitsWorld( ent, vTmp, vN );
				return;
			}
		}

		else
		{
			vec3 v3Delta;
			
			/// CORNERS ///

			// 0 = 000
			if( 	(ent->vPos[X_AXIS] > bbox->lobounds[X_AXIS]) && 
				(ent->vPos[Y_AXIS] > bbox->lobounds[Y_AXIS]) && 
				(ent->vPos[Z_AXIS] > bbox->lobounds[Z_AXIS]) )
			{
				v3Delta[X_AXIS] = ent->vPos[X_AXIS] - bbox->lobounds[X_AXIS];
				v3Delta[Y_AXIS] = ent->vPos[Y_AXIS] - bbox->lobounds[Y_AXIS];
				v3Delta[Z_AXIS] = ent->vPos[Z_AXIS] - bbox->lobounds[Z_AXIS];
				fDist = (float)sqrt( v3Delta[0] * v3Delta[0] + v3Delta[1] * v3Delta[1] + v3Delta[2] * v3Delta[2] );
				if( fDist < ent->fRadius )
				{
					// collision occured, occlude object
					t = fDist - ent->fRadius;
					memcpy( vTmp, ent->vVel, sizeof(vec3) );
					m3f_VecUnitize( vTmp );
					m3f_VecScale( vOverflow, t, vTmp );
					m3f_VecAdd( ent->vPos, ent->vPos, vOverflow );

					// do the response
					vN[0] = -1.0f; vN[1] = -1.0f; vN[2] = -1.0f;
					bg_EntHitsWorld( ent, vTmp, vN );
				}
			}

			// 1 = 001
			else if( 	(ent->vPos[X_AXIS] > bbox->lobounds[X_AXIS]) && 
					(ent->vPos[Y_AXIS] > bbox->lobounds[Y_AXIS]) && 
					(ent->vPos[Z_AXIS] > bbox->hibounds[Z_AXIS]) )
			{
				v3Delta[X_AXIS] = ent->vPos[X_AXIS] - bbox->lobounds[X_AXIS];
				v3Delta[Y_AXIS] = ent->vPos[Y_AXIS] - bbox->lobounds[Y_AXIS];
				v3Delta[Z_AXIS] = ent->vPos[Z_AXIS] - bbox->hibounds[Z_AXIS];
				fDist = (float)sqrt( v3Delta[0] * v3Delta[0] + v3Delta[1] * v3Delta[1] + v3Delta[2] * v3Delta[2] );
				if( fDist < ent->fRadius )
				{
					// collision occured, occlude object
					t = fDist - ent->fRadius;
					memcpy( vTmp, ent->vVel, sizeof(vec3) );
					m3f_VecUnitize( vTmp );
					m3f_VecScale( vOverflow, t, vTmp );
					m3f_VecAdd( ent->vPos, ent->vPos, vOverflow );

					// do the response
					vTmp[X_AXIS] = bbox->lobounds[X_AXIS];
					vTmp[Y_AXIS] = bbox->lobounds[Y_AXIS];
					vTmp[Z_AXIS] = bbox->hibounds[Z_AXIS];
					vN[0] = -1.0f; vN[1] = -1.0f; vN[2] = 1.0f;
					bg_EntHitsWorld( ent, vTmp, vN );
				}
			}

			// 2 = 010
			else if( 	(ent->vPos[X_AXIS] > bbox->lobounds[X_AXIS]) && 
					(ent->vPos[Y_AXIS] > bbox->hibounds[Y_AXIS]) && 
					(ent->vPos[Z_AXIS] > bbox->lobounds[Z_AXIS]) )
			{
				v3Delta[X_AXIS] = ent->vPos[X_AXIS] - bbox->lobounds[X_AXIS];
				v3Delta[Y_AXIS] = ent->vPos[Y_AXIS] - bbox->hibounds[Y_AXIS];
				v3Delta[Z_AXIS] = ent->vPos[Z_AXIS] - bbox->lobounds[Z_AXIS];
				fDist = (float)sqrt( v3Delta[0] * v3Delta[0] + v3Delta[1] * v3Delta[1] + v3Delta[2] * v3Delta[2] );
				if( fDist < ent->fRadius )
				{
					// collision occured, occlude object
					t = fDist - ent->fRadius;
					memcpy( vTmp, ent->vVel, sizeof(vec3) );
					m3f_VecUnitize( vTmp );
					m3f_VecScale( vOverflow, t, vTmp );
					m3f_VecAdd( ent->vPos, ent->vPos, vOverflow );

					// do the response
					vTmp[X_AXIS] = bbox->lobounds[X_AXIS];
					vTmp[Y_AXIS] = bbox->hibounds[Y_AXIS];
					vTmp[Z_AXIS] = bbox->lobounds[Z_AXIS];
					vN[0] = -1.0f; vN[1] = 1.0f; vN[2] = -1.0f;
					bg_EntHitsWorld( ent, vTmp, vN );
				}
			}

			// 3 = 011
			else if( 	(ent->vPos[X_AXIS] > bbox->lobounds[X_AXIS]) && 
					(ent->vPos[Y_AXIS] > bbox->hibounds[Y_AXIS]) && 
					(ent->vPos[Z_AXIS] > bbox->hibounds[Z_AXIS]) )
			{
				v3Delta[X_AXIS] = ent->vPos[X_AXIS] - bbox->lobounds[X_AXIS];
				v3Delta[Y_AXIS] = ent->vPos[Y_AXIS] - bbox->hibounds[Y_AXIS];
				v3Delta[Z_AXIS] = ent->vPos[Z_AXIS] - bbox->hibounds[Z_AXIS];
				fDist = (float)sqrt( v3Delta[0] * v3Delta[0] + v3Delta[1] * v3Delta[1] + v3Delta[2] * v3Delta[2] );
				if( fDist < ent->fRadius )
				{
					// collision occured, occlude object
					t = fDist - ent->fRadius;
					memcpy( vTmp, ent->vVel, sizeof(vec3) );
					m3f_VecUnitize( vTmp );
					m3f_VecScale( vOverflow, t, vTmp );
					m3f_VecAdd( ent->vPos, ent->vPos, vOverflow );

					// do the response
					vTmp[X_AXIS] = bbox->lobounds[X_AXIS];
					vTmp[Y_AXIS] = bbox->hibounds[Y_AXIS];
					vTmp[Z_AXIS] = bbox->hibounds[Z_AXIS];
					vN[0] = -1.0f; vN[1] = 1.0f; vN[2] = 1.0f;
					bg_EntHitsWorld( ent, vTmp, vN );
				}
			}

			// 4 = 100
			else if(	(ent->vPos[X_AXIS] > bbox->hibounds[X_AXIS]) && 
					(ent->vPos[Y_AXIS] > bbox->lobounds[Y_AXIS]) && 
					(ent->vPos[Z_AXIS] > bbox->lobounds[Z_AXIS]) )
			{
				v3Delta[X_AXIS] = ent->vPos[X_AXIS] - bbox->hibounds[X_AXIS];
				v3Delta[Y_AXIS] = ent->vPos[Y_AXIS] - bbox->lobounds[Y_AXIS];
				v3Delta[Z_AXIS] = ent->vPos[Z_AXIS] - bbox->lobounds[Z_AXIS];
				fDist = (float)sqrt( v3Delta[0] * v3Delta[0] + v3Delta[1] * v3Delta[1] + v3Delta[2] * v3Delta[2] );
				if( fDist < ent->fRadius )
				{
					// collision occured, occlude object
					t = fDist - ent->fRadius;
					memcpy( vTmp, ent->vVel, sizeof(vec3) );
					m3f_VecUnitize( vTmp );
					m3f_VecScale( vOverflow, t, vTmp );
					m3f_VecAdd( ent->vPos, ent->vPos, vOverflow );

					// do the response
					vTmp[X_AXIS] = bbox->hibounds[X_AXIS];
					vTmp[Y_AXIS] = bbox->lobounds[Y_AXIS];
					vTmp[Z_AXIS] = bbox->lobounds[Z_AXIS];
					vN[0] = 1.0f; vN[1] = -1.0f; vN[2] = -1.0f;
					bg_EntHitsWorld( ent, vTmp, vN );
				}
			}

			// 5 = 101
			else if(	(ent->vPos[X_AXIS] > bbox->hibounds[X_AXIS]) && 
					(ent->vPos[Y_AXIS] > bbox->lobounds[Y_AXIS]) && 
					(ent->vPos[Z_AXIS] > bbox->hibounds[Z_AXIS]) )
			{
				v3Delta[X_AXIS] = ent->vPos[X_AXIS] - bbox->hibounds[X_AXIS];
				v3Delta[Y_AXIS] = ent->vPos[Y_AXIS] - bbox->lobounds[Y_AXIS];
				v3Delta[Z_AXIS] = ent->vPos[Z_AXIS] - bbox->hibounds[Z_AXIS];
				fDist = (float)sqrt( v3Delta[0] * v3Delta[0] + v3Delta[1] * v3Delta[1] + v3Delta[2] * v3Delta[2] );
				if( fDist < ent->fRadius )
				{
					// collision occured, occlude object
					t = fDist - ent->fRadius;
					memcpy( vTmp, ent->vVel, sizeof(vec3) );
					m3f_VecUnitize( vTmp );
					m3f_VecScale( vOverflow, t, vTmp );
					m3f_VecAdd( ent->vPos, ent->vPos, vOverflow );

					// do the response
					vTmp[X_AXIS] = bbox->hibounds[X_AXIS];
					vTmp[Y_AXIS] = bbox->lobounds[Y_AXIS];
					vTmp[Z_AXIS] = bbox->hibounds[Z_AXIS];
					vN[0] = 1.0f; vN[1] = -1.0f; vN[2] = 1.0f;
					bg_EntHitsWorld( ent, vTmp, vN );
				}
			}

			// 6 = 110
			else if(	(ent->vPos[X_AXIS] > bbox->hibounds[X_AXIS]) && 
					(ent->vPos[Y_AXIS] > bbox->hibounds[Y_AXIS]) && 
					(ent->vPos[Z_AXIS] > bbox->lobounds[Z_AXIS]) )
			{
				v3Delta[X_AXIS] = ent->vPos[X_AXIS] - bbox->hibounds[X_AXIS];
				v3Delta[Y_AXIS] = ent->vPos[Y_AXIS] - bbox->hibounds[Y_AXIS];
				v3Delta[Z_AXIS] = ent->vPos[Z_AXIS] - bbox->lobounds[Z_AXIS];
				fDist = (float)sqrt( v3Delta[0] * v3Delta[0] + v3Delta[1] * v3Delta[1] + v3Delta[2] * v3Delta[2] );
				if( fDist < ent->fRadius )
				{
					// collision occured, occlude object
					t = fDist - ent->fRadius;
					memcpy( vTmp, ent->vVel, sizeof(vec3) );
					m3f_VecUnitize( vTmp );
					m3f_VecScale( vOverflow, t, vTmp );
					m3f_VecAdd( ent->vPos, ent->vPos, vOverflow );

					// do the response
					vTmp[X_AXIS] = bbox->hibounds[X_AXIS];
					vTmp[Y_AXIS] = bbox->hibounds[Y_AXIS];
					vTmp[Z_AXIS] = bbox->lobounds[Z_AXIS];
					vN[0] = 1.0f; vN[1] = 1.0f; vN[2] = -1.0f;
					bg_EntHitsWorld( ent, vTmp, vN );
				}
			}

			// 7 = 111
			else if(	(ent->vPos[X_AXIS] > bbox->hibounds[X_AXIS]) && 
					(ent->vPos[Y_AXIS] > bbox->hibounds[Y_AXIS]) && 
					(ent->vPos[Z_AXIS] > bbox->hibounds[Z_AXIS]) )
			{
				v3Delta[X_AXIS] = ent->vPos[X_AXIS] - bbox->hibounds[X_AXIS];
				v3Delta[Y_AXIS] = ent->vPos[Y_AXIS] - bbox->hibounds[Y_AXIS];
				v3Delta[Z_AXIS] = ent->vPos[Z_AXIS] - bbox->hibounds[Z_AXIS];
				fDist = (float)sqrt( v3Delta[0] * v3Delta[0] + v3Delta[1] * v3Delta[1] + v3Delta[2] * v3Delta[2] );
				if( fDist < ent->fRadius )
				{
					// collision occured, occlude object
					t = fDist - ent->fRadius;
					memcpy( vTmp, ent->vVel, sizeof(vec3) );
					m3f_VecUnitize( vTmp );
					m3f_VecScale( vOverflow, t, vTmp );
					m3f_VecAdd( ent->vPos, ent->vPos, vOverflow );

					// do the response
					vN[0] = 1.0f; vN[1] = 1.0f; vN[2] = 1.0f;
					bg_EntHitsWorld( ent, vTmp, vN );
				}
			}			
		}
	}
}

/* ------------
bg_EntHitsWorld
// this is called when an entity hits the world box
------------ */
static void bg_EntHitsWorld( entity_t *ent, vec3 vLoc, vec3 vNorm )
{
	color white = { 255, 255, 255, 255 };

	float inner_theta = 85.0;
	float outer_theta = 95.0;

	ps_Spawn( vLoc, vNorm, vZero, vZero, 600, 600, white, PS_SHAPE_SPARKS, 85, 95, 100.0f, 10.0, -10.0, PS_FUNCT_IMPULSE, 1000.0f, 100000.0f, 0.0f );

	col_Respond( ent, vLoc, vNorm );
}

