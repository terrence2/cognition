// Cognition
// .c
// Created  by Terrence Cole

// Includes
/////////////
#include "cog_global.h"

// Definitions
////////////////
typedef menu_interface_t* (__cdecl* GetMenuAPI_t)( engine_interface_t *ei );
typedef client_interface_t* (__cdecl* GetClientAPI_t)( engine_interface_t *ei );
typedef server_interface_t* (__cdecl* GetServerAPI_t)( engine_interface_t *ei );

// Local Structures
/////////////////////

// Global Prototypes
//////////////////////
/*
int module_Initialize(void);
void module_Terminate(void);
int module_OpenMenu();
void module_CloseMenu();
int module_OpenClient();
void module_CloseClient();
int module_OpenServer();
void module_CloseServer();

*/

// Local Prototypes
/////////////////////

// Local Variables
////////////////////

// *********** FUNCTIONALITY ***********
/* ------------
mod_Initialize - 
------------ */
int module_Initialize(void)
{
	return 1;
}

/* ------------
mod_Terminate -
------------ */
void module_Terminate(void)
{
	module_CloseMenu();
	module_CloseClient();
	module_CloseServer();
}

/* ------------
mod_OpemMenu - loads the menu module and trades interfaces
------------ */
int module_OpenMenu()
{
	GetMenuAPI_t GetMenuAPI;
	char *path;

	con_Print( "\nLoading Menu Module..." );
	eng_LoadingFrame();

	// find the path to look for the menu dll
	path = fs_GetMediaPath( MENU_MODULE_NAME, "" );
	if( path == NULL )
	{
		con_Print( "Menu Module Load Failed:  Unable to resolve full path." );
		return 0;
	}

	// load the menu dll
	sys_handles.cMenu = LoadLibrary( path );
	if( !sys_handles.cMenu ) 
	{
		con_Print( "\tMenu Module Load Failed:  Load Library failed for \"%s\".", path );
		return 0;
	}

	GetMenuAPI = (GetMenuAPI_t)GetProcAddress( sys_handles.cMenu, "GetMenuAPI" );
	if( GetMenuAPI == NULL ) 
	{
		con_Print( "\tMenu Module Load Failed:  GetProcAddress failed for GetMenuAPI" );
		return 0;
	}

	// trade signatures
	mi = GetMenuAPI( ei );
	if( !mi )
	{
		con_Print( "\tMenu System API Trade Failed." );
		FreeLibrary(sys_handles.cMenu);
		sys_handles.cMenu = NULL;
		return 0;
	}
	if( mi->apiversion != API_VERSION )
	{
		con_Print( "\tMenu System has wrong API version:  Detected %d  Needed: %d", mi->apiversion, API_VERSION );
		FreeLibrary(sys_handles.cMenu);
		sys_handles.cMenu = NULL;
		return 0;
	}

	// allow the menu to initialize
	if( !mi->menu_Initialize() ) 
	{
		FreeLibrary(sys_handles.cMenu);
		sys_handles.cMenu = NULL;
		return 0;
	}

	state.bDrawMenu = 1;
	ms_Show();
	con_Print( "\tMenu Module successfully loaded." );

    return 1;
}

/* ------------
mod_CloseMenu - closes the menu interface
------------ */
void module_CloseMenu()
{
	con_Print( "\nUnloading Menu Module..." );

	if( sys_handles.cMenu != NULL )
	{
		// close self
		mi->menu_Terminate();

		// remove from address space
		con_Print( "\tReleasing Menu Module..." );
		
		FreeLibrary( sys_handles.cMenu );
		sys_handles.cMenu = NULL;
		mi = NULL;
		state.bDrawMenu = 0;
	}

	con_Print( "\tMenu System Terminated Successfully." );
}

