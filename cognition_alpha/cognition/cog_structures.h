// Cognition
// cog_structures.h
// Created 04/08/02 @ 1115 by Terrence Cole

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
		int placeholder;
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
		int placeholder;
	} sys_handles_t;
#endif // WIN32


#endif // _COG_STRUCTURES_H

