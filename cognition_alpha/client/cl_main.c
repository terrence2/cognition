// Cognition
// client module
// client_main.c
// Created 1-27-03 by Terrence Cole

// Includes
/////////////
#include "cl_global.h"

// Definitions
////////////////

// Global Prototypes
//////////////////////
/*
client_interface_t*GetClientAPI( engine_interface_t *engine_interface );
int cl_Initialize(void);
void cl_Terminate(void);
int cl_Connect( char *address, char *port );
int cl_Frame( exec_state_t *state );
void cl_Draw2D( exec_state_t *state );
*/

// Global Variables
////////////////////
/*
client_interface_t ci;
engine_interface_t *ei;
*/

// *********** FUNCTIONALITY ***********
/* ------------
GetClientAPI
------------ */
client_interface_t*GetClientAPI( engine_interface_t *engine_interface )
{
	// capture the engine api
	ei = engine_interface;

	// prepare the client api
	ci.apiversion = API_VERSION;
	ci.cl_Initialize = cl_Initialize;
	ci.cl_Terminate = cl_Terminate;
	ci.cl_Connect = cl_Connect;
	ci.cl_Frame = cl_Frame;
	ci.cl_Draw2D = cl_Draw2D;

	return &ci;
}

/* ------------
cl_Initialize
------------ */
int cl_Initialize(void)
{
	// init mathlib
	if( !m_Initialize() ) return 0;
	
	// init player(s)
	if( pl_Initialize() != 1 ) return 0;

	// init the foreground
	if( !fgo_cl_Initialize() ) return 0;

	return 1;
}

/* ------------
cl_Terminate
------------ */
void cl_Terminate(void)
{
	ei->con_Print( "\t<RED>Terminating</RED> the client." );
	ei->eng_LoadingFrame();

	// get rid of the foreground
	fgo_cl_Terminate();
	
	// remove the world
	ei->wrld_Delete( plState.world );
	ei->d_SetWorld( NULL );

	// disconnect us from the server
	pl_Disconnect();

	// remove all player(s) memory
	pl_Terminate();
}

/* ------------
cl_Connect
------------ */
int cl_Connect( char *address, char *port )
{
	// connect to the requested server
	if( !ei->net_cl_Connect( address, port ) )
	{
		ei->con_Print( "<RED>Client Connect Failed:  Net Connect Failed for \"%s:%s\"", address, port );
		return 0;
	}

	return 1;
}

/* ------------
cl_Frame
------------ */
int cl_Frame( exec_state_t *state )
{
	// get the frame time
	plState.tLast = plState.tCurrent;
	plState.tCurrent = state->frame_time;
	plState.tDelta = plState.tCurrent - plState.tLast;

	// get updates to the game world and respond to misc messages
	if( !st_ParseMessages( state ) ) return 0;

	// do the accumulated player move
	if( plState.state == PL_STATE_CONNECTED )
	{
		// update the hud
		fgo_cl_Frame();

		// update the player position	
		if( !pl_MovePlayer( state ) ) return 0;
	}
	
	// send all pending data to the server
	if( ei->net_cl_Flush() == -1 )
	{
		ei->con_Print( "<RED>Clint Frame Failed:  Network Flush Failed." );
		return 0;
	}

	return 1;
}

/* ------------
cl_Draw2D
------------ */
void cl_Draw2D( exec_state_t *state )
{
	fgo_cl_Draw();
}

