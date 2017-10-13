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
// console.c
// by Terrence Cole 2-25-02

// Includes
/////////////
#include "cog_global.h"
#include "sv_console.h"
#include "cog_opengl.h"

// Global Prototypes
//////////////////////
/*
int con_Initialize();
int con_Terminate();
void con_Print( char *fmt, ... );
byte con_Toggle( char *null );
void con_KeyPress( unsigned char key, int st );
*/

// Local Prototypes
/////////////////////
// loading functionality
static int con_LoadCharsetData();

// printing functionality
static int con_pr_ResolveColors( char *buffer, char *output, char *line );
static char con_pr_ColorTagFromName( char *name );
static void con_pr_PrintSection( char *buffer );

// LOCAL STRUCTURES
////////////////////

// LOCAL VARIABLES
////////////////////


// *********** FUNCTIONALITY ***********
/* ------------
con_Initialize
------------ */
int con_Initialize(void)
{
	char buffer[1024];
	colormap_params_t cmParams;

	// assert
	if( console.inited ) con_Terminate();

	// tell us about it
	con_Print( "\nInitializing Console System..." );
	
	// setup the char sets
	memset( charSets, 0, sizeof(charSet_t) * CON_NUM_CHAR_SETS );
	
	tcstrcpy( charSets[0].name, "BLACK" );
	charSets[0].clrIn = 1;
	charSets[0].clrOut = (uint8_t)129;
	charSets[0].color[0] = 0;
	charSets[0].color[1] = 0;
	charSets[0].color[2] = 0;
	charSets[0].color[3] = 255;

	tcstrcpy( charSets[1].name, "WHITE" );
	charSets[1].clrIn = 2;
	charSets[1].clrOut = 130;
	charSets[1].color[0] = 255;
	charSets[1].color[1] = 255;
	charSets[1].color[2] = 255;
	charSets[1].color[3] = 255;

	tcstrcpy( charSets[2].name, "GRAY" );
	charSets[2].clrIn = 3;
	charSets[2].clrOut = 131;
	charSets[2].color[0] = 0xC0;
	charSets[2].color[1] = 0xC0;
	charSets[2].color[2] = 0xC0;
	charSets[2].color[3] = 255;

	tcstrcpy( charSets[3].name, "RED" );
	charSets[3].clrIn = 4;
	charSets[3].clrOut = 132;
	charSets[3].color[0] = 255;
	charSets[3].color[1] = 0;
	charSets[3].color[2] = 0;
	charSets[3].color[3] = 255;

	tcstrcpy( charSets[4].name, "ORANGE" );
	charSets[4].clrIn = 5;
	charSets[4].clrOut = 133;
	charSets[4].color[0] = 0xFF;
	charSets[4].color[1] = 0x55;
	charSets[4].color[2] = 0x00;
	charSets[4].color[3] = 255;

	tcstrcpy( charSets[5].name, "YELLOW" );
	charSets[5].clrIn = 6;
	charSets[5].clrOut = 134;
	charSets[5].color[0] = 255;
	charSets[5].color[1] = 255;
	charSets[5].color[2] = 0;
	charSets[5].color[3] = 255;

	tcstrcpy( charSets[6].name, "GREEN" );
	charSets[6].clrIn = 7;
	charSets[6].clrOut = 135;
	charSets[6].color[0] = 0;
	charSets[6].color[1] = 255;
	charSets[6].color[2] = 0;
	charSets[6].color[3] = 255;

	tcstrcpy( charSets[7].name, "BLUE" );
	charSets[7].clrIn = 11;
	charSets[7].clrOut = 136;
	charSets[7].color[0] = 0;
	charSets[7].color[1] = 0;
	charSets[7].color[2] = 255;
	charSets[7].color[3] = 255;

	tcstrcpy( charSets[8].name, "VIOLET" );
	charSets[8].clrIn = 12;
	charSets[8].clrOut = 137;
	charSets[8].color[0] = 0xEE;
	charSets[8].color[1] = 0x82;
	charSets[8].color[2] = 0xEE;
	charSets[8].color[3] = 255;
	
	tcstrcpy( charSets[9].name, "AQUA" );
	charSets[9].clrIn = 14;
	charSets[9].clrOut = 138;
	charSets[9].color[0] = 0;
	charSets[9].color[1] = 255;
	charSets[9].color[2] = 255;
	charSets[9].color[3] = 255;

	tcstrcpy( charSets[10].name, "MAGENTA" );
	charSets[10].clrIn = 16;
	charSets[10].clrOut = 139;
	charSets[10].color[0] = 255;
	charSets[10].color[1] = 0;
	charSets[10].color[2] = 255;
	charSets[10].color[3] = 255;
	
	tcstrcpy( charSets[11].name, "CORNFLOWERBLUE" );
	charSets[11].clrIn = 17;
	charSets[11].clrOut = 140;
	charSets[11].color[0] = 0x64;
	charSets[11].color[1] = 0x95;
	charSets[11].color[2] = 0xED;
	charSets[11].color[3] = 255;

	tcstrcpy( charSets[12].name, "FOUR" );
	charSets[12].clrIn = 18;
	charSets[12].clrOut = 141;
	charSets[12].color[0] = 14;
	charSets[12].color[1] = 0;
	charSets[12].color[2] = 98;
	charSets[12].color[3] = 255;

	tcstrcpy( charSets[13].name, "FIVE" );
	charSets[13].clrIn = 19;
	charSets[13].clrOut = 142;
	charSets[13].color[0] = 92;
	charSets[13].color[1] = 30;
	charSets[13].color[2] = 35;
	charSets[13].color[3] = 255;

	tcstrcpy( charSets[14].name, "SIX" );
	charSets[14].clrIn = 20;
	charSets[14].clrOut = 143;
	charSets[14].color[0] = 0;
	charSets[14].color[1] = 136;
	charSets[14].color[2] = 176;
	charSets[14].color[3] = 255;
	
	// setup the background
	state.bInConsole = 0;
	console.bg_pos[RECT_LEFT] = 0;
	console.bg_pos[RECT_RIGHT] = CON_WIDTH;
	console.bg_pos[RECT_TOP] = SCR_HEIGHT;
	console.bg_pos[RECT_BOTTOM] = SCR_HEIGHT - CON_HEIGHT;

	cmParams.bMipmap = 1;
	cmParams.bNoRescale = 0;
	cmParams.iEnv = GL_MODULATE;
	cmParams.iFilter = cm_GetGlobalFilter();
	cmParams.iWrap = GL_REPEAT;
	cmParams.iScaleBias = 0;
	
	console.cmBackground = cm_LoadConstAlpha( "console_back", "console_back", IMG_ALPHA_SOLID, &cmParams );
	if( console.cmBackground == NULL )
	{
		con_Print( "<RED>Console Init Error:  Load Console Background failed.</RED>" );
		return 0;
	}

	console.cmOverlay = cm_LoadFromFiles( "console_overlay", "console_back", "console_back_alpha", &cmParams );
	if( console.cmOverlay == NULL )
	{
		con_Print( "<RED>Console Init Error:  Load Console Overlay failed.</RED>" );
		return 0;
	}

	snprintf( buffer, 1024, "background%ld", rng_Long( 0, CON_NUM_BACKGROUNDS ) );
	console.cmLoading = cm_LoadConstAlpha( "console_loading", buffer, IMG_ALPHA_SOLID, &cmParams );
	if( console.cmLoading == NULL )
	{
		con_Print( "<RED>Console Init Error:  Load Console Loading Screen failed." );
		return 0;
	}

	console.cmLoadingOverlay = cm_LoadFromFiles( "console_loading_overlay", 
											"console_loading_overlay_color",
											"console_loading_overlay_alpha", &cmParams );
	if( console.cmLoadingOverlay == NULL )
	{
		con_Print( "<RED>Console Init Error:  Load Console Loading Screen Overlay failed." );
		return 0;
	}
	
	// load the charset
	cmParams.bMipmap = 1;
	cmParams.bNoRescale = 1;
	cmParams.iScaleBias = 0;
	cmParams.iEnv = GL_MODULATE;
	cmParams.iFilter = GL_LINEAR;
	cmParams.iWrap = GL_CLAMP;
	console.charset[0] = cm_LoadFromFiles( "charset0", "char_set_0", "char_set_mask_0", &cmParams );
	console.charset[1] = cm_LoadFromFiles( "charset1", "char_set_1", "char_set_mask_1", &cmParams );
	console.charset[2] = cm_LoadFromFiles( "charset2", "char_set_2", "char_set_mask_2", &cmParams );

	// load the info file
	con_LoadCharsetData();

	// clear scrollback and history lines
	memset( console.lines, 0, sizeof(char) * NUM_CON_LINES * CON_LINE_SIZE );
	memset( console.history, 0, sizeof(char) * NUM_CON_HISTORY_LINES * CON_HISTORY_LINE_SIZE );
	memset( console.input, 0, sizeof(char) * CON_HISTORY_LINE_SIZE );
	console.lastLine = 0;
	console.lineOffset = 0;
	console.lastHistory = 0;
	console.histOffset = -1;
	console.numIssuedCmds = 0;
	console.nextInput = 0;

	// set a border width and color
	console.borderWidth = gl_state.line_size_range[0];
	while( (console.borderWidth < DEF_CON_BORDER_WIDTH) && (console.borderWidth <= gl_state.line_size_range[1]) )
	{
		console.borderWidth += gl_state.line_size_step;
	}
	if( console.borderWidth > gl_state.line_size_range[1] )
	{
		console.borderWidth = gl_state.line_size_range[1];
	}
	console.borderColor[0] = 0;
	console.borderColor[1] = 0;
	console.borderColor[2] = 200;
	console.borderColor[3] = 215;

	console.cs_bnds_scale[0] = console.cs_bnds_scale[1] = CON_CHAR_WIDTH_SCALE_0_1;
	console.cs_bnds_scale[2] = CON_CHAR_WIDTH_SCALE_2;	

	// set inited flag to true
	console.inited = 1;

	return 1;
}


