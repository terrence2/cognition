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
// server module
// ser_world.c
// Created 1-31-03 by Terrence Cole

// Includes
/////////////
#include "ser_global.h"
#include <stdio.h>

// Definitions
////////////////

// Structures
/////////////

// Global Prototypes
//////////////////////
/*
void wrld_SendWorldState( connection_t plConnection );
int wrld_PositionNewClient( int pl );
int wrld_Event( int pl, unsigned short int size, byte *data );
*/

// Local Prototypes
//////////////////

// Global Variables
////////////////////
/*
client_interface_t ci;
engine_interface_t *ei;
ser_state_t state;
*/

// Local Variables
////////////////

// *********** FUNCTIONALITY ***********
/* ------------
wrld_SendWorldState - composes and sends the world state packet to plConnection
------------ */
void wrld_SendWorldState( connection_t plConnection )
{
//	PK_WORLD_STATE		time (4), maxClients (1), size (3 * 4), states (n)
	unsigned short int size;
	int a, b, c;

	// assert
	if( plConnection == INVALID_CONNECTION ) return;
	if( svState.world == NULL ) return;

	// compute a data size
	size = 4 + 1 + 12;

	// send base
	ei->net_sv_SendByte( plConnection, PK_STC_WORLD_STATE_START );
	ei->net_sv_SendShort( plConnection, size );
	ei->net_sv_SendLong( plConnection, svState.ser_frame_time );
	ei->net_sv_SendByte( plConnection, svState.bMaxClients );
	ei->net_sv_SendLong( plConnection, svState.world->size[0] );
	ei->net_sv_SendLong( plConnection, svState.world->size[1] );
	ei->net_sv_SendLong( plConnection, svState.world->size[2] );

	// send all states
	for( a = 0 ; a < svState.world->size[0] ; a++ )
	{
		for( b = 0 ; b < svState.world->size[1] ; b++ )
		{
			for( c = 0 ; c < svState.world->size[2] ; c++ )
			{
				ei->net_sv_SendByte( plConnection, PK_STC_WORLD_STATE_CUBE );
				ei->net_sv_SendShort( plConnection, 14 );
				ei->net_sv_SendLong( plConnection, a );
				ei->net_sv_SendLong( plConnection, b );
				ei->net_sv_SendLong( plConnection, c );
				ei->net_sv_SendByte( plConnection, (uint8_t)(svState.world->cubes[a][b][c].bHidden + 
												2 * svState.world->cubes[a][b][c].bFlagged) );
				ei->net_sv_SendByte( plConnection, svState.world->cubes[a][b][c].numAround );
			}
		}
	}

	// send the end marker
	ei->net_sv_SendByte( plConnection, PK_STC_WORLD_STATE_END );
	ei->net_sv_SendShort( plConnection, 0 );
}

