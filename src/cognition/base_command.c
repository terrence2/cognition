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
// command.c
// by Terrence Cole 12/12/01

// Includes
/////////////
#include "cog_global.h"

// Definitions
////////////////
#define MAX_COMMANDS 127
#define MAX_ALIASES 127
#define MAX_COMMAND_SIZE 127 

// Structures
///////////////
typedef struct comCommandPair_s
{
	char *command;
	void (*function)( char *buffer );
	int index;
} comCommandPair_t;

typedef struct comCommand_s
{
	byte inUse;
	char *command;
	void (*function)( char *buffer );
} comCommand_t;

typedef struct comAlias_s
{
	byte inUse;
	char *name;
	char *commands;
} comAlias_t;

// Global Prototypes
//////////////////////
/*
int com_Initialize(void);
void com_Terminate(void);
void com_Process( char *buffer );
int com_VerifyCommand( char *command_name );
command_t com_AddCommand( char *name, void (*function)( char *buffer ) );
void com_RemoveCommand( command_t command );
*/

// Local Prototypes
/////////////////////
static void com_Alias( char *buffer );

// Local Variables
////////////////////
static comCommandPair_t comStaticCommands[] = {
	{ "set", var_SetValueSync, 0 },
	{ "bind", bind_BindKey, 0 },
	{ "exit", eng_Stop, 0 },
	{ "quit", eng_Stop, 0 },
	{ "con_toggle", con_Toggle, 0 },
	{ "screenshot", bmp_ScreenShot, 0 },
	{ "mem_info", mem_PrintInfo, 0 },
	{ "colormap_info", cm_PrintInfo, 0 },
	{ "shader_info", sh_PrintInfo, 0 },
	{ "entity_info", ent_PrintInfo, 0 },
	{ "light_info", l_PrintInfo, 0 },
	{ "alias", com_Alias, 0 },
	{ "game", game_Start, 0 },
	{ "end", game_End, 0 },
	{ "server_start", game_StartServer, 0 },
	{ "server_end", game_EndServer, 0 },
	{ "connect", game_Connect, 0 },
	{ "disconnect", game_Disconnect, 0 },
	{ "value", var_ValueOf, 0 },
	{ "world_color", wrld_SetColor, 0 },
};
static int numStaticCommands = sizeof(comStaticCommands) / sizeof(comCommandPair_t);

static comAlias_t aliases[MAX_ALIASES];
static int num_aliases = 0;
static comCommand_t commands[MAX_COMMANDS];
static int num_commands = 0;

// *********** FUNCTIONALITY ***********
/* ------------
com_Initialize - reset the command list and prepare the list with static engine commands
------------ */
int com_Initialize(void)
{
	int a;

	con_Print( "\nInitializing the Command System..." );

	// clear an currently allocated memory
	com_Terminate();

	// add all static commands
	for( a = 0 ; a < numStaticCommands ; a++ )
	{
		comStaticCommands[a].index = com_AddCommand( comStaticCommands[a].command, comStaticCommands[a].function );
	}

	return 1;
}

/* ------------
com_Terminate - 
------------ */
void com_Terminate(void)
{
	int a;

	if( (num_aliases > 0) || (num_commands > 0) ) 
		con_Print( "\nTerminating the Command System..." );

	// zero the alias list
	for( a = 0 ; a < MAX_ALIASES ; a++ )
	{
		aliases[a].inUse = 0;
		SAFE_RELEASE( aliases[a].name );
		aliases[a].name = NULL;
		SAFE_RELEASE( aliases[a].commands );
		aliases[a].commands = NULL;
	}
	num_aliases = 0;

	// zero the command list
	for( a = 0 ; a < MAX_COMMANDS ; a++ )
	{
		commands[a].inUse = 0;
		commands[a].function = NULL;
		SAFE_RELEASE( commands[a].command );
		commands[a].command = NULL;
	}
	num_commands = 0;
}