/* ------------
con_Terminate
------------ */
void con_Terminate(void)
{
	int a;
	
	console.inited = 0;

	con_Print( "\nTerminating Console..." );

	// unload the color char sets
	for( a = 0 ; a < 3 ; a++ )
	{
		cm_Unload( console.charset[a] );
		console.charset[a] = NULL;
	}

	// unload the background
	cm_Unload( console.cmBackground );
	cm_Unload( console.cmOverlay );
	cm_Unload( console.cmLoading );
	cm_Unload( console.cmLoadingOverlay );

	// clear history and scrollback
	memset( console.lines, 0, sizeof(char) * NUM_CON_LINES * CON_LINE_SIZE );
	memset( console.history, 0, sizeof(char) * NUM_CON_HISTORY_LINES * CON_HISTORY_LINE_SIZE );
	memset( console.input, 0, sizeof(char) * CON_HISTORY_LINE_SIZE );
	console.lastLine = 0;
	console.lineOffset = 0;
	console.lastHistory = 0;
	console.histOffset = -1;
	console.numIssuedCmds = 0;
	console.nextInput = 0;
}


/* ------------
con_LoadCharsetData - loads set data from charset\char_data_64.txt
------------ */
static int con_LoadCharsetData()
{
	int ret;
	int in_map, in_ascii, in_num, in_width, in_lo, in_hi;
	char *path;
	file_t *fp;
	char temp_str[16];

	con_Print( "\tLoading Console Character Set Data..." );

	// open and check the file for the character offset data
	path = fs_GetMediaPath( "char_data_64.tbl", "tables" );
	fp = fs_open( path, "r" );
	if( fp == NULL )
	{
		con_Print( "Console Character Set Load Failed.  Unable to open \"char_data_64.tbl\" translation table." );
		return 0;
	}

	// iterate through the file and copy data
	ret = 7;
	fs_rewind(fp);
	while( ret == 7 )
	{
		// Data:  map index, #, ascii #, width, character, start pixel, end pixel
		// Ex:	0	0	32	18	Sp	0	18
		ret = fscanf( fp->file, "%d %d %d %d %s %d %d\n", &in_map, &in_num, &in_ascii, &in_width, temp_str, &in_lo, &in_hi );

		if( (in_ascii >= NUM_ASCII_CHARS) || (in_ascii < 0) || (ret == EOF) )
		{
			if( fs_error(fp) )
			{
				con_Print( "<RED>Console Character Set Load Error:  Exiting ascii translation table prior to completion - out of bounds data index." ); 
				return 0;
			}
		}

		if( ret == 7 )
		{
			// get the map index for this character
			console.cs_bnds_map[in_ascii] = (byte)in_map;
			if( in_map != 2 )
			{
				console.cs_bnds_lo[in_ascii] = (float)in_lo / (float)CHARSET0_WIDTH;
				console.cs_bnds_hi[in_ascii] = (float)in_hi / (float)CHARSET0_WIDTH;
				console.cs_bnds_width[in_ascii] = (float)in_width / (float)CHARSET0_WIDTH;
			}
			else if( in_map == 2 )
			{
				console.cs_bnds_lo[in_ascii] = (float)in_lo / (float)CHARSET2_WIDTH;
				console.cs_bnds_hi[in_ascii] = (float)in_hi / (float)CHARSET2_WIDTH;
				console.cs_bnds_width[in_ascii] = (float)in_width / (float)CHARSET2_WIDTH;
			}
		}
		else
		{
			con_Print( "<RED>Console Char Set Data Load Error:  fscanf returned an incorrect number of tokens." );
		}
	}
	fs_close(fp);
		
	// copy over bitmap data to the console and initialize the texture
	console.cs_height = CON_CHAR_HEIGHT;
	console.cs_DownNumVisLines = (int)((float)CON_HEIGHT / console.cs_height);
	console.cs_FSNumVisLines = (int)((float)SCR_HEIGHT / console.cs_height);
	console.cs_maxLineOffset = NUM_CON_LINES - console.cs_DownNumVisLines;
	
	return 1;
}


