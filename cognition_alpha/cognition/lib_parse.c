// Cognition
// parselib.c
// Created 2-25-02 @ 0040 by Terrence Cole

// Includes
/////////////
#include "cog_global.h"

// Global Prototypes
//////////////////////
/*
int par_ExecFile( char *file_name, char *file_rel_dir );
int par_ExecCmdLine( char *cmdline );
int par_ExecBuffer( char *buffer );
char *par_StripComments( char* buffer );
char *par_StripCarriageReturns( char *buffer );
char *par_StripTabs( char *buffer );
char *par_StripQuotes( char *buffer );
char *par_StripChar( char *buffer, char strip );
char *par_ReplaceChar( char *buffer, char oldchar, char newchar );
int par_ToChar( char *buffer, char end );
int par_ToToken( char* buffer );
int par_ToSpace( char* buffer );
char *par_BufferFile( char *path );
char *par_GetTokenValue( char *src, char *token, int token_length );
int par_GetNextToken( char *buffer, int *toToken, int *tokenLen );
byte par_IsWhiteSpace( const char c );
int par_QuotedStringCopy( char *dest, const char *src, int length );
*/

// *********** FUNCTIONALITY ***********
/* ------------
par_ExecFile
------------ */
int par_ExecFile( char *file_name, char *file_rel_dir )
{
	file_t* file;
	char *full_path;
	int file_size;
	char *buffer, *temp;

	full_path = fs_GetBasePath( file_name, file_rel_dir );

	con_Print( "Executing commands from file \"%s\"", full_path );

	file = fs_open( full_path, "r" );
	if( file == NULL ) {
		con_Print( "Settings File Failed to Open" );
		return 0;
	}

	file_size = fs_GetFileLength( file );
	file_size += 32; // to avoid buffer overruns when scaning

	buffer = (char*)mem_alloc( file_size );
	if( buffer == NULL ) {
		con_Print( "Memory Allocation failed for settings file buffer" );
		return 0;
	}
	memset( buffer, 0, file_size );

	fs_rewind( file );
	fs_read( buffer, sizeof(char), file_size, file );
	fs_close( file );

	temp = buffer;
	par_ExecBuffer( temp ); // send our filled buffer to the actual work function
	SAFE_RELEASE( buffer );
	return 1;
}

/* ------------
par_ExecCmdLine
------------ */
int par_ExecCmdLine( char *cmdline )
{
	char *temp = cmdline;

	if( cmdline == NULL ) return 1;

	temp = par_ReplaceChar( temp, '-', '\n' );
	if( !par_ExecBuffer(temp) )
		return 0;

	return 1;
}

/* ------------
par_ExecBuffer
------------ */
int par_ExecBuffer( char *buffer )
{
	char *temp;
	char command[1024];
	int a, cr;
	int ret;

	temp = buffer;
	temp = par_StripComments(temp);
	temp = par_StripQuotes(temp);
	temp = par_StripTabs(temp);
	temp = par_StripChar( temp, '\r' );

	// clear the command
	memset( command, 0, sizeof(command) );
	
	while( *temp )
	{
		ret = par_ToToken(temp);
		if( ret == -1 ) return 0;
		temp += ret;

		cr = tcstrnfind( temp, "\n", 1, -1 );
		if( cr == -1 ) 
		{
			cr = tcstrlen(temp);
		}
		else
		{
			temp[cr] = ' ';
			cr--;
		}
		for( a=0 ; a<(cr+1) ; a++, temp++ )
			command[a] = *temp;

		com_Process( command );

		memset( command, 0, sizeof(command) );	
	}

	return 1;
}

