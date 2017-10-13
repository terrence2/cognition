// Cognition
// client_global.h
// created by Terrence Cole 1-27-03

#ifndef _COG_CLIENT_GLOBAL_H
#define _COG_CLIENT_GLOBAL_H

// Includes
/////////////
#include <string.h> // for memset
#include <assert.h> // for assert
#include "../shared/shared_misc.h"
#include "../shared/shared_structures.h"
#include "../shared/shared_keycodes.h"
#include "../shared/shared_library.h"
#include "../shared/shared_mathlib.h"

// Definitions
////////////////
#define SAFE_RELEASE(a) if( (a) != NULL ) { ei->mem_free( (a) ); (a) = NULL; }
#define SAFE_RELEASEv(a) if( (a) != NULL ) { ei->mem_free( (a) ); }

// far clip plane distance
#define CL_FAR_CLIP_PLANE 40000.0f

// Global Structs
////////////////
typedef struct cl_state_s 
{
	// the server sync times
	unsigned long int ser_join_time;
	unsigned long int cl_join_time;
	unsigned long int time_offset;
	unsigned long int tCurrent;
	unsigned long int tLast;
	unsigned long int tDelta;
	float fGameTime;
	
	// player info
	varlatch vlName;
	varlatch vlModel;
	varlatch vlSensitivity;
	varlatch vlMoveSpeed;
	varlatch vlNoClip;

	// player command callbacks
	command_t cmdMoveForward;
	command_t cmdMoveBackward;
	command_t cmdMoveRight;
	command_t cmdMoveLeft;
	command_t cmdMoveUp;
	command_t cmdMoveDown;

	command_t cmdArcMoveForward;
	command_t cmdArcMoveBackward;
	command_t cmdArcMoveRight;
	command_t cmdArcMoveLeft;
	command_t cmdArcMoveUp;
	command_t cmdArcMoveDown;

	command_t cmdUncover;
	command_t cmdFlag;

	command_t cmdSay;
	command_t cmdWhisper;

	// player movement
	vec3 vMvReq;	
	
	// the player ent
	int state;
	entity_t *self;
	int key;

	// the others and their keys
	entity_t **others;
	int *oKeys;

	// world states
	int game_state;
	int launch_count;
	int launch_time;
	byte bMaxClients;
	world_t *world;
	int iMinesRemaining;
	int iMinesUnder;
} cl_state_t;

// Global Variables
/////////////////////
client_interface_t ci;
engine_interface_t *ei;
cl_state_t plState;

/* ------------
cl_main.c
------------ */
client_interface_t*GetClientAPI( engine_interface_t *engine_interface );
int cl_Initialize(void);
void cl_Terminate(void);
int cl_Connect( char *address, char *port );
int cl_Frame( exec_state_t *state );
void cl_Draw2D( exec_state_t *state );

/* ------------
cl_state.c
------------ */
int st_ParseMessages( exec_state_t *state );

/* ------------
cl_player.c
------------ */
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

/* ------------
cl_foreground.c
------------ */
int fgo_cl_Initialize(void);
void fgo_cl_Terminate(void);
void fgo_cl_Reload( char *null );
void fgo_cl_Frame();
void fgo_cl_Draw();

/* ------------
cl_world.c
------------ */
int wrld_CreateFromState( unsigned short int size, byte *data );
void wrld_SetStartState( byte *data );
byte wrld_Event( byte * data );
int wrld_CalculateMinesRemaining();
int wrld_GetMinesUnderPlayer( vec3 position );
int wrld_cl_GetWorldIndex( vec3 vPos, int iPos[3] );


#endif // _COG_CLIENT_GLOBAL_H