/* ------------
com_Process
------------ */
void com_Process( char *buffer )
{
	comCommand_t *cmd;
	char *start, *newBuffer;
	int newBufferLen;
	int a;
	char token[MAX_COMMAND_SIZE+1];

	// assert
	if( buffer == NULL ) return;
	if( !*buffer ) return;

	// skip starting whitespace
	while( (*buffer) && (par_IsWhiteSpace(*buffer)) ) buffer++;

	// read 1 token
	a = 0;
	while( (*buffer) && (*buffer != ' ') && (*buffer != '\t') && (*buffer != '\n') && (*buffer != '\r') && 
			(*buffer != ';') && (a < MAX_COMMAND_SIZE) )
	{
		token[a] = *buffer;
		a++;
		buffer++;
	}
	token[a] = '\0';

	// check for a call to "wait"
	if( tcstricmp( "wait", token ) )
	{
		// read to the next token
		while( (*buffer) && (par_IsWhiteSpace(*buffer)) ) buffer++;

		// read the time
		a = 0;
		while( (*buffer) && (*buffer != ' ') && (*buffer != '\t') && (*buffer != '\n') && (*buffer != '\r') && 
				(*buffer != ';') && (a < MAX_COMMAND_SIZE) )
		{
			token[a] = *buffer;
			a++;
			buffer++;
		}
		token[a] = '\0';

		// verify that we actually have something to wait on
		while( (*buffer) && (*buffer != ';') ) buffer++;
		if( !*buffer ) return;
		buffer++;
	
		// start a timer and exit the function
		t_SetTimer( com_Process, buffer, tcatoi(token), 0 );

		return;
	}

	// search for the command
	a = 0;
	cmd = NULL;
	while( (a < MAX_COMMANDS) && (cmd == NULL) )
	{
		if( (commands[a].command != NULL) && (tcstrcmp( token, commands[a].command )) )
		{
			cmd = &commands[a];
		}
		a++;
	}

	// break with an error at this point if we don't find a valid command
	if( cmd == NULL )
	{
		// search for an alias
		for( a = 0 ; a < num_aliases ; a++ )
		{
			if( tcstrcmp( aliases[a].name, token ) )
			{
				com_Process( aliases[a].commands );
				return;
			}
		}

		con_Print( "The string \"%s\" is not a recognized command or alias.", token );
		return;
	}

	// grab the rest of this command (to the ; or the end of the string) to pass to the command function
	start = buffer;
	while( (*buffer) && (*buffer != ';') )
	{
		buffer++;
	}

	// get a new buffer with this segment of the command
	if( (buffer - start) > 0 )
	{
		newBufferLen = buffer - start + 1;
		newBuffer = mem_alloc( sizeof(char) * (newBufferLen + 1) );
		for( a = 0 ; a < newBufferLen ; a++ )
		{
			newBuffer[a] = *start;
			start++;
		}
		newBuffer[a] = '\0';
	}
	else
	{
		newBufferLen = 0;
		newBuffer = NULL;
	}
	
	// execute this command first
	cmd->function( newBuffer );
	SAFE_RELEASE( newBuffer );

	// if this was a starting or middle chunk of a list of commands then execute the rest of the commands, else exit
	if( *buffer == ';' )
	{
		// process this command
		com_Process( ++buffer );
	}

	return;
}

/* ------------
com_VerifyCommand
------------ */
int com_VerifyCommand( char *command_name )
{
	int a;

	for( a=0 ; a<MAX_COMMANDS ; a++ )
	{
		if( tcstrcmp( commands[a].command, command_name ) )
		{
			return 1;
		}
	}

	return 0;
}

/* ------------
com_AddCommand - adds a command to the command list
------------ */
command_t com_AddCommand( char *name, void (*function)( char *buffer ) )
{
	comCommand_t *cmd;
	int a;

	a = 0;
	cmd = NULL;
	while( (a < MAX_COMMANDS) && (cmd == NULL) )
	{
		if( !commands[a].inUse )
		{
			commands[a].inUse = 1;
			commands[a].command = tcDeepStrCpy(name);
			commands[a].function = function;

			num_commands++;
			return a;
		}
		a++;
	}

	return NULL_COMMAND;
}

/* ------------
com_RemoveCommand
------------ */
void com_RemoveCommand( command_t command )
{
	// assert
	if( (command >= MAX_COMMANDS) || (command < 0) ) return;
	if( !commands[command].inUse ) return;

	commands[command].inUse = 0;
	commands[command].function = NULL;
	SAFE_RELEASE( commands[command].command );
	commands[command].command = NULL;

	num_commands--;
}

/* ------------
com_Alias - add an alias to the list
------------ */
static void com_Alias( char *buffer )
{
	comAlias_t *alias;
	int a;
	char *start;
	int to, len;
	int cpyLen;
	
	// find a blank alias in the list
	a = 0;
	alias = NULL;
	while( (a < MAX_ALIASES) && (alias != NULL) )
	{
		if( !aliases[a].inUse )
		{
			alias = &aliases[a];
		}
		a++;
	}

	if( alias == NULL )
	{
		con_Print( "Alias:  No unused alias positions available." );
		return;
	}

	// read the name from the buffer
	if( !par_GetNextToken( buffer, &to, &len ) )
	{
		con_Print( "Alias:  Could not find a name." );
		return;
	}

	// copy over the name
	buffer += to;
	alias->name = (char*)mem_alloc( sizeof(char) * (len + 1) );
	if( alias->name == NULL )
	{
		con_Print( "Alias:  memory allocation failed." );
		return;
	}
	memset( alias->name, 0, sizeof(char) * (len + 1) );
	tcstrncpy( alias->name, buffer, len );
	alias->name[len+1] = '\0';

	// read to the open quotes
	buffer += len;
	while( (*buffer) && (*buffer != '\"') ) { buffer++; }
	buffer++;
	start = buffer;

	// count to the close quotes
	while( (*buffer) && (*buffer != '\"') ) { buffer++; }
	if( *buffer != '\"' ) 
	{
		con_Print( "Alias:  Could not find a closing quote." );
		return;
	}

	// copy the command
	cpyLen = buffer - start;
	alias->commands = (char*)mem_alloc( sizeof(char) * (len) );  // don't copy the close quote
	if( alias->commands == NULL )
	{
		con_Print( "Alias:  memory allocation failed." );
		return;
	}
	memset( alias->commands, 0, sizeof(char) * (len) );
	tcstrncpy( alias->commands, buffer, len - 1 );
	alias->commands[len] = '\0';

	// do the accounting
	alias->inUse = 1;
	num_aliases++;
}
