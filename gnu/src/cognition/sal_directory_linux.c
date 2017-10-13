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

// Includes
/////////////
#include "cog_global.h"
#include <fnmatch.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <unistd.h>

// Global Prototypes
//////////////////////
/*
char** dir_scan( const char* path, const char* filter, int* count );
void dir_scan_free( char** namelist, int count );
char* dir_get_current(void);
*/

// Local Data
//////////////////////
static char* dir_match = 0;
static int dir_filter( const struct dirent *ent )
{
    return !fnmatch( dir_match, ent->d_name, 0 );
}

/* ------------
dir_scan
------------ */
char** dir_scan( const char* path, const char* filter, int* count )
{
    struct dirent** dirlist;
    char** namelist;
    int a;
    
    // get a dirent list
    dir_match = (char*)filter;
    *count = scandir( path, &dirlist, dir_filter, alphasort );
    if( *count == -1 ) return 0;
    
    // allocate a string list
    namelist = (char**)malloc( sizeof(char*) * *count );
    
    // copy the list
    for( a = 0 ; a < *count ; a++ ) {
        namelist[a] = tcDeepStrCpy( dirlist[a]->d_name );
        free( dirlist[a] );
    }
    
    // free the dirents
    free( dirlist );
    
    return namelist;
}

/* ------------
dir_scan_free
------------ */
void dir_scan_free( char** namelist, int count )
{
    int a;
    
    // check for empty list
    if( !namelist ) return;
    
    // scan, freeing
    for( a = 0 ; a < count ; a++ ) {
        free( namelist[a] );
    }
    free(namelist);
}

/* ------------
dir_get_current
------------ */
char* dir_get_current(void)
{
    return getcwd(0,0);
}
