// Cognition
// client module
// cl_player.c
// Created 2-7-03 by Terrence Cole

// Includes
/////////////
#include "cl_global.h"
#include <math.h>

// Definitions
////////////////

// Global Prototypes
//////////////////////
/*
int pl_Initialize(void);
void pl_Terminate(void);
void pl_Disconnect();
void pl_SendData();
int pl_BuildOthers( byte bSize );
int pl_OtherClientJoin( unsigned short int size, byte *data );
int pl_SetPositionFromData( unsigned short int size, byte *data );
int pl_MovePlayer( exec_state_t *st );
void pl_Say( char *buffer );
void pl_Whisper( char *buffer );
*/


// Global Variables
////////////////////
/*
client_interface_t ci;
engine_interface_t *ei;
*/

// TU Local Vars
///////////////
static camera_t clCamera;
static entity_t *entClientSkybox;

// quasi-local prototypes
///////////////////////
void pl_UncoverCube( char *null );
void pl_FlagCube( char *null );
static void pl_ClipPlayerToWorld( world_t *world, entity_t *player, vec3 out, int iLocation[] );
static void pl_ClipPlayerToWorldFaces( vec3 out, wrld_cube_t ***cubes, entity_t *player, int loc[3], int size[3] );
static void pl_ClipPlayerToWorldEdges( vec3 out, wrld_cube_t ***cubes, entity_t *player, int loc[3], int size[3] );
static void pl_ClipPlayerToWorldCorners( vec3 out, wrld_cube_t ***cubes, entity_t *player, int loc[3], int size[3] );

void pl_MoveUp( char *null ); 
void pl_MoveDown( char *null ); 
void pl_MoveLeft( char *null ); 
void pl_MoveRight( char *null ); 
void pl_MoveForward( char *null ); 
void pl_MoveBackward( char *null ); 

void pl_ArcMoveUp( char *null );
void pl_ArcMoveDown( char *null ); 
void pl_ArcMoveLeft( char *null ); 
void pl_ArcMoveRight( char *null ); 
void pl_ArcMoveForward( char *null ); 
void pl_ArcMoveBackward( char *null );


// *********** FUNCTIONALITY ***********
/* ------------
pl_Initialize
------------ */
int pl_Initialize(void)
{
	// clear the game clock
	plState.ser_join_time = 0;
	plState.cl_join_time = 0;
	plState.time_offset = 0;

	// get the player name and model from the variable list
	plState.vlName = ei->var_GetVarLatch( "name" );
	plState.vlModel = ei->var_GetVarLatch( "model" );
	plState.vlSensitivity = ei->var_GetVarLatch( "sensitivity" );
	plState.vlMoveSpeed = ei->var_GetVarLatch( "movespeed" );
	plState.vlNoClip = ei->var_GetVarLatch( "noclip" );
	plState.key = -1;

	// assert
	if( plState.vlName == 0 ) return 0;
	if( plState.vlModel == 0 ) return 0;
	if( plState.vlSensitivity == 0 ) return 0;
	
	// entity_t *ent_Spawn( char *name, int flags, char *model, float *pos, float *rot, float *scale, void *ext_data, void (*think_func)(void), int nextThink );
	plState.self = ei->ent_Spawn( ei->var_GetString(plState.vlName), 0, ei->var_GetString(plState.vlModel), vZero, vZero );
	plState.state = PL_STATE_CONNECTING;

	// NULL all other ents
	plState.oKeys = NULL;
	plState.others = NULL;
	plState.bMaxClients = 0;
	plState.world = NULL;

	// check our player to see if we had an error
	if( plState.self == NULL )
	{
		ei->con_Print( "<RED>Client Error in Player Init:  Entity Load Failed for 'self'." );
		return 0;
	}
	
	// init our command callbacks in the engine
	plState.cmdMoveForward = 	ei->com_AddCommand( "+moveforward", pl_MoveForward );
	plState.cmdMoveBackward = 	ei->com_AddCommand( "+movebackward", pl_MoveBackward );
	plState.cmdMoveRight = 	ei->com_AddCommand( "+moveright", pl_MoveRight );
	plState.cmdMoveLeft = 		ei->com_AddCommand( "+moveleft", pl_MoveLeft );
	plState.cmdMoveUp = 		ei->com_AddCommand( "+moveup", pl_MoveUp );
	plState.cmdMoveDown = 	ei->com_AddCommand( "+movedown", pl_MoveDown );

	plState.cmdArcMoveForward = 	ei->com_AddCommand( "-moveforward", pl_ArcMoveForward );
	plState.cmdArcMoveBackward = ei->com_AddCommand( "-movebackward", pl_ArcMoveBackward );
	plState.cmdArcMoveRight =		ei->com_AddCommand( "-moveright", pl_ArcMoveRight );
	plState.cmdArcMoveLeft = 		ei->com_AddCommand( "-moveleft", pl_ArcMoveLeft );
	plState.cmdArcMoveUp = 		ei->com_AddCommand( "-moveup", pl_ArcMoveUp );
	plState.cmdArcMoveDown = 	ei->com_AddCommand( "-movedown", pl_ArcMoveDown );

	plState.cmdUncover = 		ei->com_AddCommand( "uncover", pl_UncoverCube );
	plState.cmdFlag = 			ei->com_AddCommand( "flag", pl_FlagCube );

	plState.cmdSay = 			ei->com_AddCommand( "say", pl_Say );
	plState.cmdWhisper = 			ei->com_AddCommand( "whisper", pl_Whisper );

	// null the move
	memset( plState.vMvReq, 0, sizeof(vec3) );

	// set a camera for the player
	memset( &clCamera, 0, sizeof(camera_t) );
	clCamera.type = CAMERA_PROJECTION;
	clCamera.vPos[0] = clCamera.vPos[1] = clCamera.vPos[2] = 0.0f;
	clCamera.vAng[0] = clCamera.vAng[1] = 0.0f;
	clCamera.fnear = 1.0f;
	clCamera.ffar = CL_FAR_CLIP_PLANE;
	clCamera.ffov = 45.0f;
	ei->d_SetCamera( &clCamera );

	// setup the hud
//	plState.hudReticle = ei->pixmap_Create( ei->win_GetRoot(), 32, 32, 

	return 1;
}