/* ------------
con_Print
------------ */
void con_Print( char *fmt, ... )
{
	// con print is a big function, the top level breaks the input into managable chunks to break up for printing
	// con_pr_ResolveColors, iterates the input into a standard output and a console line, formats tabs, etc.  (NO LINE BREAKS)

	int a;
	int lastBreak, curBreak;
	int ret;
	va_list ap;
	char buffer[1024];  // FIXME:  is this long enough?
	char output[1024];
	char line[1024];

	// assert
	if( fmt == NULL ) return;
	if( tcstrlen(fmt) <= 0 ) return;

	// init
	memset( buffer, 0, sizeof(char) * 1024 );
	memset( output, 0, sizeof(char) * 1024 );
	memset( line, 0, sizeof(char) * 1024 );
	
	// decode the format string
	va_start( ap, fmt );
	vsnprintf( buffer, 1023, fmt, ap );  // just annoyingly different enough to make everyone angry
	buffer[1023] = '\0';
	va_end( ap );

	// find chuncks of the string to resolve colors for and print
	a = 0;
	lastBreak = 0;
	curBreak = 0;
	while( buffer[a] )
	{
		if( buffer[a] == '\n' )
		{
			// print this part of the string
			buffer[a] = '\0';

			// get colorized and uncolorized versions of the tagged section
			ret = con_pr_ResolveColors( &buffer[lastBreak], output, line );
			if( !ret )
			{
				// error printed in child function
				return;
			}

			// advance the line segment
			lastBreak = a + 1;

			// print the colorized string to the console
			con_pr_PrintSection( line );
			f_Log( output );
		}

		a++;
	}

	// print the last segment of the line (possibly the first)
	// we have some special cases:
	// the last character was a carriage return, so insert it into the string
	if( a == lastBreak )
	{
		con_pr_PrintSection( "\0" );
		f_Log( "\0" );
	}
	// we have a normal line ending and we need to print it
	else
	{
		// get colorized and uncolorized versions of the tagged section
		ret = con_pr_ResolveColors( &buffer[lastBreak], output, line );
		if( !ret )
		{
			// error printed in child function
			return;
		}

		// print the colorized string to the console
		con_pr_PrintSection( line );
		f_Log( output );
	}

	return;
}

