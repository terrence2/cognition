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
// dr_text.c
// Created 3/20/03 by Terrence Cole

// Purpose:  holds general purpose drawing methods that we'll want to do in a number of different circumstances

// Includes
/////////////
#include "cog_global.h"
#include "sv_console.h"

// Definitions
////////////////
#define DEF_ADVANCE 2.0f
#define DEF_DESCENT -2.0f
#define DEF_INSPACE 2.0f

// Local Structures
/////////////////////

// Global Prototypes
//////////////////////
/*
void d_TextLineBlack( char *line, float font_size, float pos_x, float pos_y );
void d_TextLineWhite( char *line, float font_size, float pos_x, float pos_y );
void d_TextLineColor( char *line, float font_size, byte color[4], float pos_x, float pos_y );
float d_GetTextWidth( char *text, float font_size );
float d_GetTextHeight( float font_size );
float d_GetCharWidth( char c, float font_size );
*/

// Local Prototypes
/////////////////////
static float d_TextChar( char c, float font_size, float xScale, float pos_x, float pos_y );
static int d_txt_ClrIndexFromKey( char c );

// Local Variables
////////////////////

// *********** FUNCTIONALITY ***********
/* ------------
d_TextLine - draws one line of text at pos_x, pos_y with a font of font_size
------------ */
void d_TextLineBlack( char *line, float font_size, float pos_x, float pos_y )
{
	int a;
	int nextColor;
	int lastMap = 0;
	int length;
	float pos;

	// sanity check
	if( !line ) return;
	if( font_size < 0 ) return;

	// set a base independent of advance and descent
	pos_x += DEF_ADVANCE * font_size / 12.0f;
	pos_y += DEF_DESCENT * font_size / 12.0f;
	
	// compute length and start position
	length = tcstrlen( line );
	pos = pos_x;

	// sanity check
	if( length <= 0 ) return;

	// set the default render properties
	SAFE_BIND( console.charset[lastMap] );
	glColor4ubv( charSets[CON_CS_BLACK].color );

	for( a=0 ; a < length ; a++ )
	{
		if( (line[a] == CON_TAG_SPECIFIER) )
		{
			nextColor = (byte)d_txt_ClrIndexFromKey( line[a + 1] );
			a += 2;
			glColor4ubv( charSets[nextColor].color );
		}
		
		if( (console.cs_bnds_map[(int)line[a]] != lastMap) )
		{
			lastMap = console.cs_bnds_map[(int)line[a]];
			SAFE_BIND( console.charset[lastMap] );
		}

		pos += d_TextChar( line[a], font_size, console.cs_bnds_scale[console.cs_bnds_map[(int)line[a]]], pos, pos_y ) + DEF_INSPACE;
	}
}

/* ------------
d_TextInvertedLine - just like d_TextLine only white based
------------ */
void d_TextLineWhite( char *line, float font_size, float pos_x, float pos_y )
{
	int a;
	int nextColor;
	int lastMap = 0;
	int length;
	float pos;

	// sanity check
	if( !line ) return;
	if( font_size < 0 ) return;

	// set a base independent of advance and descent
	pos_x += DEF_ADVANCE * font_size / 12.0f;
	pos_y += DEF_DESCENT * font_size / 12.0f;
	
	// compute length and start position
	length = tcstrlen( line );
	pos = pos_x;

	// sanity check
	if( length <= 0 ) return;

	// set the default render properties
	SAFE_BIND( console.charset[lastMap] );
	glColor4ubv( charSets[CON_CS_WHITE].color );
	
	for( a=0 ; a < length ; a++ )
	{
		if( (line[a] == CON_TAG_SPECIFIER) )
		{
			nextColor = (byte)d_txt_ClrIndexFromKey( (int)line[a + 1] );
			a += 2;
			if( nextColor == CON_CS_BLACK ) nextColor = CON_CS_WHITE;
			glColor4ubv( charSets[nextColor].color );
		}
		
		if( (console.cs_bnds_map[(int)line[a]] != lastMap) )
		{
			lastMap = console.cs_bnds_map[(int)line[a]];
			SAFE_BIND( console.charset[lastMap] );
		}
	
		pos += d_TextChar( line[a], font_size, console.cs_bnds_scale[console.cs_bnds_map[(int)line[a]]], pos, pos_y ) + DEF_INSPACE;
	}
}

