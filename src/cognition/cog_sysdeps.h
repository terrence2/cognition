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
// cog_sysdeps.h
// created 9-7-03 @ 1654 by Terrence Cole

#ifndef _COG_SYSDEFS_H
#define _COG_SYSDEFS_H

// Global Includes
////////////////////
#ifdef WIN32

// Windows
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
				// standard defs and system stuff
#include <winsock2.h>
				// ntohs/l everywhere and networking stuff (isnt in lean & mean)

#define PACKED

#else

// *NIX, BSD, OSX(?)
#include <stddef.h>
#include <inttypes.h>
#include <stdarg.h>
				// standard types and other *nix stuff
#include <string.h>
        // memset and friends
#include <netinet/in.h>
        // ntohl and friends
#include <SDL.h>
        // all of sdl

#define PACKED __attribute__((__packed__))

#	ifndef HAVE_UINT32
#		define HAVE_UINT32
#		define UINT32 uint32_t
#	endif

#	ifndef HAVE_UINT16
#		define HAVE_UINT16
#		define UINT16 uint16_t
#	endif

#	ifndef HAVE_UINT8
#		define HAVE_UINT8
#		define UINT8 uint8_t
#	endif

#	ifndef HAVE_INT32
#		define HAVE_INT32
#		define INT32 int32_t
#	endif

#	ifndef HAVE_INT16
#		define HAVE_INT16
#		define INT16 int16_t
#	endif

#	ifndef HAVE_INT8
#		define HAVE_INT8
#		define INT8 int8_t
#	endif

#	ifndef HAVE_BITMAPFILEHEADER
#		define HAVE_BITMAPFILEHEADER
		typedef struct tagBITMAPFILEHEADER
		{
			uint16_t bfType PACKED;
			uint32_t bfSize PACKED;
			uint16_t bfReserved1 PACKED;
			uint16_t bfReserved2 PACKED;
			uint32_t bfOffBits PACKED;
		} BITMAPFILEHEADER;
#	endif

#	ifndef HAVE_BITMAPINFOHEADER
#		define HAVE_BITMAPINFOHEADER
		typedef struct tagBITMAPINFOHEADER
		{
			uint32_t biSize __attribute__((__packed__));
			int32_t biWidth __attribute__((__packed__));
			int32_t biHeight __attribute__((__packed__));
			uint16_t biPlanes __attribute__((__packed__));
			uint16_t biBitCount __attribute__((__packed__));
			uint32_t biCompression __attribute__((__packed__));
			uint32_t biSizeImage __attribute__((__packed__));
			int32_t biXPelsPerMeter __attribute__((__packed__));
			int32_t biYPelsPerMeter __attribute__((__packed__));
			uint32_t biClrUsed __attribute__((__packed__));
			uint32_t biClrImportant __attribute__((__packed__));
		} BITMAPINFOHEADER;
#	endif

#	ifndef HAVE_RGBQUAD
#		define HAVE_RGBQUAD
		typedef struct tagRGBQUAD
		{
			uint8_t rgbBlue __attribute__((__packed__));
			uint8_t rgbGreen __attribute__((__packed__));
			uint8_t rgbRed __attribute__((__packed__));
			uint8_t rgbReserved __attribute__((__packed__));
		} RGBQUAD;
#	endif

#	ifndef HAVE_BITMAPINFO
#		define HAVE_BITMAPINFO
		typedef struct tagBITMAPINFO
		{
			BITMAPINFOHEADER bmiHeader __attribute__((__packed__));
			RGBQUAD bmiColors[1] __attribute__((__packed__));
		} BITMAPINFO;
#	endif

#define BI_RGB 0
#define BI_RLE8 1
#define BI_RLE4 2
#define BI_BITFIELDS 3

#	ifndef HAVE_WAVEFORMATEX
#		define HAVE_WAVEFORMATEX
		typedef struct
		{
			int16_t   wFormatTag __attribute__((__packed__));
			int16_t   nChannels __attribute__((__packed__));
			int32_t   nSamplesPerSec __attribute__((__packed__));
			int32_t   nAvgBytesPerSec __attribute__((__packed__));
			int16_t   nBlockAlign __attribute__((__packed__));
			int16_t   wBitsPerSample __attribute__((__packed__));
			int16_t   cbSize __attribute__((__packed__));
		} WAVEFORMATEX;
#	endif
#endif // !WIN32

// all platforms
#include <stdlib.h>
				// for _fmode setting and malloc
#include <GL/glew.h>
				// primary draw functionality
//#include <GL/glext.h>
				// OpenGL extensions up to 1.2
//#include <GL/glu.h>
				// colormap.c -> gluBuild2DMipmaps
#include <stdio.h>
				//  *printf
#include <math.h> 
				// for sqrt, trig tbl and arc trig
#include <assert.h>
				// for debug assertions

#endif // _COG_SYSDEFS_H
