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
// .c
// Created  by Terrence Cole

// Includes
/////////////
#include "cog_global.h"
#include <dlfcn.h>

// Definitions
////////////////
typedef menu_interface_t* (*GetMenuAPI_t)( engine_interface_t *ei );
typedef client_interface_t* (*GetClientAPI_t)( engine_interface_t *ei );
typedef server_interface_t* (*GetServerAPI_t)( engine_interface_t *ei );

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

	con_Print( "\nLoading Menu Module..." );
	eng_LoadingFrame();

	// load the menu dll
	sys_handles.cMenu = dlopen( MENU_MODULE_NAME, RTLD_LAZY );
	if( !sys_handles.cMenu ) 
	{
		con_Print( "\tMenu Module Load Failed:  Load Library failed for \"%s\": %s.", MENU_MODULE_NAME, dlerror() );
		return 0;
	}

	GetMenuAPI = (GetMenuAPI_t)dlsym( sys_handles.cMenu, "GetMenuAPI" );
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
		dlclose(sys_handles.cMenu);
		sys_handles.cMenu = NULL;
		return 0;
	}
	if( mi->apiversion != API_VERSION )
	{
		con_Print( "\tMenu System has wrong API version:  Detected %d  Needed: %d", mi->apiversion, API_VERSION );
		dlclose(sys_handles.cMenu);
		sys_handles.cMenu = NULL;
		return 0;
	}

	// allow the menu to initialize
	if( !mi->menu_Initialize() ) 
	{
		dlclose(sys_handles.cMenu);
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
		
		dlclose( sys_handles.cMenu );
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

	con_Print( "\nLoading Client Module..." );

	// load the menu dll
	sys_handles.cClient = dlopen( CLIENT_MODULE_NAME, RTLD_LAZY );
	if( !sys_handles.cClient ) 
	{
		con_Print( "\tClient Module Load Failed:  Load Library failed for \"%s\": %s.", CLIENT_MODULE_NAME, dlerror() );
		return 0;
	}

	GetClientAPI = (GetClientAPI_t)dlsym( sys_handles.cClient, "GetClientAPI" );
	if( GetClientAPI == NULL ) 
	{
		con_Print( "\tClient Module Load Failed:  GetProcAddress failed for GetClientAPI" );
		dlclose(sys_handles.cClient );
		sys_handles.cClient = NULL;
		return 0;
	}

	// trade signatures
	ci = GetClientAPI( ei );
	if( !ci )
	{
		con_Print( "\tClient System API Trade Failed." );
		dlclose(sys_handles.cClient);
		sys_handles.cClient = NULL;
		return 0;
	}
	if( ci->apiversion != API_VERSION )
	{
		con_Print( "\tClient System has wrong API version:  Detected %d  Needed: %d", ci->apiversion, API_VERSION );
		dlclose(sys_handles.cClient);
		sys_handles.cClient = NULL;
		return 0;
	}

	// allow the menu to initialize
	if( !ci->cl_Initialize() )
	{
		dlclose( sys_handles.cClient );
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
		
		dlclose( sys_handles.cClient );
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
	
	con_Print( "\n<BLUE>Loading</BLUE> Server Module..." );
	eng_LoadingFrame();

	// load the menu dll
	sys_handles.cServer = dlopen( SERVER_MODULE_NAME, RTLD_LAZY );
	if( !sys_handles.cServer ) 
	{
		con_Print( "\tServer Module Load Failed:  Load Library failed for \"%s\": %s.", SERVER_MODULE_NAME, dlerror() );
		return 0;
	}
	
	GetServerAPI = (GetServerAPI_t)dlsym( sys_handles.cServer, "GetServerAPI" );
	if( GetServerAPI == NULL ) 
	{
		con_Print( "\tServer Module Load Failed:  GetProcAddress failed for GetServerAPI" );
		dlclose( sys_handles.cServer );
		sys_handles.cServer = NULL;
		return 0;
	}
	
	// trade signatures
	si = GetServerAPI( ei );
	if( !si )
	{
		con_Print( "\tServer System API Trade Failed." );
		dlclose( sys_handles.cServer );
		sys_handles.cServer = NULL;
		return 0;
	}
	if( si->apiversion != API_VERSION )
	{
		con_Print( "\tServer System has wrong API version:  Detected %d  Needed: %d", si->apiversion, API_VERSION );
		dlclose( sys_handles.cServer );
		sys_handles.cServer = NULL;
		return 0;
	}
	
	// allow the menu to initialize
	if( !si->ser_Initialize() )
	{
		dlclose( sys_handles.cServer );
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
		
		dlclose( sys_handles.cServer );
		sys_handles.cServer = NULL;
		si = NULL;
	}
	
	con_Print( "\tServer System Terminated Successfully." );
	eng_LoadingFrame();
}
