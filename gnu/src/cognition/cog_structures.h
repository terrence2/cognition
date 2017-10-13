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
// cog_structures.h
// Created 04/08/02 @ 1115 by Terrence Cole

#include "cog_sysdeps.h"

#ifndef _COG_STRUCTURES_H
#define _COG_STRUCTURES_H

/****** ENGINE INTERNAL MANAGEMENT STRUCTS ******/
typedef struct file_s
{
	FILE *file;
	char *signature;
	unsigned long int startOffset;
	unsigned long int endOffset;
	unsigned long int offset;
	byte inPack;
} file_t;

#define BS_BUFFER_SIZE 64
typedef struct bitstream_s
{
	file_t *fIn;
	byte bBuffer[BS_BUFFER_SIZE]; // store 64 static bytes

	// bit markers
	short bitsMax; // the maximum bits available in the buffer
	short bitsRemaining; // the number of bits currently in the buffer

	// floating position indicators
	byte *bytePtr; // buffer + [0,63] describing the positional offset in buffer (current position, next read)
	byte bitPtr;  // [0,7] describing positional offset in bitBuf (current position, next read)

	// states
	byte bIsEmpty;
	byte bHasLastByte;
	byte bInError;

	// list
	struct bitstream_s *next;
	struct bitstream_s *prev;
} bitstream_t;

// DISPLAY
#ifdef WIN32
	typedef struct display_s
	{
		int x, y;
		int w, h;
		char winClassName[64];
		WNDPROC WinProc;
		DWORD winClassStyle;
		DWORD winStyle;
		DWORD winStyleEx;
		HINSTANCE winOwnerInstance;
		HICON hIcon;
		HCURSOR hCursor;
		HBRUSH hBackBrush;
		HWND hWnd;
		HDC glDC;
		HGLRC glRC;
		int gl_mode;
		int nCmdShow;
	} display_t;
#else
	typedef struct display_s
	{
        int w, h;
        SDL_Surface* sdl_surface;
        Uint32 sdl_flags;
	} display_t;
#endif // WIN32

// SYSTEM HANDLES
#ifdef WIN32
	typedef struct
	{
		HINSTANCE cInstance;
		HINSTANCE cMenu;
		HINSTANCE cClient;
		HINSTANCE cServer;
		WNDPROC cWndProc;
		HWND cMouseCap;
		char *cmdLine;
		int nCmdShow;
	} sys_handles_t;
#else
	typedef struct sys_handles_s
	{
		void* cMenu;
        void* cClient;
        void* cServer;
        char* cmdLine;
	} sys_handles_t;
#endif // WIN32


#endif // _COG_STRUCTURES_H
