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
// game_particlesys.c
// Created  by Terrence Cole

// Includes
/////////////
#include "cog_global.h"

// Definitions
////////////////
#define PS_MAX_PARTICLES 75

#define PS_WORLD 2
#define PS_ENTITY 4

/*
#define PS_FUNCT_CONST 2  // the unit step u(-(t-t0)) on [0,t] ;  A*u(t)
#define PS_FUNCT_IMPULSE 3  // delta(t), area 1 ; SUM( A*d(t-kB) ), k in N, C stores next spike time
#define PS_FUNCT_LINEAR 4  // [0,t], B=starting rate, A=rate change per millisecond ; |At+B|
#define PS_FUNCT_SIN 5  // [0,t], B=length of period, A=parts per millisecond multiplier ; |A*sin(2*PI*t/B)|
#define PS_FUNCT_EXP 7 // [0,t],AC=constant, B=coeficient ; A*e^(Bt)
#define PS_FUNCT_RING 8  // |C*e^At * sin( 2*PI*t/B )|  underdamped mass-spring or RLC

#define PS_SHAPE_SPARKS 2
#define PS_SHAPE_FIRE 4
#define PS_SHAPE_SMOKE 8
*/

// particle entropy limits (uniform distribution in this range)
#define PS_COLOR_EPSILON (int16_t)25
#define PS_SIZE_EPSILON 0.0f
#define PS_GROWTH_EPSILON 0.0f
#define PS_SPEED_EPSILON 0.0f
#define PS_ANG_VEL_MIN 0
#define PS_ANG_VEL_MAX 2000
const vec3 vZero = {0.0f,0.0f,0.0f};

// Local Structures
/////////////////////
/*
typedef struct particle_s
{
	// scheduled time of death in ms
	uint32_t ulEndTime; 

	// the look and feel
	colormap_t *shape;
	color color;

	// the system descriptor
	vec3 vPos;
	vec3 vVel;
	float fAng; // radians
	float fAngVel; // radians per second
	float fSize;
	float fSizeVel;
} particle_t; // 13 * 4 bytes = 52

typedef struct psystem_s
{
	// tracking and maintenance
	uint32_t uiNumParticles; // the current number of particles
	int bType; // attachment point / animation type:  PS_ENTITY, PS_WORLD
	uint32_t uiLiveTime; // time in ms for this system to live
	uint32_t uiSysStartTime; // frame time of creation
	uint32_t uiSysKillTime;  // scheduled death time for this system

	// if bType is PS_ENTITY, this is the entity attachment
	entity_t *ent; // the entity attached
	tag_t *tStart; // the start point tag
	tag_t *tEnd; // the end point tag
	
	// the system descriptor
	vec3 vPos; // absolute world
	vec3 vVel; // movement in world
	vec3 vAng; // the emission direction
	vec2 vAngVel; // the rotation (world only)

	// the emission descriptor
	// (Probabalistic rates should be Normal Distribution;
	//  they are actually implemented as Constant Distribution
	//  for the sake of speed and sanity of implementation.)
	float fStartAng;  // particles shoot randomly in start angle to end angle
	float fEndAng;
	float fMeanRate; // the average speed of emitted particles
	float fMeanSize;
	float fMeanGrowthRate; // the average growth rate for emitted particles
	uint32_t uiPartLiveTime; // particles die after this much time passes

	// output function
	uint32_t uiSysFunction; // the parameterization of the particle emission rate
	float fA, fB, fC;

	// shape descriptor
	color cMeanColor;
	uint32_t iMapIndexLow;
	uint32_t iMapIndexHigh;

	// the particles
	heap_t* hParts;

	// internal linking
	struct psystem_s *next;
	struct psystem_s *prev;
} psystem_t;
*/
typedef struct 
{
	char name[64];
	char alpha[64];
	colormap_t *cm;
} part_shape_t;

static part_shape_t psPartShapes[] =
{
	// sparks
	{ "psPart24", "psSpark0-8", NULL },  // 0
	{ "psPart24", "psSpark1-8", NULL },  // 1
	{ "psPart24", "psSpark2-8", NULL },  // 2

	// fire
//	{ "fire0", "fire0_alpha", NULL }, // ??
//	{ "fire1", "fire1_alpha", NULL },
	// ...
};
static uint32_t iNumShapes = sizeof(psPartShapes) / sizeof(part_shape_t);
#define PS_FIRST_SPARK 0
#define PS_LAST_SPARK 2
#define PS_FIRST_FIRE 2
#define PS_LAST_FIRE 3
#define PS_FIRST_SMOKE 4
#define PS_LAST_SMOKE 5

