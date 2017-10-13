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
// variable.c
// Created 2-25-02 @ 1042 by Terrence Cole

// Includes
/////////////
#include "cog_global.h"

// Definitions
////////////////
#define VAR_MAX_STRING_SIZE 128
#define VAR_STATES_FILE "var_states.txt"
#define VAR_FLAG_PROTECTED 2

// Structures
///////////////
typedef struct variable_s
{
	char* sName;
	char  sValue[VAR_MAX_STRING_SIZE + 1];
	float fValue;
	int	  iflags;
	int	  ilatches;
	void (*SyncFunction)(void);
} variable_t;

// Global Prototypes
//////////////////////
/*
int var_Initialize(void);
void var_Terminate(void);
varlatch var_GetVarLatch( char *varname );
float var_GetFloat( varlatch handle );
char *var_GetString( varlatch handle );
float var_GetFloatFromName( char *var_name )
char *var_GetStringFromName( char *var_name )
void var_UpdateFloat( varlatch handle, float value );
void var_UpdateString( varlatch handle, char *value );
void var_SetValue( char *buffer );
void var_SetValueSync( char *buffer );
void var_UpdateVar( char *varname, char *varvalue );
void var_ValueOf( char *varName );
*/

// Local Prototypes
/////////////////////
static varlatch var_FindVariable( char *varname );

