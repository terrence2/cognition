// Cognition
// game_game.c
// by Terrence Cole 1/30/03

// Includes
/////////////
#include "cog_global.h"

// Definitions
////////////////
/*
#define GAME_TYPE_NOGAME 0
#define GAME_TYPE_SINGLE 1
#define GAME_TYPE_MULTI 2
*/

// Structures
///////////////

// Global Prototypes
//////////////////////
/*
int game_Initialize(void);
void game_Terminate(void);

void game_ShowLoading();
void game_ShowMenu();
void game_ShowGame();

void game_Start( char *type );
void game_End( char *null );
void game_Connect( char *address );
void game_Disconnect( char *null );
void game_StartServer( char *type );
void game_EndServer( char *null );
*/

// Local Prototypes
/////////////////////
static byte game_args_GetGameArgs( char *type, int *vGameType, int *vWidth, int *vHeight, int *vDepth, int *vMineCount  );

// Local Variables
////////////////////

// *********** FUNCTIONALITY ***********
/* ------------
game_Initialize - sets the default game state
------------ */
int game_Initialize(void)
{
	return 1;
}

/* ------------
game_Terminate - ends the current game and frees all memory associted with gameplay
------------ */
void game_Terminate(void)
{
}

/* ------------
game_ShowLoading
------------ */
void game_ShowLoading()
{
	state.bInLoading = 1;
	state.bDrawMenu = 0;
	ms_Hide();
}

/* ------------
game_ShowLoading
------------ */
void game_ShowMenu()
{
	state.bInLoading = 0;
	state.bDrawMenu = 1;
	ms_Show();
}

/* ------------
game_ShowGame
------------ */
void game_ShowGame()
{
	state.bInLoading = 0;
	state.bDrawMenu = 0;
	ms_Hide();
}


/* ------------
game_Start 
// opens a new single player game
------------ */
void game_Start( char *type )
{
	int iGameType, iWidth, iHeight, iDepth, iMineCount;
	int vSize[3];

	// assert
	if( type == NULL )
	{
		con_Print( "<RED>Game:  Game Start:  A game type was not specified." );
		con_Print( "<GREEN>Syntax:</GREEN><BLUE>  game \"x\" \"y\" \"z\" \"# mines\"</BLUE>" );
		return;
	}

	// preconditions
	if( sys_handles.cServer != NULL )
	{
		con_Print( "<RED>Game:  Game Start:  A server is already running!" );
		con_Print( "\tConnect to it with \"connect localhost\" or end it with" );
		con_Print( "\t\"end_server\" and then try again." );
		return ;
	}
	if( sys_handles.cClient != NULL )
	{
		con_Print( "<RED>Game:  Game Start:  A client is already running!" );
		con_Print( "\tEnd a current single player game with \"end\" or" );
		con_Print( "\tend a multi-player game with \"disconnect\" then" );
		con_Print( "\ttry again." );
		return;
	}
	
	// resolve the command line to a set of tokens we can use
	if( !game_args_GetGameArgs( type, &iGameType, &iWidth, &iHeight, &iDepth, &iMineCount ) )
	{
		con_Print( "Game: Game Start Failed: Parse Args Failed." );
		return;
	}

	// put us in loading drawing
	game_ShowLoading();

	// put us in loading drawing mode
	con_Print( "Starting Game Type %d: ( %d, %d, %d ) with %d mines.", 
					iGameType, iWidth, iHeight, iDepth, iMineCount );	
	eng_LoadingFrame();

	// open a server and start it in localhost
	if( !module_OpenServer() )
	{
		con_Print( "<RED>Game Start Failed!  Unable to open the server module.</RED>" );
		state.bInLoading = 0;
		state.bDrawMenu = 1;
		ms_Show();
		return;
	}

	// open the server to one client on localhost
	vSize[0] = iWidth;
	vSize[1] = iHeight;
	vSize[2] = iDepth;
	if( !si->ser_Start( 1, 0, vSize, iGameType, iMineCount ) )
	{
		con_Print( "<RED>Game Start Failed!  Server start failed." );
		module_CloseServer();
		state.bInLoading = 0;
		state.bDrawMenu = 1;
		ms_Show();
		return;
	}

	// open the client
	if( !module_OpenClient() )
	{
		con_Print( "<RED>Game Start Failed!  Unable to open the client module.</RED>" );
		module_CloseServer();
		state.bInLoading = 0;
		state.bDrawMenu = 1;
		ms_Show();
		return;
	}

	// make the client connect to the server
	if( !ci->cl_Connect( LOCALHOST, var_GetStringFromName("sv_port")) )
	{
		con_Print( "<RED>Game Start Failed!  Client was unable to connect." );
		module_CloseClient();
		module_CloseServer();
		state.bInLoading = 0;
		state.bDrawMenu = 1;
		ms_Show();
		return;
	}

	// the client will end draw mode when the connection completes
	// game_HideLoading();

	// set the game type appropriatly
	state.bGameType = GAME_TYPE_SINGLE;
}

