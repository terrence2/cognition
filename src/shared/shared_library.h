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
// library.h
// Created 4-8-02 by Terrence Cole

#ifndef _LIBRARY_H
#define _LIBRARY_H

#include "shared_structures.h"

/* ------------
library.c
------------ */
int tcatoi( char* src );
float tcatof (char* src);
int tcstrcpy (char* dest, const char* src);
int tcstrncpy (char* dest, const char* src, int count);
short tcstrcmp( const char* str1, const char* str2 );
short tcstricmp( const char *str1, const char *str2 );
short tcstrncmp( const char* str1, const char* str2, int count);
short tcstrincmp( const char *str1, const char *str2, int count );
int tcstrlen( const char *str );
void tcstrcat (char* dest, const char* src);
void tcstrncat (char* dest, const char* src, int count );
int tcstrnfind (char* src, char* search, int size, int count);
char *tcDeepStrCpy( const char *str );
unsigned long int tcPositionToLong( float pos );
float tcLongToPosition( unsigned long int pos );
unsigned short int tcAngleToShort( float ang );
float tcShortToAngle( unsigned short int ang );
uint32_t tcReorder32( uint32_t data );
uint16_t tcReorder16( uint16_t data );

#endif // _LIBRARY_H
