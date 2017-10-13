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

// Surface To Air
// console.h
// by Terrence Cole 2-25-02

#ifndef _CONSOLE_H
#define _CONSOLE_H

// Definitions
////////////////
// console drawing
#define CON_WIDTH 1024
#define CON_HEIGHT 384

#define CON_UP 1
#define CON_DOWN 2

#define NUM_CON_UP_LINES 3

#define DEF_CON_BORDER_WIDTH 2

#define CHARSET0_WIDTH 2048
#define CHARSET1_WIDTH 2048
#define CHARSET2_WIDTH 512
#define CHARSET_HEIGHT 64

// character set
#define NUM_ASCII_CHARS 256
#define CON_CHAR_TAB_SPACES 5
#define CON_CHAR_HEIGHT 15.0f
#define CON_CHAR_WIDTH_SCALE_0_1 440.0f
#define CON_CHAR_WIDTH_SCALE_2 CON_CHAR_WIDTH_SCALE_0_1 / 4.0f

// history lines
#define NUM_CON_HISTORY_LINES 32
#define CON_HISTORY_LINE_SIZE 128

#define NUM_CON_LINES 256
#define CON_LINE_SIZE 256

// Structures
///////////////
typedef struct
{
	// state data
	float bg_pos[4];
	byte inited;
	float borderWidth;
	byte borderColor[4];

	// background
	colormap_t *cmBackground;
	colormap_t *cmOverlay;
	colormap_t *cmLoading;
	colormap_t *cmLoadingOverlay;
	colormap_t *charset[3];

	byte cs_bnds_map[NUM_ASCII_CHARS];
	float cs_bnds_lo[NUM_ASCII_CHARS];
	float cs_bnds_hi[NUM_ASCII_CHARS];
	float cs_bnds_width[NUM_ASCII_CHARS];
	float cs_bnds_scale[3];
	
	float cs_height;
	int cs_DownNumVisLines;
	int cs_FSNumVisLines;
	int cs_maxLineOffset;

	// the scrollback buffer
	char lines[NUM_CON_LINES][CON_LINE_SIZE];
	int lastLine;
	int lineOffset;

	// the command history
	char history[NUM_CON_HISTORY_LINES][CON_HISTORY_LINE_SIZE];
	int lastHistory;
	int histOffset;
	int numIssuedCmds;

	// the current input buffer
	char input[CON_HISTORY_LINE_SIZE];
	int nextInput;
} console_state_t;

#define CS_NAME_SIZE 16
typedef struct charSet_s
{
	char name[CS_NAME_SIZE];
	uint8_t clrIn;
	uint8_t clrOut;
	byte color[4];
} charSet_t;

// color information
#define CON_NUM_CHAR_SETS 15
// tag entry point
#define CON_TAG_SPECIFIER 21

charSet_t charSets[CON_NUM_CHAR_SETS];
/*
= {
	{ "BLACK", 1, 129, { 0, 0, 0 }, NULL },
	{ "WHITE", 2, 130, { 255, 255, 255 }, NULL },
	{ "GRAY", 3, 131, { 0xC0, 0xC0, 0xC0 }, NULL },
	
	{ "RED", 4, 132, { 255, 0, 0 }, NULL },
	{ "ORANGE", 5, 133, { 0xFF, 0xA5, 0x00 }, NULL },
	{ "YELLOW", 6, 134, { 255, 255, 0 }, NULL },
	{ "GREEN", 7, 135, { 0, 255, 0 }, NULL },
	{ "BLUE", 8, 136, { 0, 0, 255 }, NULL },
	{ "VIOLET", 9, 137, { 0xEE, 0x82, 0xEE }, NULL },

	{ "AQUA", 10, 138, { 0, 255, 255 }, NULL },
	{ "MAGENTA", 11, 139, { 255, 0, 255 }, NULL },

	{ "CORNFLOWERBLUE", 12, 140, { 0x64, 0x95, 0xED }, NULL },

	{ "FOUR", 13 141, { 14, 0, 98 }, NULL },
	{ "FIVE", 14 142, { 92, 30, 35 }, NULL },
	{ "SIX", 15 143, { 0, 136, 176 }, NULL },
};
*/
// Header Variables
/////////////////////


// the global console
console_state_t console;


#endif // _CONSOLE_H
