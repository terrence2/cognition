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
// filesys.c
// Created 2-25-02 @ 0045 by Terrence Cole

// requires 'base' for init

// Includes
/////////////
#include "../shared/linked_list.h"
#include "cog_global.h"
#include <fcntl.h> // for _O_BINARY default mode setting

// Definitions
////////////////
#define VOL_NAME_SIZE 12
#define FS_NAME_SIZE 12
#define TEMP_PATH_SCALAR 6
#define MAX_ORDERED_NAMES 1024
#define DEFAULT_MEDIA_DIRECTORY "main"
#ifdef WIN32
#	define SYSTEM_PATH_SEPARATOR "\\"
#	define SYSTEM_PATH_SEPARATOR_C '\\'
#else
#	define SYSTEM_PATH_SEPARATOR "/"
#	define SYSTEM_PATH_SEPARATOR_C '/'
#endif // WIN32
#define PATH_SEPARATOR "/"
#define PATH_SEPARATOR_C '/'
#define ZIP_HEADER_SIGNATURE 0x04034b50

// Local Structures
//////////////////
typedef struct file_system_info_s
{
    char *working_dir;
    char *media_dir;
    char *vol_name;
    char *fs_name;
    char *tmp_full_path;
    int tmp_full_path_length;
    LinkedList llPackedFiles;

    uint32_t vol_serialnumber;
    uint32_t fs_maxcomponentlength;
    uint32_t fs_flags;
} file_system_info_t;

typedef struct
{
	int signature;
	short version_needed;
	short bit_flags;
	short compression;
	short last_mod_file_time;
	short last_mod_file_date;
	int crc_32;
	int compressed_size;
	int uncompressed_size;
	short file_name_length;
	short extra_field_length;

	char *file_name;
	byte *extra_field;
} zip_file_header_t;

// Global Headers
/////////////////
/*
typedef struct
{
	FILE *file;
	char *signature;
	unsigned long int startOffset;
	unsigned long int endOffset;
	unsigned long int offset;
	byte inPack;
} file_t;
*/

// Global Prototypes
//////////////////////
/*
// systems
int fs_Initialize(void);
void fs_Terminate(void);
int fs_PrintFileSystemInformation();

// path structure control
char *fs_GetBaseDirectory();
char *fs_GetMediaDirectory();
char *fs_SetMediaDirectory( const char *name );
char *fs_GetBasePath( const char *filename, const char *subPath );
char *fs_GetMediaPath( const char *filename, const char *subPath );
unsigned long fs_GetFileLength( file_t *file );

// file usage stuff
file_t *fs_open( const char *filename, const char *mode );
int fs_close( file_t *stream );
size_t fs_read( void *buffer, size_t size, size_t count, file_t *stream );
size_t fs_write( const void *buffer, size_t size, size_t count, file_t *stream );
int fs_seek( file_t *stream, long offset, int origin );
long fs_tell( file_t *stream );
int fs_flush( file_t *stream );
void fs_rewind( file_t *stream );
int fs_error( file_t *stream );
int fs_eof( file_t *stream);
int fs_RemoveDirectory( char *rel_dir );
int fs_CreateDirectory( char *rel_dir );

// misc helpers
int fs_GetNextOrderedName( char *name, char *dir );
byte fs_FileExists( char *name );
*/


// Local Prototypes
/////////////////////
static file_system_info_t *fs_GetFileSysInfo();
static int fs_IterateAllPackedFiles();
static void fs_ToSystemPath( char *path );
static void fs_DestroyPackedFileList( LinkedList list );
static int fs_uz_GetHeader( zip_file_header_t *header, FILE *fp );
static int fs_ReadLong( FILE *fp );
static short fs_ReadShort( FILE *fp );
static byte fs_ReadByte( FILE *fp );
unsigned long fs_GetFileLengthFILE( FILE *file );

// Local Variables
////////////////////
static file_system_info_t *fs_info = NULL;

// *********** FUNCTIONALITY ***********
/* ------------
sys_InitFileSystem
------------ */
int fs_Initialize(void)
{
	// set the default file translation mode to what it should be anyway
#ifdef WIN32 
	_fmode = _O_BINARY; // stupid windows
#endif 
    
	// assert
	if( fs_info != NULL ) fs_Terminate();

	// get some information about what we're running from
	fs_info = fs_GetFileSysInfo();
	fs_info->llPackedFiles = LINKEDLIST_NOT_A_LIST;
	fs_SetMediaDirectory( DEFAULT_MEDIA_DIRECTORY );

	return 1;
}

/* ------------
fs_TerminateFileSystem
------------ */
void fs_Terminate(void)
{
	con_Print( "\nTerminating File System..." );
	if( fs_info != NULL )
	{
		con_Print( "\tReleasing file system information..." );

		// destroy path stuff
		SAFE_RELEASE( fs_info->fs_name );
		SAFE_RELEASE( fs_info->vol_name );
		SAFE_RELEASE( fs_info->working_dir );
		SAFE_RELEASE( fs_info->tmp_full_path );
		SAFE_RELEASE( fs_info->media_dir );

		// destroy the list of packed files
		fs_DestroyPackedFileList( fs_info->llPackedFiles );

		// destroy the base info
		SAFE_RELEASE( fs_info );
	}

	con_Print( "\tFile System Terminated Successfully." );
}