// Global Prototypes
//////////////////////
/*
int ps_Initialize(void);
void ps_Terminate(void);
psystem_t *ps_Spawn( vec3 loc, vec3 dir, vec3 vel, vec2 ang_vel, 
						int live_time, int part_live_time,
						color mean_color, int shape,
						float inner_theta, float outer_theta, 
						float mean_speed, float mean_size, float mean_growth, 
						int function, float fA, float fB, float fC );
psystem_t *ps_SpawnOnEnt( entity_t *ent, tag_t *start, tag_t *end,
						int live_time, int part_live_time,
						color mean_color, int shape,
						float inner_theta, float outer_theta, 
						float mean_speed, float mean_size, float mean_growth, 
						int function, float fA, float fB, float fC );
void ps_Kill( psystem_t *ps );
void ps_UpdateSystems();
void ps_Draw();
*/

// Local Prototypes
/////////////////////
static int ps_PartsComparator( void *p1, void *p2 );
static particle_t *ps_CreateParticle( psystem_t *ps );
static psystem_t *ps_SpawnUniversal( 
						int live_time, int part_live_time,
						color mean_color, int shape,
						float inner_theta, float outer_theta, 
						float mean_speed, float mean_size, float mean_growth, 
						int function, float fA, float fB, float fC );
static void ps_LinkTail( psystem_t *ps );
static void ps_Unlink( psystem_t *ps );


// Local Variables
////////////////////
static psystem_t *psHead = NULL;
static psystem_t *psTail = NULL;

// *********** FUNCTIONALITY ***********
/* ------------
ps_Initialize
------------ */
int ps_Initialize(void)
{
	uint32_t a;
	double tStart, tEnd;
	char buffer[1024];
	colormap_params_t cmParams;

	con_Print( "\n<BLUE>Initializing</BLUE> the particle system tracker." );
	eng_LoadingFrame();
	tStart = ts_GetSaneTime();

	// assert
	if( psHead != NULL )
	{
		con_Print( "<RED>Particle System Init Assert Failed:  head is non NULL" );
		eng_Stop( "210001" );
		return 0;
	}
	if( psTail != NULL )
	{
		con_Print( "<RED>Particle System Init Assert Failed:  tail is non NULL" );
		eng_Stop( "210002" );
		return 0;
	}

	// find cm's for each of our shapes
	for( a = 0 ; a < iNumShapes ; a++ )
	{
		// setup the cmParams
		memset( &cmParams, 0, sizeof(colormap_params_t) );
		cmParams.bMipmap = 1;
		cmParams.bNoRescale = 0;
		cmParams.iEnv = GL_MODULATE;
		cmParams.iFilter = cm_GetGlobalFilter();
		cmParams.iWrap = GL_CLAMP;
		cmParams.iScaleBias = 0;
		snprintf( buffer, 1024, "%s_WITH_%s", psPartShapes[a].name, psPartShapes[a].alpha );
		psPartShapes[a].cm = cm_LoadFromFiles( buffer, psPartShapes[a].name, psPartShapes[a].alpha, &cmParams );
		if( psPartShapes[a].cm == NULL )
		{
			con_Print( "Particle System Init Failed:  Could not load Colormap for %s", psPartShapes[a].name );
			return 0;
		}
	}

	tEnd = ts_GetSaneTime();
	con_Print( "\tDone:  %.3f sec", tEnd - tStart );
	eng_LoadingFrame();
	
	return 1;
}