/* ---------------
par_StripComments
--------------- */
char *par_StripComments( char* buffer )
{
	char *temp;
	int start, end;
	int a;

	temp = buffer;

	// stage 1:  single line quotes
	while( *temp ) 
	{
		start = tcstrnfind( temp, "//", 2, -1 );
		if( start == -1 ) start = tcstrlen(temp);
		temp += start;
		if( tcstrlen(temp) == 0 ) break;
		end = tcstrnfind( temp, "\n", 1, -1 );
		if( end < 0 ) end = tcstrlen(temp) - 1;
		for( a=0 ; a<(end+1) ; a++ )
			temp[a] = ' ';
	}
	
	// stage 2: multi-line quotes
	temp = buffer;
	while( *temp ) 
	{
		start = tcstrnfind( temp, "/*", 2, -1 );
		if( start == -1 ) start = tcstrlen(temp);
		temp += start;
		if( tcstrlen(temp) == 0 ) break;
		end = tcstrnfind( temp, "*/", 2, -1 );
		if( end == -1 ) end = tcstrlen(temp);
		for( a=0 ; a<(end+2) ; a++ )
			temp[a] = ' ';
	}

	return buffer;
}

/* ---------------
par_StripCarriageReturns
--------------- */
char *par_StripCarriageReturns( char *buffer )
{
	char *temp = buffer;

	// replace all \n with spaces
	while( *temp )
	{
		if( (*temp == '\n') )
			*temp = ' ';
		temp++;
	}

	return buffer;
}

/* ---------------
par_StripTabs
--------------- */
char *par_StripTabs( char *buffer )
{
	char *temp = buffer;

	// replace all \t with spaces
	while( *temp )
	{
		if( *temp == '\t' )
			*temp = ' ';
		temp++;
	}

	return buffer;
}

/* ---------------
par_StripQuotes
--------------- */
char *par_StripQuotes( char *buffer )
{
	char *temp = buffer;

	// replace all " with spaces
	while( *temp )
	{
		if( *temp == '\"' )
			*temp = ' ';
		temp++;
	}

	return buffer;
}

/* ---------------
par_StripChar
--------------- */
char *par_StripChar( char *buffer, char strip )
{
	char *temp = buffer;

	// replace all " with spaces
	while( *temp )
	{
		if( *temp == strip )
			*temp = ' ';
		temp++;
	}

	return buffer;
}

/* ---------------
par_ReplaceChar
--------------- */
char *par_ReplaceChar( char *buffer, char oldchar, char newchar )
{
	char *temp = buffer;

	while( *temp )
	{
		if( *temp == oldchar )
			*temp = newchar;
		temp++;
	}

	return buffer;
}

/* ---------------
par_ToChar
--------------- */
int par_ToChar( char *buffer, char end )
{
	int a = 0;
	while( *buffer )
	{
		if( *buffer != end )
		{
			return a;
		}
		buffer++;
		a++;
	}
	return -1;
}

/* ---------------
par_ToToken
--------------- */
int par_ToToken( char* buffer )
{
	int a = 0;
	while( *buffer )
	{
		if( (*buffer != ' ') && (*buffer != '\n') && (*buffer != '\t') )
		{
			return a;
		}
		buffer++;
		a++;
	}
	return -1;
}

/* ---------------
par_ToSpace
--------------- */
int par_ToSpace( char* buffer )
{
	int a = 0;
	while( *buffer )
	{
		if( (*buffer == ' ') || (*buffer == '\n') || (*buffer == '\t') )
		{
			return a;
		}
		buffer++;
		a++;
	}
	if( a > 0 ) return a + 1;
	return -1;
}

/* ---------------
par_BufferFile - creates and returns a memory buffer containing the character contents of the file at path
			   - returns NULL on failure
--------------- */
char *par_BufferFile( char *path )
{
	char *buffer = NULL;
	file_t *fp;
	int length;

	// open the file for reading
	fp = fs_open( path, "r" );
	if( fp == NULL ) return NULL;

	// allocate a buffer to hold the contents
	length = fs_GetFileLength( fp ) + 1;
	buffer = (char*)mem_alloc( length );
	if( buffer == NULL ) return NULL;
	memset( buffer, 0, length );
	buffer[length - 1] = '\0';

	// read over the contents and close
	fs_read( buffer, sizeof(char), length - 1, fp );
	fs_close( fp );

	return buffer;
}

