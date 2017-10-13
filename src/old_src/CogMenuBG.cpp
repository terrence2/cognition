// Cognition
// CogMenuBG.cpp
// Created by Terrence Cole
// 12/25/03

// Includes
/////////////
#include "CogMenuBG.h"
#include <assert.h>

// Definitions
////////////////
#define MENU_FAR_CLIP_PLANE 10000.0f

// Local Structures
/////////////////////

// Global Prototypes
//////////////////////
/*
*/

// Local Prototypes
/////////////////////

// Local Variables
////////////////////
static vec3 vZero = { 0.0f, 0.0f, 0.0f };
static vec3 vUnity = { 1.0f, 1.0f, 1.0f };

// *********** FUNCTIONALITY ***********
/* ------------
Background init
------------ */
CogMenuBG::CogMenuBG()
{
	vec4 vLtPos1 = { 0.7f, 0.7f, 1.0f, 0.0f }; 
	vec4 vLtAmb = { 0.25f, 0.25f, 0.22f, 1.0f };
	vec4 vLtDif1 = { 1.0f, 1.0f, 0.93f, 1.0f };
	vec4 vLtSpec = { 1.0f, 1.0f, 1.0f, 1.0f };
	vec3 vLtSptDir = { 0.0f, 0.0f, 0.0f };

	// spawn a light for the scene
	m_Light = ei()->l_Spawn( LIGHT_DIRECTIONAL, vLtPos1, vLtAmb, vLtDif1, vLtSpec, 0, 45, vLtSptDir, 1.0f, 0.0f, 0.0f );
	if( m_Light == NULL )
	{
		ei()->con_Print( "Menu Background Error on Init:  Light Spawn returned a NULL light." );
		return;
	}

	// set up a camera
	memset( &m_Camera, 0, sizeof(camera_t) );
	m_Camera.type = CAMERA_PROJECTION;
	m_Camera.vPos[0] = m_Camera.vPos[1] = m_Camera.vPos[2] = 0.0f;
	m_Camera.vAng[0] = m_Camera.vAng[1] = 0.0f;
	m_Camera.fnear = 1.0f;
	m_Camera.ffar = MENU_FAR_CLIP_PLANE;
	m_Camera.ffov = 45.0f;
	ei()->d_SetCamera( &m_Camera );

	// set up a skybox
	m_eSkybox = ei()->ent_Spawn( "skybox", 0, "skybox.def", (float*)vZero, (float*)vZero, (float*)vUnity, NULL, NULL, 0 );
	m_eSkybox->bHintHidden = 1;
	m_eSkybox->rotation[YAW] = 180.0f;
	ei()->d_SetSkybox( m_eSkybox );

	// name
	string name = "mPart0";
	vec3 pos = { 0.0f, 0.0f, -100.0f };
	vec3 vel = { 0.0f, 0.0f, 0.0f };
	vec3 ang_velocity = { 40.0f, 20.0f };
	vec3 rot = { 270.0f, 0.0f };
	vec3 scale = { 1.0f, 1.0f, 1.0f };

	// spawn the particle
	m_particles[0] = NULL;
	m_particles[1] = NULL;
	m_particles[0] = ei()->ent_Spawn( (char*)name.c_str(), ENT_FLAG_MENU_ITEM, "comet_25_frozen_white.def", (float*)vZero, (float*)vZero, (float*)vUnity, NULL, NULL, 0 );
//	m_particles[1] = ei()->ent_Spawn( (char*)name.c_str(), ENT_FLAG_MENU_ITEM, "comet_100_ice_trans.def", (float*)vZero, (float*)vZero, (float*)vUnity, NULL, NULL, 0 );

	if( m_particles[0] )
	{
		m_particles[0]->position[X_AXIS] = 00.0f;
		m_particles[0]->position[Y_AXIS] = 0.0f;
		m_particles[0]->position[Z_AXIS] = -50.0f;

		m_particles[0]->velocity[X_AXIS] = 0.0f;
		m_particles[0]->velocity[Y_AXIS] = 0.0f;
		m_particles[0]->velocity[Z_AXIS] = 0.0f;

		m_particles[0]->ang_velocity[PITCH] = 0.0f;
		m_particles[0]->ang_velocity[YAW] = 40.0f;

		m_particles[0]->rotation[PITCH] = 45.0f;
		m_particles[0]->rotation[YAW] = 90.0f;
	}
	if( m_particles[1] )
	{
		m_particles[1]->position[X_AXIS] = 0.0f;
		m_particles[1]->position[Y_AXIS] = 0.0f;
		m_particles[1]->position[Z_AXIS] = -200.0f;

		m_particles[1]->velocity[X_AXIS] = 0.0f;
		m_particles[1]->velocity[Y_AXIS] = 0.0f;
		m_particles[1]->velocity[Z_AXIS] = 0.0f;

		m_particles[1]->ang_velocity[PITCH] = 0.0f;
		m_particles[1]->ang_velocity[YAW] = 0.0f;

		m_particles[1]->rotation[PITCH] = 0.0f;
		m_particles[1]->rotation[YAW] = 0.0f;
	}
}

/* ------------
Frame
------------ */
void CogMenuBG::Frame( exec_state_t *state )
{
	float dt = state->delta_time / 1000.0f;
		
	if( m_particles[0] )
	{
		// use the acceleration to change the velocity
		// v2 = v1 + a * t
		m_particles[0]->velocity[X_AXIS] += m_particles[0]->accel[X_AXIS] * dt;
		m_particles[0]->velocity[Y_AXIS] += m_particles[0]->accel[Y_AXIS] * dt;
		m_particles[0]->velocity[Z_AXIS] += m_particles[0]->accel[Z_AXIS] * dt;

		// use the velocity to calculate a change in position
		m_particles[0]->position[X_AXIS] += m_particles[0]->velocity[X_AXIS] * dt;
		m_particles[0]->position[Y_AXIS] += m_particles[0]->velocity[Y_AXIS] * dt;
		m_particles[0]->position[Z_AXIS] += m_particles[0]->velocity[Z_AXIS] * dt;

		// use the w to calculate a new rotation
		m_particles[0]->rotation[PITCH] += m_particles[0]->ang_velocity[PITCH] * dt;
		m_particles[0]->rotation[YAW] += m_particles[0]->ang_velocity[YAW] * dt;

		// cap the rotation on X
		while( m_particles[0]->rotation[PITCH] >= 360.0 ) m_particles[0]->rotation[PITCH] -= 360.0f;
		while( m_particles[0]->rotation[PITCH] < 0.0f ) m_particles[0]->rotation[PITCH] += 360.0f;

		// cap the rotation on Y
		while( m_particles[0]->rotation[YAW] >= 360.0 ) m_particles[0]->rotation[YAW] -= 360.0f;
		while( m_particles[0]->rotation[YAW] < 0.0f ) m_particles[0]->rotation[YAW] += 360.0f;
	}
}
	
/* ------------
Background cleanup
------------ */
CogMenuBG::~CogMenuBG()
{
	if( m_particles[0] ) ei()->ent_Remove( m_particles[0] );
	if( m_particles[1] ) ei()->ent_Remove( m_particles[1] );
	
	// kill the lights
	ei()->l_Kill( m_Light );
	
	// remove the camera
	ei()->d_SetCamera(NULL);

	// remove the skybox
	if( m_eSkybox )
	{
		ei()->d_SetSkybox( NULL );
		ei()->ent_Remove( m_eSkybox );
	}
}