/* ------------
ps_Terminate
------------ */
void ps_Terminate(void)
{
	uint32_t a;
	int cnt = 0;
	psystem_t *psTmp;
	
	con_Print( "\n<RED>Terminating</RED> the particle system." );

	// free the particle shapes
	for( a = 0 ; a < iNumShapes ; a++ )
	{
		cm_Unload( psPartShapes[a].cm );
	}

	// systems remain to destroy
	if( psHead != NULL || psTail != NULL )
	{
		while( psHead != NULL )
		{
			// get the current position
			psTmp = psHead;

			// advance the head
			psHead = psHead->next;

			// delete the remaining particles
			for( a = 0 ; a < psTmp->uiNumParticles ; a++ )
			{
				void *mem = q_Dequeue(psTmp->qParts);
				SAFE_RELEASEv( mem );
			}

			// unload the heap
			q_Delete( psTmp->qParts );
	
			// destroy the current position
			SAFE_RELEASE( psTmp );

			// increment
			cnt++;
		}
	
		con_Print( "\tKilled %d particle systems.", cnt );
	}
}

/* ------------
ps_Spawn
// spawns a particle system in world coordinates
// loc: position of system
// dir: emission direction of system
// vel: movement of system
// ang_vel: rotation rate of system
// live_time: ttl for the system, or 0 for non-dying (in ms)
// part_live_time: live time for individual particles (in ms)
// mean_color: average color of an emitted particle (constant PD)
// shape: shape group PS_SHAPE_ FIRE/SMOKE/SPARKS
// inner/outer_theta: emission in a cone centered at dir with inner and outer sides as theta from dir
// mean_speed: average velocity of an emitted particle (constant PD)
// mean_growth:  average growth rate of an emitted particle (constant PD)
// function: system rate function for emission
// fA, fB, fC: System parameters. See function definitions for meaning
------------ */
psystem_t *ps_Spawn( vec3 loc, vec3 dir, vec3 vel, vec2 ang_vel, 
						int live_time, int part_live_time,
						color mean_color, int shape,
						float inner_theta, float outer_theta, 
						float mean_speed, float mean_size, float mean_growth, 
						int function, float fA, float fB, float fC )
{
	psystem_t *newPS;

	// debug assertions
	assert( loc );
	assert( dir );
	assert( vel );
	assert( ang_vel );
	assert( mean_color );

	// runtime assertions 
	if( !loc || !dir || !vel || !ang_vel || !mean_color )
	{
		con_Print( "<RED>Particle System Spawn (World) Called with Invalid Parameters!" );
		return NULL;
	}

	// do univeral init
	newPS = ps_SpawnUniversal( live_time, part_live_time,
							mean_color, shape,
							inner_theta, outer_theta,
							mean_speed, mean_size, mean_growth,
							function, fA, fB, fC );
	if( !newPS )
	{
		con_Print( "<RED>Particle System Spawn Universal Failed." );
		return NULL;
	}
	
	// ent stuff is null
	newPS->ent = NULL;
	newPS->tStart = NULL;
	newPS->tEnd = NULL;
	
	// the system descriptor
	memcpy( newPS->vPos, loc, sizeof(vec3) );
	memcpy( newPS->vVel, vel, sizeof(vec3) );
	memcpy( newPS->vAngVel, ang_vel, sizeof(vec2) );

	// a vector direction to an angular direction
	m3f_CartToSphereD( dir, newPS->vAng );

	return newPS;
}

/* ------------
ps_Spawn
// spawns a particle system in world coordinates
// ent: the entity to attach to
// start: the starting tag of the entity
// end: the ending tag of the entity
// live_time: ttl for the system, or 0 for non-dying (in ms)
// part_live_time: live time for individual particles (in ms)
// mean_color: average color of an emitted particle (constant PD)
// shape: shape group PS_SHAPE_ FIRE/SMOKE/SPARKS
// inner/outer_theta: emission in a cone centered at dir with inner and outer sides as theta from dir
// mean_speed: average velocity of an emitted particle (constant PD)
// mean_growth:  average growth rate of an emitted particle (constant PD)
// function: system rate function for emission
// fA, fB, fC: System parameters. See function definitions for meaning
------------ */
psystem_t *ps_SpawnOnEnt( entity_t *ent, tag_t *start, tag_t *end,
						int live_time, int part_live_time,
						color mean_color, int shape,
						float inner_theta, float outer_theta, 
						float mean_speed, float mean_size, float mean_growth, 
						int function, float fA, float fB, float fC )
{
	psystem_t *ps;

	// debug assert
	assert( ent );
	assert( start );
	assert( end );

	// runtime assert
	if( !ent || !start || !end )
	{
		con_Print( "<RED>Particle System Spawn on Entity Called win NULL args." );
		return NULL;
	}

	// do univeral init
	ps = ps_SpawnUniversal( live_time, part_live_time,
							mean_color, shape,
							inner_theta, outer_theta,
							mean_speed, mean_size, mean_growth,
							function, fA, fB, fC );
	if( !ps )
	{
		con_Print( "<RED>Particle System Spawn Universal Failed." );
		return NULL;
	}

	// do entity specific init
	ps->ent = ent;
	ps->tStart = start;
	ps->tEnd = end;

	// the system descriptor
	memset( ps->vPos, 0, sizeof(vec3) );
	memset( ps->vVel, 0, sizeof(vec3) );
	memset( ps->vAng, 0, sizeof(vec3) );
	memset( ps->vAngVel, 0, sizeof(vec2) );

	return ps;
}

