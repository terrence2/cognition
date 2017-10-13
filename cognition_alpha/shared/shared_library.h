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
