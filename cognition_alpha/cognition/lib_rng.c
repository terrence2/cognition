// Cognition
// rng.c
// Created 12-03-02 @ 0915 by Terrence Cole

// Includes
/////////////
#include "cog_global.h"
#include <time.h>

// Definitions
////////////////
#define SEED_ITERATIONS 10000

// Global Prototypes
//////////////////////
/*
int rng_Initialize();
void rng_Terminate();
double rng_Double( double lo, double hi );
float rng_Float( float lo, float hi );
long rng_Long( long lo, long hi );
short rng_Short( short lo, short hi );
byte rng_Byte( byte lo, byte hi );
*/

// Local Prototypes
/////////////////////

// Header Variables
/////////////////////

// *********** FUNCTIONALITY ***********
/* ------------
rng_Initialize - seeds rng and iterates the usual flaws at the beginning of the sequence
------------ */
int rng_Initialize()
{
	int a;
	double tstart, tend;

	con_Print( "\n<BLUE>Seeding</BLUE> Random Number Generator..." );
	eng_LoadingFrame();

	tstart = t_GetSaneTime();
	srand( time(NULL) );
	for( a = 0 ; a < SEED_ITERATIONS ; a++ )
		rand();
	tend = t_GetSaneTime();

	con_Print( "\tDone:  %d iterations in %.3f sec.", SEED_ITERATIONS, tend - tstart );
	eng_LoadingFrame();

	return 1;
}

/* ------------
rng_Terminate
------------ */
void rng_Terminate()
{
	// this is a stub
}

/* ------------
rng_Double - returns a double precision random number [lo,hi)
------------ */
double rng_Double( double lo, double hi )
{
	double delta = hi - lo;
	double off = (double)rand() / (double)RAND_MAX;
	return lo + (delta * off);
}

/* ------------
rng_Float - returns a single precision random number [lo,hi)
------------ */
float rng_Float( float lo, float hi )
{
	float delta = hi - lo;
	double off = (double)rand() / (double)RAND_MAX;
	return lo + (float)((double)delta * off);
}

/* ------------
rng_Long - returns a long integer random number [lo,hi)
------------ */
long rng_Long( long lo, long hi )
{
	long delta = hi - lo;
	double off = (double)rand() / (double)RAND_MAX;
	return lo + (long)((double)delta * off);
}

/* ------------
rng_Short - returns a short integer random number [lo,hi)
------------ */
short rng_Short( short lo, short hi )
{
	short delta = (short)(hi - lo);
	double off = (double)rand() / (double)RAND_MAX;
	return (short)(lo + (short)((double)delta * off));
}

/* ------------
rng_Byte - returns a byte length integer random number [lo,hi)
------------ */
byte rng_Byte( byte lo, byte hi )
{
	byte delta = (byte)(hi - lo);
	double off = (double)rand() / (double)RAND_MAX;
	return (byte)(lo + (byte)((double)delta * off));
}