/* ------------
ps_Kill
------------ */
void ps_Kill( psystem_t *ps )
{
	uint32_t a;
	void **mem;
	
	// debug assertions
	assert( ps );

	// runtime assertions
	if( !ps ) 
	{
		con_Print( "<RED>Particle System Kill called on NULL descriptor!" );
		return;
	}

	// remove it from the list
	ps_Unlink( ps );

	// delete the remaining particles
	/* ideally we have this loop, but this massivly need acceleration
	for( a = 0 ; a < ps->uiNumParticles ; a++ )
	{
		void *mem = q_Dequeue(ps->qParts);
		SAFE_RELEASEv( mem );
	}
	*/
	mem = q_GetBase(ps->qParts);
	for( a = 0 ; a < PS_MAX_PARTICLES ; a++ )
	{
		SAFE_RELEASE( mem[a] );
	}
	
	// unload the heap
	q_Delete( ps->qParts );

	// return memory to system
	SAFE_RELEASE( ps );
}

/* ------------
ps_UpdateSystems - updates all systems / animations
------------ */
void ps_UpdateSystems()
{
	psystem_t *ps = psHead;
	psystem_t *nextPS;
	uint32_t uiNewParts;
	uint32_t uiTmp;
	float fTimeOffset;
	float dt = (float)state.delta_time / 1000.0f; // time increment in seconds
	int bDone;
	uint32_t a;

	// kill old systems
	while( ps != NULL )
	{
		if( ps->uiSysKillTime < state.frame_time )
		{
			nextPS = ps->next;
			ps_Kill( ps );
			ps = nextPS;
		}
		if( ps ) ps = ps->next;
	}
	
	// iterate
	ps = psHead;
	while( ps != NULL )
	{
		// compute existance time
		fTimeOffset = (float)(state.frame_time - ps->uiSysStartTime);

		// WORLD type specific stuff
		if( ps->bType == PS_WORLD )
		{
			// update the position of the shooter
			ps->vPos[X_AXIS] += ps->vVel[X_AXIS] * dt;
			ps->vPos[Y_AXIS] += ps->vVel[Y_AXIS] * dt;
			ps->vPos[Z_AXIS] += ps->vVel[Z_AXIS] * dt;

			// update the angle of the shooter
			ps->vAng[PITCH] += ps->vAngVel[PITCH] * dt;
			ps->vAng[YAW] += ps->vAngVel[YAW] * dt;
		}
		// ENTITY type specific stuff
		else 
		{
			// update the position of the shooter
			vec3 vTo;
			vec3 vDir;

			// rotate and translate the tags into global coordinates
			m3f_TransformToGlobal( ps->vPos, ps->tStart->pos, ps->ent->vPos, ps->ent->vRot );
			m3f_TransformToGlobal( vTo, ps->tEnd->pos, ps->ent->vPos, ps->ent->vRot );

			// compute the emission direction, compute angles
			m3f_VecSubtract( vDir, vTo, ps->vPos );
			m3f_AngleFromVec( ps->vAng, vDir );

			// get the current velocity
			memcpy( ps->vVel, ps->ent->vVel, sizeof(vec3) );

			// we don't need the angular velocity
		}

		// kill all old particles
		bDone = 0;
		do {
			particle_t *top = (particle_t*)q_LookAtHead(ps->qParts);

			// we have reached the bottom of the heap, or the lowest times to remove
			if( top == NULL  || (top->ulEndTime > state.frame_time) )
			{
				bDone = 1;
			}
			// we need to remove all other particles
			else
			{
				top = q_Dequeue(ps->qParts);
				SAFE_RELEASE(top);
				ps->uiNumParticles--;
			}
		} while( !bDone );

		// animate the remaining particles
		for( a = 0 ; a < ps->uiNumParticles ; a++ )
		{
			// get a quick reference
			particle_t *part = q_GetAt( ps->qParts, a );

			// update angle and size
			part->fAng += part->fAngVel * dt;
			part->fSize += part->fSizeVel * dt;
			if( part->fSize < 0.0f ) part->fSize = 0.0f;

			// update position
			part->vPos[0] += part->vVel[0] * dt;
			part->vPos[1] += part->vVel[1] * dt;
			part->vPos[2] += part->vVel[2] * dt;

			// fade out
			uiTmp = part->ulEndTime - state.frame_time;
			if( uiTmp > 255 ) uiTmp = 255;
			part->part_color[3] = (byte)uiTmp;
		}

		// compute the number of new parts to create
		switch( ps->uiSysFunction )
		{
		case PS_FUNCT_CONST:
			ps->fPartialParts += ps->fA * dt;
			uiNewParts = (uint32_t)ps->fPartialParts;
			ps->fPartialParts -= (float)uiNewParts;
			break;
		case PS_FUNCT_IMPULSE:
			if( ps->fC > fTimeOffset )
			{
				uiNewParts = (uint32_t)ps->fA;
				ps->fC += ps->fB;
			}
			else
			{
				uiNewParts = 0;
			}
			break;
		case PS_FUNCT_LINEAR:
			ps->fPartialParts += ABS(ps->fA * fTimeOffset + ps->fB);
			uiNewParts = (uint32_t)ps->fPartialParts;
			ps->fPartialParts -= (float)uiNewParts;
			break;
		case PS_FUNCT_SIN:
			ps->fPartialParts += ABS(ps->fA * (float)sin( (fTWOPI * fTimeOffset) / ps->fB ));
			uiNewParts = (uint32_t)ps->fPartialParts;
			ps->fPartialParts -= (float)uiNewParts;
			break;
		case PS_FUNCT_EXP:
			ps->fPartialParts += (ps->fA * (float)exp( ps->fB * fTimeOffset ));
			uiNewParts = (uint32_t)ps->fPartialParts;
			ps->fPartialParts -= (float)uiNewParts;
			break;
		case PS_FUNCT_RING:
			ps->fPartialParts += ABS( ps->fC * (float)exp( ps->fA * fTimeOffset ) * (float)sin( (fTWOPI * fTimeOffset) / ps->fB ) );
			uiNewParts = (uint32_t)ps->fPartialParts;
			ps->fPartialParts -= (float)uiNewParts;			
			break;
		default:
			uiNewParts = 0;
			break;
		}

		// create the new particles in this system
		for( a = 0 ; a < uiNewParts ; a++ )
		{
			particle_t *part = ps_CreateParticle( ps );

			// allocation error, halt now
			if( part == NULL ) 
			{
				return;
			}
			
			if( !q_Enqueue( ps->qParts, part ) )
			{
				SAFE_RELEASE( part );
				return; // queue overflow, halt now
			}
			
			ps->uiNumParticles++;
		}

		// next
		ps = ps->next;
	}
}