/* ------------
pl_Terminate
------------ */
void pl_Terminate(void)
{
	int a;

	// clear the game clock
	plState.ser_join_time = 0;
	plState.cl_join_time = 0;
	plState.time_offset = 0;
	
	// reset US
	plState.key = -1;
	plState.vlName = 0;
	plState.vlModel = 0;
	plState.vlSensitivity = 0;
	plState.vlMoveSpeed = 0;
	if( plState.self != NULL ) ei->ent_Remove( plState.self );

	// delete others
	for( a = 0 ; a < plState.bMaxClients ; a++ )
	{
		if( plState.oKeys[a] > 0 )
		{
			ei->ent_Remove( plState.others[a] );
			plState.oKeys[a] = -1;
		}
	}

	SAFE_RELEASE( plState.others );
	SAFE_RELEASE( plState.oKeys );

	// reset world
	if( plState.world != NULL ) ei->wrld_Delete( plState.world );
	plState.bMaxClients = 0;

	// remove command callbacks in the engine
	ei->com_RemoveCommand( plState.cmdMoveForward );
	ei->com_RemoveCommand( plState.cmdMoveBackward );
	ei->com_RemoveCommand( plState.cmdMoveRight );
	ei->com_RemoveCommand( plState.cmdMoveLeft );
	ei->com_RemoveCommand( plState.cmdMoveUp );
	ei->com_RemoveCommand( plState.cmdMoveDown );

	ei->com_RemoveCommand( plState.cmdArcMoveForward );
	ei->com_RemoveCommand( plState.cmdArcMoveBackward );
	ei->com_RemoveCommand( plState.cmdArcMoveRight );
	ei->com_RemoveCommand( plState.cmdArcMoveLeft );
	ei->com_RemoveCommand( plState.cmdArcMoveUp );
	ei->com_RemoveCommand( plState.cmdArcMoveDown );

	ei->com_RemoveCommand( plState.cmdUncover );
	ei->com_RemoveCommand( plState.cmdFlag );

	ei->com_RemoveCommand( plState.cmdSay );
	ei->com_RemoveCommand( plState.cmdWhisper );

	// reset the camera
	ei->d_SetCamera( NULL );

	// null the move
	memset( plState.vMvReq, 0, sizeof(vec3) );
}

/* ------------
pl_Disconnect - this tells the server that we are leaving;  the bye function
------------ */
void pl_Disconnect()
{
	// send the goodbye packet
	ei->net_cl_SendByte( PK_CTS_PLAYER_DISCONNECT );
	ei->net_cl_SendShort( 0 );
	ei->net_cl_Flush();

	// disconnect and close the socket
	ei->net_cl_Disconnect();
}

/* ------------
pl_SendData - sends the server the current player data - part of the handshake, unused elsewhere
------------ */
void pl_SendData()
{
	unsigned short int size;

	// get the size
	size = tcstrlen(ei->var_GetString(plState.vlName)) + tcstrlen(ei->var_GetString(plState.vlModel)) + 1;
	
	// send the server data about our player
	ei->net_cl_SendByte( PK_CTS_PLAYER_DATA );
	ei->net_cl_SendShort( size );
	ei->net_cl_SendString( ei->var_GetString(plState.vlName) );
	ei->net_cl_SendByte( 0 );
	ei->net_cl_SendString( ei->var_GetString(plState.vlModel) );
}

/* ------------
pl_BuildOthers - malloc ent pointers and keys from heap to store other joining clients
			- bSize is the number of clients that can join this game
------------ */
int pl_BuildOthers( byte bSize )
{
	int a;
	
	// assert
	if( bSize <= 0 ) return 0;

	// get key memory
	plState.oKeys = (int*)ei->mem_alloc( sizeof(int) * bSize );
	if( plState.oKeys == NULL )
	{
		ei->con_Print( "<RED>Client Error:  Unable to allocate memory for client key list!" );
		return 0;
	}

	// init to INVALID_KEY; deactivated
	for( a = 0 ; a < bSize ; a++ )
	{
		plState.oKeys[a] = INVALID_KEY;
	}

	// get ent ptr mem
	plState.others = (entity_t**)ei->mem_alloc( sizeof(entity_t*) * bSize );
	if( plState.others == NULL )
	{
		ei->con_Print( "<RED>Client Error:  Unable to allocate memory for client entity list!" );
		return 0;
	}
	memset( plState.others, 0, sizeof(entity_t*) * bSize );

	return 1;
}