// Local Variables
////////////////////
//	       NAME				S_VALUE			F_VALUE		FLAGS					LATCHES
//	        ----			-------			-------		-----					-------
static variable_t varList[] = {

	/* ------ VIDEO ------*/
	{ "render_width", "640", 640, 0, 0, NULL },
		// the screen width for fullscreen rendering
	{ "render_height", "480", 480, 0, 0, NULL },
		// the screen height for fullscreen rendering
	{ "render_bitdepth", "16", 16, 0, 0, NULL },
		// the bitdepth for fullscreen rendering
	{ "render_windowed", "1", 1, 0, 0, NULL },
		// if zero, tells the video system to do fullscreen rendering
	{ "render_max_refreshrate", "60", 60, 0, 0, NULL },
		// tells the OGL SAL the fastest refresh it can put the monitor into

	/* ------ INPUT ------*/
	{ "input_mode", "auto", 1, 0, 0, NULL },
		// dependent on the operating system.  In auto mode, selects
		// the "best" option on startup in descending order.  If set
		// to an iterface, attempts to load that interface first.
		// In Windows, the options are "direct_input" and "win32".

	/* ------ SOUND ------*/
	{ "snd_mode", "auto", 0, 0, 0, NULL },
		// sets the audio device to use for playing sounds { auto, directsound }
	{ "snd_rate", "44100", 44100, 0, 0, NULL },
		// sets the sound sample rate
	{ "snd_depth", "16", 16, 0, 0, NULL },
		// sets the sound bitdepth
	{ "snd_channels", "32", 32, 0, 0, NULL },
		// the maximum number of sounds allowed to play simultaniously
	{ "snd_volume", "100.0", 10.0, 0, 0, s_UpdateVolume },
		// sets the global volume gain
	{ "snd_sfx_volume", "100.0", 10.0, 0, 0, s_UpdateVolume },
		// sets the sound fx volume
	{ "snd_music_volume", "100.0", 10.0, 0, 0, s_UpdateVolume },
		// sets the music volume
	{ "snd_sfx_preload", "0.2", 0.2f, 0, 0, NULL },
		// number of seconds of audio to preload per sfx stream
	{ "snd_music_preload", "0.5", 0.5, 0, 0, NULL },
		// number of seconds of audio to preload for the music player
	{ "snd_sfx_quality", "high", 0.0, 0, 0, NULL },
		// sets the sound spacialization method

	/* ------ CLIENT / PLAYER ------*/
	{ "sensitivity", "1.0", 1, 0, 0, NULL },
		// the default mouse sensitivity, linear scale factor
	{ "menu_sensitivity", "2.5", 2.5, 0, 0, NULL },
		// the default sensitivity of the mouse in menu selection mode
	{ "name", "Player", 0, 0, 0, NULL },
		// the name to use for the player on the local client
	{ "model", "ship01.def", 0, 0, 0, NULL },
		// the default name of the player model
	{ "movespeed", "100", 100, 0, 0, NULL },
		// the default move speed of the player
	{ "mInvertX", "0", 0, 0, 0, NULL },
		// inverts movement on the mouse X axis
	{ "mInvertY", "0", 0, 0, 0, NULL },
		// inverts movement on the mouse Y axis
	{ "crosshair", "crosshair0", 0, 0, 0, NULL },
		// sets the player's crosshair
	{ "cl_skybox", "skybox01.def", 0, 0, 0, NULL },
		// sets the displayed skybox
	{ "noclip", "0", 0, 0, 0, NULL },
		// noclip on or off
	{ "world_color_r", "0", 0, 0, 0, NULL },
	{ "world_color_g", "0", 0, 0, 0, NULL },
	{ "world_color_b", "255", 255.0f, 0, 0, NULL },
		// the world color

	/*------ GAME MEDIA ------*/
	{ "media_directory", "main", 1, 0, 0, NULL },
		// specifies an alternate directory to search for media and modules
		// when loading a game.
	{ "texture_detail", "3", 3, 0, 0, NULL },
		// the global texture rescale
	{ "texture_filter", "linear", 0, 0, 0, NULL },
		// the global texture filter, nearest or linear
	{ "cl_background_complexity", "1200", 1200, 0, 0, NULL },
		// the global background scene complexity, controls how many models are loaded in init
	{ "cl_use_pretty_particles", "1", 1, 0, 0, NULL },
		// determines the particle transform mode (screen plane or screen viewpoint)

	/*------ SERVER ------*/
	{ "sv_port", DEF_SERVER_PORT, 0, 0, 0, NULL },
		// specifies the port to load the server connection listener on
	{ "sv_message", "sv_message.txt", 0, 0, 0, NULL },
		// specifies the message sent to all clients when they connect
	{ "sv_max_clients", "32", 32, 0, 0, NULL },
		// the number of clients to allow to connect to the server in multiplayer games

	/*------ 2D Game Options ------*/
	{ "2d_use_questionmarks", "0", 0, 0, 0, NULL },
		// whether or not to use checkmarks on right button press
	{ "2d_width", "10", 10, 0, 0, NULL },
		// the grid width
	{ "2d_height", "10", 10, 0, 0, NULL },
		// the grid height
	{ "2d_game_type", "2", 2, 0, 0, NULL },
		// 2 for edge and corner, 1 for edges only
	{ "2d_mine_count", "25", 25, 0, 0, NULL },
		// the number of mines in the game

	/*------ 3D Game Options ------*/
	{ "3d_game_type", "1", 1, 0, 0, NULL },
		// 1 = N6, 2=N18, 3=N26
	{ "3d_height", "5", 5, 0, 0, NULL },
		// the y dimension
	{ "3d_width", "5", 5, 0, 0, NULL },
		// the x dimension
	{ "3d_depth", "5", 5, 0, 0, NULL },
		// the z dimension
	{ "3d_use_questionmarks", "0", 0, 0, 0, NULL },
		// use or not to use questionmarks on 2nd right click
	{ "3d_mine_count", "20", 20, 0, 0, NULL },
		// the number of mines
};
static int varListSize = sizeof(varList) / sizeof(variable_t);

// *********** FUNCTIONALITY ***********
/* ------------
var_Initialize
------------ */
int var_Initialize(void)
{
	con_Print( "Importing variable values from last session..." );

	// import last settings from file
	par_ExecFile( VAR_STATES_FILE, "" );

	return 1;
}

/* ------------
var_Terminate
------------ */
void var_Terminate(void)
{
	int a;
	char *full_path;
	file_t *settings;

	con_Print( "\nClosing Variable System..." );

	con_Print( "\tOpening \"%s\" for writting...", VAR_STATES_FILE );
	full_path = fs_GetBasePath( VAR_STATES_FILE, "" );
	settings = fs_open( full_path, "w" );
	if( settings == NULL ) {
		con_Print( "\tVariable System Termination Failed:  File \"%s\" failed to open for write.", full_path );
		return;
	}

	con_Print( "\tWritting variable states..." );
	fprintf( settings->file, "/* --------------------\nThis file contains the variable state at the time of the last successful program termination.  \nAll user changes and updates should be made in autoexec.ini\nDelete an item to reset it to its default state for the next run.\n-------------------- */\n\n" );

	for( a=0 ; a<varListSize ; a++ )
	{
		fprintf( settings->file, "set \"%s\" \"%s\"\n", varList[a].sName, varList[a].sValue );
	}

	fs_close( settings );

	con_Print( "\tAll variables successfully exported to file \"%s\"", full_path );

	return;
}