/* ------------
ps_Draw
------------ */
void ps_Draw()
{
	uint32_t a;
	psystem_t *ps = psHead;
	particle_t *part;

	// iterate systems
	while( ps != NULL )
	{
		// iterate particles
		for( a = 0 ; a < ps->uiNumParticles ; a++ )
		{
			part = q_GetAt( ps->qParts, a );
			assert( part != NULL );
			trans_Sprite( part->vPos, part->fSize, part->fAng, part->part_color, part->shape );
		}
	
		// next
		ps = ps->next;
	}
}
			
/********* UTILITY FUNCTIONS **********/
/* ------------
ps_PartsComparator
// The Comparator Function returns > 0 for: item 1 > item 2.
// It returns 0 for: item 1 = item 2.
// It return < 0 for: item 1 < item 2.
// My heap sorts data from "least" to "greatest".
// Therefore, older particles are "less" then new particles.
------------ */
static int ps_PartsComparator( void *p1, void *p2 )
{
	// NULL pointers are possible and should be considered "greater"
	if( !p1 && !p2 ) return 0;
	if( !p1 ) return 1;
	if( !p2 ) return -1;
	
	// the comparator
	if( ((particle_t*)p1)->ulEndTime < ((particle_t*)p2)->ulEndTime ) return -1;
	else if( ((particle_t*)p1)->ulEndTime > ((particle_t*)p2)->ulEndTime ) return 1;
	return 0;
}

