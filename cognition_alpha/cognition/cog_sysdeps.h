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


#else

// *NIX, BSD, OSX(?)
#include <stddef.h>
#include <inttypes.h>
#include <stdarg.h>
				// standard types and other *nix stuff

#	ifndef HAVE_UINT32
#		define HAVE_UINT32
#		define UINT32 uint32_t
#	endif

#	ifndef HAVE_UINT16
#		define HAVE_UINT16
#		define uint16_t uint16_t
#	endif

#	ifndef HAVE_UINT8
#		define HAVE_UINT8
#		define uint8_t uint8_t
#	endif

#	ifndef HAVE_INT32
#		define HAVE_INT32
#		define int32_t int32_t
#	endif

#	ifndef HAVE_INT16
#		define HAVE_INT16
#		define int16_t int16_t
#	endif

#	ifndef HAVE_INT8
#		define HAVE_INT8
#		define int8_t int8_t
#	endif

#	ifndef HAVE_BITMAPFILEHEADER
#		define HAVE_BITMAPFILEHEADER
		typedef struct tagBITMAPFILEHEADER
		{
			uint16_t bfType;
			uint32_t bfSize;
			uint16_t bfReserved1;
			uint16_t bfReserved2;
			uint32_t bfOffBits;
		} BITMAPFILEHEADER;
#	endif

#	ifndef HAVE_BITMAPINFOHEADER
#		define HAVE_BITMAPINFOHEADER
		typedef struct tagBITMAPINFOHEADER
		{
			uint32_t biSize;
			int32_t biWidth;
			int32_t biHeight;
			uint16_t biPlanes;
			uint16_t biBitCount;
			uint32_t biCompression;
			uint32_t biSizeImage;
			int32_t biXPelsPerMeter;
			int32_t biYPelsPerMeter;
			uint32_t biClrUsed;
			uint32_t biClrImportant;
		} BITMAPINFOHEADER;
#	endif

#	ifndef HAVE_RGBQUAD
#		define HAVE_RGBQUAD
		typedef struct tagRGBQUAD
		{
			uint8_t rgbBlue;
			uint8_t rgbGreen;
			uint8_t rgbRed;
			uint8_t rgbReserved;
		} RGBQUAD;
#	endif

#	ifndef HAVE_BITMAPINFO
#		define HAVE_BITMAPINFO
		typedef struct tagBITMAPINFO
		{
			BITMAPINFOHEADER bmiHeader;
			RGBQUAD bmiColors[1];
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
			int16_t   wFormatTag;
			int16_t   nChannels;
			int32_t   nSamplesPerSec;
			int32_t   nAvgBytesPerSec;
			int16_t   nBlockAlign;
			int16_t   wBitsPerSample;
			int16_t   cbSize;
		} WAVEFORMATEX;
#	endif
#endif // !WIN32

// all platforms
#include <stdlib.h>
				// for _fmode setting and malloc
#include <GL/gl.h>
				// primary draw functionality
#include <GL/glext.h>
				// OpenGL extensions up to 1.2
#include <GL/glu.h>
				// colormap.c -> gluBuild2DMipmaps
#include <stdio.h>
				//  *printf
#include <math.h> 
				// for sqrt, trig tbl and arc trig
#include <assert.h>
				// for debug assertions

#endif // _COG_SYSDEFS_H