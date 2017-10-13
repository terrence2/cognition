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
