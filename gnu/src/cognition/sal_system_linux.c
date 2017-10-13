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
// system.c
// Created 2-25-02 @ 1023 by Terrence Cole

// Includes
/////////////
#include "cog_global.h"
#include "sal_input_sdl.h"
#include "SDL.h"
#include <unistd.h>
#include <sys/utsname.h>

// Definitions
////////////////

// Structures
///////////////

// Global Prototypes
//////////////////////
/*
void sys_ProcessSystemMessages();
void sys_PrintSystemInformation();
void sys_Exit( int errorcode );
*/

// Local Variables
////////////////////

// *********** FUNCTIONALITY ***********

/* ------------
sys_CheckMessages
------------ */
void sys_ProcessSystemMessages()
{
	SDL_Event event;
	SDL_KeyboardEvent* kb_event;
	SDL_MouseButtonEvent* mb_event;
	SDL_MouseMotionEvent* mm_event;

	// always reset the delta, so that if we don't 
	// get a mouse move event, we won't warp the pointer
	state.mAxisDelta[YAW] = 0;
	state.mAxisDelta[PITCH] = 0;
	
	while( SDL_PollEvent(&event) ) {
		switch( event.type ) {
		case SDL_KEYDOWN:
			kb_event = (SDL_KeyboardEvent*)&event;
			in_sdl_Event( in_sdl_MapKey( (int)(kb_event->keysym.sym) ), 1 );
			break;
		case SDL_KEYUP:
			kb_event = (SDL_KeyboardEvent*)&event;
			in_sdl_Event( in_sdl_MapKey( (int)(kb_event->keysym.sym) ), 0 );
			break;
		case SDL_MOUSEBUTTONDOWN:
			mb_event = (SDL_MouseButtonEvent*)&event;
			in_sdl_Event( in_sdl_MapButton( mb_event->button ), 1 );
			break;
		case SDL_MOUSEBUTTONUP:
			mb_event = (SDL_MouseButtonEvent*)&event;
			in_sdl_Event( in_sdl_MapButton( mb_event->button ), 0 );
			break;
		case SDL_MOUSEMOTION:
			mm_event = (SDL_MouseMotionEvent*)&event;
			state.mAxisPrev[YAW] = state.mAxis[YAW];
			state.mAxisPrev[PITCH] = state.mAxis[PITCH];
			state.mAxisDelta[YAW] = -mm_event->xrel;
			state.mAxisDelta[PITCH] = -mm_event->yrel;
			state.mAxis[YAW] -= mm_event->xrel;
			state.mAxis[PITCH] -= mm_event->yrel;
			break;
		case SDL_QUIT:
			eng_Stop( "32" );
			break;
		default: break;
		}
	}
}


/* ------------
entry point
------------ */
int main( int argc, char** argv )
{
	int a;
	int len;
	char* cmdline = 0;

	// init sdl
	if( SDL_Init(SDL_INIT_TIMER|SDL_INIT_VIDEO) ) {
		printf( "Cognition Error: SDL Init Failed." );
		return 1;
	}

	// build a command line that is like a command file 
	len = 0;
	for( a = 0 ; a < argc ; a++ ) {
		len += tcstrlen(argv[a]) + 1;
	}
	len++;
	cmdline = (char*)malloc( sizeof(char) * len );
	memset( cmdline, 0, sizeof(char) * len );
	for( a = 0 ; a < argc ; a++ ) {
		tcstrcat(cmdline,argv[a]);
		tcstrcat(cmdline," ");
	}
	sys_handles.cmdLine = cmdline;

	// do the game
	eng_Start();

	return 0;
}

/* ------------
sys_Exit
------------ */
void sys_Exit( int errorcode )
{
	if( errorcode != 0 )
	{
		fprintf( stderr, "An error occured during execution:  Exiting.\nPlease check runlog.txt for details." );
	}

	SDL_Quit();
	exit(errorcode);
}

/* ------------
sys_ReportSystemInformation
------------ */
void sys_PrintSystemInformation()
{
    struct utsname buf;
    char hostbuf[256];
    int ret;
    
    // get system info
    ret = uname( &buf );
    if( ret ) {
        con_Print( "\tFAILED: querying system info with uname" );
        return;
    }
    con_Print( "\tOperating System: %s", buf.sysname );
	con_Print( "\tVersion: %s", buf.version );
	con_Print( "\tRelease: %s", buf.release );
    con_Print( "\tMachine: %s", buf.machine );

    // get more system info
    ret = gethostname( hostbuf, 256 );
    if( ret ) {
        con_Print( "\tFAILED: querying host name with gethostname" );
        return;
    }
    con_Print( "\tHostname: %s\n", hostbuf );
}