/* ------------
pl_OtherClientJoin - add the other client to our ent list, load the model, and the key, etc.
------------ */
int pl_OtherClientJoin( unsigned short int size, byte *data )
{
	// key (4), name (n), break (1), model (n), break (1), pos (3 * 4), ang (2 * 2)
	int pl, a;
	unsigned long int ulIndex;
	vec3 vTmpPos;
	vec3 vTmpAng;
	char cTmpName[PLAYER_MAX_NAME_SIZE];
	char cTmpModel[PLAYER_MAX_NAME_SIZE];

	// assert:
	if( plState.oKeys == NULL ) return 0;
	if( plState.others == NULL ) return 0;

	// find an empty player state
	pl = 0;
	while( (pl < plState.bMaxClients) && (plState.oKeys[pl] != INVALID_KEY) )
	{
		pl++;
	}

	// assert
	if( (pl >= plState.bMaxClients) || (pl < 0) )
	{
		ei->con_Print( "<RED>Client Error:  Non-local Client Join.  Unable to find a free client entity." );
		return 0;
	}

	// copy over data
	ulIndex = 0;
	plState.oKeys[pl] = ei->net_GetLong( &data[ulIndex] );
	ulIndex += 4;
	ulIndex += ei->net_GetString( cTmpName, &data[ulIndex], PLAYER_MAX_NAME_SIZE );
	ulIndex += 1;
	ulIndex += ei->net_GetString( cTmpModel, &data[ulIndex], PLAYER_MAX_NAME_SIZE );

	// copy in the position
	for( a = 0 ; a < 3 ; a++ )
	{
		vTmpPos[a] = tcLongToPosition( ei->net_GetLong( &data[ulIndex] ) );
		ulIndex += 4;
	}

	// copy in the angle
	for( a = 0 ; a < 2 ; a++ )
	{
		vTmpAng[a] = tcShortToAngle( ei->net_GetShort( &data[ulIndex] ) );
		ulIndex += 2;
	}

	// spawn the other client entity
	plState.others[pl] = ei->ent_Spawn( cTmpName, 0, cTmpModel, vTmpPos, vTmpAng );
	if( plState.others[pl] == NULL )
	{
		ei->con_Print( "<RED>Client Error:  Non-local Client Join.  Unable to spawn the client entity." );
		return 0;
	}

	// tell the user about it
	ei->con_Print( "Client Message:  Player \"%s\" has joined the game.", plState.others[pl]->name );
	return 1;
}

/* ------------
pl_SetPositionFromData - decodes a server packet to a position of the client
------------ */
int pl_SetPositionFromData( unsigned short int size, byte *data )
{
	// assert
	if( size != 12 ) return 0;
	if( data == NULL ) return 0;

	// do the position assignment
	plState.self->vPos[X_AXIS] = tcLongToPosition( ei->net_GetLong( data ) );
	plState.self->vPos[Y_AXIS] = tcLongToPosition( ei->net_GetLong( &data[4] ) );
	plState.self->vPos[Z_AXIS] = tcLongToPosition( ei->net_GetLong( &data[8] ) );

	// do the camera assignment
	clCamera.vPos[X_AXIS] = plState.self->vPos[X_AXIS];
	clCamera.vPos[Y_AXIS] = plState.self->vPos[Y_AXIS];
	clCamera.vPos[Z_AXIS] = plState.self->vPos[Z_AXIS];

	// get a world position for the camera
	wrld_cl_GetWorldIndex( clCamera.vPos, clCamera.wrldPos );

	return 1;
}


/* ---------- EVENT PROCS  --------- */
/* ------------
pl_UncoverCube - default bind LClick - looks for the next visibile cube from center l.o.s. and attempts to uncover it
------------ */
void pl_UncoverCube( char *null )
{
	vec3 vAxisForward = { 0.0f, 0.0f, -1.0f };
	vec3 vForward;
	int vCube[3];
	byte bRet;
	byte bTmp;

	// get a forward vector from our data
	m3f_VecFromAngles( plState.self->vRot, vAxisForward, vForward );
	m3f_VecUnitize( vForward );

	// get the cube at which the ray intersects
	bRet = ei->wrld_RayIntersect( plState.world, plState.self->vPos, vForward, vCube );

	if( !bRet ) return;

	// unhide the cube
	// do in the engine to account for all the special uncover tasks
	ei->wrld_ClearSquare( plState.world, vCube[0], vCube[1], vCube[2] );

	// send it up to the server for rebroadcast
	bTmp = 0;
	bTmp = plState.world->cubes[vCube[0]][vCube[1]][vCube[2]].bHidden & (plState.world->cubes[vCube[0]][vCube[1]][vCube[2]].bFlagged << 1);
	
	// #define PK_CTS_WORLD_EVENT	4 //		pos (3 * 4), newState (1)
	ei->net_cl_SendByte( PK_CTS_WORLD_EVENT );
	ei->net_cl_SendShort( 13 );
	ei->net_cl_SendLong( vCube[0] );
	ei->net_cl_SendLong( vCube[1] );
	ei->net_cl_SendLong( vCube[2] );
	ei->net_cl_SendByte( bTmp );
}

/* ------------
pl_FlagCube - default bind is RMB - looks for the next visible cube directly in front of us and marks it as flagged
------------ */
void pl_FlagCube( char *null )
{
	vec3 vAxisForward = { 0.0f, 0.0f, -1.0f };
	vec3 vForward;
	int vCube[3];
	byte bRet;
	
	// get a forward vector from our data
	m3f_VecFromAngles( plState.self->vRot, vAxisForward, vForward );
	
	// get the cube at which the ray intersects
	bRet = ei->wrld_RayIntersect( plState.world, plState.self->vPos, vForward, vCube );
	
	if( !bRet ) return;
	
	// unhide the cube
	plState.world->cubes[vCube[0]][vCube[1]][vCube[2]].bFlagged ^= 1;
}

void pl_MoveUp( char *null ) { plState.vMvReq[Y_AXIS] = 1.0f; }
void pl_MoveDown( char *null ) { plState.vMvReq[Y_AXIS] = -1.0f; }
void pl_MoveLeft( char *null ) { plState.vMvReq[X_AXIS] = -1.0f; }
void pl_MoveRight( char *null ) { plState.vMvReq[X_AXIS] = 1.0f; }
void pl_MoveForward( char *null ) { plState.vMvReq[Z_AXIS] = -1.0f; }
void pl_MoveBackward( char *null ) { plState.vMvReq[Z_AXIS] = 1.0f; }

