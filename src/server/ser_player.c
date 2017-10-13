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
// ser_player.c
// Created 2-01-03 by Terrence Cole

// Includes
/////////////
#include "ser_global.h"

// Definitions
////////////////

// Global Prototypes
//////////////////////
/*
int pl_Initialize( int iMaxClients );
void pl_Terminate(void);
void pl_GetNewClients();
void pl_Disconnect( int key, char *reason );
void pl_AbnormalDisconnect( int key );

void pl_SendState( int player, connection_t to );
void pl_SetState( int player, unsigned short int size, byte *data );

void pl_NameChange( int pl, char *newName );
void pl_ModelChange( int pl, char *newModel );
void pl_PositionChange( int pl, unsigned short int size, byte *data );
void pl_AngleChange( int pl, unsigned short int size, byte *data );

*/

// Local Prototypes
//////////////////
static byte pl_AddClient( connection_t plConnection );
static void pl_DenyClient( connection_t plConnection, char *reason );


// Global Variables
////////////////////
/*
client_interface_t ci;
engine_interface_t *ei;
*/

// Local Variables
///////////////
static int svNumConnections = 0;

// *********** FUNCTIONALITY ***********
/* ------------
pl_Initialize
------------ */
int pl_Initialize( int iMaxClients )
{
	int a;
	
	// assert
	if( iMaxClients <= 0 ) return 0;

	// get memory
	svState.players = ei->mem_alloc( sizeof(ser_player_t) * iMaxClients );
	if( svState.players == NULL )
	{
		ei->con_Print( "Server Player Init Failed:  Unable to allocate memory for players." );
		return 0;
	}
	memset( svState.players, 0, sizeof(ser_player_t) * iMaxClients );

	// init players
	for( a = 0 ; a < iMaxClients ; a++ )
	{
		svState.players[a].state = PL_STATE_DISCONNECTED;
		svState.players[a].connection = INVALID_CONNECTION;
		svState.players[a].key = -1;
		svState.players[a].model = NULL;
		svState.players[a].ulLastUpdate = 0;
	}

	// set state vars
	svState.bMaxClients = (byte)iMaxClients;
	svState.iNumClients = 0;

	return 1;
}

/* ------------
pl_Terminate
------------ */
void pl_Terminate(void)
{
	int a;

	// assert
	if( svState.players == NULL ) return;

	// free all player resources
	for( a = 0 ; a < svState.bMaxClients ; a++ )
	{
		if( svState.players[a].state != PL_STATE_CONNECTED )
		{
			pl_Disconnect( svState.players[a].key, "Server Shutdown In Progress." );
			if( svState.players[a].model != NULL ) ei->mod_Unload( svState.players[a].model );
		}
	}

	// free the player list
	SAFE_RELEASE( svState.players );
}

/* ------------
pl_GetNewClients - checks for and handles incoming connections
------------ */
void pl_GetNewClients()
{
	connection_t newClient = INVALID_CONNECTION;
	char *addr;
	int bLocal;

	while( (newClient = ei->net_sv_Accept( &addr )) != INVALID_CONNECTION )
	{
		// is the network address on the localhost?
		bLocal = tcstrcmp( "127.0.0.1", addr );

		// check to see if we will allow the client to connect
		// local game
		if( !svState.bPublicServer )
		{
			if( (svState.iNumClients < svState.bMaxClients) && bLocal )
			{
				// accept new client
				pl_AddClient(newClient);
			}
			else
			{
				// disallow entry
				if( !bLocal )
				{
					pl_DenyClient( newClient, "You are connecting via an external address to a localhost only game." );
				}
				else
				{
					pl_DenyClient( newClient, "The server is full." );
				}
			}
		}
		// public game
		else
		{
			if( (svState.iNumClients < svState.bMaxClients) )
			{
				// accept client
				pl_AddClient(newClient);
			}
			else
			{
				// disallow entry
				pl_DenyClient(newClient, "The server is full." );
			}
		}
	}
}

