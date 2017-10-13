// Cognition
// client module
// cl_world.c
// Created 2-2-3 by Terrence Cole

// Includes
/////////////
#include "cl_global.h"
#include <stdio.h>

// Definitions
////////////////

// Global Prototypes
//////////////////////
/*
int wrld_CreateFromState( unsigned short int size, byte *data );
void wrld_SetStartState( byte *data );
byte wrld_Event( byte * data );
int wrld_CalculateMinesRemaining();
int wrld_GetMinesUnderPlayer( vec3 position );
int wrld_cl_GetWorldIndex( vec3 vPos, int iPos[3] );
*/

// Global Variables
////////////////////
/*
client_interface_t ci;
engine_interface_t *ei;
*/

// *********** FUNCTIONALITY ***********
/* ------------
wrld_CreateFromState
------------ */
int wrld_CreateFromState( unsigned short int size, byte *data )
{
//	time (4), maxClients (1), size (3 * 4), states (2 * n)
	int a;
	byte bSize = 4 + 1 + 12;
	int iIndex;
	int vSize[3];

	// assert
	if( size != bSize )
	{
		ei->con_Print( "<RED>Client World Create From State:  Incorrect data size." );		
		return 0;
	}
	
	// get the time
	iIndex = 0;
	plState.ser_join_time = ei->net_GetLong( &data[iIndex] );
	iIndex += 4;

	// correct for lag
	plState.cl_join_time = ei->t_GetTime();
	plState.time_offset = plState.cl_join_time - plState.ser_join_time;

	// store the max clients
	plState.bMaxClients = data[iIndex];
	iIndex += 1;

	// build the player holding area now that we know how many to allow
	if( !pl_BuildOthers( plState.bMaxClients ) )
	{
		ei->con_Print( "<RED>Client Error: Create World From State:  Player Build Others Failed." );
		return 0;
	}

	// store the world size
	for( a = 0 ; a < 3 ; a++ )
	{	
		vSize[a] = ei->net_GetLong( &data[iIndex] );
		iIndex += 4;
	}

	// create the world
	plState.world = ei->wrld_Create( vSize, 6, 0 );
	if( plState.world == NULL )
	{
		ei->con_Print( "<RED>Client World Create From State:  Create World Failed in Engine." );
		return 0;
	}

	// zero this and let it count up from the start states
	plState.world->numMines = 0;

	return 1;
}

