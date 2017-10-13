// Cognition
// library.c
// Created 2-25-02 @ 0036 by Terrence Cole

// Includes
/////////////
#include "../Cognition/cog_global.h"

// Global Prototypes
//////////////////////
/*
int tcatoi( char* src );
float tcatof (char* src);
int tcstrcpy (char* dest, const char* src);
int tcstrncpy (char* dest, const char* src, int count);
short tcstrcmp( const char* str1, const char* str2 );
short tcstricmp( const char *str1, const char *str2 );
short tcstrncmp( const char* str1, const char* str2, int count);
short tcstrincmp( const char *str1, const char *str2, int count );
int tcstrlen (char* str);
void tcstrcat (char* dest, const char* src);
void tcstrncat (char* dest, const char* src, int count );
int tcstrnfind (char* src, char* search, int size, int count);
char *tcDeepStrCpy( const char *str );
uint32_t tcReorder32( uint32_t data );
uint16_t tcReorder16( uint16_t data );
*/

// *********** FUNCTIONALITY ***********
/* ------------
tcatoi
------------ */
int tcatoi( char* src )
{
	int val = 0;
	int sign;
	int c;

	if( *src == '-' )
	{
		sign = -1;
		src++;
	}
	else
	{
		sign = 1;
	}	

	// check for hex
	if( (src[0] == '0') && (src[1] == 'x' || src[1] == 'X') )
	{
		src += 2;
		for( ;; )
		{
			c = *src++;
			if( c >= '0' && c <= '9' )
				val = (val<<4) + c - '0';
			else if( c >= 'a' && c <= 'f' )
				val = (val<<4) + c - 'a' + 10;
			else if( c >= 'A' && c <= 'F' )
				val = (val<<4) + c - 'A' + 10;
			else
				return val*sign;
		}
	}
	
	// check for character
	if( src[0] == '\'' )
	{
		return sign * src[1];
	}
	
	// assume decimal
	for( ;; )
	{
		c = *src++;
		if( c <'0' || c > '9' )
			return val*sign;
		val = val*10 + c - '0';
	}
}

/* ------------
tcatof
------------ */
float tcatof (char* src)
{
	float	dest = 0.0f;
	char	sign = 0;
	int		decimal = -1;
	int		total = 0;
	char	c;
	
	// check the sign
	if( src[0] == '-' )
	{
		sign = -1;
		*src++;
	}
	else
		sign = 1;

	// now for the decimals
	for( ;; )
	{
		c = *src++;
		if( c == '.' )
		{
			decimal = total;
			continue;
		}
		if( (c>'9') || (c<'0') )
			break;
		dest = dest*10 + (c-'0');
		total++;
	}
	if( decimal == -1 )
		return dest*sign;
	while( total > decimal )
	{
		dest /= 10;
		total--;
	}

	return dest*sign;
}

/* ------------
tcstrcpy
// returns the number of characters copied from src to dest
------------ */
int tcstrcpy (char* dest, const char* src)
{
	int cnt = 0;

	assert( dest != NULL );
	assert( src != NULL );

	while( *src )
	{
		*dest++ = *src++;
		cnt++;
	}
	*dest++ = 0;

	return cnt;
}

/* ------------
tcstrncpy
// returns the number of characters copied from src to dest
------------ */
int tcstrncpy (char* dest, const char* src, int count)
{
	int cnt = 0;

	assert( dest != NULL );
	assert( src != NULL );
	
	while( *src && count-- )
	{
		*dest++ = *src++;
		cnt++;
	}
	if( count )	*dest++ = 0;

	return cnt;
}

/* ------------
tcstrcmp
------------ */
short tcstrcmp( const char* str1, const char* str2 )
{
	for( ;; )
	{
		if( *str1 != *str2 )
			return 0;
		if( !*str1 )
			return 1;
		str1++;
		str2++;
	}
}

/* ------------
tcstriecmp
------------ */
short tcstricmp( const char *str1, const char *str2 )
{
	char t1, t2;

	for( ;; )
	{
		// check for end conditions
		if( (!*str1) && (*str2) )
			return 0;
		if( (*str1) && (!*str2) )
			return 0;
		if( (!*str1) && (!*str2) )
			return 1;

		// convert str1 to lowercase
		if( (*str1 >= 65) && (*str1 <= 90) )
			t1 = (char)(*str1 + 32);
		else
			t1 = *str1;

		// convert str2 to lowercase
		if( (*str2 >= 65) && (*str2 <= 90) )
			t2 = (char)(*str2 + 32);
		else
			t2 = *str2;

		// make the comparison
		if( t1 != t2 )
			return 0;		

		str1++;
		str2++;
	}
}

/* ------------
tcstrncmp
------------ */
short tcstrncmp( const char* str1, const char* str2, int count)
{
	for( ;; )
	{
		if( !count-- )
			return 1;
		if( *str1 != *str2 )
			return 0;
		if( !*str1 )
			return 1;
		str1++;
		str2++;
	}
}

