// sv_window.h
// Cognition
// globals for window tracking and window drawing

#ifndef _SV_WINDOW_H
#define _SV_WINDOW_H

// INCLUDES
/////////////
#include "cog_global.h"

// GLOBAL DEFINES
///////////////////
#define WIN_MAX_CHILDREN 512
#define WIN_MAX_WINDOWS 256

// GLOBAL STRUCTS
///////////////////
typedef struct win_struct_s
{
	// flags
	int bInUse;
	int bHidden;
	int bBorder;
	int bFilled;
	int bTopmost;
	int bMouseOverLastFrame;

	// properties
	int x, y, w, h;
	int left, right, top, bottom;
	vec2 corners[4]; // for quick drawing
	uint8_t color[4];
	void *data;

	// callbacks
	void (*keyevent)( void *data, uint8_t key, uint8_t key_state );
	void (*mousein)( void *data, int x, int y );
	void (*mouseout)( void *data, int x, int y );
	void (*draw)( void *data );

	// linkings
	struct win_struct_s *parent;
	struct win_struct_s *children[WIN_MAX_CHILDREN];
	int iNumChildren;
} win_struct_t;

// GLOBAL VARS
///////////////
colormap_t *cmBorder;
colormap_t *cmFilled;
win_struct_t windows[WIN_MAX_WINDOWS];
window_t winRoot;
win_struct_t *wsRoot;
colormap_t *cmBorder;
colormap_t *cmFiller;
const static vec2 win_tc0 = { 0.0f, 0.0f };
const static vec2 win_tc1 = { 1.0f, 0.0f };
const static vec2 win_tc2 = { 1.0f, 1.0f };
const static vec2 win_tc3 = { 0.0f, 1.0f };


#endif // _SV_WINDOW_H

