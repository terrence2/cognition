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
// server_main.c
// Created 1-27-03 by Terrence Cole

// Includes
/////////////
#include "ser_global.h"

// Definitions
////////////////

// Global Prototypes
//////////////////////
/*
server_interface_t*GetServerAPI( engine_interface_t *engine_interface );
int ser_Initialize(void);
void ser_Terminate(void);
int ser_Start( int iMaxClients, byte bPublicServer, int *size, int dimension, int numMines );
void ser_Frame(void);
int ser_MultiCastByte( byte b );
int ser_MultiCastShort( unsigned short int us );
int ser_MultiCastLong( unsigned long int ul );
int ser_MultiCastString( char *str );
*/

// Global Variables
////////////////////
server_interface_t *si = 0;
engine_interface_t *ei = 0;
ser_state_t svState;

// *********** FUNCTIONALITY ***********
/* ------------
GetServerAPI
------------ */
server_interface_t*GetServerAPI( engine_interface_t *engine_interface )
{
	// capture the engine api
	ei = engine_interface;

	// create the new server interface
	si = ei->mem_alloc( sizeof(server_interface_t) );
	memset( si, 0, sizeof(server_interface_t) );
	si->apiversion = API_VERSION;
	si->ser_Initialize = ser_Initialize;
	si->ser_Terminate = ser_Terminate;
	si->ser_Start = ser_Start;
	si->ser_Frame = ser_Frame;

	return si;
}

/* ------------
ser_Initialize
------------ */
int ser_Initialize(void)
{
	svState.bMaxClients = 1;
	svState.bPublicServer = 0;
	svState.iNumClients = 0;
	svState.launch_time = MAX_INT;
	svState.next_start_time = MAX_INT;
	svState.players = NULL;
	svState.ser_frame_time = 0;
	svState.state = SER_STATE_LOUNGE;
	svState.world = NULL;
	return 1;
}

/* ------------
ser_Terminate
------------ */
void ser_Terminate(void)
{
	ei->con_Print( "\n<RED>Terminating</RED> Server..." );

	// close down the player structures
	pl_Terminate();
	
	// shutdown any open connections
	ei->net_sv_CloseNetwork();

	// close this world
	ei->wrld_Delete( svState.world );

	// free our interface
	ei->mem_free( si );
}

/* ------------
ser_Start
------------ */
int ser_Start( int iMaxClients, byte bPublicServer, int *size, int dimension, int numMines )
{
	char *port;
	
	ei->con_Print( "\nServer <GREEN>Start:</GREEN>" );
	ei->eng_LoadingFrame();

	// setup states
	svState.bPublicServer = bPublicServer;
	svState.state = SER_STATE_LOUNGE;

	// init the players
	pl_Initialize( iMaxClients );

	// world
	svState.world = ei->wrld_Create( size, dimension, numMines ) ;
	if( svState.world == NULL )
		return 0;
	
	// get the port
	ei->con_Print( "\nServer Start:\n\tOpening the network..." );
	ei->eng_LoadingFrame();
	port = ei->var_GetStringFromName( "sv_port" );
	if( (port == NULL) || ((unsigned short)tcatoi(port) < 1024) )
	{
		// open up the network on the default if we're out of range
		if( !ei->net_sv_OpenNetwork( DEF_SERVER_PORT ) ) return 0;
		ei->con_Print( "\tServer Opened on Port %d", DEF_SERVER_PORT );
		ei->eng_LoadingFrame();
	}
	else
	{
		// open on the specified if the port is in range
		if( !ei->net_sv_OpenNetwork( port ) ) return 0;
		ei->con_Print( "\tServer Opened on Port %s", port );
		ei->eng_LoadingFrame();
	}
	
	return 1;
}