/* ------------
fs_PrintFileSystemInformation
------------ */
int fs_PrintFileSystemInformation()
{
	byte compressed = 0;

	// assert
	if( fs_info == NULL )
	{
		con_Print( "File System: PrintFileSystemInformation: Assert Failed: fs_info is NULL" );
		eng_Stop( "120001" );
		return 0;
	}

	con_Print( "<BLUE>Reporting</BLUE> File System Information" );

#ifdef WIN32
	if( fs_info->fs_flags & FS_VOL_IS_COMPRESSED ) {
		compressed = 1;
	}
#endif

	con_Print( "\tFile System: %s", fs_info->fs_name );
	con_Print( "\tMax Component Length: %d", fs_info->fs_maxcomponentlength );

	con_Print( "\tVolume Name: %s", fs_info->vol_name );
	con_Print( "\tVolume Serial Number: %d", fs_info->vol_serialnumber );
	con_Print( "\tWorking Directory: %s", fs_info->working_dir );
	con_Print( "\tVolume Compressed: %d", compressed );

	return 1;
}

/* ----- path structure control ------ */
char *fs_GetBaseDirectory() 
{ 
	// assert
	if( fs_info == NULL )
	{
		con_Print( "File System: GetBaseDirectory: Assert Failed: fs_info is NULL" );
		eng_Stop( "120002" );
		return 0;
	}
	if( fs_info->working_dir == NULL )
	{
		con_Print( "File System: GetBaseDirectory: Assert Failed: fs_info->working_dir is NULL" );
		eng_Stop( "120006" );
		return 0;
	}
	
	return fs_info->working_dir; 
}

char *fs_GetMediaDirectory() 
{ 
	// assert
	if( fs_info == NULL )
	{
		con_Print( "File System: GetMediaDirectory: Assert Failed: fs_info is NULL" );
		eng_Stop( "120003" );
		return 0;
	}
	if( fs_info->media_dir == NULL )
	{
		con_Print( "File System: GetMediaDirectory: Assert Failed: fs_info->media_dir is NULL" );
		eng_Stop( "120007" );
		return 0;
	}
	
	return fs_info->media_dir; 
}

/* ------------
fs_SetMediaDirectory - sets the current media directory to name and regenerates the list of packed files
------------ */
char *fs_SetMediaDirectory( const char *name )
{
	char *base = fs_GetBaseDirectory();
	int len;

	// assert
	if( tcstrlen(name) <= 0 ) 
	{
		con_Print( "File System: SetMediaDirectory: Assert Failed: strlen of name is <= 0" );
		eng_Stop( "120004" );
		return NULL;
	}
	if( fs_info == NULL )
	{
		con_Print( "File System: SetMediaDirectory: Assert Failed: fs_info is NULL" );
		eng_Stop( "120005" );
		return NULL;
	}

	// free the previous path
	SAFE_RELEASE( fs_info->media_dir );

	// count a new length
	len = tcstrlen(base);
	len += tcstrlen(name);
	len += 2; // 1 path separator and 1 null terminator

	// get memory for the string
	fs_info->media_dir = mem_alloc( sizeof(char) * len );
	if( fs_info->media_dir == NULL )
	{
		return NULL;
	}
	memset( fs_info->media_dir, 0, sizeof(char) * len );

	// copy the data over
	tcstrcpy( fs_info->media_dir, fs_info->working_dir );
	tcstrcat( fs_info->media_dir, SYSTEM_PATH_SEPARATOR );
	tcstrcat( fs_info->media_dir, name );
	fs_info->media_dir[len - 1] = '\0';
	fs_ToSystemPath(fs_info->media_dir); // if the media path is more than 2 directorys below the current path

	// tell us about it
	con_Print( "File System:  Base Media Directory changed to \"%s\"", fs_info->media_dir );

	// refresh the files located in the media directory
	fs_IterateAllPackedFiles();

	return fs_info->media_dir;
}

/* ------------
fs_GetBasePath - returns an fqp for the requested filename under the working directory
------------ */
char *fs_GetBasePath( const char *filename, const char *subPath )
{
	// assert
	if( filename == NULL )
	{
		con_Print( "File System: GetBasePath: Assert Failed: filename is NULL" );
		eng_Stop( "120008" );
		return NULL;
	}
	if( subPath == NULL )
	{
		con_Print( "File System: GetBasePath: Assert Failed: subPath is NULL" );
		eng_Stop( "120009" );
		return NULL;
	}
	if( fs_info == NULL )
	{
		con_Print( "File System: GetBasePath: Assert Failed: fs_info is NULL" );
		eng_Stop( "120010" );
		return NULL;
	}
	if( fs_info->tmp_full_path == NULL )
	{
		con_Print( "File System: GetBasePath: Assert Failed: fs_info->tmp_full_path is NULL" );
		eng_Stop( "120011" );
		return NULL;
	}
	if( fs_info->working_dir == NULL )
	{
		con_Print( "File System: GetBasePath: Assert Failed: fs_info->working_dir is NULL" );
		eng_Stop( "120012" );
		return NULL;
	}
	
	// init
	memset( fs_info->tmp_full_path, 0, sizeof(char) * fs_info->tmp_full_path_length );

	// construct
	tcstrcpy( fs_info->tmp_full_path, fs_info->working_dir );
	tcstrcat( fs_info->tmp_full_path, SYSTEM_PATH_SEPARATOR );
	if( tcstrlen(subPath) > 0 )
	{
		tcstrcat( fs_info->tmp_full_path, subPath );
		tcstrcat( fs_info->tmp_full_path, SYSTEM_PATH_SEPARATOR );
	}
	tcstrcat( fs_info->tmp_full_path, filename );

	// verify
	fs_ToSystemPath( fs_info->tmp_full_path );

	// return
	return fs_info->tmp_full_path;
}