/* ------------
wrld_SetStartState - takes the data from one PK_STC_WORLD_STATE_CUBE packet
------------ */
void wrld_SetStartState( byte *data )
{
	int pos[3];
	char entName[1024];
	char entModName[1024];
	vec3 cent;
	
	// assert
	if( data == NULL )
	{
		ei->con_Print( "<RED>Client World Set Start State:  Null data pointer." );
		return;
	}
	if( plState.world == NULL )
	{
		ei->con_Print( "<RED>Client World Set Start State:  Null world pointer in state." );
		return;
	}
	
	pos[0] = ei->net_GetLong( data );
	pos[1] = ei->net_GetLong( &data[4] );
	pos[2] = ei->net_GetLong( &data[8] );

	// assert the position
	if( (pos[0] < 0) || (pos[0] >= plState.world->size[0]) ||
		(pos[1] < 0) || (pos[1] >= plState.world->size[1]) ||
		(pos[2] < 0) || (pos[2] >= plState.world->size[2]) )
	{
		ei->con_Print( "<RED>Client World Set Start State:  Position outside of world bounds." );
		return;
	}

	// get the data
	plState.world->cubes[pos[0]][pos[1]][pos[2]].bHidden = data[12] & 0x01;
	plState.world->cubes[pos[0]][pos[1]][pos[2]].bFlagged = (data[12] >> 1) & 0x01;
	plState.world->cubes[pos[0]][pos[1]][pos[2]].numAround = data[13];

	// if a mine, increment the mine count
	if( plState.world->cubes[pos[0]][pos[1]][pos[2]].numAround == WRLD_CUBE_MINE )
	{
		plState.world->numMines++;
	}

	// spawn the center number entity
	sprintf( entName, "WorldCubeCenter(%d,%d,%d)\0", pos[0], pos[1], pos[2] );
	cent[X_AXIS] = plState.world->wrldBnds.lobounds[X_AXIS] + ((float)pos[0] + 0.5f) * WORLD_CUBE_SIZE;
	cent[Y_AXIS] = plState.world->wrldBnds.lobounds[Y_AXIS] + ((float)pos[1] + 0.5f) * WORLD_CUBE_SIZE;
	cent[Z_AXIS] = plState.world->wrldBnds.lobounds[Z_AXIS] + ((float)pos[2] + 0.5f) * WORLD_CUBE_SIZE;
	
	//entity_t *ent_Spawn( char *name, int flags, char *model, float *pos, float *rot, float *scale, void *ext_data, void (*think_func)(void), int nextThink );
	if( plState.world->cubes[pos[0]][pos[1]][pos[2]].numAround == WRLD_CUBE_MINE )
	{
		// load a mine entity for this square
		plState.world->cubes[pos[0]][pos[1]][pos[2]].entCenter = ei->ent_Spawn( entName, 0, "mine.def", cent, vZero );

		// check the entity load
		if( plState.world->cubes[pos[0]][pos[1]][pos[2]].entCenter == NULL )
		{
			ei->con_Print( "<RED>Client World Error in SetStartState:  Entity Load Failed." );
			return;
		}

		// the entity renderer should never draw these
		plState.world->cubes[pos[0]][pos[1]][pos[2]].entCenter->bHintHidden = 1;
	}	
	else if( (plState.world->cubes[pos[0]][pos[1]][pos[2]].numAround > 0) && (plState.world->cubes[pos[0]][pos[1]][pos[2]].numAround <= 26) )
	{
		// get a string describing this square and load an entity
		sprintf( entModName, "number%d.def", plState.world->cubes[pos[0]][pos[1]][pos[2]].numAround );
		plState.world->cubes[pos[0]][pos[1]][pos[2]].entCenter = ei->ent_Spawn( entName, 0, entModName, cent, vZero );

		// check the entity load
		if( plState.world->cubes[pos[0]][pos[1]][pos[2]].entCenter == NULL )
		{
			ei->con_Print( "<RED>Client World Error in SetStartState:  Entity Load Failed." );
			return;
		}

		// the entity renderer never draws these
		plState.world->cubes[pos[0]][pos[1]][pos[2]].entCenter->bHintHidden = 0;
	}
	else  // 0 around
	{
		plState.world->cubes[pos[0]][pos[1]][pos[2]].entCenter = NULL;
	}

//	ei->con_Print( "\tAdded Cube State [ %d, %d, %d ],  Hidden: %d,  Flagged: %d,  Data:  %d", pos[0], pos[1], pos[2], data[12] & 0x01, (data[12] >> 1) & 0x01, data[13] );
}

/* ------------
wrld_Event - responds to server world events, marks the indicated square with the indicated state
		- returns >0 on success, 0 on failure
------------ */
byte wrld_Event( byte * data )
{
	int pos[3];
	
	// assert
	if( data == NULL )
	{
		ei->con_Print( "<RED>Client World Event:  Null data pointer." );
		return 0;
	}
	if( plState.world == NULL )
	{
		ei->con_Print( "<RED>Client World Event:  Null world pointer in state." );
		return 0;
	}

	// get the position
	pos[0] = ei->net_GetLong( data );
	pos[1] = ei->net_GetLong( &data[4] );
	pos[2] = ei->net_GetLong( &data[8] );

	// assert our position, so we don't die on gibberish
	if( (pos[0] < 0) || (pos[0] >= plState.world->size[0]) ) return 0;
	if( (pos[1] < 0) || (pos[1] >= plState.world->size[1]) ) return 0;
	if( (pos[2] < 0) || (pos[2] >= plState.world->size[2]) ) return 0;

	// respond to the event
	if( !( data[12] & 0x01) )
	{
		// if this square has become unhidden, call clear to do all the special tasks associated with the clear
		ei->wrld_ClearSquare( plState.world, pos[0], pos[1], pos[2] );
	}

	// we can just set a flagged state
	plState.world->cubes[pos[0]][pos[1]][pos[2]].bFlagged = (data[12] >> 1) & 0x01;
		
	return 1;
}

