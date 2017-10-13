// Cognition
// menu
// menu_background.c
// by Terrence Cole created 4/1/02

// Includes
/////////////
#include "menu_global.h"

#include <math.h>

// Definitions
////////////////
#define NUM_BG_PARTICLES 10
// #define GRAV_CONST 0.0016f
#define GRAV_CONST 0.0

// Structures
///////////////
typedef struct
{
	sound_t *snd;
	char *file;
} snd_bind_t;

snd_bind_t sndBind[] =
{
	{ NULL, "pachelbel-48" },
	{ NULL, "mcp-6871" },
	{ NULL, "bit-yes" },
	{ NULL, "mcp-take_your_chances" },
	{ NULL, "mcp-how_many" },
	{ NULL, "purposefully_null" },
};
uint32_t iNumMenuSounds = sizeof(sndBind) / sizeof(snd_bind_t);

// Global Prototypes
//////////////////////
/*
int bg_LoadMenuBG();
void bg_UnloadMenuBG();
void bg_Animate( exec_state_t *state );
*/

// Local Prototypes
/////////////////////
static void menu_PrecacheSounds(void);
// precaches all sounds in sndBind; presumably all needed for the menu system

static void menu_UnloadSounds(void);
// unloads all of the precached sounds in sndBind; presumably all the sounds in the menu system


// Local Variables
////////////////////
static entity_t *bg_particles[NUM_BG_PARTICLES];
static entity_t *entMenuSkybox = NULL;
static camera_t menuCamera;
static light_t *sun = NULL;

#define BG_PART_LIST_SIZE 2
static char *bg_part_list[BG_PART_LIST_SIZE] =
{
	{ "rock01.def" },
	{ "rock02.def" },
};

static int iTmp = -1;