/* ------------
fs_GetMediaPath - returns the fqp for filename under the subdirectory subPath in the media directory
------------ */
char *fs_GetMediaPath( const char *filename, const char *subPath )
{
	// assert
	if( filename == NULL )
	{
		con_Print( "File System: GetMediaPath: Assert Failed: filename is NULL" );
		eng_Stop( "120013" );
		return NULL;
	}
	if( subPath == NULL )
	{
		con_Print( "File System: GetMediaPath: Assert Failed: subPath is NULL" );
		eng_Stop( "120014" );
		return NULL;
	}
	if( fs_info == NULL )
	{
		con_Print( "File System: GetMediaPath: Assert Failed: fs_info is NULL" );
		eng_Stop( "120015" );
		return NULL;
	}
	if( fs_info->tmp_full_path == NULL )
	{
		con_Print( "File System: GetMediaPath: Assert Failed: fs_info->tmp_full_path is NULL" );
		eng_Stop( "120016" );
		return NULL;
	}
	if( fs_info->media_dir == NULL )
	{
		con_Print( "File System: GetMediaPath: Assert Failed: fs_info->media_dir is NULL" );
		eng_Stop( "120017" );
		return NULL;
	}

	// init
	memset( fs_info->tmp_full_path, 0, sizeof(char) * fs_info->tmp_full_path_length );

	// construct
	tcstrcpy( fs_info->tmp_full_path, fs_info->media_dir );
	tcstrcat( fs_info->tmp_full_path, SYSTEM_PATH_SEPARATOR );
	if( (subPath != NULL) && (tcstrlen(subPath) > 0) )
	{
		tcstrcat( fs_info->tmp_full_path, subPath );
		tcstrcat( fs_info->tmp_full_path, SYSTEM_PATH_SEPARATOR );
	}
	tcstrcat( fs_info->tmp_full_path, filename );

	// verify
	fs_ToSystemPath( fs_info->tmp_full_path );

	// return
	return fs_info->tmp_full_path;
}

/* ------------
fs_GetFileLength
------------ */
unsigned long fs_GetFileLength( file_t *file )
{
	unsigned long size;

	fs_seek( file, 0, SEEK_END);
	size = fs_tell( file );
	fs_seek( file, 0, SEEK_SET );

	return size;
}

/* ----- file usage stuff ----- */
/* ---------------
fs_open - acts like fopen, but looks for an opened packfile first
--------------- */
file_t *fs_open( const char *filename, const char *mode )
{
	file_t *file;
	FILE *stream;

	// assert
	if( tcstrlen(filename) <= 0 )
	{
		con_Print( "File System: fs_open: Assert Failed: tcstrlen filename <= 0" );
		eng_Stop( "120018" );
		return NULL;
	}
	if( tcstrlen(mode) <= 0 )
	{
		con_Print( "File System: fs_open: Assert Failed: tcstrlen mode <= 0" );
		eng_Stop( "120019" );
		return NULL;
	}
	if( fs_info == NULL )
	{
		con_Print( "File System: fs_open: Assert Failed: fs_info is NULL" );
		eng_Stop( "120020" );
		return NULL;
	}

	stream = fopen( filename, mode );
	if( stream == NULL )
	{
		if( fs_info->llPackedFiles != LINKEDLIST_NOT_A_LIST )
		{
			// try to find a listed packed file
			ll_iter_reset( fs_info->llPackedFiles );
			while( !ll_iter_isdone( fs_info->llPackedFiles ) )
			{
				file = ll_iter_data( fs_info->llPackedFiles );
				if( tcstrcmp( file->signature, filename ) ) return file;
				
				ll_iter_next( fs_info->llPackedFiles );
			}
		}
		
		// if we didn't find either a normal file or a packed file return
		con_Print( "File System Error:  Unable to open file \"%s\"", filename );
		return NULL;
	}
	
	// pass through to fopen if it wasn't found in the list
	file = (file_t *)mem_alloc( sizeof(file_t) );
	if( file == NULL ) 
	{
		con_Print( "File System Error:  Unable to allocate memory for file handle for file \"%s\"", filename );
		return NULL;
	}

	file->signature = tcDeepStrCpy(filename);
	if( file->signature == NULL )
	{
		con_Print( "File System Error:  Unable to make a copy of the filename for file \"%s\"", filename );
		fclose( file->file );
		SAFE_RELEASE( file );
		return NULL;
	}

	file->file = stream;
	file->startOffset = 0;
	file->endOffset = fs_GetFileLengthFILE(file->file);
	file->inPack = 0;
	file->offset = 0;

	return file;
}

/* ---------------
fs_close - frontend for close that handles packed files
--------------- */
int fs_close( file_t *stream )
{
	int ret;

	if( stream == NULL )
	{
		con_Print( "File System: fs_close: Assert Failed: close is NULL" );
		eng_Stop( "120021" );
		return EOF;
	}
	if( stream->file == NULL )
	{
		con_Print( "File System: fs_close: Assert Failed: stream->file is NULL" );
		eng_Stop( "120022" );
		return EOF;
	}

	// complete the abstraction:  these are closed at termination
	if( stream->inPack )	return 0;

	// free memory associated with this object
	SAFE_RELEASE( stream->signature );
	ret = fclose( stream->file );
	SAFE_RELEASE( stream );

	return ret;
}