/* ------------
wrld_MinesRemaining - computes the remaining mines every frame after getting server messages
				- we do this instead of counting at entry and exit, because there are numberous
				- points of contact between the client and server and we want to allow for adding additional
				- game types without too much pain
				- returns mines remaining, no error conditions;  returns 0 if no info is available
------------ */
int wrld_CalculateMinesRemaining()
{
	int a, b, c;
	int iMinesRemaining;

	// we'll be running this before we have a world, so check carefully
	if( plState.world == NULL ) return 0;

	// set the number of mines remaining in the world
	iMinesRemaining = 0;
	for( a = 0 ; a < plState.world->size[0] ; a++ )
	{
		for( b = 0 ; b < plState.world->size[1] ; b++ )
		{
			for( c = 0 ; c < plState.world->size[2] ; c++ )
			{
				// update the mines remaining
				if( plState.world->cubes[a][b][c].bFlagged ) iMinesRemaining--;
				if( plState.world->cubes[a][b][c].numAround == WRLD_CUBE_MINE ) iMinesRemaining++;
			}
		}
	}

	return iMinesRemaining;
}

/* ------------
wrld_GetMinesAroundAtPosition - finds the current cube and returns the mines in the current cube
							- returns -1 on an error
------------ */
int wrld_GetMinesUnderPlayer( vec3 position )
{	
	int vLoc[3];
	float fCurPos;
	
	// check for an active player and world before we try to use it
	if( plState.self == NULL ) return -1;
	if( plState.world == NULL ) return -1;
	
	vLoc[0] = -1;
	fCurPos = plState.world->wrldBnds.lobounds[0];
	while( plState.self->vPos[0] > fCurPos ) { fCurPos += WORLD_CUBE_SIZE; vLoc[0]++; }
	
	vLoc[1] = -1;
	fCurPos = plState.world->wrldBnds.lobounds[1];
	while( plState.self->vPos[1] > fCurPos ) { fCurPos += WORLD_CUBE_SIZE; vLoc[1]++; }
		
	vLoc[2] = -1;
	fCurPos = plState.world->wrldBnds.lobounds[2];
	while( plState.self->vPos[2] > fCurPos ) { fCurPos += WORLD_CUBE_SIZE; vLoc[2]++; }
	
	// assert our position
	if( (vLoc[0] < 0) || (vLoc[0] >= plState.world->size[0]) ||
		(vLoc[1] < 0) || (vLoc[1] >= plState.world->size[1]) ||
		(vLoc[2] < 0) || (vLoc[2] >= plState.world->size[2]) )
	{
		return -1;
	}
	
	// if we are noclipped into a hidden region, don't give away where mines are
	if( plState.world->cubes[vLoc[0]][vLoc[1]][vLoc[2]].bHidden ) return -1;
	
	// do the assignment
	return plState.world->cubes[vLoc[0]][vLoc[1]][vLoc[2]].numAround;
}

/* ------------
wrld_cl_GetWorldIndex - 
	- takes vPos as the current player position and sets iPos to the location of the
	player in the world cube of iPos
	- iPos locked to range -1 to ?
	-only valid if we have a world and a player state
------------ */
int wrld_cl_GetWorldIndex( vec3 vPos, int iPos[3] )
{
	float fCurPos;

	// assert
	if( plState.world == NULL )
	{
		ei->con_Print( "<RED>Assert Failed in wrld_cl_GetWorldIndex" );
		return -1;
	}
	// assert vPos
	// assert iPos

	iPos[0] = -1;
	fCurPos = plState.world->wrldBnds.lobounds[0];
	while( vPos[0] > fCurPos ) { fCurPos += WORLD_CUBE_SIZE; iPos[0]++; }
	
	iPos[1] = -1;
	fCurPos = plState.world->wrldBnds.lobounds[1];
	while( vPos[1] > fCurPos ) { fCurPos += WORLD_CUBE_SIZE; iPos[1]++; }
		
	iPos[2] = -1;
	fCurPos = plState.world->wrldBnds.lobounds[2];
	while( vPos[2] > fCurPos ) { fCurPos += WORLD_CUBE_SIZE; iPos[2]++; }
	
	return 1;
}