/* ------------
con_pr_ResolveColors
------------ */
static int con_pr_ResolveColors( char *buffer, char *output, char *line )
{
	int a, b;
	int outPtr, linePtr;
	byte bInTag;
	
	// decode the tagged string to it's normal and keyed equivalents

	// we want to take our input string buffer (with tags <COLOR>....</COLOR>
	// and create 2 new strings, line with custom tags that our console drawer can
	// translate into color changes and a string output with no color info at all that we can
	// send straight to our file or screen logs

	// assert
	if( (buffer == NULL) || (output == NULL) || (line == NULL) ) 
	{
		con_Print( "Console Resolve Color Tags Failed:  A Required input buffer was NULL" );
		return 0;
	}
	
	outPtr = linePtr = 0;
	bInTag = 0;
	for( a = 0 ; (a < 1023) && (buffer[a]) && (outPtr < 1023) && (linePtr < 1019) ; a++ )
	{
		// expand tabs manually into line at this point
		if( buffer[a] == '\t' )
		{
			for( b = 0 ; b < CON_CHAR_TAB_SPACES ; b++ )
			{
				line[linePtr] = ' ';
				linePtr++;
			}
			// output gets a normal tab char
			output[outPtr] = '\t';
			outPtr++;
		}
		// check for escape on < and > signs
		else if( buffer[a] == '\\' )
		{
			if( (buffer[a + 1] == '<') )
			{
				output[outPtr] = line[linePtr] = '<';
				outPtr++;
				linePtr++;
				a++;
			}
			else if( (buffer[a + 1] == '>') )
			{
				output[outPtr] = line[linePtr] = '<';
				outPtr++;
				linePtr++;
				a++;
			}
			else
			{
				output[outPtr] = line[linePtr] = buffer[a];
				outPtr++;
				linePtr++;
			}
		}
		// enter a new color tag or close an open tag
		else if( (buffer[a] == '<' ) )
		{
			// ignore the rest of the tag
			bInTag = 1;
			
			// close an open tag
			if( buffer[a + 1] == '/' )
			{
				// assert - we don't want to crash on strings ending as "...</"
				if( a + 2 > 1023 )
				{
					con_Print( "<RED>Console Print Error:  Badly formated string.  Color format close ends string.  String too long?</RED>" );
					return 0;
				}
				
				line[linePtr] = CON_TAG_SPECIFIER;
				// n.b. closing color tags are useless but a necessary illusion/abstraction
				line[linePtr + 1] = con_pr_ColorTagFromName( "BLACK" ); 
				linePtr += 2;
			}
			// open a new tag
			else
			{
				line[linePtr] = CON_TAG_SPECIFIER;
				line[linePtr + 1] = con_pr_ColorTagFromName( &buffer[a + 1] );
				linePtr += 2;
			}
		}
		else if( (buffer[a] == '>') )
		{
			bInTag = 0;
		}
		else
		{
			if( !bInTag )
			{
				output[outPtr] = line[linePtr] = buffer[a];
				outPtr++;
				linePtr++;
			}
		}
	}

	// perpend an end to each string
	output[outPtr] = '\0'; // guaranteed <= 1023
	line[linePtr] = '\0';

	// assert -> if we end in a tag it is a bad thing
	if( bInTag )
	{
		con_Print( "<RED>Console Print Error:  The string ended inside of a color tag.  String too long?</RED>" );
		return 0;
	}

	// we are done with exchanging color tags
	return 1;
}