/* ---------------
fs_read - acts like fread, but handles the buffer positioning needed inside of a packed file
--------------- */
size_t fs_read( void *buffer, size_t size, size_t count, file_t *stream )
{
	int ret;
	
	if( buffer == NULL )
	{
		con_Print( "File System: fs_read: Assert Failed: buffer  is NULL" );
		eng_Stop( "120023" );
		return 0;
	}
	if( stream == NULL )
	{
		con_Print( "File System: fs_read: Assert Failed: stream is NULL" );
		eng_Stop( "120024" );
		return 0;
	}
	if( stream->file == NULL ) 
	{
		con_Print( "File System: fs_read: Assert Failed: stream->file is NULL" );
		eng_Stop( "120025" );
		return 0;
	}
	if( size <= 0 )
	{
		con_Print( "File System: fs_read: Assert Failed: size <= 0" );
		eng_Stop( "120026" );
		return 0;
	}
	if( count <= 0 )
	{
		con_Print( "File System: fs_read: Assert Failed: count <= 0" );
		eng_Stop( "120027" );
		return 0;
	}

	// seek to offset in the packed or unpacked file
	fseek( stream->file, stream->startOffset + stream->offset, SEEK_SET );

	// we need to accuratly reflect eof conditions in a packed file
	if( (stream->offset + (size * count)) > stream->endOffset )
	{
		// max count is what can fit in this file, only get that much and no more
		count = (stream->endOffset - stream->offset) / size;
	}

	// do the read
	ret = fread( buffer, size, count, stream->file );

	// adjust offset to reflect actual
	stream->offset = ftell(stream->file) - stream->startOffset;

	return ret;
}

/* ---------------
fs_write - acts like fwrite for normal files and blocks writes to pack files
--------------- */
size_t fs_write( const void *buffer, size_t size, size_t count, file_t *stream )
{
	int ret;

	// assert
	if( buffer == NULL )
	{
		con_Print( "File System: fs_write: Assert Failed: buffer is NULL" );
		eng_Stop( "120028" );
		return 0;
	}
	if( stream == NULL )
	{
		con_Print( "File System: fs_write: Assert Failed: stream is NULL" );
		eng_Stop( "120029" );
		return 0;
	}
	if( stream->file == NULL )
	{
		con_Print( "File System: fs_write: Assert Failed: stream->file is NULL" );
		eng_Stop( "120030" );
		return 0;
	}
	if( size <= 0 )
	{
		con_Print( "File System: fs_write: Assert Failed: size <= 0" );
		eng_Stop( "120031" );
		return 0;
	}
	if( count <= 0 ) 
	{
		con_Print( "File System: fs_write: Assert Failed: count <= 0" );
		eng_Stop( "120032" );
		return 0;
	}

	if( stream->inPack ) return 0; // don't write to packed files

	// do the write
	ret = fwrite( buffer, size, count, stream->file );
	stream->offset = ftell(stream->file) - stream->startOffset;
	return ret;
}

/* ---------------
fs_seek - acts like fseek - but handles pointer dereferences in packed  files
--------------- */
int fs_seek( file_t *stream, long offset, int origin )
{
	int ret;

	// assert
	if( stream == NULL )
	{
		con_Print( "File System: fs_seek: Assert Failed: stream is NULL" );
		eng_Stop( "120033" );
		return 1;
	}
	if( stream->file == NULL )
	{
		con_Print( "File System: fs_seek: Assert Failed: stream->file is NULL" );
		eng_Stop( "120034" );
		return 1;
	}

	// assign a new offset
	switch( origin )
	{
	case SEEK_CUR:
		stream->offset += offset;
		break;
	case SEEK_END:
		stream->offset = (stream->endOffset - stream->startOffset) + offset;
		break;
	case SEEK_SET:
		stream->offset = offset;
		break;
	default:
		con_Print( "File System:  fs_seek: Assert Failed: origin not a recognized token" );
		eng_Stop( "120035" );
		return 1;
	}

	// do the actual seek
	ret = fseek( stream->file, stream->offset + stream->startOffset, SEEK_SET );
	return ret;
}

/* ---------------
fs_tell - returns the current position of the stream
--------------- */
long fs_tell( file_t *stream )
{
	// assert
	if( stream == NULL )
	{
		con_Print( "File System: fs_tell: Assert Failed: stream is NULL" );
		eng_Stop( "120036" );
		return 0;
	}
	if( stream->file == NULL )
	{
		con_Print( "File System: fs_tell: Assert Failed: stream->file is NULL" );
		eng_Stop( "120037" );
		return 0;
	}
	
	return ftell( stream->file ) - stream->startOffset;
}

/* ---------------
fs_eof - returns > 0 when the offset is past the end of the file
--------------- */
int fs_eof( file_t *stream)
{
	// assert
	if( stream == NULL )
	{
		con_Print( "File System: fs_eof: Assert Failed: stream is NULL" );
		eng_Stop( "120038" );
		return 1;
	}
	if( stream->file == NULL )
	{
		con_Print( "File System: fs_eof: Assert Failed: stream->file is NULL" );
		eng_Stop( "120039" );
		return 1;
	}

	if( stream->inPack )
	{
		if( (stream->offset + stream->startOffset) > stream->endOffset ) return 1;
		return 0;
	}
	else
	{
		return feof(stream->file);
	}	
}