/* ------------
pl_Disconnect - shutsdown the client of key, with message, and multicasts the leave to the rest of the clients
------------ */
void pl_Disconnect( unsigned int key, char *message )
{
	int a;

	// assert
	if( key < 0 ) return;

	// search
	for( a = 0 ; a < svState.bMaxClients ; a++ )
	{
		if( svState.players[a].key == key )
		{
			// compose a disconnect packet to the client
			ei->net_sv_SendByte( svState.players[a].connection, PK_STC_PLAYER_DISCONNECT );
			ei->net_sv_SendShort( svState.players[a].connection, (unsigned short int)tcstrlen(message) );
			ei->net_sv_SendString( svState.players[a].connection, message );

			// send the packet and shut down the client
			ei->net_sv_Flush( svState.players[a].connection );
			ei->net_sv_Disconnect( svState.players[a].connection );
			svState.players[a].connection = INVALID_CONNECTION;
			svState.players[a].state = PL_STATE_DISCONNECTED;
			svState.players[a].ulLastUpdate = 0;

			// tell everyone else about the disconnect
			ser_MultiCastByte( PK_SBC_PLAYER_DISCONNECT );
			ser_MultiCastShort( 4 );
			ser_MultiCastLong( svState.players[a].key );

			// clear media and data
			svState.players[a].key = -1;
			ei->mod_Unload( svState.players[a].model );
			memset( svState.players[a].pos, 0, sizeof(vec3) );
			memset( svState.players[a].ang, 0, sizeof(vec3) );
			memset( svState.players[a].name, 0, sizeof(byte) * PLAYER_MAX_NAME_SIZE );
			memset( svState.players[a].modName, 0, sizeof(byte) * PLAYER_MAX_NAME_SIZE );

			// decriment the player count
			svState.iNumClients --;

			// if this was the last player to leave, reset the state to lounging
			if( svState.iNumClients <= 0 )
			{
				svState.state = SER_STATE_LOUNGE;
			}

			return;
		}
	}
}

/* ------------
pl_AbnormalDisconnect - clears player slot on an abnormal disconnect
------------ */
void pl_AbnormalDisconnect( unsigned int key )
{
	int a;
	
	// assert
	if( key < 0 )
	{
		ei->con_Print( "Server Player:  AbnormalDisconnect:  Assert Failed:  key < 0" );
		ei->eng_Stop( "16001" );
		return;
	}
	
	// search
	for( a = 0 ; a < svState.bMaxClients ; a++ )
	{
		if( svState.players[a].key == key )
		{
			// disconnect the player
			ei->net_sv_Disconnect( svState.players[a].connection );
			svState.players[a].connection = INVALID_CONNECTION;
			svState.players[a].state = PL_STATE_DISCONNECTED;
			svState.players[a].ulLastUpdate = 0;
	
			// tell everyone else about the disconnect
			ser_MultiCastByte( PK_SBC_PLAYER_DISCONNECT );
			ser_MultiCastShort( 4 );
			ser_MultiCastLong( svState.players[a].key );
	
			// clear media and data
			svState.players[a].key = -1;
			ei->mod_Unload( svState.players[a].model );
			memset( svState.players[a].pos, 0, sizeof(vec3) );
			memset( svState.players[a].ang, 0, sizeof(vec3) );
			memset( svState.players[a].name, 0, sizeof(byte) * PLAYER_MAX_NAME_SIZE );
			memset( svState.players[a].modName, 0, sizeof(byte) * PLAYER_MAX_NAME_SIZE );

			// decriment the player count
			svState.iNumClients --;
			
			// if this was the last player to leave, reset the state to lounging
			if( svState.iNumClients <= 0 )
			{
				svState.state = SER_STATE_LOUNGE;
			}
			
			return;
		}
	}

}

/* ------------
pl_AddClient - returns 0 if we are full or 1 if the client was added successfully
------------ */
byte pl_AddClient( connection_t plConnection )
{
	int a;
	char *str = ei->var_GetStringFromName( "sv_message" );
	
	// find an empty client slot
	for( a = 0 ; a < svState.bMaxClients ; a++ )
	{
		if( svState.players[a].state == PL_STATE_DISCONNECTED )
		{
			// get the player a key
			svNumConnections++;
			// FIXME:  occude currenly connected key numbers
			svState.players[a].key = svNumConnections;

			// set the state as connecting
			svState.players[a].state = PL_STATE_CONNECTING;
			svState.players[a].connection = plConnection;
			svState.players[a].ulLastUpdate = svState.ser_frame_time;

			// send the client the connection acceptance and the key for this game
			ei->net_sv_SendByte( plConnection, PK_STC_JOIN_ACCEPT );
			ei->net_sv_SendShort( plConnection, (unsigned short int)(4 + tcstrlen(str)) );
			ei->net_sv_SendLong( plConnection, svState.players[a].key );
			ei->net_sv_SendString( plConnection, str );

			// increment the client count
			svState.iNumClients++;
			return 1;
		}
	}

	// assert ! we should never get here
	ei->con_Print( "<RED>Assert Failed In Server Player:  Could not find an open client in Add Client!" );
	return 0;
}

