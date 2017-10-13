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
static LONGLONG t_tic_start = 0;
static double t_tics_per_sec = 0.0;
static double t_tics_per_milsec = 0.0;
static unsigned long t_prev_time = 0;

// *********** FUNCTIONALITY ***********
/* ------------
ts_InitializeTime
------------ */
int ts_Initialize(void)
{
	int a;
	LARGE_INTEGER tic_freq;
	LARGE_INTEGER tic_count;

	if( !QueryPerformanceFrequency( &tic_freq ) ) 
	{
		con_Print( "Initialize Timer Error:  No Timer Available!" );
		return 0;
	}

	t_tics_per_sec = (double)tic_freq.QuadPart;
	t_tics_per_milsec = t_tics_per_sec / 1000.0;

	QueryPerformanceCounter( &tic_count );
	t_tic_start = tic_count.QuadPart;

	con_Print( "Timer System Initialized." );
	con_Print( "\tStarting tics: %g", t_tic_start );
	con_Print( "\tTics per Second: %f", t_tics_per_sec );
	con_Print( "\tTics per Millisecond: %f", t_tics_per_milsec );

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
	LARGE_INTEGER tics;
	unsigned long time;

	// assert
	if( t_tic_start == 0 ) return 0; // if we haven't inited this is meaningless

	QueryPerformanceCounter( &tics );

	time = (int)(( (double)(tics.QuadPart - t_tic_start) ) / t_tics_per_milsec );

	// keep the time from stalling if we are framing faster than the clock (lol)
	if( time <= t_prev_time )
		time += 1;

	t_prev_time = time;

	// calc delta tics
	return time;
}

/* ------------
ts_GetSaneTime
------------ */
double ts_GetSaneTime(void) 
{
	return ((double)ts_GetTime() / 1000.0);
}