void pl_ArcMoveUp( char *null ) { if( plState.vMvReq[Y_AXIS] > 0.0f ) { plState.vMvReq[Y_AXIS] = 0.0f; } }
void pl_ArcMoveDown( char *null ) { if( plState.vMvReq[Y_AXIS] < 0.0f ) { plState.vMvReq[Y_AXIS] = 0.0f; } }
void pl_ArcMoveLeft( char *null ) { if( plState.vMvReq[X_AXIS] < 0.0f ) { plState.vMvReq[X_AXIS] = 0.0f; } }
void pl_ArcMoveRight( char *null ) { if( plState.vMvReq[X_AXIS] > 0.0f ) { plState.vMvReq[X_AXIS] = 0.0f; } }
void pl_ArcMoveForward( char *null ) { if( plState.vMvReq[Z_AXIS] < 0.0f ) { plState.vMvReq[Z_AXIS] = 0.0f; } }
void pl_ArcMoveBackward( char *null ) { if( plState.vMvReq[Z_AXIS] > 0.0f ) { plState.vMvReq[Z_AXIS] = 0.0f; } }


/* ------------
pl_MovePlayer - moves the player to a new position based on the move request
------------ */
int pl_MovePlayer( exec_state_t *st )
{
	float factor;
	vec3 out;
	camera_t *camTmp;

	// if we are in menu mode, don't move the player
	if( st->bDrawMenu ) return 1;

	// check the camera, and aggressivly make it our own
	camTmp = ei->d_GetCamera();
	if( (camTmp == NULL) || (camTmp != &clCamera) )
	{
		ei->con_Print( "Client Note:  recapturing camera." );
		ei->d_SetCamera( &clCamera );
	}

	// we now need to update the player angle with the current mouse axis offset
	factor = ((float)st->delta_time / 1000.0f) * ei->var_GetFloat(plState.vlSensitivity);

	// compute a new angle
	clCamera.vAng[PITCH] += (float)(st->mAxisDelta[PITCH]) * factor;
	clCamera.vAng[YAW] += (float)(st->mAxisDelta[YAW]) * factor;

	// lock yaw angle to [0,360)
	while( clCamera.vAng[YAW] < 0.0f ) clCamera.vAng[YAW] += 360.0f;
	while( clCamera.vAng[YAW] >= 360.0f ) clCamera.vAng[YAW] -= 360.0f;

	// lock pitch to [0,360)
	while( clCamera.vAng[PITCH] < 0.0f ) clCamera.vAng[PITCH] += 360.0f;
	while( clCamera.vAng[PITCH] >= 360.0f ) clCamera.vAng[PITCH] -= 360.0f;

	// lock roll to [0,0]
	clCamera.vAng[ROLL] = 0.0f;

	// set gimbal limits for pitch
	if( (clCamera.vAng[PITCH] > 180.0f) && (clCamera.vAng[PITCH] < 270.0f) ) clCamera.vAng[PITCH] = 270.0f;
	if( (clCamera.vAng[PITCH] <= 180.0f) && (clCamera.vAng[PITCH] > 90.0f) ) clCamera.vAng[PITCH] = 90.0f;

	// tell the engine to move us
	ei->d_MoveCamera( ei->var_GetFloat(plState.vlMoveSpeed), clCamera.vAng,  plState.vMvReq, out );

	// clip our player to the world
	if( !((int)ei->var_GetFloat( plState.vlNoClip )) )
	{
		pl_ClipPlayerToWorld( plState.world, plState.self, out, clCamera.wrldPos );
	}
	
	// compute the camera velocity for this frame in each axis
	camTmp->vVel[X_AXIS] = out[X_AXIS] * 1000.0f / (float)plState.tDelta; // in units per second
	camTmp->vVel[Y_AXIS] = out[Y_AXIS] * 1000.0f / (float)plState.tDelta;
	camTmp->vVel[Z_AXIS] = out[Z_AXIS] * 1000.0f / (float)plState.tDelta; 
	
	// assign this angle to the model as well
	if( plState.self != NULL )
	{
		plState.self->vRot[PITCH] = clCamera.vAng[PITCH];
		plState.self->vRot[YAW] = clCamera.vAng[YAW];
		plState.self->vRot[ROLL] = clCamera.vAng[ROLL];

		// use out to adjust ourselves
		plState.self->vPos[X_AXIS] = (clCamera.vPos[X_AXIS] += out[X_AXIS]);
		plState.self->vPos[Y_AXIS] = (clCamera.vPos[Y_AXIS] += out[Y_AXIS]);
		plState.self->vPos[Z_AXIS] = (clCamera.vPos[Z_AXIS] += out[Z_AXIS]);
	}
	else
	{
		clCamera.vPos[X_AXIS] += out[X_AXIS];
		clCamera.vPos[Y_AXIS] += out[Y_AXIS];
		clCamera.vPos[Z_AXIS] += out[Z_AXIS];
	}

	// get a new camera sector
	wrld_cl_GetWorldIndex( clCamera.vPos, clCamera.wrldPos );

	// delta compress
	if( (out[X_AXIS] != 0) || (out[Y_AXIS] != 0) || (out[Z_AXIS] != 0) )
	{
		// tell the server we moved
		ei->net_cl_SendByte( PK_CTS_PLAYER_POS );
		ei->net_cl_SendShort( 12 );
		ei->net_cl_SendLong( tcPositionToLong( plState.self->vPos[X_AXIS] ) );
		ei->net_cl_SendLong( tcPositionToLong( plState.self->vPos[Y_AXIS] ) );
		ei->net_cl_SendLong( tcPositionToLong( plState.self->vPos[Z_AXIS] ) );
	}

	// delta compress
	if( (st->mAxisDelta[YAW] != 0) || (st->mAxisDelta[PITCH] != 0) )
	{
		// tell the server we tilted
		ei->net_cl_SendByte( PK_CTS_PLAYER_ANG );
		ei->net_cl_SendShort( 4 );
		ei->net_cl_SendShort( tcAngleToShort( plState.self->vRot[PITCH] ) );
		ei->net_cl_SendShort( tcAngleToShort( plState.self->vRot[YAW] ) );
	}
	
	return 1;
}

