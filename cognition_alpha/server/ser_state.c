// Cognition
// server module
// ser_state.c
// Created 1-31-03 by Terrence Cole

// Includes
/////////////
#include "ser_global.h"

// Definitions
////////////////

/*
1)  Wait for a client to join and to finish loading all media.  (LOUNGE)
2)  Show starting screen and start countdown.  (LAUNCH)
3)  Do game.  Wait for all clients to die. (PLAY)
4)  GoTo 1). (RESTART)
*/

// Structures
/////////////

// Global Prototypes
//////////////////////
/*
void st_ParseIncoming();
*/

// Local Prototypes
//////////////////
static void st_ClientMessage( unsigned long int ulFrom, unsigned long int ulFor, char *str, unsigned short int usStrLen );


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
st_ParseIncoming
------------ */
void st_ParseIncoming()
{
	// iterators
	int a, b;
	byte bDone;

	// return codes
	int ret;

	// inputs
	byte type;
	unsigned short int size;
	byte *data;

	// assert
	// FIXME: what exactly do i need to assert here?
	
	// the message loop
	for( a = 0 ; a < svState.bMaxClients ; a++ )
	{
		ser_player_t *player = &(svState.players[a]);
		
		// handle connected players
		if( (player) && (player->state != PL_STATE_DISCONNECTED) && (player->connection != INVALID_CONNECTION) )
		{
			bDone = 0;  // we use this so we can break the loop if a player disconnects
			while( !bDone )
			{
				ret = ei->net_sv_GetClientPacket( player->connection, &type, &size, &data );
				if( ret > 0 )
				{
					switch( type )
					{
					case PK_CTS_PLAYER_DATA:  // end 2nd stage of handshake
						// receive client data
						pl_SetState( a, size, data );

						// send the world state and player states starting the 3rd stage of handshake
						wrld_SendWorldState( player->connection );
						ei->con_Print( "2)  Player data logged to server.  World State transfer started." );
						
						break;
						
					case PK_CTS_READY_TO_PLAY:
						// put this player into the world
						wrld_PositionNewClient( a );

						// tell our player where he really is
						ei->net_sv_SendByte( player->connection, PK_STC_WORLD_PLAYER_POS );
						ei->net_sv_SendShort( player->connection, 12 );
						ei->net_sv_SendLong( player->connection, tcPositionToLong( player->pos[X_AXIS] ) );
						ei->net_sv_SendLong( player->connection, tcPositionToLong( player->pos[Y_AXIS] ) );
						ei->net_sv_SendLong( player->connection, tcPositionToLong( player->pos[Z_AXIS] ) );

						// tell the player the current game state
						ei->net_sv_SendByte( player->connection, PK_STC_WORLD_STATE_GAME );
						ei->net_sv_SendShort( player->connection, 4 );
						ei->net_sv_SendLong( player->connection, svState.state );
						
						// b-cast all other clients this players info 
						for( b = 0 ; b < svState.bMaxClients ; b++ )
						{
							if( (svState.players[b].state != PL_STATE_DISCONNECTED) && (a != b) )
							{
								pl_SendState( a, svState.players[b].connection );
							}
						}

						// connect our client
						player->state = PL_STATE_CONNECTED;

						ei->con_Print( "4)  Player positioned in world.  Ready to play." );

						// if this is the first connected player, move up the start time to now
						if( svState.iNumClients > 0 && svState.state == SER_STATE_LOUNGE )
						{
							svState.next_start_time = ei->t_GetTime();
						}
						
						break;
	/*
					case PK_CTS_PLAYER_NAME://		str (n)
						pl_NameChange( a, (char*)data );
						break;
					case PK_CTS_PLAYER_MODEL:// 		str (n)
						pl_ModelChange( a, (char*)data );
						break;

	*/

					case PK_CTS_PLAYER_POS://			pos (3 * 4)
						pl_PositionChange( a, size, data );
						break;

					case PK_CTS_PLAYER_ANG://			ang (2 * 2)
						pl_AngleChange( a, size, data );
						break;
					
					case PK_CTS_PLAYER_DISCONNECT://	
						pl_Disconnect( player->key, "Client requested disconnect." );
						bDone = 1;
						break;
						
					case PK_CTS_MESSAGE://			for (4), str (n)
						st_ClientMessage( player->key, ei->net_GetLong( data ), &(data[4]), (uint16_t)(size - 4) );
						break;

					// WORLD EVENTS
					case PK_CTS_WORLD_EVENT: // 		cube (3 * 4), state (1)
						wrld_Event( a, size, data );
						break;

					default:  // oob messages mean something has gone terribly wrong
						// we should never ever receive any of these packets from the client
						ei->con_Print( "<RED>Server Message:  Disconnecting player \"%s\" for invalid packet received.", svState.players[a].name );
						pl_Disconnect( player->key, "Server forced disconnect for bad data sent." );
						
						break;

					}
				}
				else if( ret == -1 )
				{
					// this client has ended abnormally, so end the connection
					ei->con_Print( "The connection with %s has ended abnormally and been aborted.", svState.players[a].name );
					pl_AbnormalDisconnect( svState.players[a].key );
					bDone = 1;
				}
				else
				{
					bDone = 1;
				}
			}
		}
	}
}

/* ------------
st_ClientMessage - retransmitts the received message to the right clients
------------ */
static void st_ClientMessage( unsigned long int ulFrom, unsigned long int ulFor, char *str, unsigned short int usStrLen )
{
	int b;
	
	// b-cast the string to the 'for'
	if( ulFor == 0 )
	{
		for( b = 0 ; b < svState.bMaxClients ; b++ )
		{
			if( (svState.players[b].state != PL_STATE_DISCONNECTED) && (svState.players[b].key != ulFrom) )
			{
				ei->net_sv_SendByte( svState.players[b].connection, PK_STC_MESSAGE );
				ei->net_sv_SendShort( svState.players[b].connection, usStrLen );
				ei->net_sv_SendString( svState.players[b].connection, str );
			}
		}

		return;
	}
	else
	{
		for( b = 0 ; b < svState.bMaxClients ; b++ )
		{
			if( svState.players[b].key == ulFor )
			{
				ei->net_sv_SendByte( svState.players[b].connection, PK_STC_MESSAGE );
				ei->net_sv_SendShort( svState.players[b].connection, usStrLen );
				ei->net_sv_SendString( svState.players[b].connection, str );
				return;
			}
		}
	}
}


