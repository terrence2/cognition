// Cognition
// timesys.c
// Created 2-25-02 @ 0053 by Terrence Cole

// Includes
/////////////
#include "cog_global.h"

// Defines
////////////
#define MAX_TIMERS 32

// Global Prototypes
//////////////////////
/*
int t_Initialize(void);
void t_Terminate(void);
unsigned long int t_GetTime(void);
double t_GetSaneTime(void);
cog_timer_t t_SetTimer( void (*funct)( char *buffer ), char *command, int milli, int repeating );
void t_KillTimer( cog_timer_t timer );
void t_CheckTimers(void);
*/

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
static LONGLONG t_tic_start = 0;
static double t_tics_per_sec = 0.0;
static double t_tics_per_milsec = 0.0;
static unsigned long t_prev_time = 0;

static timer_struct_t timers[MAX_TIMERS];

// *********** FUNCTIONALITY ***********
/* ------------
t_InitializeTime
------------ */
int t_Initialize(void)
{
	int a;
	LARGE_INTEGER tic_freq;
	LARGE_INTEGER tic_count;

	// initialize the timers
	for( a = 0 ; a < MAX_TIMERS ; a++ )
	{
		timers[a].active = 0;
		timers[a].function = NULL;
		timers[a].command = NULL;
		timers[a].repeat = 0;
		timers[a].timeout = 0;
		timers[a].delay = 0;
	}

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
t_TerminateTime
------------ */
void t_Terminate(void)
{
}

/* ------------
t_GetTime - returns the time ellapsed in milliseconds since the time system was started
------------ */
unsigned long int t_GetTime(void)
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
t_GetSaneTime
------------ */
double t_GetSaneTime(void) 
{
	return ((double)t_GetTime() / 1000.0);
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
	timers[timer].timeout = t_GetTime() + milli;
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