/* ------------
pl_Say - the callback for when someone types 'say'; passes the message to the server for dispersal
------------ */
void pl_Say( char *buffer )
{
	// assert
	if( buffer == NULL ) return;

	// transmit
	ei->net_cl_SendByte( PK_CTS_MESSAGE );
	ei->net_cl_SendShort( (unsigned short int)(tcstrlen(buffer) + 4) );
	ei->net_cl_SendLong( 0 );
	ei->net_cl_SendString( buffer );
}

/* ------------
pl_Whisper - the callback for when someone types 'whisper'; searches for the player of name; 
			passes the message to the server for dispersal
------------ */
void pl_Whisper( char *buffer )
{
	char plName[PLAYER_MAX_NAME_SIZE + 1];
	byte bDone = 0;
	int index;
	int pl;
	
	// assert
	if( buffer == NULL ) return;

	// find a player
	while( (*buffer) && (*buffer == ' ') || (*buffer == '\t') ) buffer++;

	// read in the player
	index = 0;
	while( (index < PLAYER_MAX_NAME_SIZE) && (*buffer) && (*buffer != ' ') && (*buffer != '\t') )
	{
		plName[index] = *buffer;
		index++;
		buffer++;
	}
	plName[index] = '\0';

	// find a player
	bDone = 0;
	pl = 0;
	while( (!bDone) && (pl < plState.bMaxClients) )
	{
		if( tcstricmp( plState.others[pl]->name, plName ) )
		{
			bDone = 1;
		}
		else
		{
			pl++;
		}
	}

	// send the message with the proper key
	ei->net_cl_SendByte( PK_CTS_MESSAGE );
	ei->net_cl_SendShort( (unsigned short int)(tcstrlen(buffer) + 4) );
	ei->net_cl_SendLong( plState.oKeys[pl] );
	ei->net_cl_SendString( buffer );
}

/* ------------
pl_ClipPlayerToWorld
out == movement request; change to what we can do before leaving

// Strategy
// 1) clip players against the faces of the cubes to which they are pointing
// 2) clip players against edges of all filled adjacent cubes
// 3) clip players agains corners of all cattycorner filled cubes
------------ */
static void pl_ClipPlayerToWorld( world_t *world, entity_t *player, vec3 out, int iLocation[] )
{
	assert( world != NULL );
	assert( player != NULL );
	assert( out != NULL );
	assert( iLocation != NULL );

	if( world == NULL )
	{
		ei->con_Print( "<RED>Assert Failed in ClipPlayerToWorld:  world is NULL." );
		return;
	}
	if( player == NULL )
	{
		ei->con_Print( "<RED>Assert Failed in ClipPlayerToWorld:  player is NULL." );
		return;
	}
	if( out == NULL )
	{
		ei->con_Print( "<RED>Assert Failed in ClipPlayerToWorld:  out is NULL." );
		return;
	}
	if( iLocation == NULL )
	{
		ei->con_Print( "<RED>Assert Failed in ClipPlayerToWorld:  iLocation is NULL." );
		return;
	}

	pl_ClipPlayerToWorldFaces( out, plState.world->cubes, player, iLocation, plState.world->size );
	pl_ClipPlayerToWorldEdges( out, plState.world->cubes, player, iLocation, plState.world->size );
	pl_ClipPlayerToWorldCorners( out, plState.world->cubes, player, iLocation, plState.world->size );
}