/* ------------
ser_Frame - runs one frame of execution on the server - should be called every 100 ms from the Engine
------------ */
void ser_Frame(void)
{
	int a;
	unsigned long int time = ei->ts_GetTime();
	
	// get the time
	svState.ser_frame_time = ei->ts_GetTime();

	// get new clients off of the incoming que
	pl_GetNewClients();

	// check for messages from our clients
	st_ParseIncoming();  // this dispatches to world and arbitrates disputes

	// check for state transitions
	if( svState.state == SER_STATE_LOUNGE 
		&& svState.iNumClients > 0 
		&& time >= svState.next_start_time )
	{
		ei->con_Print( "<GREEN>Broadcasting</GREEN> Launch Message" );
		svState.state = SER_STATE_LAUNCH;
		svState.launch_time = time + SER_STATE_LAUNCH_TIME;
		for( a = 0 ; a < svState.bMaxClients ; a++ )
		{
			ser_player_t *player = &(svState.players[a]);	
			if( player->state != PL_STATE_DISCONNECTED )
			{
				// tell the player the current game state
				ei->net_sv_SendByte( player->connection, PK_STC_WORLD_STATE_GAME );
				ei->net_sv_SendShort( player->connection, 4 );
				ei->net_sv_SendLong( player->connection, svState.state );
			}
		}
	}
	else if( svState.state == SER_STATE_LAUNCH
			&& time >= svState.launch_time )
	{
		ei->con_Print( "<GREEN>Broadcasting</GREEN> Start Message" );
		svState.state = SER_STATE_PLAYING;
		for( a = 0 ; a < svState.bMaxClients ; a++ )
		{
			ser_player_t *player = &(svState.players[a]);	
			if( player->state != PL_STATE_DISCONNECTED )
			{
				// tell the player the current game state
				ei->net_sv_SendByte( player->connection, PK_STC_WORLD_STATE_GAME );
				ei->net_sv_SendShort( player->connection, 4 );
				ei->net_sv_SendLong( player->connection, svState.state );
			}
		}
	}
	
	// send all data
	for( a = 0 ; a < svState.bMaxClients ; a++ )
	{
		if( svState.players[a].state != PL_STATE_DISCONNECTED )
		{
			if( ei->net_sv_Flush( svState.players[a].connection ) == -1 )
			{
				// this client has ended abnormally, so end the connection
				ei->con_Print( "The connection with %s has ended abnormally and been aborted." );
				pl_AbnormalDisconnect( svState.players[a].key );
			}
		}
	}
}

/* ------------
ser_MultiCast**** - transmits a data to all connected clients with the indicated size - frontends for net_SendData API
				- returns the number of clients successfully sent to
------------ */
int ser_MultiCastByte( byte b )
{
	byte a;
	int cnt = 0;

	for( a = 0 ; a < svState.bMaxClients ; a++ )
	{
		if( svState.players[a].state != PL_STATE_DISCONNECTED )
		{
			ei->net_sv_SendByte( svState.players[a].connection, b );
			cnt++;
		}
	}

	return cnt;
}

int ser_MultiCastShort( unsigned short int us )
{
	byte a;
	int cnt = 0;

	for( a = 0 ; a < svState.bMaxClients ; a++ )
	{
		if( svState.players[a].state != PL_STATE_DISCONNECTED )
		{
			ei->net_sv_SendShort( svState.players[a].connection, us );
			cnt++;
		}
	}

	return cnt;
}

int ser_MultiCastLong( unsigned long int ul )
{
	byte a;
	int cnt = 0;

	for( a = 0 ; a < svState.bMaxClients ; a++ )
	{
		if( svState.players[a].state != PL_STATE_DISCONNECTED )
		{
			ei->net_sv_SendLong( svState.players[a].connection, ul );
			cnt++;
		}
	}

	return cnt;
}

int ser_MultiCastString( char *str )
{
	byte a;
	int cnt = 0;

	for( a = 0 ; a < svState.bMaxClients ; a++ )
	{
		if( svState.players[a].state != PL_STATE_DISCONNECTED )
		{
			ei->net_sv_SendString( svState.players[a].connection, str );
			cnt++;
		}
	}

	return cnt;
}
