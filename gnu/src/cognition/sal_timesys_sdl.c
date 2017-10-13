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
// timesys.c
// Created 2-25-02 @ 0053 by Terrence Cole

// Includes
/////////////
#include "cog_global.h"
#include "SDL.h"

// Global Prototypes
//////////////////////
/*
int ts_Initialize(void);
void ts_Terminate(void);
unsigned long int ts_GetTime(void);
double ts_GetSaneTime(void);
*/

// Local Variables
////////////////////

// *********** FUNCTIONALITY ***********
/* ------------
ts_InitializeTime
------------ */
int ts_Initialize(void)
{
    // inited with sdl
	con_Print( "Timer System Initialized." );
	con_Print( "\tStarting tics: %g", SDL_GetTicks() );
	con_Print( "\tTics per Second: %f", 1000.0f );
	con_Print( "\tTics per Millisecond: %f", 1.0f );

	return 1;
}

/* ------------
ts_TerminateTime
------------ */
void ts_Terminate(void)
{
}

/* ------------
ts_GetTime - returns the time ellapsed in milliseconds since the time system was started
------------ */
unsigned long int ts_GetTime(void)
{
    return SDL_GetTicks();
}

/* ------------
ts_GetSaneTime
------------ */
double ts_GetSaneTime(void) 
{
	return ((double)ts_GetTime() / 1000.0);
}