/* ------------
ps_CreateParticle
// returns a single new particle for ps
------------ */
static particle_t *ps_CreateParticle( psystem_t *ps )
{
	int16_t cTmp;
	float fRate;
	vec3 vAng, vDir, vEDir;
	particle_t *part = (particle_t*)mem_alloc( sizeof(particle_t) );
	if( part == NULL )
	{
		con_Print( "<RED>Particle System: memory allocatio failed!" );
		return NULL;
	}
	// n.b. no memset since we're manually touching everything here
	
	// set the death time
	part->ulEndTime = state.frame_time + ps->uiPartLiveTime;

	// pick a random shape for this particle
	part->shape = psPartShapes[ rng_Long( ps->iMapIndexLow, ps->iMapIndexHigh + 1 ) ].cm;

	// pick a color for the particle
	cTmp = (int16_t)ps->cMeanColor[0] + (int16_t)rng_Short( -PS_COLOR_EPSILON, PS_COLOR_EPSILON );
	if( cTmp < 0 ) cTmp = 0;
	if( cTmp > 255 ) cTmp = 255;
	part->part_color[0] = (byte)cTmp;

	cTmp = (int16_t)ps->cMeanColor[1] + (int16_t)rng_Short( -PS_COLOR_EPSILON, PS_COLOR_EPSILON );
	if( cTmp < 0 ) cTmp = 0;
	if( cTmp > 255 ) cTmp = 255;
	part->part_color[1] = (byte)cTmp;

	cTmp = (int16_t)ps->cMeanColor[2] + (int16_t)rng_Short( -PS_COLOR_EPSILON, PS_COLOR_EPSILON );
	if( cTmp < 0 ) cTmp = 0;
	if( cTmp > 255 ) cTmp = 255;
	part->part_color[2] = (byte)cTmp;

	cTmp = (int16_t)ps->cMeanColor[3] + (int16_t)rng_Short( -PS_COLOR_EPSILON, PS_COLOR_EPSILON );
	if( cTmp < 0 ) cTmp = 0;
	if( cTmp > 255 ) cTmp = 255;
	part->part_color[3] = (byte)cTmp;

	// inherit position
	memcpy( part->vPos, ps->vPos, sizeof(vec3) );

	// get a size and growth rate
	part->fSize = ps->fMeanSize + rng_Float( -PS_SIZE_EPSILON, PS_SIZE_EPSILON );
	part->fSizeVel = ps->fMeanGrowthRate + rng_Float( -PS_GROWTH_EPSILON, PS_GROWTH_EPSILON );

	// get an angle and rotation rate
	part->fAng = rng_Float( 0.0f, fTWOPI );
	part->fAngVel = rng_Float( 0.0f, PS_ANG_VEL_MAX );

	// get an emission Angle in the logical cone
	vAng[PITCH] = rng_Float( ps->fStartAng, ps->fEndAng );
	vAng[YAW] = rng_Float( 0.0f, 360.0f );
	vAng[ROLL] = 0.0f;
	
	// convert the angle to a direction
	m3f_SphereToCartD( vAng, vDir );

	// transform the emission direction vector into the system angles
	m3f_TransformToGlobal( vEDir, vDir, vZero, ps->vAng );

	// get and set an emission velocity
	fRate = ps->fMeanRate + rng_Float( -PS_SPEED_EPSILON, PS_SPEED_EPSILON );
	m3f_VecScale( vEDir, fRate, vEDir );

	// inherit the system velocity
	m3f_VecAdd( part->vVel, ps->vVel, vEDir );

	return part;
}