/* ---------------
par_GetTokenValue - returns a buffer filled with the whitespace delimited value immediatly following the first instance of token
				  - returns NULL on failure
--------------- */
char *par_GetTokenValue( char *src, char *token, int token_length )
{
	char *value;
	int a, ret, len, index = 0;;

	// look at all of source
	while( *src )
	{
		// if the first character matches, test the string
		if( *src == token[0] )
		{
			// check to make sure this is the first character in a token
			if( par_IsWhiteSpace( *(src - 1) ) || !index )
			{
				// check to make sure that this sequence is the same length as the requested token
				len = par_ToSpace(src);
				if( len == token_length )
				{
					// do the copy if we think it may be the one
					if( tcstrncmp( src, token, token_length ) )
					{
						// we have found the string so advance to the next token
						src += token_length; // past the token

						// advance to the value
						ret = par_ToToken(src);
						if( ret < 1 ) return NULL;
						src += ret;

						// get the length
						len = par_ToSpace(src) + 1;

						// allocate a buffer for the value
						value = (char*)mem_alloc( sizeof(char) * len );
						if( value == NULL ) return NULL;
						memset( value, 0, sizeof(char) * len );

						// copy the value
						for( a = 0 ; a < len - 2 ; a++ )
						{
							value[a] = *src;
							src++;
						}
						value[len - 1] = '\0';

						return value;
					}
				}
			}
		}

		index++;
		src++;
	}

	return NULL;
}

/* ----------
par_GetNextToken - returns > 0 if we found a token, 0 otherwise.
				   If > 0 toToken is the offset to the token start and tokenLen is the length
				   n.b.  This only tells us where the token is and what it looks like, the user has to extract it
---------- */
int par_GetNextToken( char *buffer, int *toToken, int *tokenLen )
{
	char *start = buffer;

	// assert
	if( buffer == NULL ) return 0;
	if( toToken == NULL ) return 0;
	if( tokenLen == NULL ) return 0;

	// skip whitespace
	while( *buffer && (par_IsWhiteSpace(*buffer)) )
	{
		buffer++;
	}
	if( !*buffer ) return 0;

	// count the prespace
	*toToken = buffer - start;
	start = buffer;

	// read to the end of the token
	while( *buffer && (*buffer != ' ') && (*buffer != '\t') && (*buffer != '\n') && (*buffer != '\r') )
	{
		buffer++;
	}
	
	// count the string
	*tokenLen = buffer - start;
	if( *tokenLen <= 0 ) return 0;

	return 1;
}

/* ----------
par_IsWhiteSpace - returns > 0 if c is an ASCII character that represents 'whitespace'
				 - else returns 0
---------- */
byte par_IsWhiteSpace( const char c )
{
	if( (c == ' ') || (c == '\t') || (c == '\n') || (c == '\r') ) return 1;

	return 0;
}

/* ----------
par_QuotedStringCopy 
// copies the string in quotes from source to an unquoted string destination
// if src[0] != \", then it performs a standard string copy of only 1 token
// returns the number of characters advanced through src
---------- */
int par_QuotedStringCopy( char *dest, const char *src, int length )
{
	int cnt = 0;
	
	// do a standard string copy if we don't have a delimeter
	if( *src != '\"' ) 
	{
		while( *src && !par_IsWhiteSpace(*src) )
		{
			*dest++ = *src++;
			cnt++;
		}
		return cnt;
	}

	// advance past the first "
	src++;
	cnt++;

	// copy until we run out, overextend our destination, or hit the next quote
	while( length-- && *src && *src != '\"' )
	{
		*dest++ = *src++;
		cnt++;
	}
	if( length )  *dest = 0;

	// advance past the close quote
	if( *src == '\"' ) cnt++;

	return cnt;
}