/* ------------
tcstrincmp
------------ */
// Upper Case 65 -> 90
// Lower Case 97 -> 122
// 32 byte offset
short tcstrincmp( const char *str1, const char *str2, int count )
{
	char t1, t2;

	for( ;; )
	{	// is the count finished
		if( !count-- )
			return 1;
		// is str1 empty
		if( !*str1 )
			return 1;
		// convert str1 to lowercase
		if( (*str1 >= 65) && (*str1 <= 90) )
			t1 = (char)(*str1 + 32);
		else
			t1 = *str1;
		// convert str2 to lowercase
		if( (*str2 >= 65) && (*str2 <= 90) )
			t2 = (char)(*str2 + 32);
		else
			t2 = *str2;

		// make the comparison
		if( t1 != t2 )
			return 0;

		str1++;
		str2++;
	}
	
}

/* ------------
tcstrlen
------------ */
int tcstrlen( const char *str )
{
	int i = 0;
	while(str[i])
		i++;
	return i;
}

/* ------------
tcstrcat
------------ */
void tcstrcat (char* dest, const char* src)
{
	dest += tcstrlen(dest);
	tcstrcpy( dest, src );
}

/* ------------
tcstrncat
------------ */
void tcstrncat (char* dest, const char* src, int count)
{
	dest += tcstrlen(dest);
	tcstrncpy( dest, src, count );
}

/* ------------
tcstrnfind
------------ */
int tcstrnfind (char* src, char* search, int size, int count)
{
	int c = 0;
	while (*src) {
		if (!count--)
			return -1;
		if (tcstrncmp(src, search, size))
			return c;
		*src++;
		c++;
	}
	return -1;
}

/* ------------
tcDeepStrCpy
------------ */
char *tcDeepStrCpy( const char *str )
{
	unsigned int len;
	char *cpy;

	// assert we have a string
	if( str == NULL ) return NULL;

	// assert the string is non-null
	len = tcstrlen(str);
	if( len <= 0 ) return NULL;
	
	cpy = (char*)ei->mem_alloc( sizeof(char) * (len + 1) );
	if( cpy == NULL ) 
	{
		ei->con_Print( "<RED>tcDeepStrCpy Failed:  memory allocation failed for new string for length %d.", len + 1 );
		return NULL;
	}
	memset( cpy, 0, sizeof(char) * (len + 1) );
	memcpy( cpy, str, len );

	return cpy;
}

// map Negative from 0 to 2^31
// map Positive from 2^31 to 2^32 - 1

// we want to preserve some decimals:
// offset in base10 by 4 places right, then recenter
// this gives us a maximum floating (int) range of +/-2^31/10^4
// so, max floating (int) is 214748.3647

#define PTL_MAX_POSITION_BNDS 214748.3647
#define ATS_MAX_ANGLE_BNDS 360.0
#define ATS_ASPECT_RATIO 160.0f // ~< (2^16 / 360) has an exact decimal inverse!
#define STA_ASPECT_RATIO 0.00625 // = exactly 1 / 160

/* ------------
tcPositionToLong - takes a floating position to a long
------------ */
unsigned long int tcPositionToLong( float pos )
{
	// assert
	if( pos <= -PTL_MAX_POSITION_BNDS ) return 0;
	if( pos >= PTL_MAX_POSITION_BNDS ) return MAX_UINT;
	
	return (unsigned long int)((double)pos * 10000.0) + MAX_INT;
}

/* ------------
tcLongToPosition - takes a positional long to a float
------------ */
float tcLongToPosition( unsigned long int pos )
{
	if( pos > MAX_INT ) return (float)((double)(pos - MAX_INT) / 10000.0);
	else return -(float)((double)(MAX_INT - pos) / 10000.0);
}

/* ------------
tcAngleToShort - 0 to 65536 gives 182+ pts of resolution to each degree
------------ */
unsigned short int tcAngleToShort( float ang )
{
	// snap our angles to 0 to 360
	while( ang > 360.0f ) ang -= 360.0f;
	while( ang <= 0.0f ) ang += 360.0f;

	return (unsigned short int)( ang * ATS_ASPECT_RATIO );
}

/* ------------
tcShortToAngle - returns a short angle to a float angle using the inverse transform
------------ */
float tcShortToAngle( unsigned short int ang )
{
	return (float)( (float)ang * STA_ASPECT_RATIO );
}

/* ------------
tcReorder32 
// this converts from LITTLE_ENDIAN to BIG_ENDIAN and vice versa
------------ */
uint32_t tcReorder32( uint32_t data )
{
	uint32_t iOut = 0;

	// this is a slightly different form of this transform from the standard one i've seen
	// 1, 2, 3, 4 -> 4, 3, 2, 1;  instead of adding in, we mask then or it in
	iOut = data >> 24;
	iOut |= (data >> 8) & 0x0000FF00;
	iOut |= (data << 8) & 0x00FF0000;
	iOut |= (data << 24) & 0xFF000000;
	return iOut;
}

/* ------------
tcReorder16
// this converts from LITTLE_ENDIAN to BIG_ENDIAN and vice versa
------------ */
uint16_t tcReorder16( uint16_t data )
{
	uint16_t sOut = 0;

	sOut = data >> 8;
	sOut |= (data << 8) & 0xFF00;
	return sOut;
}