/* ------------
st_PositionNewClient - looks for a random 0 cube that has 0's as many spots as possible around it
------------ */
int wrld_PositionNewClient( int pl )
{
	byte bDone;
	int a, b, c;
	int vTmp[3];
	unsigned long int ulCurOff, ulMaxOff, ulOffset;

	// assert
	if( svState.world == NULL ) return 0;

	// setup
	bDone = 0;
	ulCurOff = 0;
	ulMaxOff = svState.world->numCubes - svState.world->numMines;
	ulOffset = ei->rng_Long( 0, ulMaxOff );
	vTmp[0] = vTmp[1] = vTmp[2] = -1;
	a = b = c = 0;

	// find the position
	while( !bDone && (a < svState.world->size[0]) )
	{
		b = 0;
		while( !bDone && (b < svState.world->size[1]) )
		{
			c = 0;
			while( !bDone && (c < svState.world->size[2]) )
			{
				// advance only on non-mine squares
				if( svState.world->cubes[a][b][c].numAround != WRLD_CUBE_MINE ) ulCurOff++;
				
				// if this is the offset to which we are seeking, set the mine and start over
				if( ulCurOff == ulOffset )
				{
					vTmp[0] = a;
					vTmp[1] = b;
					vTmp[2] = c;
					bDone = 1;
				}
				
				// increment in Z
				c++;
			}
			
			// increment in Y
			b++;
		}

		// increment on X
		a++;
	}

	// assert our found square
	if( (vTmp[0] < 0) || (vTmp[0] >= svState.world->size[0]) ||
		(vTmp[1] < 0) || (vTmp[1] >= svState.world->size[1]) ||
		(vTmp[2] < 0) || (vTmp[2] >= svState.world->size[2]) )
	{
		ei->con_Print( "<RED>Server World Error:  Search for a mineless square failed." );
		return 0;
	}

	// put us in the center of the minimum square
	svState.players[pl].pos[X_AXIS] = svState.world->wrldBnds.lobounds[X_AXIS] + (svState.world->cubeSize * ((float)vTmp[0] + 0.5f));
	svState.players[pl].pos[Y_AXIS] = svState.world->wrldBnds.lobounds[Y_AXIS] + (svState.world->cubeSize * ((float)vTmp[1] + 0.5f));
	svState.players[pl].pos[Z_AXIS] = svState.world->wrldBnds.lobounds[Z_AXIS] + (svState.world->cubeSize * ((float)vTmp[2] + 0.5f));

	// check to ensure that we have a mineless square
	if( svState.world->cubes[vTmp[0]][vTmp[1]][vTmp[2]].numAround == WRLD_CUBE_MINE )
	{
		ei->con_Print( "<RED>Server World Error:  Could not find a mineless cube in which to start the player." );
		return 0;
	}

	// clear this square
	ei->wrld_ClearSquare( svState.world, vTmp[0], vTmp[1], vTmp[2] );

	// send the new state
	ser_MultiCastByte( PK_SBC_WORLD_EVENT );
	ser_MultiCastShort( 13 );
	ser_MultiCastLong( vTmp[0] );
	ser_MultiCastLong( vTmp[1] );
	ser_MultiCastLong( vTmp[2] );
	ser_MultiCastByte( (uint8_t)(svState.world->cubes[vTmp[0]][vTmp[1]][vTmp[2]].bHidden + 
					2 * svState.world->cubes[vTmp[0]][vTmp[1]][vTmp[2]].bFlagged) );

	return 1;
}

/* ------------
wrld_Event - called on receipt of a client event packet, adjusts the state of the indicated square, calls clear,
			- and broadcasts to all other clients
------------ */
int wrld_Event( int pl, unsigned short int size, byte *data )
{
	int a;
	byte bTmp;
	int vCube[3];

	// #define PK_CTS_WORLD_EVENT	4 //		pos (3 * 4), newState (1)
	vCube[0] = ei->net_GetLong( &(data[0]) );
	vCube[1] = ei->net_GetLong( &(data[4]) );
	vCube[2] = ei->net_GetLong( &(data[8]) );
	bTmp = data[13];

	// assert the cube position
	if( (vCube[0] < 0) || (vCube[0] >= svState.world->size[0]) ||
		(vCube[1] < 0) || (vCube[1] >= svState.world->size[1]) ||
		(vCube[2] < 0) || (vCube[2] >= svState.world->size[2]) )
	{
		ei->con_Print( "Server World Event Failed:  received out of bounds data from client %s", svState.players[pl].name );
		return 0;
	}

	// change the data at that location
	svState.world->cubes[vCube[0]][vCube[1]][vCube[2]].bHidden = bTmp & 0x01;
	svState.world->cubes[vCube[0]][vCube[1]][vCube[2]].bFlagged = (bTmp >> 1) & 0x01;

	// if the cube might have become unhidden, attempt to uncover it
	if( !(svState.world->cubes[vCube[0]][vCube[1]][vCube[2]].bHidden) )
	{
		// let the engine worry about recursion, etc
		ei->wrld_ClearSquare( svState.world, vCube[0], vCube[1], vCube[2] );
	}

	// rebroadcast
	for( a = 0 ; a < svState.bMaxClients ; a++ )
	{
		if( (a != pl) && (svState.players[a].connection != INVALID_CONNECTION) && (svState.players[a].state != PL_STATE_DISCONNECTED) )
		{
			// #define PK_SBC_WORLD_EVENT	19 //	pos (3 * 4), newState (1)
			ei->net_sv_SendByte( svState.players[a].connection, PK_SBC_WORLD_EVENT );
			ei->net_sv_SendShort( svState.players[a].connection, 13 );
			ei->net_sv_SendLong( svState.players[a].connection, vCube[0] );
			ei->net_sv_SendLong( svState.players[a].connection, vCube[1] );
			ei->net_sv_SendLong( svState.players[a].connection, vCube[2] );
			ei->net_sv_SendByte( svState.players[a].connection, bTmp );
		}
	}

	return 1;
}