/* ------------
game_Ends - stops the current single player game - not like reset, don't use this if you don't want to start a new game
------------ */
void game_End( char *null )
{
	// preconditions
	if( state.bGameType != GAME_TYPE_SINGLE )
	{
		con_Print( "<RED>Game:  Game End:  A single player game is not active!" );
		return;
	}

	// put us back in loading mode
	game_ShowLoading();
	con_Print( "<RED>Ending</RED> current game." );

	// close and disconnect the client
	if( sys_handles.cClient != NULL )
	{
		// close the client
		module_CloseClient();
	}
	else
	{
		con_Print( "<RED>Game:  Game End:  Warning:  Client module is not active on close!" );
	}

	// close and disable the server
	if( sys_handles.cServer != NULL )
	{
		// close the server
		module_CloseServer();
	}
	else
	{
		con_Print( "<RED>Game: Game End:  Warning:  Server module is not active on close!" );
	}

	// put us back in menu drawing mode and gameless state
	game_ShowMenu();
	state.bGameType = GAME_TYPE_NOGAME;
}

/* ------------
game_ServerStart - opens a new multi player server with the specified traits
				- formate of 'type' is "x y z numMines"
------------ */
void game_StartServer( char *type )
{
	int iGameType, iWidth, iHeight, iDepth, iMineCount;
	int vSize[3];

	// error conditions
	if( type == NULL )
	{
		con_Print( "<RED>Game: Server Start:  A game type was not specified." );
		con_Print( "<GREEN>Syntax:</GREEN><BLUE>  server_start \"x\" \"y\" \"z\" \"# mines\"</BLUE>" );
		return;
	}
	if( sys_handles.cServer != NULL )
	{
		con_Print( "<RED>Game:  Server Start:  A server is already active!" );
		return;
	}
	if( state.bGameType == GAME_TYPE_SINGLE )
	{
		con_Print( "<RED>Game: Server Start:  A single player game is in progress!" );
		return;
	}

	// resolve the command line to a set of tokens we can use
	if( !game_args_GetGameArgs( type, &iGameType, &iWidth, &iHeight, &iDepth, &iMineCount ) )
	{
		con_Print( "Game: Server Start Failed:  Parse Args Failed." );
		return;
	}
	vSize[0] = iWidth;
	vSize[1] = iHeight;
	vSize[2] = iDepth;

	// we want to do the load completely in the background
	// in the case where there is a client running, we don't want to interrupt play for the server load
	con_Print( "Starting Server:  Game Type %d: ( %d, %d, %d ) with %d mines.", 
					iGameType, iWidth, iHeight, iDepth, iMineCount );	

	
	// open a server and start it in localhost
	if( !module_OpenServer() )
	{
		con_Print( "<RED>Game Start Failed!  Unable to open the server module.</RED>" );
		return;
	}
	
	// open the server to sv_max_clients on sv_port
	if( !si->ser_Start( (int)var_GetFloatFromName("sv_max_clients"), 1, vSize, iGameType, iMineCount ) )
	{
		con_Print( "<RED>Game Start Failed!  Server start failed." );
		module_CloseServer();
		return;
	}

	// put us back in menu draw mode:  we don't have a client to draw from
	state.bGameType = GAME_TYPE_MULTI;
}

/* ------------
game_ServerStart - closes the currently running server, if any
------------ */
void game_EndServer( char *null )
{
	// preconditions
	if( sys_handles.cServer == NULL )
	{
		con_Print( "<RED>Game:  Server End:  A server is not active!" );
		return;
	}
	if( state.bGameType == GAME_TYPE_SINGLE )
	{
		con_Print( "<RED>Game:  Server End:  Use \"end\" to terminate a single player game!" );
		return;
	}

	// again, we want to handle server stuff in the background to avoid messing with client games

	// unload the server
	module_CloseServer();

	// we don't want to have nothing running, so load a menu if it isn't active
	if( (sys_handles.cClient == NULL) )
	{		
		// set us to menu draw
		state.bDrawMenu = 1;
	}
}