/* ---------------
fs_flush - causes all buffered writes to be written to the physical media
--------------- */
int fs_flush( file_t *stream )
{
	// assert
	if( stream == NULL )
	{
		con_Print( "File System: fs_flush: Assert Failed: stream is NULL" );
		eng_Stop( "120040" );
		return 0;
	}
	if( stream->file == NULL )
	{
		con_Print( "File System: fs_flush: Assert Failed: stream->file is NULL" );
		eng_Stop( "120041" );
		return 0;
	}

	// do write to pack files
	if( stream->inPack ) return 0;

	return fflush( stream->file );
}

/* ---------------
fs_rewind - acts like rewind, calls fs_seek with begin indicated
--------------- */
void fs_rewind( file_t *stream )
{
	fs_seek( stream, 0L, SEEK_SET );
}

/* ---------------
fs_error - return any errors in the given stream
--------------- */
int fs_error( file_t *stream ) 
{ 
	// assert
	if( stream == NULL )
	{
		con_Print( "File System: fs_error: Assert Failed: stream is NULL" );
		eng_Stop( "120042" );
		return 0;
	}
	if( stream->file == NULL )
	{
		con_Print( "File System: fs_error: Assert Failed: stream->file is NULL" );
		eng_Stop( "120043" );
		return 0;
	}

	return ferror(stream->file); 
}

#if 0
/* ---------------
fs_RemoveDirectory
--------------- */
int fs_RemoveDirectory( char *rel_dir )
{
	HANDLE hfile;
	WIN32_FIND_DATA fd;
	DWORD error;
	char *path;
	int ret;
	unsigned int iter;

	memset( &fd, 0, sizeof(WIN32_FIND_DATA) );

	path = fs_GetMediaPath( "*.*", rel_dir );

	hfile = FindFirstFile( path, &fd );
	if( (hfile == INVALID_HANDLE_VALUE) || (hfile == NULL) ) {
		// directory does not exist
		return 1;
	}

	iter = 0;
	while( iter < 32768 )
	{
		ret = FindNextFile( hfile, &fd );
		if( !ret )
		{
			error = GetLastError();
			if( error == ERROR_NO_MORE_FILES )
			{
				FindClose( hfile );
				path = fs_GetMediaPath( "", rel_dir );
				path[ tcstrlen(path)-1 ] = '\0';
				if( !RemoveDirectory( path ) )
				{
					error = GetLastError();
					con_Print( "File System Warning:  Remove Directory Failed with Error %d", error ); 
					return 0;
				}
				return 1;
			}
			else
			{
				con_Print( "File System Warning:  Find File Returned Unknown Error" );
				FindClose( hfile );
				return 0;
			}
		}

		if( fd.cFileName[0] == '.' )
		{
			continue;
		}

		path = fs_GetMediaPath( fd.cFileName, rel_dir );
		if( !DeleteFile( path ) )
		{
			con_Print( "File System Warning:  Unable to Delete File %s", fd.cFileName );
			FindClose( hfile );
			return 0;
		}

		iter++;
	}

	return 0;
}
#endif

#if 0
/* ---------------
sys_CreateDirectory
--------------- */
int fs_CreateDirectory( char *rel_dir )
{
	char *path;
	int ret;
 
	path = fs_GetMediaPath( "", rel_dir );
	if( !CreateDirectory( path, NULL ) )
	{
		ret = GetLastError();

		con_Print( "File System Warning:  Unable to create directory %s", path );
		return 0;
	}

	return 1;
}
#endif

/* ---------------
fs_GetNextOrderedName
--------------- */
int fs_GetNextOrderedName( char *name, char *dir )
{
#ifndef WIN32
    int count;
    char** names;
    char* path;
    int number;
    int a;
    
    // get full path
    path = fs_GetBasePath( name, dir );
    
    // find all existing items
    names = dir_scan( dir, name, &count );
    if( !names ) {
        con_Print( "File System Warning:  File Search Open Failed" );
		return 0;
	}
    
    // get the last number present
    a = 0;
    while( names[count-1][a] ) {
        if( names[count-1][a] < '0' || names[count-1][a] > '9' ) {
            names[count-1][a] = ' ';
        }
        a++;
    }
    number = tcatoi( names[count-1] ) + 1;
    
    dir_scan_free( names, count );
    return number;

#else
	int number = 0;
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
#endif
}

/* ------------
fs_FileExists - returns true if the file of complete name exists, 0 otherwise, (quite!)
------------ */
byte fs_FileExists( char *name )
{
	FILE *stream = NULL;
	file_t *file;
	
	// assert
	if( name == NULL )
	{
		con_Print( "FileSystem:  FileExists Assert Failed:  name is NULL" );
		eng_Stop( "120044" );
		return 0;
	}
	if( tcstrlen(name) <= 0 )
	{
		con_Print( "FileSystem:  FileExists Assert Failed:  name length <= 0" );
		eng_Stop( "120045" );
		return 0;
	}

	// attempt an open with a read type that will fail on non-existance
	stream = fopen( name, "r" );
	if( stream != NULL )
	{
		// found it
		return 1;
	}
	else
	{
		// try to find a listed packed file
		ll_iter_reset( fs_info->llPackedFiles );
		while( !ll_iter_isdone( fs_info->llPackedFiles ) )
		{
			file = ll_iter_data( fs_info->llPackedFiles );
			if( tcstrcmp( file->signature, name ) ) 
			{
				// found it packed
				return 1;
			}
			ll_iter_next( fs_info->llPackedFiles );
		}
	
		// if we didn't find either a normal file or a packed file return
		return 0;
	}
}