/* ------------
d_ColorText - draws the entire line in the specified color as with the other text functions, without embedded tags
------------ */
void d_TextLineColor( char *line, float font_size, byte color[4], float pos_x, float pos_y )
{
	int a;
	int lastMap = 0;
	int length;
	float pos;

	// sanity check
	if( !line ) return;
	if( font_size < 0 ) return;

	// set a base independent of advance and descent
	pos_x += DEF_ADVANCE * font_size / 12.0f;
	pos_y += DEF_DESCENT * font_size / 12.0f;
	
	// compute length and start position
	length = tcstrlen( line );
	pos = pos_x;

	// sanity check
	if( length <= 0 ) return;

	// bind in a default
	SAFE_BIND( console.charset[lastMap] );
	glColor4ubv( color );

	for( a = 0 ; a < length ; a++ )
	{
		// check for a transition to a new character section
		if( (console.cs_bnds_map[(int)line[a]] != lastMap) )
		{
			lastMap = console.cs_bnds_map[(int)line[a]];
			SAFE_BIND( console.charset[lastMap] );
		}

		// draw the characters
		pos += d_TextChar( line[a], font_size, console.cs_bnds_scale[console.cs_bnds_map[(int)line[a]]], pos, pos_y ) + DEF_INSPACE;
	}
}

/* ------------
d_TextChar - draws one character, with the specified font, and scale, at position x,y
			- returns the chararacter width of the just drawn character
------------ */
static float d_TextChar( char c, float font_size, float xScale, float pos_x, float pos_y )
{
	float width, height;

	width = console.cs_bnds_width[(int)c] * xScale * font_size / 12.0f;
	height = console.cs_height * font_size / 12.0f; // pre scaled

	glBegin( GL_QUADS );

		glTexCoord2f( console.cs_bnds_lo[(int)c], 0 );
		glVertex2f( pos_x, pos_y );

		glTexCoord2f( console.cs_bnds_hi[(int)c], 0 );
		glVertex2f( pos_x + width, pos_y );

		glTexCoord2f( console.cs_bnds_hi[(int)c], 1 );
		glVertex2f( pos_x + width, pos_y + height );

		glTexCoord2f( console.cs_bnds_lo[(int)c], 1 );
		glVertex2f( pos_x, pos_y + height );

	glEnd();

	return width;
}

/* ------------
d_txt_ClrIndexFromKey - takes an embedded color key and returns the color index of that key
------------ */
static int d_txt_ClrIndexFromKey( char c )
{
	int a;
	
	for( a = 0 ; a < CON_NUM_CHAR_SETS ; a++ )
	{
		if( c == charSets[a].clrIn ) return a;
	}

	return 0;
}

/* ------------
d_GetTextWidth - returns the width of the given string
------------ */
float d_GetTextWidth( char *text, float font_size )
{
	int a;
	int length;
	float fTotWidth = 0.0f;

	if( !text ) return 0;

	// iterate the string
	length = tcstrlen( text );
	for( a = 0 ; a < length ; a++ )
	{
		// FIXME:  constrain text[a] to values that won't crash us
		fTotWidth += DEF_INSPACE + (console.cs_bnds_width[(int)text[a]] * console.cs_bnds_scale[console.cs_bnds_map[(int)text[a]]] * font_size / 12.0f);
	}

	return fTotWidth;
}

/* ------------
d_GetTextHeight - returns the height of the text at the given size
------------ */
float d_GetTextHeight( float font_size )
{
	return console.cs_height * font_size / 12.0f; // pre scaled
}

/* ------------
d_GetCharWidth
------------ */
float d_GetCharWidth( char c, float font_size )
{
	return DEF_INSPACE + (console.cs_bnds_width[(int)c] * console.cs_bnds_scale[console.cs_bnds_map[(int)c]] * font_size / 12.0f);
}