// *********** FUNCTIONALITY ***********
/* ------------
bg_LoadMenuBG - initialize all of the menu background items to their default settings
			  - returns > 0 on success or 0 on fail
------------ */
int bg_LoadMenuBG()
{
	int a = 0;
	char name[32];
	float fTmp;
	vec3 pos, rot, scale;
	vec4 sunPos = { 0.7f, 0.7f, 1.0f, 0.0f }; 
	vec4 sunAmb = { 0.25f, 0.25f, 0.22f, 1.0f };
	vec4 sunDif = { 1.0f, 1.0f, 0.93f, 1.0f };
	vec4 sunSpec = { 1.0f, 1.0f, 1.0f, 1.0f };
	vec3 spot_dir = { 0.0f, 0.0f, 0.0f };
	
//	fgo_params_t fgoParams;
//	char shName[] = "menu_outoforder.sh";

	// precache needed media
	menu_PrecacheSounds();

	// get a skybox
	entMenuSkybox = ei->ent_Spawn( "MenuSkybox", 0, "skybox.def", (float*)vZero, (float*)vZero, (float*)vUnity, NULL, NULL, 0 );
	entMenuSkybox->bHintHidden = 1;
	entMenuSkybox->rotation[YAW] = 180.0f;
	ei->d_SetSkybox( entMenuSkybox );

	// set a camera for the player
	memset( &menuCamera, 0, sizeof(camera_t) );
	menuCamera.type = CAMERA_PROJECTION;
	menuCamera.vPos[0] = menuCamera.vPos[1] = menuCamera.vPos[2] = 0.0f;
	menuCamera.vAng[0] = menuCamera.vAng[1] = 0.0f;
	menuCamera.fnear = 1.0f;
	menuCamera.ffar = 10000.0f;
	menuCamera.ffov = 45.0f;
	ei->d_SetCamera( &menuCamera );
/*
	// load an "out of order" sign
	memset( &fgoParams, 0, sizeof(fgo_params_t) );
	fgoParams.shader_name = shName;
	fgoParams.shW = 480;
	fgoParams.shH = 480;
	fgoParams.shY = 0;
	fgoParams.shX = (640 - 480) / 2;
	fgoOutOfOrder = ei->fgo_Load( &fgoParams );
*/
	// 	entity_t* (*ent_Spawn)( char *name, int flags, char *model, float *pos, float *rot, float *scale, void *ext_data, void (*think_func)(void) );

	// tan(45) = 1.0f

	// initialize the 'sub-planetary' objects
	for( a = 0 ; a < NUM_BG_PARTICLES ; a++ )
	{
		// name
		sprintf( name, "bg_particle%d", a );
		
		// position, using some linear interpolation to keep stuff in the veiw frustum
		pos[Z_AXIS] = ei->rng_Float( -10000.0f, -1.0f );
		pos[X_AXIS] = ei->rng_Float( pos[Z_AXIS], -pos[Z_AXIS] );
		fTmp = pos[Z_AXIS] * menuCamera.ftop / menuCamera.fright; // down-scale by the aspect ratio
		pos[Y_AXIS] = ei->rng_Float( fTmp, -fTmp );
		
		// starting rotation
		rot[PITCH] = ei->rng_Float( 0.0f, 359.0f ); 
		rot[YAW] = ei->rng_Float( 0.0f, 359.0f ); 

		// scaling is ALWAYS unity
		scale[X_AXIS] = 1.0f; scale[Y_AXIS] = 1.0f; scale[Z_AXIS] = 1.0f;

		// spawn the particle
		bg_particles[a] = ei->ent_Spawn( name, ENT_FLAG_MENU_ITEM, "rock_100_ice.def", pos, rot, scale, NULL, NULL, 0 );
		
		// initialize the particle
		bg_particles[a]->velocity[X_AXIS] = ei->rng_Float( -500.0f, 500.0f );
		bg_particles[a]->velocity[Y_AXIS] = ei->rng_Float( -1000.0f, 0.0f );
		bg_particles[a]->velocity[Z_AXIS] = ei->rng_Float( -500.0f, 500.0f );

		bg_particles[a]->ang_velocity[PITCH] = ei->rng_Float( 0.0f, 200.0f );
		bg_particles[a]->ang_velocity[YAW] = ei->rng_Float( 0.0f, 200.0f );

		// set a mass
		bg_particles[a]->mass = ei->rng_Long( 100, 150 );
	}

	bg_particles[0]->position[X_AXIS] = 00.0f;
	bg_particles[0]->position[Y_AXIS] = 0.0f;
	bg_particles[0]->position[Z_AXIS] = -100.0f;

	bg_particles[0]->velocity[X_AXIS] = 0.0f;
	bg_particles[0]->velocity[Y_AXIS] = 0.0f;
	bg_particles[0]->velocity[Z_AXIS] = 0.0f;

	bg_particles[0]->ang_velocity[PITCH] = 40.0f;
	bg_particles[0]->ang_velocity[YAW] = 20.0f;

	bg_particles[0]->rotation[PITCH] = 270.0f;
	bg_particles[0]->rotation[YAW] = 0.0f;

	bg_particles[1]->position[X_AXIS] = 0.0f;
	bg_particles[1]->position[Y_AXIS] = 0.0f;
	bg_particles[1]->position[Z_AXIS] = -200.0f;

	bg_particles[1]->velocity[X_AXIS] = 0.0f;
	bg_particles[1]->velocity[Y_AXIS] = 0.0f;
	bg_particles[1]->velocity[Z_AXIS] = 0.0f;

	bg_particles[1]->ang_velocity[PITCH] = -10.0f;
	bg_particles[1]->ang_velocity[YAW] = 0.0f;

	bg_particles[1]->rotation[PITCH] = 0.0f;
	bg_particles[1]->rotation[YAW] = 0.0f;

	// spawn lights
	sun = ei->l_Spawn( LIGHT_DIRECTIONAL, sunPos, sunAmb, sunDif, sunSpec, 0, 45, spot_dir, 1.0f, 0.0f, 0.0f );
	if( sun == NULL )
	{
		ei->con_Print( "<RED>Menu Background Load Failed:  unable to spawn a sun." );
		return 0;
	}

//	iTmp = ei->s_PlaySound( sndBind[1].snd, 1, &bg_particles[0]->position, &bg_particles[0]->velocity );

	return 1;
}

/* ------------
bg_UnloadMenuBG
------------ */
void bg_UnloadMenuBG()
{
	int a;

	ei->s_StopSound( iTmp );
	
	// unload and reset the skybox
	ei->ent_Remove( entMenuSkybox );
	ei->d_SetSkybox( NULL );

	// unload the out of order sign
//	if( fgoOutOfOrder != NULL ) ei->fgo_Unload( fgoOutOfOrder );

	// unload the camera
	ei->d_SetCamera( NULL );

	// kill the sun
	if( sun != NULL ) { ei->l_Kill( sun ); sun = NULL; }
	
	// kill the bg particles
	for( a = 0 ; a < NUM_BG_PARTICLES ; a++ )
	{
		ei->ent_Remove( bg_particles[a] );
		bg_particles[a] = NULL;
	}
	
	// unload media
	menu_UnloadSounds();
	
}

