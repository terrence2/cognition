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

#ifdef WIN32

// Includes
/////////////
#include "cog_global.h"

// Global Prototypes
//////////////////////
char** dir_scan( const char* path, const char* filter, int* count );
void dir_scan_free( char** namelist, int count );
char* dir_get_current(void);

/* ------------
dir_scan
------------ */
char** dir_scan( const char* path, const char* filter, int* count )
{
	HANDLE hfile;
	WIN32_FIND_DATA fd;
	DWORD error;
	char *path;
	int ret;

	memset( &fd, 0, sizeof(WIN32_FIND_DATA) );

	path = fs_GetBasePath( name, dir );

	hfile = FindFirstFile( path, &fd );
	if( (hfile == INVALID_HANDLE_VALUE) || (hfile == NULL) ) {
		con_Print( "File System Warning:  File Search Open Failed" );
		return 0;
	}

	while( number <= MAX_ORDERED_NAMES )
	{
		number++;

		ret = FindNextFile( hfile, &fd );
		if( !ret )
		{
			error = GetLastError();
			if( error == ERROR_NO_MORE_FILES )
			{
				FindClose( hfile );
				return number;
			}
			else
			{
				con_Print( "File System Warning:  Find File Returned Unknown Error" );
				FindClose( hfile );
				return 0;
			}
		}
	}

	// too many screenshots in current directory
	con_Print( "File System Warning:  Find next ordered file gave up after %d repetitions", MAX_ORDERED_NAMES );
	FindClose( hfile );

	return number;
}

#endif // WIN32