/* ------------
pl_ClipPlayerToWorldFaces
out == movement request; change to what we can do before leaving
------------ */
static void pl_ClipPlayerToWorldFaces( vec3 out, wrld_cube_t ***cubes, entity_t *player, int loc[3], int size[3] )
{
	int comp[3];
	float fDist;

	// on the X axis
	if( (out[X_AXIS] > 0.0f) && (loc[X_AXIS] < (size[X_AXIS] - 1)) )
	{
		comp[0] = loc[0] + 1;
		comp[1] = loc[1];
		comp[2] = loc[2];
		if( (comp[X_AXIS] >= 0) && (comp[X_AXIS] < size[X_AXIS]) &&
			(comp[Y_AXIS] >= 0) && (comp[Y_AXIS] < size[Y_AXIS]) &&
			(comp[Z_AXIS] >= 0) && (comp[Z_AXIS] < size[Z_AXIS]) &&
			(cubes[comp[0]][comp[1]][comp[2]].bHidden) )
		{
			// loX 0, 3, 4, 7
			fDist = player->vPos[X_AXIS] + player->fRadius + out[X_AXIS] - cubes[comp[0]][comp[1]][comp[2]].verts[0][X_AXIS];
			if( fDist > 0.0f ) out[X_AXIS] -= fDist;
		}
	}
	else if( (out[X_AXIS] < 0.0f) && (loc[X_AXIS] > 0) )
	{
		comp[0] = loc[0] - 1;
		comp[1] = loc[1];
		comp[2] = loc[2];
		if( (comp[X_AXIS] >= 0) && (comp[X_AXIS] < size[X_AXIS]) &&
			(comp[Y_AXIS] >= 0) && (comp[Y_AXIS] < size[Y_AXIS]) &&
			(comp[Z_AXIS] >= 0) && (comp[Z_AXIS] < size[Z_AXIS]) &&
			(cubes[comp[0]][comp[1]][comp[2]].bHidden) )
		{
			// hiX 1, 2, 5, 6
			fDist = player->vPos[X_AXIS] - player->fRadius + out[X_AXIS] - cubes[comp[0]][comp[1]][comp[2]].verts[6][X_AXIS];
			if( fDist < 0.0f ) out[X_AXIS] -= fDist;
		}
	}
	else
	{
		// no movement means no collision (on x)
	}

	// on the Y axis
	if( (out[Y_AXIS] > 0.0f) && (loc[Y_AXIS] < (size[Y_AXIS] - 1)) )
	{
		comp[0] = loc[0];
		comp[1] = loc[1] + 1;
		comp[2] = loc[2];
		if( (comp[X_AXIS] >= 0) && (comp[X_AXIS] < size[X_AXIS]) &&
			(comp[Y_AXIS] >= 0) && (comp[Y_AXIS] < size[Y_AXIS]) &&
			(comp[Z_AXIS] >= 0) && (comp[Z_AXIS] < size[Z_AXIS]) &&
			(cubes[comp[0]][comp[1]][comp[2]].bHidden) )
		{
			// loY = 0, 1, 4, 5
			fDist = player->vPos[Y_AXIS] + player->fRadius + out[Y_AXIS] - cubes[comp[0]][comp[1]][comp[2]].verts[0][Y_AXIS];
			if( fDist > 0.0f ) out[Y_AXIS] -= fDist;
		}

	}
	else if( (out[Y_AXIS] < 0.0f) && (loc[Y_AXIS] > 0) )
	{
		comp[0] = loc[0];
		comp[1] = loc[1] - 1;
		comp[2] = loc[2];
		if( (comp[X_AXIS] >= 0) && (comp[X_AXIS] < size[X_AXIS]) &&
			(comp[Y_AXIS] >= 0) && (comp[Y_AXIS] < size[Y_AXIS]) &&
			(comp[Z_AXIS] >= 0) && (comp[Z_AXIS] < size[Z_AXIS]) &&
			(cubes[comp[0]][comp[1]][comp[2]].bHidden) )
		{
			// hiY = 2, 3, 6, 7			
			fDist = player->vPos[Y_AXIS] - player->fRadius + out[Y_AXIS] - cubes[comp[0]][comp[1]][comp[2]].verts[6][Y_AXIS];
			if( fDist < 0.0f ) out[Y_AXIS] -= fDist;
		}
	}
	else
	{
		// no movement means no collision (on y)
	}

	// on the Z axis
	if( (out[Z_AXIS] > 0.0f) && (loc[Z_AXIS] < (size[Z_AXIS] - 1)) )
	{
		comp[0] = loc[0];
		comp[1] = loc[1];
		comp[2] = loc[2] + 1;
		if( (comp[X_AXIS] >= 0) && (comp[X_AXIS] < size[X_AXIS]) &&
			(comp[Y_AXIS] >= 0) && (comp[Y_AXIS] < size[Y_AXIS]) &&
			(comp[Z_AXIS] >= 0) && (comp[Z_AXIS] < size[Z_AXIS]) &&
			(cubes[comp[0]][comp[1]][comp[2]].bHidden) )
		{
			// loZ = 0, 1, 2, 3
			fDist = player->vPos[Z_AXIS] + player->fRadius + out[Z_AXIS] - cubes[comp[0]][comp[1]][comp[2]].verts[0][Z_AXIS];
			if( fDist > 0.0f ) out[Z_AXIS] -= fDist;
		}

	}
	else if( (out[Z_AXIS] < 0.0f) && (loc[Z_AXIS] > 0) )
	{
		comp[0] = loc[0];
		comp[1] = loc[1];
		comp[2] = loc[2] - 1;
		if( (comp[X_AXIS] >= 0) && (comp[X_AXIS] < size[X_AXIS]) &&
			(comp[Y_AXIS] >= 0) && (comp[Y_AXIS] < size[Y_AXIS]) &&
			(comp[Z_AXIS] >= 0) && (comp[Z_AXIS] < size[Z_AXIS]) &&
			(cubes[comp[0]][comp[1]][comp[2]].bHidden) )
		{
			// hiZ = 4, 5, 6, 7
			fDist = player->vPos[Z_AXIS] - player->fRadius + out[Z_AXIS] - cubes[comp[0]][comp[1]][comp[2]].verts[6][Z_AXIS];
			if( fDist < 0.0f ) out[Z_AXIS] -= fDist;
		}
	}
	else
	{
		// no movement means no collision (on y)
	}
}