/* ------------
var_GetVarLatch
------------ */
varlatch var_GetVarLatch( char *varname )
{
	varlatch handle;

	// assert
	if( varname == NULL ) return NULL;

	//get the handle
	handle = var_FindVariable( varname );

	// test the handle
	if( handle == NULL )
	{
		con_Print( "<RED>Variable System Warning:  Get Variable Latch Failed for \"%s\".", varname );
		return NULL;
	}

	// increment the latches for our accounting
	((variable_t*)handle)->ilatches++;
	
	return handle;
}

/* ------------
var_GetFloat
------------- */
float var_GetFloat( varlatch handle )
{
	// assert
	if( handle == NULL )
	{
		con_Print( "<RED>Variable System Warning:  Attempted variable access on a NULL handle in GetFloat." );
		return 0.0f;
	}

	// do the work
	return ((variable_t*)handle)->fValue;
}

/* ------------
var_GetString
------------- */
char *var_GetString( varlatch handle )
{
	// assert 
	if( handle == NULL )
	{
		con_Print( "<RED>Variable System Warning:  Attempted variable access on a NULL handle in GetString." );
		return NULL;
	}

	// do the work
	return ((variable_t*)handle)->sValue;
}

/* ------------
var_GetFloatFromString
------------- */
float var_GetFloatFromName( char *var_name )
{
	// do a small assert and let the big functions handle the rest
	if( var_name == NULL ) return 0.0f;
	
	return var_GetFloat( var_GetVarLatch( var_name ) );
}

/* ------------
var_GetStringFromString
------------ */
char *var_GetStringFromName( char *var_name )
{
	// do a small assert and let the big functions handle the rest
	if( var_name == NULL ) return NULL;

	return var_GetString( var_GetVarLatch( var_name ) );
}

/* ------------
var_UpdateFloat
------------ */
void var_UpdateFloat( varlatch handle, float value )
{
	variable_t *var = (variable_t*)handle;

	if( handle == NULL )
	{
		con_Print( "Variable System Warning:  cannot update from string -- null handle" );
		return;
	}

	var->fValue = value;
	sprintf( var->sValue, "%f", var->fValue );
}

/* ------------
var_UpdateString
------------ */
void var_UpdateString( varlatch handle, char *value )
{
	int length;
	variable_t *var = (variable_t*)handle;

	if( handle == NULL ) 
	{
		con_Print( "Variable System Warning:  cannot update from string -- null handle" );
		return;
	}
	if( value == NULL )
	{
		con_Print( "Variable System Warning:  cannot update from string -- null string value" );
		return;
	}

	length = tcstrlen(value);
	if( length > VAR_MAX_STRING_SIZE )
		length = VAR_MAX_STRING_SIZE;
	
	memset( var->sValue, 0, VAR_MAX_STRING_SIZE );
	tcstrncpy( var->sValue, value, length );
	var->sValue[length] = '\0';

	var->fValue = tcatof( var->sValue );
}

/* ------------
var_SetValue -- extract the next 2 values from the buffer and tell the var system to update itself
------------ */
void var_SetValue( char *buffer )
{
	int to, len, cpyLen;
	char varName[ VAR_MAX_STRING_SIZE + 1];
	char varVal[ VAR_MAX_STRING_SIZE + 1];

	// get to first token
	if( !par_GetNextToken( buffer, &to, &len ) )
	{
		con_Print( "Set failed:  Could not find a variable name." );
		con_Print( "Usage:  set <variable> <value>" );
		return;
	}

	// advance and copy the token
	buffer += to;
	if( len > VAR_MAX_STRING_SIZE ) cpyLen = VAR_MAX_STRING_SIZE;
	else cpyLen = len;
	tcstrncpy( varName, buffer, cpyLen );
	varName[cpyLen] = '\0';
	buffer += len;

	// get the next token
	if( !par_GetNextToken( buffer, &to, &len ) )
	{
		con_Print( "Set failed:  Could not find a variable value." );
		con_Print( "Usage:  set <variable> <value>" );
		return;
	}

	// advance and copy the token
	buffer += to;
	if( len > VAR_MAX_STRING_SIZE ) cpyLen = VAR_MAX_STRING_SIZE;
	else cpyLen = len;
	tcstrncpy( varVal, buffer, cpyLen );
	varVal[cpyLen] = '\0';

	// update the value
	var_UpdateVar( varName, varVal );
}