/* ------------
ps_SpawnUniversal
// contains functionality relevant for world centric and ent centric psystem spawning
------------ */
static psystem_t *ps_SpawnUniversal( 
						int live_time, int part_live_time,
						color mean_color, int shape,
						float inner_theta, float outer_theta, 
						float mean_speed, float mean_size, float mean_growth, 
						int function, float fA, float fB, float fC )
{
	psystem_t *newPS;

	// Debug Assert
	assert( mean_color );

	// runtime assert
	if( !mean_color )
	{
		con_Print( "<RED>Particle System Spawn (Universal) Failed:  NULL mean_color." );
		return NULL;
	}
	
	// create the new system	
	newPS = (psystem_t*)mem_alloc( sizeof(psystem_t) );
	if( newPS == NULL )
	{	
		con_Print( "<RED>Particle System Spawn (Universal) Failed:  could not allocate memory for newPS, psystem_t" );
		return NULL;
	}
	memset( newPS, 0, sizeof(psystem_t) );

	// tracking and maintenance
	newPS->uiNumParticles = 0;
	newPS->fPartialParts = 0.0f;
	newPS->bType = PS_WORLD;
	newPS->uiLiveTime = live_time;
	newPS->uiSysStartTime = state.frame_time;
	newPS->uiSysKillTime = (live_time == 0) ? (0xFFFFFFFF) : (live_time + state.frame_time);

	// the emission descriptor
	newPS->fStartAng = inner_theta;
	newPS->fEndAng = outer_theta;
	newPS->fMeanRate = mean_speed;
	newPS->fMeanSize = mean_size;
	newPS->fMeanGrowthRate = mean_growth;
	newPS->uiPartLiveTime = part_live_time;
	
	// output function
	newPS->uiSysFunction = function;
	newPS->fA = fA;
	newPS->fB = fB;
	newPS->fC = fC;
	// we need some special processing for the functions
	switch( function )
	{
	case PS_FUNCT_CONST:
		newPS->fA = ABS(fA);
		break;
	case PS_FUNCT_IMPULSE:
		newPS->fA = ABS(fA);
		newPS->fB = ABS(fB);
		newPS->fC = (float)(state.frame_time + (uint32_t)fB);
		break;
	case PS_FUNCT_EXP:
		newPS->fA = ABS(fA);
	}

	// shape descriptor
	memcpy( newPS->cMeanColor, mean_color, sizeof(color) );
	switch( shape )
	{
	case PS_SHAPE_SPARKS:
		newPS->iMapIndexLow = PS_FIRST_SPARK;
		newPS->iMapIndexHigh = PS_LAST_SPARK;
		break;
	case PS_SHAPE_FIRE:
		newPS->iMapIndexLow = PS_FIRST_FIRE;
		newPS->iMapIndexHigh = PS_LAST_FIRE;
		break;
	case PS_SHAPE_SMOKE:
		newPS->iMapIndexLow = PS_FIRST_SMOKE;
		newPS->iMapIndexHigh = PS_LAST_SMOKE;
		break;
	default:
		newPS->iMapIndexLow = PS_FIRST_SPARK;
		newPS->iMapIndexHigh = PS_LAST_SPARK;
		break;
	}

	// the particle heap
	newPS->qParts = q_Create( PS_MAX_PARTICLES );
	if( newPS->qParts == NULL_QUEUE )
	{
		SAFE_RELEASE( newPS );
		return NULL;
	}

	// link it
	ps_LinkTail( newPS );

	// give back the fully inited ps
	return newPS;
}

/* ------------
ps_LinkTail - internal list maintainence, adds fgo to the list at the tail
------------ */
static void ps_LinkTail( psystem_t *ps )
{
	if( psHead == NULL ) psHead = ps;

	if( psTail == NULL )
	{
		psTail = ps;
		ps->next = NULL;
		ps->prev = NULL;
		return;
	}

	ps->prev = psTail;
	ps->next = NULL;

	psTail->next = ps;
	psTail = ps;
}

/* ------------
ps_Unlink - removes fgo from the forground object list - repairs the list for fgo's absence
------------ */
static void ps_Unlink( psystem_t *ps )
{
	if( ps->prev != NULL ) ps->prev->next = ps->next;
	if( ps->next != NULL ) ps->next->prev = ps->prev;

	if( psTail == ps ) psTail = ps->prev;
	if( psHead == ps ) psHead = ps->next;

	ps->prev = NULL;
	ps->next = NULL;
}