/* ------------
pl_ClipPlayerToWorldEdges
out == movement request; change to what we can do before leaving
------------ */
static void pl_ClipPlayerToWorldEdges( vec3 out, wrld_cube_t ***cubes, entity_t *player, int loc[3], int size[3] )
{
	int a;
	int bCubeVert;
	int comp[3];
	vec3 vTo;
	float fDist;
	float fOutOnTo;
	float fOvershoot;
	
	// isolate on the y plane
	for( a = 0 ; a < 4 ; a++ )
	{
		// pre-process the special cases for each pair
		comp[Y_AXIS] = loc[Y_AXIS];
		switch( a )
		{
			case 0:  // loX, loZ
				comp[X_AXIS] = loc[X_AXIS] - 1;
				comp[Z_AXIS] = loc[Z_AXIS] - 1;
				bCubeVert = 5;  // 5, 6 
				break;
			case 1: // loX, hiZ
				comp[X_AXIS] = loc[X_AXIS] - 1;
				comp[Z_AXIS] = loc[Z_AXIS] + 1;
				bCubeVert = 1;  // 1, 2 
				break;
			case 2: // hiX, loZ
				comp[X_AXIS] = loc[X_AXIS] + 1;
				comp[Z_AXIS] = loc[Z_AXIS] - 1;
				bCubeVert = 4;  // 4, 7
				break;
			case 3: // hiX, hiZ
				comp[X_AXIS] = loc[X_AXIS] + 1;
				comp[Z_AXIS] = loc[Z_AXIS] + 1;
				bCubeVert = 0; // 0, 3
				break;
			default:
				return;
		}

		// process in a general manner each of this planes possible edge collisions
		if( (comp[X_AXIS] >= 0) && (comp[X_AXIS] < size[X_AXIS]) &&
			(comp[Y_AXIS] >= 0) && (comp[Y_AXIS] < size[Y_AXIS]) &&
			(comp[Z_AXIS] >= 0) && (comp[Z_AXIS] < size[Z_AXIS]) &&
			(cubes[comp[0]][comp[1]][comp[2]].bHidden) )
		{	
			vTo[X_AXIS] = cubes[comp[0]][comp[1]][comp[2]].verts[bCubeVert][X_AXIS] - player->vPos[X_AXIS];
			vTo[Z_AXIS] = cubes[comp[0]][comp[1]][comp[2]].verts[bCubeVert][Z_AXIS] - player->vPos[Z_AXIS];
			fDist = (float)sqrt( vTo[X_AXIS] * vTo[X_AXIS] + vTo[Z_AXIS] * vTo[Z_AXIS] );

			// amount of out in direction of considered edge:  |V|cos(w)
			// |V|cos(w) -> cos(w) = V*W / |V||W| -> |V|cos(w) = V*W / |W|
			fOutOnTo = (out[X_AXIS] * vTo[X_AXIS] + out[Z_AXIS] * vTo[Z_AXIS]) / fDist;

			// compute the overlap (negative)
			fOvershoot = fDist - player->fRadius - fOutOnTo;
			if( fOvershoot < 0.0f )
			{
				// add the overshoot to the output
				out[X_AXIS] += (vTo[X_AXIS] / fDist) * fOvershoot;
				out[Z_AXIS] += (vTo[Z_AXIS] / fDist) * fOvershoot;
			}
		}
	}

	// isolate on the x plane
	for( a = 0 ; a < 4 ; a++ )
	{
		// pre-process the special cases for each pair
		comp[X_AXIS] = loc[X_AXIS];
		switch( a )
		{
			case 0:  // lo, lo
				comp[Y_AXIS] = loc[Y_AXIS] - 1;
				comp[Z_AXIS] = loc[Z_AXIS] - 1;
				bCubeVert = 6; // 6, 7
				break;
			case 1: // lo, hi
				comp[Y_AXIS] = loc[Y_AXIS] - 1;
				comp[Z_AXIS] = loc[Z_AXIS] + 1;
				bCubeVert = 2; // 2, 3
				break;
			case 2: // hi, lo
				comp[Y_AXIS] = loc[Y_AXIS] + 1;
				comp[Z_AXIS] = loc[Z_AXIS] - 1;
				bCubeVert = 4; // 4, 5 
				break;
			case 3: // hi, hi
				comp[Y_AXIS] = loc[Y_AXIS] + 1;
				comp[Z_AXIS] = loc[Z_AXIS] + 1;
				bCubeVert = 0; // 0, 1
				break;
			default:
				return;
		}

		// process in a general manner each of this planes possible edge collisions
		if( (comp[X_AXIS] >= 0) && (comp[X_AXIS] < size[X_AXIS]) &&
			(comp[Y_AXIS] >= 0) && (comp[Y_AXIS] < size[Y_AXIS]) &&
			(comp[Z_AXIS] >= 0) && (comp[Z_AXIS] < size[Z_AXIS]) &&
			(cubes[comp[0]][comp[1]][comp[2]].bHidden) )
		{	
			vTo[Y_AXIS] = cubes[comp[0]][comp[1]][comp[2]].verts[bCubeVert][Y_AXIS] - player->vPos[Y_AXIS];
			vTo[Z_AXIS] = cubes[comp[0]][comp[1]][comp[2]].verts[bCubeVert][Z_AXIS] - player->vPos[Z_AXIS];
			fDist = (float)sqrt( vTo[Y_AXIS] * vTo[Y_AXIS] + vTo[Z_AXIS] * vTo[Z_AXIS] );

			// amount of out in direction of considered edge:  |V|cos(w)
			// |V|cos(w) -> cos(w) = V*W / |V||W| -> |V|cos(w) = V*W / |W|
			fOutOnTo = (out[Y_AXIS] * vTo[Y_AXIS] + out[Z_AXIS] * vTo[Z_AXIS]) / fDist;

			// compute the overlap (negative)
			fOvershoot = fDist - player->fRadius - fOutOnTo;
			if( fOvershoot < 0.0f )
			{
				// add the overshoot to the output
				out[Y_AXIS] += (vTo[Y_AXIS] / fDist) * fOvershoot;
				out[Z_AXIS] += (vTo[Z_AXIS] / fDist) * fOvershoot;
			}
		}
	}

	// isolate on the z plane
	for( a = 0 ; a < 4 ; a++ )
	{
		// pre-process the special cases for each pair
		comp[Z_AXIS] = loc[Z_AXIS];
		switch( a )
		{
			case 0:  // lo, lo
				comp[X_AXIS] = loc[X_AXIS] - 1;
				comp[Y_AXIS] = loc[Y_AXIS] - 1;
				bCubeVert = 6; // 2, 6
				break;
			case 1: // lo, hi
				comp[X_AXIS] = loc[X_AXIS] - 1;
				comp[Y_AXIS] = loc[Y_AXIS] + 1;
				bCubeVert = 1; // 1, 5
				break;
			case 2: // hi, lo
				comp[X_AXIS] = loc[X_AXIS] + 1;
				comp[Y_AXIS] = loc[Y_AXIS] - 1;
				bCubeVert = 3; // 3, 7 
				break;
			case 3: // hi, hi
				comp[X_AXIS] = loc[X_AXIS] + 1;
				comp[Y_AXIS] = loc[Y_AXIS] + 1;
				bCubeVert = 0; // 0, 4
				break;
			default:
				return;
		}

		// process in a general manner each of this planes possible edge collisions
		if( (comp[X_AXIS] >= 0) && (comp[X_AXIS] < size[X_AXIS]) &&
			(comp[Y_AXIS] >= 0) && (comp[Y_AXIS] < size[Y_AXIS]) &&
			(comp[Z_AXIS] >= 0) && (comp[Z_AXIS] < size[Z_AXIS]) &&
			(cubes[comp[0]][comp[1]][comp[2]].bHidden) )
		{	
			vTo[X_AXIS] = cubes[comp[0]][comp[1]][comp[2]].verts[bCubeVert][X_AXIS] - player->vPos[X_AXIS];
			vTo[Y_AXIS] = cubes[comp[0]][comp[1]][comp[2]].verts[bCubeVert][Y_AXIS] - player->vPos[Y_AXIS];
			fDist = (float)sqrt( vTo[X_AXIS] * vTo[X_AXIS] + vTo[Y_AXIS] * vTo[Y_AXIS] );

			// amount of out in direction of considered edge:  |V|cos(w)
			// |V|cos(w) -> cos(w) = V*W / |V||W| -> |V|cos(w) = V*W / |W|
			fOutOnTo = (out[X_AXIS] * vTo[X_AXIS] + out[Y_AXIS] * vTo[Y_AXIS]) / fDist;

			// compute the overlap (negative)
			fOvershoot = fDist - player->fRadius - fOutOnTo;
			if( fOvershoot < 0.0f )
			{
				// add the overshoot to the output
				out[X_AXIS] += (vTo[X_AXIS] / fDist) * fOvershoot;
				out[Y_AXIS] += (vTo[Y_AXIS] / fDist) * fOvershoot;
			}
		}
	}
}