/* ------------
var_SetValueSync
// updates the value as with a normal call to set value, then calls SyncFunction if non-null to 
// syncronize the state of the engine to the changed variables
------------ */
void var_SetValueSync( char *buffer )
{
	varlatch handle;
	int to, len, cpyLen;
	char varName[ VAR_MAX_STRING_SIZE + 1];
	char varVal[ VAR_MAX_STRING_SIZE + 1];

	// get to first token
	if( !par_GetNextToken( buffer, &to, &len ) )
	{
		con_Print( "Set failed:  Could not find a variable name." );
		con_Print( "Usage:  set <variable> <value>" );
		return;
	}

	// advance and copy the token
	buffer += to;
	if( len > VAR_MAX_STRING_SIZE ) cpyLen = VAR_MAX_STRING_SIZE;
	else cpyLen = len;
	tcstrncpy( varName, buffer, cpyLen );
	varName[cpyLen] = '\0';
	buffer += len;

	// get the next token
	if( !par_GetNextToken( buffer, &to, &len ) )
	{
		con_Print( "Set failed:  Could not find a variable value." );
		con_Print( "Usage:  set <variable> <value>" );
		return;
	}

	// advance and copy the token
	buffer += to;
	if( len > VAR_MAX_STRING_SIZE ) cpyLen = VAR_MAX_STRING_SIZE;
	else cpyLen = len;
	tcstrncpy( varVal, buffer, cpyLen );
	varVal[cpyLen] = '\0';

	// update the value
	var_UpdateVar( varName, varVal );

	// get the handle if it exists
	handle = var_FindVariable( varName );
	if( handle == NULL ) 
	{
		return;
	}

	// call the update function if it exists
	if( ((variable_t*)handle)->SyncFunction != NULL )
	{
		((variable_t*)handle)->SyncFunction();
	}
}

/* ------------
var_UpdateValue -- update the variable with the given name with the given value
------------ */
void var_UpdateVar( char *varname, char *varvalue )
{
	varlatch handle;
	variable_t *var;

	handle = var_FindVariable( varname );
	if( handle == NULL ) 
	{
		con_Print( "Variable System:  Unable to find a variable named: \"%s\"", varname );
		return;
	}

	var = (variable_t*)handle;
	if( var->iflags & VAR_FLAG_PROTECTED )
	{
		con_Print( "The variable \"%s\" is protected by the system and cannot be changed", var->sName );
		return;
	}

	var_UpdateString( handle, varvalue );
}

/* ------------
var_FindVariable
------------ */
static varlatch var_FindVariable( char *varname )
{
	int a;

	for( a=0 ; a<varListSize ; a++ )
	{
		if( tcstrncmp( varname, varList[a].sName, tcstrlen(varList[a].sName) ) )
		{
			return (varlatch)&varList[a];
		}
	}

	con_Print( "Variable System Warning:  Find Variable failed on search for \"%s\"", varname );
	return NULL;
}

/* ------------
var_ValueOf - public interface that allows the user to pass the name of a variable and get back the current value of the variable
------------ */
void var_ValueOf( char *varName )
{
	varlatch vlTmp;

	// this is a distinct possiblity
	if( varName == NULL ) return;

	// advance to the token
	while( *varName && par_IsWhiteSpace(*varName) ) varName++;

	// check for the end of string
	if( !*varName ) return;

	// find the variable
	vlTmp = var_FindVariable( varName );
	if( vlTmp == NULL ) return; // error already printed

	// print the value of the variable
	con_Print( "Value:  %s", var_GetString( vlTmp ) );
}