/* ------------
con_pr_ColorTagFromName
------------ */
static char con_pr_ColorTagFromName( char *name )
{
	int a;
	
	for( a = 0 ; a < CON_NUM_CHAR_SETS ; a++ )
	{
		if( tcstrincmp( charSets[a].name, name, tcstrlen(charSets[a].name) ) )
		{
			return charSets[a].clrIn;
		}
	}

	return charSets[0].clrIn;  // return a default color of black on unrecognized tags
}

/* ------------
con_pr_PrintSection
------------ */
static void con_pr_PrintSection( char *buffer )
{
	int a;
	
	// assert
	if( buffer == NULL ) return;

	// we may have to stop here
	if( !console.inited ) return; // not ready to print to the console

	// now insert line into the scrollback buffer
	console.lastLine++;
	if( (console.lastLine > NUM_CON_LINES) || (console.lastLine < 0) ) console.lastLine = 0;
	memset( console.lines[console.lastLine], 0, sizeof(char) * CON_LINE_SIZE );
	
	// copy over the line
	a = 0;
	while( *buffer )
	{
		console.lines[console.lastLine][a] = *buffer;
		buffer++;
		a++;
	}
}

/* ------------
con_Toggle
------------ */
void con_Toggle( char *null )
{
	if( state.bInConsole == 0 )
	{
		state.bInConsole = 1;
		console.bg_pos[RECT_BOTTOM] = SCR_HEIGHT - CON_HEIGHT;;
	}
	else
	{
		state.bInConsole = 0;
		console.bg_pos[RECT_BOTTOM] = SCR_HEIGHT - CON_HEIGHT;
	}
	
	// this part is quasi-constant
	console.bg_pos[RECT_LEFT] = 0;
	console.bg_pos[RECT_RIGHT] = SCR_WIDTH;
	console.bg_pos[RECT_TOP] = SCR_HEIGHT;
}