/* ------------
game_Connect - loads a client and connects it to address:port
------------ */
void game_Connect( char *address )
{
	int iToToken, iTokenLen;
	char cAddrBuf[1024];
	char cPortBuf[1024];
	int i;
	
	// preconditions
	if( address == NULL )
	{
		con_Print( "<RED>Game:  Connect:  The server address was not specified!" );
		con_Print( "<GREEN>Syntax:</GREEN><BLUE>  connect \"address:port\"</BLUE>" );
		return;
	}
	if( state.bGameType == GAME_TYPE_SINGLE )
	{
		con_Print( "<RED>Game:  Connect:  A single player game is in progress!" );
		return;
	}
	if( sys_handles.cClient != NULL )
	{
		con_Print( "<RED>Game:  Connect:  A client game is already active!" );
		return;
	}

	// throw up a loading screen
	state.bInLoading = 1;
	state.bDrawMenu = 0;
	ms_Hide();

	// get the address token parameters
	if( !par_GetNextToken( address, &iToToken, &iTokenLen ) )
	{
		
		con_Print( "<RED>Game: Connect:  The server address was not specified!" );
		con_Print( "<GREEN>Syntax:</GREEN><BLUE>  connect \"address:port\"</BLUE>" );
		state.bInLoading = 0;
		state.bDrawMenu = 1;
		ms_Show();
		return;
	}

	// jump to the token
	address += iToToken;

	// parse out the address from the command line
	i = 0;
	memset( cAddrBuf, 0, 1024 * sizeof(char) );
	while( (*address) && (*address != ':') && (!par_IsWhiteSpace(*address)) && (i < 1023) )
	{
		cAddrBuf[i] = *address;
		address++;
		i++;
	}

	// check for errors
	if( (i >= 1022) )
	{
		con_Print( "<RED>Game: Connect:  Address Buffer Flooded:  cannot continue with connect." );
		state.bInLoading = 1;
		state.bDrawMenu = 0;
		ms_Show();
		return;
	}

	// load the port
	i = 0;
	memset( cPortBuf, 0, 1024 );
	while( (*address) && (!par_IsWhiteSpace(*address)) && (i < 1023) )
	{
		cPortBuf[i] = *address;
		address++;
		i++;
	}

	// check for an error
	if( i >= 1022 )
	{
		con_Print( "<RED>Game:  Connect:  Port Buffer Flooded:  will not continue with connect." );
		state.bInLoading = 1;
		state.bDrawMenu = 0;
		ms_Show();
		return;
	}

	// check to see if we got a port, if not use the current default
	if( i <= 0 )
	{
		tcstrcpy( cPortBuf, var_GetStringFromName( "sv_port" ) );
	}

	// load the client
	eng_LoadingFrame();
	if( !module_OpenClient() )
	{
		con_Print( "<RED>Game:  Connect:  Open Client Module Failed!" );
		state.bInLoading = 1;
		state.bDrawMenu = 0;
		ms_Show();
		return;
	}

	// connect the client
	eng_LoadingFrame();
	if( !ci->cl_Connect( cAddrBuf, cPortBuf ) )
	{
		con_Print( "<RED>Game:  Connect:  Connect Failed to \"%s:%s\".", cAddrBuf, cPortBuf );
		module_CloseClient();
		state.bInLoading = 1;
		state.bDrawMenu = 0;
		ms_Show();
		return;
	}

	// set the state to game
	eng_LoadingFrame();
	state.bInLoading = 0;
	state.bGameType = GAME_TYPE_MULTI;
}

/* ------------
game_Disconnect - disconnects and unloads the connected multiplayer client
------------ */
void game_Disconnect( char *null )
{
	// preconditions
	if( state.bGameType == GAME_TYPE_SINGLE )
	{
		con_Print( "<RED>Game: Disconnect:  Cannot disconnect from a single player game!" );
		return;
	}
	if( sys_handles.cClient == NULL )
	{
		con_Print( "<RED>Game: Disconnect:  No client is loaded!" );
		return;
	}

	// set the view to loading
	state.bInLoading = 0;
	state.bDrawMenu = 1;
	ms_Show();
	
	eng_LoadingFrame();

	// unload the client
	module_CloseClient();

	// the game state will be based on whether we have a server still up
	if( sys_handles.cServer == NULL ) state.bGameType = GAME_TYPE_NOGAME;
}

/* ------------
game_args_GetGameArgs - takes the type arg passed to a game start and parses it into 5 tokens
------------ */
static byte game_args_GetGameArgs( char *type, int *vGameType, int *vWidth, int *vHeight, int *vDepth, int *vMineCount  )
{
	int a, b;
	char *pBuffer = type;
	int iToToken, iTokenLen, iTmpLen;
	int vTmp[5];
	char sizeBuffer[256];	
	
	// the empty case
	if( type == NULL ) return 0;

	// resolve the command line to a set of tokens we can use
	for( a = 0 ; a < 5 ; a++ )
	{
		// get the next element of the size vector
		if( !par_GetNextToken( pBuffer, &iToToken, &iTokenLen ) )
		{
			con_Print( "<RED>Game: Parse Arguments Failed!</RED>" );
			con_Print( "<GREEN>Syntax:</GREEN><BLUE>  game \"type\" \"x\" \"y\" \"z\" \"# mines\"</BLUE>" );
			return 0;
		}
	
		// advance to the token
		pBuffer += iToToken;
	
		// lock the buffer into range
		if( iTokenLen > 255 ) iTmpLen = 255;
		else iTmpLen = iTokenLen;
	
		// copy the next token to a temp buffer
		for( b = 0 ; b < iTmpLen ; b++ )
		{
			sizeBuffer[b] = pBuffer[b];
		}
		sizeBuffer[iTmpLen] = '\0';
	
		// advance past the token
		pBuffer += iTokenLen;
	
		// get the item into a temp array
		vTmp[a] = tcatoi( sizeBuffer );
	}

	// copy from temp out to named variables
	*vGameType = vTmp[0];
	*vWidth = vTmp[1];
	*vHeight = vTmp[2];
	*vDepth = vTmp[3];
	*vMineCount = vTmp[4];

	return 1;
}