/* ----- LOCAL FUNCTIONS ----- */
/* ------------
fs_GetFileSysInfo - iterates information from the system about this file system
------------ */
static file_system_info_t *fs_GetFileSysInfo()
{
	file_system_info_t *fsinfo;
	int ret;
	int dir_len;
	char temp_char_str[1];

    // get the info structure
	fsinfo = (file_system_info_t*)mem_alloc( sizeof(file_system_info_t) );
	if( fsinfo == NULL )
	{
		// not that there's anything to log an error to yet
		con_Print( "File System Initialization Failed:  Unable to allocate memory for fs_info." );
		return NULL;
	}
	memset( fsinfo, 0, sizeof(file_system_info_t) );
    
    // get the current directory
    fsinfo->working_dir = dir_get_current();
    if( !fsinfo->working_dir )
	{
		SAFE_RELEASE( fsinfo );
		fs_Terminate();
		return NULL;
	}

	// allocate space for the volume and filesys info
	fsinfo->fs_name = (char*)mem_alloc( FS_NAME_SIZE );
	if( fsinfo->fs_name == NULL )
	{  // nothing to log an error to yet
		SAFE_RELEASE( fsinfo );
		con_Print( "File System Initialization Failed:  Unable to allocate memory for file sys name." );
		return NULL;
	}
	memset( fsinfo->fs_name, 0, FS_NAME_SIZE );

    
#ifdef WIN32
	fsinfo->vol_name = (char*)mem_alloc( VOL_NAME_SIZE );
	if( fsinfo->vol_name == NULL )
	{  // nothing to log an error to yet
        		SAFE_RELEASE( fsinfo->working_dir );

		SAFE_RELEASE( fsinfo->fs_name );
		SAFE_RELEASE( fsinfo );
		con_Print( "File System Initialization Failed:  Unable to allocate memory for file sys volume name." );
		return NULL;
	}
	memset( fsinfo->vol_name, 0, VOL_NAME_SIZE );

	ret = GetVolumeInformation( NULL,  // volume info for current volume 
								fsinfo->vol_name, VOL_NAME_SIZE, 
								&fsinfo->vol_serialnumber,
								&fsinfo->fs_maxcomponentlength,
								&fsinfo->fs_flags, 
								fsinfo->fs_name, FS_NAME_SIZE );

	if( !ret ) 
	{
        		SAFE_RELEASE( fsinfo->working_dir );

		// nothing to log an error to yet
		SAFE_RELEASE( fsinfo->vol_name );
		SAFE_RELEASE( fsinfo->fs_name );
		SAFE_RELEASE( fsinfo );
		fs_Terminate();
		return NULL;
	}

	// a call to get current directory that is too short returns the length needed
	dir_len = GetCurrentDirectory( 1, temp_char_str );
	fsinfo->working_dir = (char*)mem_alloc( dir_len );
	if( fsinfo->working_dir == NULL )
	{
        		SAFE_RELEASE( fsinfo->working_dir );

		SAFE_RELEASE( fsinfo->vol_name );
		SAFE_RELEASE( fsinfo->fs_name );
		SAFE_RELEASE( fsinfo );
		fs_Terminate();
		return NULL;
	}

    // initialize this for the GetFullPath function to use
	fsinfo->tmp_full_path_length = fsinfo->fs_maxcomponentlength * TEMP_PATH_SCALAR;
	fsinfo->tmp_full_path = (char*)mem_alloc( fsinfo->fs_maxcomponentlength * TEMP_PATH_SCALAR );
	if( fsinfo->tmp_full_path == NULL )
	{
		SAFE_RELEASE( fsinfo->working_dir );
		SAFE_RELEASE( fsinfo->vol_name );
		SAFE_RELEASE( fsinfo->fs_name );
		SAFE_RELEASE( fsinfo );
		return NULL;
	}
	memset( fsinfo->tmp_full_path, 0, fsinfo->fs_maxcomponentlength * TEMP_PATH_SCALAR );

	// set the linked list to no list or previous list
	fsinfo->llPackedFiles = LINKEDLIST_NOT_A_LIST;
#else 
    
    // initialize this for the GetFullPath function to use
    fsinfo->fs_maxcomponentlength = 1024;
	fsinfo->tmp_full_path_length = fsinfo->fs_maxcomponentlength * TEMP_PATH_SCALAR;
	fsinfo->tmp_full_path = (char*)mem_alloc( fsinfo->fs_maxcomponentlength * TEMP_PATH_SCALAR );
	if( fsinfo->tmp_full_path == NULL )
	{
		SAFE_RELEASE( fsinfo->working_dir );
		SAFE_RELEASE( fsinfo->vol_name );
		SAFE_RELEASE( fsinfo->fs_name );
		SAFE_RELEASE( fsinfo );
		return NULL;
	}
	memset( fsinfo->tmp_full_path, 0, fsinfo->fs_maxcomponentlength * TEMP_PATH_SCALAR );

	// set the linked list to no list or previous list
	fsinfo->llPackedFiles = LINKEDLIST_NOT_A_LIST;
    
    fsinfo->vol_name = 0;
    fsinfo->vol_serialnumber = 0;
    fsinfo->fs_name = 0;
    fsinfo->fs_maxcomponentlength = 0;
    
#endif 
	return fsinfo;
}

