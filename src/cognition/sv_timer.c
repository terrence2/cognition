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
int t_Initialize(void);
void t_Terminate(void);
cog_timer_t t_SetTimer( void (*funct)( char *buffer ), char *command, int milli, int repeating );
void t_KillTimer( cog_timer_t timer );
void t_CheckTimers(void);
*/

// Defines
////////////
#define MAX_TIMERS 32

// Local Structs
//////////////////
// typedef int cog_timer_t;
typedef struct timer_struct_s
{
	byte active;
	void (*function)( char *buffer );
	char *command;
	unsigned long int timeout;
	int delay;
	int repeat;
} timer_struct_t;

// Local Variables
////////////////////
static timer_struct_t timers[MAX_TIMERS];

/* ------------
t_Initialize 
------------ */
int t_Initialize(void)
{
    memset( timers, 0, sizeof(timer_struct_t) * MAX_TIMERS );
    return 1;
}

/* ------------
t_Terminate
------------ */
void t_Terminate(void)
{
    memset( timers, 0, sizeof(timer_struct_t) * MAX_TIMERS );
}

/* ------------
t_SetTimer - returns a timer on success or NULL_TIMER on fail
------------ */
cog_timer_t t_SetTimer( void (*funct)( char *buffer ), char *command, int milli, int repeating )
{
	int a;
	int timer;

	// assert
	if( funct == NULL ) return NULL_TIMER;
	if( milli < 0 ) return NULL_TIMER;

	// find a timer
	a = 0;
	timer = -1;
	while( (a < MAX_TIMERS) && (timer == -1) )
	{
		if( !timers[a].active )
		{
			timer = a;
		}
	}

	// assert
	if( timer < 0 ) return NULL_TIMER;

	// set the timer
	timers[timer].active = 1;
	timers[timer].function = funct;
	timers[timer].repeat = repeating;
	timers[timer].timeout = ts_GetTime() + milli;
	timers[timer].delay = milli;
	if( command != NULL )
	{
		timers[timer].command = tcDeepStrCpy(command);
	}

	return timer;
}

/* ------------
t_KillTimer - disactivates the timer and clears its data and frees all associated memory
------------ */
void t_KillTimer( cog_timer_t timer )
{
	// assert
	if( (timer < 0) || (timer >= MAX_TIMERS) ) return;

	timers[timer].active = 0;
	timers[timer].function = NULL;
	timers[timer].timeout = 0;
	timers[timer].delay = 0;
	timers[timer].repeat = 0;
	SAFE_RELEASE( timers[timer].command );
}

/* ------------
t_CheckTimers - called to see if any timer has timed out
			  - if a timer has timed out it calls the function with command and then kills or resets it based on repeat
------------ */
void t_CheckTimers(void)
{
	int a;
	
	for( a = 0 ; a < MAX_TIMERS ; a++ )
	{
		if( (timers[a].active) && (timers[a].timeout < state.frame_time) )
		{
			timers[a].function( timers[a].command );
			if( timers[a].repeat )
			{
				timers[a].timeout += timers[a].delay;
			}
			else
			{
				t_KillTimer(a);
			}
		}
		
	}
}