/* ------------
mod_OpenClient
------------ */
int module_OpenClient()
{
	GetClientAPI_t GetClientAPI;
	char *path;

	con_Print( "\nLoading Client Module..." );

	// find the path to look for the menu dll
	path = fs_GetMediaPath( CLIENT_MODULE_NAME, "" );
	if( path == NULL )
	{
		con_Print( "Client Module Load Failed:  Unable to resolve full path." );
		return 0;
	}

	// load the menu dll
	sys_handles.cClient = LoadLibrary( path );
	if( !sys_handles.cClient ) 
	{
		con_Print( "\tClient Module Load Failed:  Load Library failed for \"%s\".", path );
		return 0;
	}

	GetClientAPI = (GetClientAPI_t)GetProcAddress( sys_handles.cClient, "GetClientAPI" );
	if( GetClientAPI == NULL ) 
	{
		con_Print( "\tClient Module Load Failed:  GetProcAddress failed for GetClientAPI" );
		FreeLibrary(sys_handles.cClient );
		sys_handles.cClient = NULL;
		return 0;
	}

	// trade signatures
	ci = GetClientAPI( ei );
	if( !ci )
	{
		con_Print( "\tClient System API Trade Failed." );
		FreeLibrary(sys_handles.cClient);
		sys_handles.cClient = NULL;
		return 0;
	}
	if( ci->apiversion != API_VERSION )
	{
		con_Print( "\tClient System has wrong API version:  Detected %d  Needed: %d", ci->apiversion, API_VERSION );
		FreeLibrary(sys_handles.cClient);
		sys_handles.cClient = NULL;
		return 0;
	}

	// allow the menu to initialize
	if( !ci->cl_Initialize() )
	{
		FreeLibrary( sys_handles.cClient );
		sys_handles.cClient = NULL;
		return 0;
	}

	con_Print( "\tClient Module successfully loaded." );

	return 1;
}

/* ------------
mod_CloseClient
------------ */
void module_CloseClient()
{
	con_Print( "\nUnloading Client Module..." );

	if( sys_handles.cClient != NULL )
	{
		// close self
		ci->cl_Terminate();

		// remove from address space
		con_Print( "\tReleasing Client Module..." );
		
		FreeLibrary( sys_handles.cClient );
		sys_handles.cClient = NULL;
		ci = NULL;
	}

	con_Print( "\tClient System Terminated Successfully." );
}

/* ------------
mod_OpenServer
------------ */
int module_OpenServer()
{
	GetServerAPI_t GetServerAPI;
	char *path;
	
	con_Print( "\n<BLUE>Loading</BLUE> Server Module..." );
	eng_LoadingFrame();
	
	// find the path to look for the menu dll
	path = fs_GetMediaPath( SERVER_MODULE_NAME, "" );
	if( path == NULL )
	{
		con_Print( "Server Module Load Failed:	Unable to resolve full path." );
		return 0;
	}
	
	// load the menu dll
	sys_handles.cServer = LoadLibrary( path );
	if( !sys_handles.cServer ) 
	{
		con_Print( "\tServer Module Load Failed:  Load Library failed for \"%s\".", path );
		return 0;
	}
	
	GetServerAPI = (GetServerAPI_t)GetProcAddress( sys_handles.cServer, "GetServerAPI" );
	if( GetServerAPI == NULL ) 
	{
		con_Print( "\tServer Module Load Failed:  GetProcAddress failed for GetServerAPI" );
		FreeLibrary( sys_handles.cServer );
		sys_handles.cServer = NULL;
		return 0;
	}
	
	// trade signatures
	si = GetServerAPI( ei );
	if( !si )
	{
		con_Print( "\tServer System API Trade Failed." );
		FreeLibrary( sys_handles.cServer );
		sys_handles.cServer = NULL;
		return 0;
	}
	if( si->apiversion != API_VERSION )
	{
		con_Print( "\tServer System has wrong API version:  Detected %d  Needed: %d", si->apiversion, API_VERSION );
		FreeLibrary( sys_handles.cServer );
		sys_handles.cServer = NULL;
		return 0;
	}
	
	// allow the menu to initialize
	if( !si->ser_Initialize() )
	{
		FreeLibrary( sys_handles.cServer );
		sys_handles.cServer = NULL;
		return 0;
	}
	
	con_Print( "\tServer Module <GREEN>successfully</GREEN> loaded." );
	eng_LoadingFrame();
	
	return 1;
}

/* ------------
mod_CloseServer
------------ */
void module_CloseServer()
{
	con_Print( "\nUnloading Server Module..." );
	eng_LoadingFrame();
	
	if( sys_handles.cServer != NULL )
	{
		// close self
		si->ser_Terminate();
	
		// remove from address space
		con_Print( "\tReleasing Server Module..." );
		
		FreeLibrary( sys_handles.cServer );
		sys_handles.cServer = NULL;
		si = NULL;
	}
	
	con_Print( "\tServer System Terminated Successfully." );
	eng_LoadingFrame();
}