/* ------------
bg_Animate - step the menu background animation based on the current state
------------ */
void bg_Animate( exec_state_t *state )
{
	int a;
	camera_t *camTmp;
	vec3 hit;
//	int b;
//	vec3 vTmp;
//	float fLen, dLen;
//	double fGrav;
	float elapsed_sec = state->delta_time / 1000.0f;
	float fTmp;
	
	if( iTmp == -1 ) iTmp = ei->s_PlaySound( sndBind[1].snd, 1, &bg_particles[0]->position, &bg_particles[0]->velocity );

	

	// rotate the skybox
	// perform a small rotation on the skybox
//	entMenuSkybox->rotation[PITCH] += 1.0f / (float)state->delta_time;
//	entMenuSkybox->rotation[YAW] += 6.0f / (float)state->delta_time;

/*
	// compute a new acceleration for each of the objects
	// F=m*a
	// F=G*M1*M2/r^2
	// a1 = G * M1 * M2 / ( M1 * r^2 )
	// a1 = G * M2 / r^2
	for( a = 0 ; a < NUM_BG_PARTICLES ; a++ )
	{
		memset( bg_particles[a]->accel, 0, sizeof(vec3) );
		
		for( b = 0 ; b < NUM_BG_PARTICLES ; b++ )
		{
			if( a != b )
			{
				// get the raw direction
				vTmp[X_AXIS] = bg_particles[b]->position[X_AXIS] - bg_particles[a]->position[X_AXIS];
				vTmp[X_AXIS] = bg_particles[b]->position[Y_AXIS] - bg_particles[a]->position[Y_AXIS];
				vTmp[X_AXIS] = bg_particles[b]->position[Z_AXIS] - bg_particles[a]->position[Z_AXIS];

				// compute the length directly
				fLen = m3f_VecLength( vTmp );

				// compute a normalized direction
				dLen = 1 / fLen;
				vTmp[X_AXIS] *= dLen;
				vTmp[Y_AXIS] *= dLen;
				vTmp[Z_AXIS] *= dLen;

//ei->con_Print( "%f", fLen * fLen );

				// compute the gravity on the object from this viewpoint
				fGrav = (GRAV_CONST * (double)bg_particles[a]->mass * (double)bg_particles[b]->mass) / ((double)fLen * (double)fLen);
ei->con_Print( "%f", fGrav );

				// scale the direction into the acceleration modified by the mass
				fGrav /= bg_particles[a]->mass;
				bg_particles[a]->accel[X_AXIS] += (float)((double)vTmp[X_AXIS] * fGrav);
				bg_particles[a]->accel[Y_AXIS] += (float)((double)vTmp[Y_AXIS] * fGrav);
				bg_particles[a]->accel[Z_AXIS] += (float)((double)vTmp[Z_AXIS] * fGrav);

			}
		}
	}
*/

	// check the camera, and aggressivly make it our own
	camTmp = ei->d_GetCamera();
	if( (camTmp == NULL) || (camTmp != &menuCamera) )
	{
		ei->con_Print( "Menu Note:  recapturing camera." );
		ei->d_SetCamera( &menuCamera );
	}

	// compute a new position based on the new acceleration and old velocity
	for( a = 0 ; a < NUM_BG_PARTICLES ; a++ )
	{
		// use the acceleration to change the velocity
		// v2 = v1 + a * t
		bg_particles[a]->velocity[X_AXIS] += bg_particles[a]->accel[X_AXIS] * elapsed_sec;
		bg_particles[a]->velocity[Y_AXIS] += bg_particles[a]->accel[Y_AXIS] * elapsed_sec;
		bg_particles[a]->velocity[Z_AXIS] += bg_particles[a]->accel[Z_AXIS] * elapsed_sec;

		// use the velocity to calculate a change in position
		bg_particles[a]->position[X_AXIS] += bg_particles[a]->velocity[X_AXIS] * elapsed_sec;
		bg_particles[a]->position[Y_AXIS] += bg_particles[a]->velocity[Y_AXIS] * elapsed_sec;
		bg_particles[a]->position[Z_AXIS] += bg_particles[a]->velocity[Z_AXIS] * elapsed_sec;

		// use the w to calculate a new rotation
		bg_particles[a]->rotation[PITCH] += bg_particles[a]->ang_velocity[PITCH] * elapsed_sec;
		bg_particles[a]->rotation[YAW] += bg_particles[a]->ang_velocity[YAW] * elapsed_sec;

		// cap the rotation on X
		while( bg_particles[a]->rotation[PITCH] >= 360.0 ) bg_particles[a]->rotation[PITCH] -= 360.0f;
		while( bg_particles[a]->rotation[PITCH] < 0.0f ) bg_particles[a]->rotation[PITCH] += 360.0f;

		// cap the rotation on Y
		while( bg_particles[a]->rotation[YAW] >= 360.0 ) bg_particles[a]->rotation[YAW] -= 360.0f;
		while( bg_particles[a]->rotation[YAW] < 0.0f ) bg_particles[a]->rotation[YAW] += 360.0f;


		// do the keyframe animation
		if( bg_particles[a]->mod->moo->num_frames > 1 )
		{
			// frame sub part delta is the ( time elapsed / time per frame ) => time elapsed * frames per millisecond
			bg_particles[a]->mod->moo->sub_frame += (float)((double)state->delta_time * bg_particles[a]->mod->moo->frames_per_millisecond * .01);
			while( bg_particles[a]->mod->moo->sub_frame > 1.00f ) 
			{
				bg_particles[a]->mod->moo->sub_frame -= 1.00f;
				bg_particles[a]->mod->moo->cur_frame += 1;
			}
			while( bg_particles[a]->mod->moo->cur_frame >= bg_particles[a]->mod->moo->num_frames )
			{
				bg_particles[a]->mod->moo->cur_frame -= bg_particles[a]->mod->moo->num_frames;
			}
		}
	}
/*
	// iterate
	ei->ll_iter_reset(state->ent_list);
	while( !ei->ll_iter_isdone(state->ent_list) )
	{
		ent = (entity_t*)ei->ll_iter_data(state->ent_list);

		if( ent->flags & ENT_FLAG_SKYBOX )
		{
			// perform a small rotation on the skybox
			ent->rotation[PITCH] += 2.0f / (float)state->delta_time;
			ent->rotation[YAW] += 1.5f / (float)state->delta_time;
			ent->rotation[ROLL] += 1.25f / (float)state->delta_time;
		}
		else if( ent->flags & ENT_FLAG_MENU_ITEM )
		{
			vec3 hit;

			if( ent->name[0] == 'b' )
			{
				if( ei->col_CheckEnts( hit, bg_particles[0], bg_particles[1], COL_PRIORITY_MEDIUM ) )
				{
					bg_particles[0]->accel[X_AXIS] = -bg_particles[0]->velocity[X_AXIS];
					bg_particles[1]->accel[X_AXIS] = -bg_particles[1]->velocity[X_AXIS];
//					bg_particles[0]->velocity[X_AXIS] = -bg_particles[0]->velocity[X_AXIS];
//					bg_particles[1]->velocity[X_AXIS] = -bg_particles[1]->velocity[X_AXIS];
//					bg_particles[0]->velocity[X_AXIS] = 0;
//					bg_particles[1]->velocity[X_AXIS] = 0;

				}
			}
*/
				if( ei->col_CheckEnts( hit, bg_particles[1], bg_particles[0], COL_PRIORITY_HIGH ) )
				{
//					bg_particles[0]->accel[X_AXIS] = bg_particles[0]->velocity[X_AXIS];
//					bg_particles[1]->accel[X_AXIS] = bg_particles[1]->velocity[X_AXIS];
					bg_particles[0]->velocity[X_AXIS] = 0.0f; //-bg_particles[0]->velocity[X_AXIS];
					bg_particles[1]->velocity[X_AXIS] = 0.0f; //-bg_particles[1]->velocity[X_AXIS];
				}

/*
		}
		
		if( ent->mod != NULL && ent->mod->moo != NULL )
		{
		}

		ei->ll_iter_next(state->ent_list);
	}
	*/

	fTmp = state->frame_time / 100000.0f;
	bg_particles[0]->position[X_AXIS]  = (float)(100.0 * sin(  fTmp * 180.0 / 3.14159 ));

	fTmp = state->frame_time / 100000.0f;
	bg_particles[0]->position[Z_AXIS]  = (float)(100.0 * cos( fTmp * 180.0 / 3.14158 ));

}

/* ------------
menu_PrecacheSounds
// precaches all sounds in sndBind; presumably all needed for the menu system
------------ */
static void menu_PrecacheSounds(void)
{
	uint32_t a;
	
	for( a = 0 ; a < iNumMenuSounds; a++ )
	{
		if( sndBind[a].snd == NULL )
		{
			sndBind[a].snd = ei->s_Load( sndBind[a].file );
		}
	}
}

/* ------------
menu_UnloadSounds
// unloads all of the precached sounds in sndBind; presumably all the sounds in the menu system
------------ */
static void menu_UnloadSounds(void)
{
	uint32_t a;

	for( a = 0 ; a < iNumMenuSounds ; a++ )
	{
		if( sndBind[a].snd != NULL )
		{
			ei->s_Unload( sndBind[a].snd );
			sndBind[a].snd = NULL;
		}
	}
}




