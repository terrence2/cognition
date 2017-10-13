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
// client module
// cl_state.c
// Created 2-2-3 by Terrence Cole

// Includes
/////////////
#include "cl_global.h"

// Definitions
////////////////

// Global Prototypes
//////////////////////
/*
int st_ParseMessages( exec_state_t *state );
*/

// Global Variables
////////////////////
/*
client_interface_t ci;
engine_interface_t *ei;
*/

// *********** FUNCTIONALITY ***********
/* ------------
st_ParseMessages - returns the state of the game in transition - a zero means that we have lost the connection
				- with the server and should close up shop, > 0 means status normal
------------ */
int st_ParseMessages( exec_state_t *state )
{
	int ret;
	byte bDone;
	
	// inputs
	byte type;
	unsigned short int size;
	byte *data = NULL;

	// assert
	if( plState.state == PL_STATE_DISCONNECTED ) return 0;

	// read packets while we can
	bDone = 0;
	while( !bDone )
	{
		ret = ei->net_cl_GetServerPacket( &type, &size, &data );

		if( ret > 0 )
		{
			switch( type )
			{
			case PK_STC_JOIN_ACCEPT:  // 1st stage connect
				// store the key for this player and session
				plState.key = ei->net_GetLong( data );
				if( plState.key <= 0 ) 
				{
					ei->con_Print( "<RED>Client Error:  Server sent a player key with an invalid value:  %d", plState.key );
					return 0;
				}

				ei->con_Print( "1)  Connection <BLUE>Accepted</BLUE>." );
				ei->con_Print( "\tServer Message:  <VIOLET>\"%s\"", (data + 4) );

				// send the player info to the server
				pl_SendData();
				break;

			case PK_STC_JOIN_DENY:
				ei->con_Print( "<RED>Client Connect Error:  \n\tServer denied join request.  \n\tReason: %s", (char*)data );
				return 0;
				
			case PK_STC_WORLD_STATE_START:  // collect the game state to end the 3rd stage of handshake
				// we should also be getting a bunch of PK_STC_PLAYER_STATE packets right after this (not b4)
				// it doesn't really matter when we accept them though, as long as we get them at some point
				// n.b. we can spawn other players in at any time from player state packets

				// break up our data and get a world
				if( !wrld_CreateFromState( size, data ) )
				{
					ei->con_Print( "<RED>Client Error:  Create world from state failed." );
					return 0;
				}

				ei->con_Print( "3)  Receiving World State..." );
				
				break;

			case PK_STC_WORLD_STATE_CUBE:
				wrld_SetStartState( data );
				break;

			case PK_STC_WORLD_STATE_END:
				// send the ready to play message to tell the server to b-cast our connection to other
				// clients and to start sending us normal updates
				ei->net_cl_SendByte( PK_CTS_READY_TO_PLAY );
				ei->net_cl_SendShort( 0 );

				ei->con_Print( "\tWorld State Complete." );
				ei->d_SetWorld( plState.world );
				ei->game_ShowGame();
				
				break;

			case PK_STC_WORLD_STATE_GAME:
				assert( size == 4 );
				if( size != 4 )
				{
					ei->con_Print( "<RED>Server Sent Broken Game State Packet." );
				}

				// set the game state locally
				plState.game_state = ei->net_GetLong( data );

				// set the engine drawing state to the game state
				if( plState.game_state == SER_STATE_LAUNCH )
				{
					plState.launch_count = 4;
					plState.launch_time = plState.tCurrent + SER_STATE_LAUNCH_TIME;
				}
				
				break;

			case PK_STC_WORLD_PLAYER_POS:
				// receive the player data
				if( !pl_SetPositionFromData( size, data ) )
				{
					ei->con_Print( "<RED>Client Error:  Set Position From Data Packet Failed." );
					return 0;
				}
				
				// when we get the first and only player pos from the server
				plState.state = PL_STATE_CONNECTED;

				ei->con_Print( "5)  Game <BLUE>started</BLUE>." );
				
				break;

			case PK_STC_PLAYER_STATE:
				// a new player has joined the game, add the data to our little lists
				if( !pl_OtherClientJoin( size, data ) )
				{
					// if this fails we can't continue
					ei->con_Print( "<RED>Client Error:  Create other client from state failed." );
					return 0;
				}

				break;

			case PK_SBC_WORLD_EVENT:

				if( !wrld_Event( data ) )
				{
					ei->con_Print( "<RED>Client Error:  World Event response failed!" );
					return 0;
				}

				break;
				
			default:
				break;
			}
		}
		else if( ret == -1 )
		{
			// the server has died /kicked us abnormally, or something
			ei->con_Print( "<RED>The server has ended the connection abnormally." );
			return 0;
		}
		else
		{
			bDone = 1;
		}
	}

	// check for the error
	if( ret < 0 ) {	return 0; }
	return 1;
}