/* ------------
fs_IterateAllPackedFiles - opens and searches all zip files in the media directory
------------ */
static int fs_IterateAllPackedFiles()
{
#ifndef WIN32
    int a;
    int cnt;
    char* scafPath;
    char* path;
    FILE* stream;
    char** files;
    zip_file_header_t header;
    file_t *file;
    
    // assert
	if( fs_info == NULL )
	{
		con_Print( "File System:  IterateAllPackedFiles:  Assert Failed fs_info is NULL" );
		eng_Stop( "120046" );
		return 0;
	}
	
	// destroy previous contents
	fs_DestroyPackedFileList(fs_info->llPackedFiles);

	// get a new list
	fs_info->llPackedFiles = ll_GetNewList();

	// tell us about it
	con_Print( "\nScanning media packages:" );

	// find all zip files
	scafPath = fs_GetMediaPath( "", "" );
    files = dir_scan( scafPath, "*.zip", &cnt );
    for( a = 0 ; a < cnt ; a++ ) {
		// look for zipped files in this library
		con_Print( "\tAdding %s", files[a] );

		// open this zip file
		path = fs_GetMediaPath( files[a], "" );
		stream = fopen( path, "rb" );
		while( stream != NULL && fs_uz_GetHeader( &header, stream ) )
		{
			// verify that this is a valid zip file that we can use
			if( !(header.bit_flags & 0x0004) && !(header.compression != 0) )
			{ 
				// allocate for the file
				file = (file_t*)mem_alloc( sizeof(file_t) );
				if( file == NULL )
				{
					con_Print( "Iterate Pack Files Failed:  Unable to allocate memory for file." );
					return 0;
				}
				memset( file, 0, sizeof(file_t) );

				// setup basic info
				file->file = stream;
				file->inPack = 1;
				file->offset = 0;
				file->signature = tcDeepStrCpy( fs_GetMediaPath( header.file_name, "" ) );

				// setup the file offsets and seek to the next file
				file->startOffset = ftell( stream );
				fseek( stream, header.compressed_size, SEEK_CUR );
				file->endOffset = ftell( stream );
	
				// add the file to the list of files
				ll_AddUnordered( fs_info->llPackedFiles, file );
				file = NULL;

				// free memory for the next pass
				SAFE_RELEASE( header.file_name );
				SAFE_RELEASE( header.extra_field );
				memset( &header, 0, sizeof(zip_file_header_t) );
			}
			else
			{
				// cannot handle extened headers or compressed files
				con_Print( "File System Warning:  wrongly formatted zipped file found in zip \"%s\", at file \"%s\"", files[a], header.file_name );
				SAFE_RELEASE( header.file_name );
				SAFE_RELEASE( header.extra_field );
			}
		}
	}
    dir_scan_free( files, cnt );
    
#else
	byte bMoreFiles = 1;
	char *scafPath, *path;
	FILE *stream;
	file_t *file;
	zip_file_header_t header;
	HANDLE finder;
	WIN32_FIND_DATA found;
    
	// assert
	if( fs_info == NULL )
	{
		con_Print( "File System:  IterateAllPackedFiles:  Assert Failed fs_info is NULL" );
		eng_Stop( "120046" );
		return 0;
	}
	
	// destroy previous contents
	fs_DestroyPackedFileList(fs_info->llPackedFiles);

	// get a new list
	fs_info->llPackedFiles = ll_GetNewList();

	// tell us about it
	con_Print( "\nScanning media packages:" );

	// find all zip files
	scafPath = fs_GetMediaPath( "*.zip", "" );
	finder = FindFirstFile( scafPath, &found );
	while( (finder != INVALID_HANDLE_VALUE) && (bMoreFiles > 0) )
	{
		// look for zipped files in this library
		con_Print( "\tAdding %s", found.cFileName );

		// open this zip file
		path = fs_GetMediaPath( found.cFileName, "" );
		stream = fopen( path, "r" );
		while( stream != NULL && fs_uz_GetHeader( &header, stream ) )
		{
			// verify that this is a valid zip file that we can use
			if( !(header.bit_flags & 0x0004) && !(header.compression != 0) )
			{ 
				// allocate for the file
				file = (file_t*)mem_alloc( sizeof(file_t) );
				if( file == NULL )
				{
					con_Print( "Iterate Pack Files Failed:  Unable to allocate memory for file." );
					return 0;
				}
				memset( file, 0, sizeof(file_t) );

				// setup basic info
				file->file = stream;
				file->inPack = 1;
				file->offset = 0;
				file->signature = tcDeepStrCpy( fs_GetMediaPath( header.file_name, "" ) );

				// setup the file offsets and seek to the next file
				file->startOffset = ftell( stream );
				fseek( stream, header.compressed_size, SEEK_CUR );
				file->endOffset = ftell( stream );
	
				// add the file to the list of files
				ll_AddUnordered( fs_info->llPackedFiles, file );
				file = NULL;

				// free memory for the next pass
				SAFE_RELEASE( header.file_name );
				SAFE_RELEASE( header.extra_field );
				memset( &header, 0, sizeof(zip_file_header_t) );
			}
			else
			{
				// cannot handle extened headers or compressed files
				con_Print( "File System Warning:  wrongly formatted zipped file found in zip \"%s\", at file \"%s\"", found.cFileName, header.file_name );
				SAFE_RELEASE( header.file_name );
				SAFE_RELEASE( header.extra_field );
			}
		}
	
		// get the next path or set to exit
		if( !FindNextFile( finder, &found ) )
		{
			bMoreFiles = 0;
		}
	}
	FindClose( finder );
#endif
    
	return 1;
}

