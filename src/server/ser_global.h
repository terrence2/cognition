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
// ser_global.h
// created by Terrence Cole 1-27-03

#ifndef _COG_SERVER_GLOBAL_H
#define _COG_SERVER_GLOBAL_H

// Includes
/////////////
#include <string.h> // for memset
#include "../shared/shared_misc.h"
#include "../shared/shared_structures.h"
#include "../shared/shared_keycodes.h"
#include "../shared/shared_library.h"

// Definitions
////////////////
#define SAFE_RELEASE(a) if( (a) != NULL ) { ei->mem_free( (a) ); (a) = NULL; }

// Global Structures
//////////////////
typedef struct ser_player_s
{
	// management
	int state;  // PL_STATE_CONNECTING, CONNECTED
	connection_t connection;

	// data about us
	unsigned int key;
	char name[PLAYER_MAX_NAME_SIZE];
	char modName[PLAYER_MAX_NAME_SIZE];

	// location
	unsigned long int ulLastUpdate;
	vec3 pos;
	vec3 ang;

	// model data
	model_t *model;
} ser_player_t;

typedef struct ser_state_s
{
	// server properties
	byte bPublicServer;
	byte bMaxClients;
	int iNumClients;

	// server state
	int state;

	// timing
	unsigned long int ser_frame_time;
	unsigned long int next_start_time;
	unsigned long int launch_time;

	// server data
	world_t *world;
	ser_player_t *players;
} ser_state_t;

// Global Variables
/////////////////////
extern server_interface_t *si;
extern engine_interface_t *ei;
extern ser_state_t svState;

// Definitions
/////////////
// player accounting
#define PLAYER_STATE_EMPTY 0
#define PLAYER_STATE_CONNECTING 1
#define PLAYER_STATE_DEAD 2

/* ------------
ser_main.c
------------ */
server_interface_t*GetServerAPI( engine_interface_t *engine_interface );
int ser_Initialize(void);
void ser_Terminate(void);
int ser_Start( int iMaxClients, byte bPublicServer, int *size, int dimension, int numMines );
void ser_Frame(void);
int ser_MultiCastByte( byte b );
int ser_MultiCastShort( unsigned short int us );
int ser_MultiCastLong( unsigned long int ul );
int ser_MultiCastString( char *str );


/* ------------
ser_state.c
------------ */
void st_ParseIncoming();

/* ------------
ser_entity.c
------------ */

/* ------------
ser_world.c
------------ */
void wrld_SendWorldState( connection_t plConnection );
int wrld_PositionNewClient( int pl );
int wrld_Event( int pl, unsigned short int size, byte *data );


/* ------------
ser_player.c
------------ */
int pl_Initialize( int iMaxClients );
void pl_Terminate(void);
void pl_GetNewClients();
void pl_Disconnect( unsigned int key, char *reason );
void pl_AbnormalDisconnect( unsigned int key );

void pl_SendState( int player, connection_t to );
void pl_SetState( int player, unsigned short int size, byte *data );

void pl_NameChange( int pl, char *newName );
void pl_ModelChange( int pl, char *newModel );
void pl_PositionChange( int pl, unsigned short int size, byte *data );
void pl_AngleChange( int pl, unsigned short int size, byte *data );





#endif // _COG_SERVER_GLOBAL_H