/* ------------
con_KeyPress
------------ */
void con_KeyPress( unsigned char key, int st )
{	
	static int shifted = 0;
	int tmp;

	// use a case statement to match non-alphanumeric and keypad keys to their actions
	switch( key )
	{
	case K_BACKSPACE:
		if( st <= 0 ) return;
		if( tcstrlen(console.input) <= 0 ) return;

		// do the backspace
		console.nextInput--;
		if( console.nextInput < 0 ) console.nextInput = 0;
		console.input[console.nextInput] = 0;
		return;
	
	case K_CAPSLOCK:
		if( st == 0 ) return;
		if( shifted == 0 ) {shifted = 1;}
		else {shifted = 0;}
		return;
	
	case K_ENTER:
	case KP_ENTER:
		if( st < 1 ) return;
		if( tcstrlen(console.input) <= 0 ) return;

		// insert the new command into the command history
		console.lastHistory++;
		console.histOffset = -1;
		if( (console.lastHistory >= NUM_CON_HISTORY_LINES) || (console.lastHistory <= 0) ) console.lastHistory = 0;
		memset( console.history[console.lastHistory], 0, sizeof(char) * CON_HISTORY_LINE_SIZE );
		tcstrcpy( console.history[console.lastHistory], console.input );

		// print the command
		con_Print( console.input );
		
		// execute the command
		com_Process( console.input );
		console.numIssuedCmds++;

		// clear the input line
		memset( console.input, 0, sizeof(char) * CON_HISTORY_LINE_SIZE );
		console.nextInput = 0;
		return;
	
	case K_LSHIFT:
	case K_RSHIFT:
		if( st > 0 ) {shifted = 1;}
		else {shifted = 0;}
		return;

	case K_PGDN:
		if( st < 1 ) return;
		console.lineOffset -= (console.cs_DownNumVisLines - 1);
		if( console.lineOffset < 0 ) console.lineOffset = 0;
		return;

	case K_PGUP:
		if( st < 1 ) return;
		console.lineOffset += (console.cs_DownNumVisLines - 1);
		if( console.lineOffset > console.cs_maxLineOffset ) console.lineOffset = console.cs_maxLineOffset;
		return;
	
	case K_DELETE:
		if( st == 0 ) return;
		memset( console.input, 0, sizeof(char) * CON_HISTORY_LINE_SIZE );
		return;

	case KP_MINUS:
		key = '-';
		break;

	case KP_PLUS:
		if( st <= 0 ) return;
		key = '+';
		break;

	case KP_MULTIPLY:
		key = '*';
		break;

	case KP_PERIOD:
		key = '.';
		break;
	
	case KP_7:
		key = '7';
		break;

	case KP_8:
		key = '8';
		break;

	case KP_9:
		key = '9';
		break;

	case KP_4:
		key = '4';
		break;

	case KP_5:
		key = '5';
		break;

	case KP_6:
		key = '6';
		break;

	case KP_1:
		key = '1';
		break;

	case KP_2:
		key = '2';
		break;

	case KP_3:
		key = '3';
		break;

	case KP_0:
		key = '0';
		break;

	case K_UP:
		if( st <= 0 ) return;

		if( (console.histOffset >= (NUM_CON_HISTORY_LINES - 1)) || (console.histOffset >= (console.numIssuedCmds - 1)) ) { return; }
		if( (console.histOffset == -1) && (console.numIssuedCmds == 0) ) { return; }

		// increase the history offset pointer
		console.histOffset++;

		// find the buffer insertion and lock it into range
		tmp = console.lastHistory - console.histOffset;
		while( tmp < 0 ) tmp += NUM_CON_HISTORY_LINES;
		while( tmp >= NUM_CON_HISTORY_LINES ) tmp -= NUM_CON_HISTORY_LINES;

		// do the copy
		memset( console.input, 0, sizeof(char) * CON_HISTORY_LINE_SIZE );
		tcstrcpy( console.input, console.history[tmp] );		
		console.nextInput = tcstrlen( console.input );

		return;

	case K_DOWN:
		if( st <= 0 ) return;

		if( console.histOffset <= -1 ) { return; }
		
		// decrement the history offset pointer
		console.histOffset--;

		if( console.histOffset < 0 )  // we want a new line to type in here
		{
			console.histOffset = -1;
			memset( console.input, 0, sizeof(char) * CON_HISTORY_LINE_SIZE );
			console.nextInput = 0;
			return;
		}

		// find the buffer insertion point and lock it to range
		tmp = console.lastHistory - console.histOffset;
		while( tmp < 0 ) tmp += NUM_CON_HISTORY_LINES;
		while( tmp >= NUM_CON_HISTORY_LINES ) tmp -= NUM_CON_HISTORY_LINES;

		// clear and copy
		memset( console.input, 0, sizeof(char) * CON_HISTORY_LINE_SIZE );
		tcstrcpy( console.input, console.history[tmp] );
		console.nextInput = tcstrlen( console.input );

		return;

	// the following keys as yet have no meaing and are not handled above
	case K_TAB:
	case K_F1:
	case K_F2:
	case K_F3:
	case K_F4:
	case K_F5:
	case K_F6:
	case K_F7:
	case K_F8:
	case K_F9:
	case K_F10:
	case K_F11:
	case K_F12:
	case K_SCROLLLOCK:
	case K_PAUSE:
	case K_END:
	case K_HOME:
	case K_INSERT:
	case K_LEFT:
	case K_RIGHT:
	case K_LALT:
	case K_RALT:
	case K_LCTRL:
	case K_RCTRL:
	case KP_NUMLOCK:
	case M_LEFT:
	case M_RIGHT:
	case M_MIDDLE:
	case M_WHEEL:
	case M_WHEELDOWN:
	case M_WHEELUP:
	case M_XAXIS:
	case M_YAXIS:
		return;
	}

	// don't do anything on up-strokes
	if( st < 1 ) return;

	// -- check the shift state --
	// most of the ascii keymap doesn't match a paradigm that makes any sense next to a keyboard
	// therefore, we have to manually map the shifted keys to thier ascii equivalents
	if( shifted )
	{
		// the alpha set is easy
		if( (key >= 'a') && (key <= 'z') ) key -= 32;
		
		// for the top row keys
		else if( key == '`' ) key = 126;
		else if( key == '1' ) key = 33;
		else if( key == '2' ) key = 64;
		else if( key == '3' ) key = 35;
		else if( key == '4' ) key = 36;
		else if( key == '5' ) key = 37;
		else if( key == '6' ) key = 94;
		else if( key == '7' ) key = 38;
		else if( key == '8' ) key = 42;
		else if( key == '9' ) key = 40;
		else if( key == '0' ) key = 41;
		else if( key == '-' ) key = 95;
		else if( key == '=' ) key = 43;

		// map the braces and v-bar in the second from top
		else if( key == '[') key = 91;
		else if( key == ']') key = 93;

		// map the middle row
		else if( key == ';' ) key = 58;
		else if( key == '\'') key = 34;

		// for the second from bottom
		else if( key == ',' ) key = 60;
		else if( key == '.' ) key = 62;
		else if( key == '/' ) key = 63;
	}

	// add the keypress
	if( console.nextInput >= CON_HISTORY_LINE_SIZE ) return;
	console.input[console.nextInput] = (char)key;
	console.nextInput++;
}