/* ------------
fs_ToSystemPath - replaces any generic path separators in path (/) with system dependent path separators
------------ */
static void fs_ToSystemPath( char *path )
{
	while( *path )
	{
		if( *path == PATH_SEPARATOR_C ) *path = SYSTEM_PATH_SEPARATOR_C;
		path++;
	}
}

/* ------------
fs_DestroyPackedFileList - destroy the list of files located in list
------------ */
static void fs_DestroyPackedFileList( LinkedList list )
{
	file_t *tfile;

	// silent assert
	if( list == LINKEDLIST_NOT_A_LIST ) return;
	
	// destroy the list of packed files
	ll_iter_reset( list );
	while( !ll_iter_isdone( list ) )
	{
		// destroy the file's properties
		tfile = ll_iter_data( list );
		fclose( tfile->file );
		SAFE_RELEASE( tfile->signature );
		SAFE_RELEASE( tfile );

		// next
		ll_iter_next( list );
	}

	// kill the list
	ll_DeleteList( list );
}

/* ----------
uz_GetHeader - reads the header at the file position into header 
---------- */
static int fs_uz_GetHeader( zip_file_header_t *header, FILE *fp )
{
	int a;

	// assert
	if( fp == NULL )
	{
		con_Print( "File System: uz_GetHeader:  Assert Failed fp is NULL" );
		eng_Stop( "120047" );
		return 0;
	}

	if( header == NULL ) return 0;
	{
		con_Print( "File System: uz_GetHeader:	Assert Failed header is NULL" );
		eng_Stop( "120048" );
		return 0;
	}

	header->signature = fs_ReadLong(fp);
	if( header->signature != ZIP_HEADER_SIGNATURE ) return 0;

	header->version_needed = fs_ReadShort(fp);
	header->bit_flags = fs_ReadShort(fp);
	header->compression = fs_ReadShort(fp);
	header->last_mod_file_time = fs_ReadShort(fp);
	header->last_mod_file_date = fs_ReadShort(fp);
	header->crc_32 = fs_ReadLong(fp);
	header->compressed_size = fs_ReadLong(fp);
	header->uncompressed_size = fs_ReadLong(fp);
	header->file_name_length = fs_ReadShort(fp);
	header->extra_field_length = fs_ReadShort(fp);

	if( header->file_name_length > 0 )
	{
		header->file_name = (char*)mem_alloc( sizeof(char) * (header->file_name_length + 1) );
		if( header->file_name == NULL )
		{
			con_Print( "Zip Load Error:  Unable to allocate memory for file name." );
			return 0;
		}
		memset( header->file_name, 0, sizeof(char) * (header->file_name_length + 1) );
		
		for( a = 0 ; a < header->file_name_length ; a++ )
		{
			header->file_name[a] = fs_ReadByte(fp);
		}
		header->file_name[header->file_name_length] = 0;
	}
	else
	{
		return 0;
	}

	if( header->extra_field_length > 0 )
	{
		header->extra_field = (byte*)mem_alloc( sizeof(byte) * header->extra_field_length );
		if( header->extra_field == NULL )
		{
			con_Print( "Zip Load Error:  Unable to allocate memory for file extended field."  );
			SAFE_RELEASE( header->file_name );
			return 0;
		}
		memset( header->extra_field, 0, sizeof(byte) * header->extra_field_length );
		
		for( a = 0 ; a < header->extra_field_length ; a++ )
		{
			header->extra_field[a] = fs_ReadByte(fp);
		}
	}
	else
	{
		header->extra_field = NULL;
	}

	return 1;
}

/* ----------
fs_ReadLong - pulls a long in little endian format from the fstream
---------- */
int fs_ReadLong( FILE *fp )
{
	byte in = 0;
	int out = 0;

	fread( &in, sizeof(byte), 1, fp );
	out = in;
	fread( &in, sizeof(byte), 1, fp );
	out += in << 8;
	fread( &in, sizeof(byte), 1, fp );
	out += in << 16;
	fread( &in, sizeof(byte), 1, fp );
	out += in << 24;

	return out;
}

/* ----------
fs_ReadShort - pulls a short in little endian format from the fstream
---------- */
short fs_ReadShort( FILE *fp )
{
	byte in;
	short out = 0;

	fread( &in, sizeof(byte), 1, fp );
	out = in;
	fread( &in, sizeof(byte), 1, fp );
	out += in << 8;

	return out;
}

/* ----------
fs_ReadByte - pulls a byte from the fstream
---------- */
byte fs_ReadByte( FILE *fp )
{
	byte in;

	fread( &in, sizeof(byte), 1, fp );
	return in;
}

/* ----------
fs_GetFileLengthFILE - raw file access to get a length
---------- */
unsigned long fs_GetFileLengthFILE( FILE *file )
{
	unsigned long size;

	fseek( file, 0, SEEK_END);
	size = ftell( file );
	fseek( file, 0, SEEK_SET );

	return size;
}