/* ------------
pl_DenyClient
------------ */
static void pl_DenyClient( connection_t plConnection, char *reason )
{
	// compose a denyal packet
	ei->net_sv_SendByte( plConnection, PK_STC_JOIN_DENY );
	ei->net_sv_SendShort( plConnection, (unsigned short int)tcstrlen(reason) );
	ei->net_sv_SendString( plConnection, reason );

	// flush the packet to the client then shutdown that connection
	ei->net_sv_Flush( plConnection );
	ei->net_sv_Disconnect( plConnection );
}

/* ------------
pl_SendState - sends the current state of the player to 'to' in the form of  PK_STC_PLAYER_STATE
			- key (4), name (n), break (1), model (n), break (1), pos (3 * 4), ang (2 * 2)
------------ */
void pl_SendState( int player, connection_t to )
{
	unsigned short int size;

	// assert
	if( (player < 0) || (player >= svState.bMaxClients) ) return;
	if( to == INVALID_CONNECTION ) return;

	// construct the size
	size = 4 + tcstrlen(svState.players[player].name) + 1 + tcstrlen(svState.players[player].modName) + 1 + 12 + 4;

	// send all the data
	ei->net_sv_SendByte( to, PK_STC_PLAYER_STATE );
	ei->net_sv_SendShort( to, size );
	ei->net_sv_SendLong( to, svState.players[player].key );
	ei->net_sv_SendString( to, svState.players[player].name );
	ei->net_sv_SendByte( to, 0 );
	ei->net_sv_SendString( to, svState.players[player].modName );
	ei->net_sv_SendByte( to, 0 );
	ei->net_sv_SendLong( to, tcPositionToLong( svState.players[player].pos[X_AXIS] ) );
	ei->net_sv_SendLong( to, tcPositionToLong( svState.players[player].pos[Y_AXIS] ) );
	ei->net_sv_SendLong( to, tcPositionToLong( svState.players[player].pos[Z_AXIS] ) );
	ei->net_sv_SendShort( to, tcAngleToShort( svState.players[player].ang[PITCH] ) );
	ei->net_sv_SendShort( to, tcAngleToShort( svState.players[player].ang[YAW] ) );
}

/* ------------
pl_SetState - called immediatly after connect for a new client, contains the name, a 0, then the model name
------------ */
void pl_SetState( int player, unsigned short int size, byte *data )
{
	int iNameIn = 0;
	int iDataIn = 0;
	
	while( (iDataIn < size) && (data[iDataIn]) && (iNameIn < PLAYER_MAX_NAME_SIZE) )
	{
		svState.players[player].name[iNameIn] = data[iDataIn];
		iDataIn++;
		iNameIn++;
	}

	iDataIn++;
	iNameIn = 0;
	while( (iDataIn < size) && (data[iDataIn]) && (iNameIn < PLAYER_MAX_NAME_SIZE) )
	{
		svState.players[player].modName[iNameIn] = data[iDataIn];
		iDataIn++;
		iNameIn++;
	}
}

/* ------------
pl_NameChange - called to change the players name
------------ */
void pl_NameChange( int pl, char *newName )
{
	// check that we have a valid string
	if( tcstrlen(newName) <= 0 )
	{
		// the player has sent us garbage
		// ignore the new name and continue as before
		return;
	}
	
	// do the copy
	tcstrncpy( svState.players[pl].name, newName, PLAYER_MAX_NAME_SIZE );
	svState.players[pl].ulLastUpdate= svState.ser_frame_time;

	// FIXME: broadcast to everyone
	// broadcast it to everyone
}

/* ------------
pl_ModelChange - called to change the players model
------------ */
void pl_ModelChange( int pl, char *newModel )
{
	// check that we have a valid string
	if( tcstrlen(newModel) <= 0 )
	{
		// the player has sent us garbage
		// ignore the new name and continue as before
		return;
	}
	
	// do the copy
	tcstrncpy( svState.players[pl].modName, newModel, PLAYER_MAX_NAME_SIZE );
	svState.players[pl].ulLastUpdate = svState.ser_frame_time;

	// reload a new model
	ei->mod_Unload( svState.players[pl].model );
	svState.players[pl].model = ei->mod_Load( newModel );
	if( svState.players[pl].model == NULL )
	{
		ei->con_Print( "<RED>Server Error:  Model Load failed for client model." );
		pl_Disconnect( svState.players[pl].key, "The requested model is not available on the server." );
		return;
	}
}