/* ------------
pl_ClipPlayerToWorldCorners
out == movement request; change to what we can do before leaving
------------ */
static void pl_ClipPlayerToWorldCorners( vec3 out, wrld_cube_t ***cubes, entity_t *player, int loc[3], int size[3] )
{
	int a;
	int comp[3];
	vec3 vTo;
	float fDist;
	float fOutOnTo;
	float fOvershoot;
	
	for( a = 0 ; a < 8 ; a++ )
	{
		// setup for each case
		switch( a )
		{
		case 0: // hi, hi, hi
			comp[0] = loc[0] + 1;
			comp[1] = loc[1] + 1;
			comp[2] = loc[2] + 1;
			break;
		case 1: // lo, hi, hi
			comp[0] = loc[0] - 1;
			comp[1] = loc[1] + 1;
			comp[2] = loc[2] + 1;
			break;
		case 2:  // lo, lo, hi
			comp[0] = loc[0] - 1;
			comp[1] = loc[1] - 1;
			comp[2] = loc[2] + 1;
			break;
		case 3: // hi, lo, hi
			comp[0] = loc[0] + 1;
			comp[1] = loc[1] - 1;
			comp[2] = loc[2] + 1;
			break;
		case 4: // hi, hi, lo
			comp[0] = loc[0] + 1;
			comp[1] = loc[1] + 1;
			comp[2] = loc[2] - 1;
			break;
		case 5: // lo, hi. lo
			comp[0] = loc[0] - 1;
			comp[1] = loc[1] + 1;
			comp[2] = loc[2] - 1;
			break;
		case 6: // lo, lo, lo
			comp[0] = loc[0] - 1;
			comp[1] = loc[1] - 1;
			comp[2] = loc[2] - 1;
			break;
		case 7: // hi, lo, lo
			comp[0] = loc[0] + 1;
			comp[1] = loc[1] - 1;
			comp[2] = loc[2] - 1;
			break;
		default:
			return;
		}

		if( (comp[X_AXIS] >= 0) && (comp[X_AXIS] < size[X_AXIS]) &&
			(comp[Y_AXIS] >= 0) && (comp[Y_AXIS] < size[Y_AXIS]) &&
			(comp[Z_AXIS] >= 0) && (comp[Z_AXIS] < size[Z_AXIS]) &&
			(cubes[comp[0]][comp[1]][comp[2]].bHidden) )
		{	
			// Taut: bCubeVert == a
			m3f_VecSubtract( vTo, cubes[comp[0]][comp[1]][comp[2]].verts[a], player->vPos );
			fDist = (float)sqrt( m3f_VecDot( vTo, vTo ) );

			// amount of out in direction of considered edge:  |V|cos(w)
			// |V|cos(w) -> cos(w) = V*W / |V||W| -> |V|cos(w) = V*W / |W|
			fOutOnTo = m3f_VecDot( vTo, out ) /fDist; 

			// compute overlap (-)
			fOvershoot = fDist - player->fRadius - fOutOnTo;
			if( fOvershoot < 0.0f )
			{
				// add overshoot to output
				out[X_AXIS] += (vTo[X_AXIS] / fDist) * fOvershoot;
				out[Y_AXIS] += (vTo[Y_AXIS] / fDist) * fOvershoot;
				out[Z_AXIS] += (vTo[Z_AXIS] / fDist) * fOvershoot;
			}
		}
	}
}


