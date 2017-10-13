// Cognition
// filelog.c
// Created 2-25-02 @ 0051 by Terrence Cole

// Includes
/////////////
#include "cog_global.h"
#include <time.h> // for time and ctime

// Definitions
////////////////
#define FILE_LOG "runlog.txt"

// Global Prototypes
//////////////////////
/*
int f_Initialize(void);
void f_Terminate(void);
void f_LogVA( char *fmt, ... );
void f_Log( char *str );
*/

// Local Variables
////////////////////
static FILE *runlog = NULL;

// *********** FUNCTIONALITY ***********
/* ------------
f_InitializeLog
------------ */
int f_Initialize(void)
{
	time_t timer;
	char *strTime;
	char *path;

	path = fs_GetBasePath( FILE_LOG, "" );
	runlog = fopen( path, "w" );
	if( runlog == NULL ) {
		return 0;
	}

	// update log with open time
	time( &timer );
	strTime = ctime( &timer ); // statically allocated by system

	f_Log( "---------------------------------" );
	f_LogVA( "File Log Opened: %s", strTime );

	return 1;
}

/* ------------
f_TerminateLog
------------ */
void f_Terminate(void)
{	
	time_t timer;
	char *strTime;

	// update log with close time
	time( &timer );
	strTime = ctime( &timer );

	f_Log( " " );
	f_LogVA( "File Log Closed: %s", strTime );
	f_Log( "---------------------------------" );

	if( runlog != NULL ) fclose( runlog );
	runlog = NULL;
}

/* ------------
f_Log
------------ */
void f_Log( char *str )
{
	if( (runlog == NULL) || (str == NULL) )
		return;

	fprintf( runlog, "%s\r\n", str );
	fflush( runlog );
}

/* ------------
f_LogVA
------------ */
void f_LogVA( char *fmt, ... )
{
	va_list ap;
	char buffer[4096];

	if( (runlog == NULL) || (fmt == NULL) )
		return;

	if( !tcstrlen(fmt) )
		return; //. catch console line feeds

	va_start( ap, fmt );
	vsprintf( buffer, fmt, ap );
	va_end( ap );

	fprintf( runlog, "%s\r\n", buffer );
	fflush( runlog );
}