/* ------------
pl_PositionChange - called by the network message que on position change packets
				- updates the server with the "current" client position 
------------ */
void pl_PositionChange( int pl, unsigned short int size, byte *data )
{
	unsigned long int ulvTmp[3];
	int b;

	// assert
	if( (pl < 0) || (pl >= svState.bMaxClients) ) return;
	if( svState.players == NULL ) return;
	if( size != 12 ) return;
	if( data == NULL ) return;

	svState.players[pl].ulLastUpdate = svState.ser_frame_time;
	
	ulvTmp[0] = ei->net_GetLong( &(data[0]) );
	ulvTmp[1] = ei->net_GetLong( &(data[4]) );
	ulvTmp[2] = ei->net_GetLong( &(data[8]) );

	// verify the new data
	// pl_ArbitratePosition( ... );
					
	svState.players[pl].pos[0] = tcLongToPosition( ulvTmp[0] );
	svState.players[pl].pos[1] = tcLongToPosition( ulvTmp[1] );
	svState.players[pl].pos[2] = tcLongToPosition( ulvTmp[2] );
	
	// retransmit the information to all clients
	for( b = 0 ; b < svState.bMaxClients ; b++ )
	{
		if( (svState.players[b].state != PL_STATE_DISCONNECTED) && 
			(svState.players[b].key != svState.players[pl].key) )
		{
			ei->net_sv_SendByte( svState.players[b].connection, PK_SBC_PLAYER_POS );
			ei->net_sv_SendShort( svState.players[b].connection, 16 );
			ei->net_sv_SendLong( svState.players[b].connection, svState.ser_frame_time );
			ei->net_sv_SendLong( svState.players[b].connection, ulvTmp[0] );
			ei->net_sv_SendLong( svState.players[b].connection, ulvTmp[1] );
			ei->net_sv_SendLong( svState.players[b].connection, ulvTmp[2] );
		}
	}

//	ei->con_Print( "( %f, %f, %f ) : ( %f, %f )", svState.players[pl].pos[0], svState.players[pl].pos[1], svState.players[pl].pos[2],
//										svState.players[pl].ang[0], svState.players[pl].ang[1] );
}

/* ------------
pl_AngleChange - called by the network message que on angle change packets
				- updates the server with the "current" client position 
------------ */
void pl_AngleChange( int pl, unsigned short int size, byte *data )
{
	unsigned short int usvTmp[2];
	int b;
	
	// assert
	if( (pl < 0) || (pl >= svState.bMaxClients) ) return;
	if( svState.players == NULL ) return;
	if( size != 4 ) return;
	if( data == NULL ) return;

	svState.players[pl].ulLastUpdate = svState.ser_frame_time;
	
	usvTmp[0] = ei->net_GetShort( &(data[0]) );
	usvTmp[1] = ei->net_GetShort( &(data[2]) );
	
	// verify the new data
	// pl_ArbitrateAngle( ... );
	
	svState.players[pl].ang[0] = tcShortToAngle( usvTmp[0] );
	svState.players[pl].ang[1] = tcShortToAngle( usvTmp[1] );
	
	// retransmit the information to all clients
	for( b = 0 ; b < svState.bMaxClients ; b++ )
	{
		if( (svState.players[b].state != PL_STATE_DISCONNECTED) && 
			(svState.players[b].key != svState.players[pl].key) )
		{
			ei->net_sv_SendByte( svState.players[b].connection, PK_SBC_PLAYER_ANG );
			ei->net_sv_SendShort( svState.players[b].connection, 8 );
			ei->net_sv_SendLong( svState.players[b].connection, svState.ser_frame_time );
			ei->net_sv_SendLong( svState.players[b].connection, usvTmp[0] );
			ei->net_sv_SendLong( svState.players[b].connection, usvTmp[1] );
		}
	}
//	ei->con_Print( "( %f, %f, %f ) : ( %f, %f )", svState.players[pl].pos[0], svState.players[pl].pos[1], svState.players[pl].pos[2],
//										svState.players[pl].ang[0], svState.players[pl].ang[1] );
}
