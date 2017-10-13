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
// targa.c
// Created 4-11-02 @ 1030 by Terrence Cole

// Includes
/////////////
#include "cog_global.h"

// Local Definitions
//////////////////////
#define TGA_COLORMAP 2
#define TGA_GRAYSCALE 3
#define TGA_RLE_COLORMAP 10
#define TGA_RLE_GRAYSCALE 11

// Global Definitions
///////////////////////

// Local Structures
/////////////////////
/*
*/
typedef struct tagTARGAFILEHEADER
{
	byte imageIDLength __attribute__((__packed__));  // size of additional data after header
	byte colorMapType __attribute__((__packed__));  // always 0
	byte imageTypeCode __attribute__((__packed__));  // uncompressed BGR(A) is 2 ; uncompressed grayscale is 3
	short int colorMapOrigin __attribute__((__packed__));  // always 0
	short int colorMapLength __attribute__((__packed__));  // always 0
	byte colorMapEntrySize __attribute__((__packed__)); // always 0
	short int imageXOrigin __attribute__((__packed__)); // always 0
	short int imageYOrigin __attribute__((__packed__)); // always 0
	short int imageWidth __attribute__((__packed__)); // width of image in pixels (lo-hi)
	short int imageHeight __attribute__((__packed__)); // height of image in pixels (lo-hi)
	byte bitCount __attribute__((__packed__)); // bpp - 16, 24, or 32
	byte imageDescriptor __attribute__((__packed__)); // 24 bit = 0x00;  32 bit = 0x08
} TARGAFILEHEADER;

// Global Prototypes
//////////////////////
/*
byte *tga_Load( char *path, unsigned int *width, unsigned int *height, byte *bpp );
*/

// Local Prototypes
/////////////////////
static byte *tga_LoadFromFile( char *name, TARGAFILEHEADER **info );


// Local Variables
////////////////////

// *********** FUNCTIONALITY ***********
/* ------------
tga_Load
------------ */
byte *tga_Load( char *path, unsigned int *width, unsigned int *height, byte *bpp )
{
	byte *bits;
	TARGAFILEHEADER *info = NULL;
	int a, i, iNumPix;
	byte bTmp;

	// assert
	if( path == NULL )
	{
		con_Print( "<RED>TGA Load Assert Failed:  path is NULL" );
		eng_Stop( "50003" );
		return NULL;
	}
	if( width == NULL )
	{
		con_Print( "<RED>TGA Load Assert Failed:  width is NULL" );
		eng_Stop( "50004" );
		return NULL;
	}
	if( height == NULL )
	{
		con_Print( "<RED>TGA Load Assert Failed:  height is NULL" );
		eng_Stop( "50005" );
		return NULL;
	}
	if( bpp == NULL )
	{
		con_Print( "<RED>TGA Load Assert Failed:  bpp is NULL" );
		eng_Stop( "50006" );
		return NULL;
	}

	// load from file
	bits = tga_LoadFromFile( path, &info );
	if( bits == NULL )
	{
		SAFE_RELEASE( info );
		return NULL;
	}

	// copy parameters
	*bpp = (unsigned char)info->bitCount;
	*width = info->imageWidth;
	*height = info->imageHeight;

	// 24 bit tga images are stored BGR
	if( *bpp == 24 )
	{
		iNumPix = info->imageWidth * info->imageHeight;
		for( a = 0 ; a < iNumPix ; a++ )
		{
			i = a * 3;
			bTmp = bits[i];
			bits[i] = bits[i + 2];
			bits[i + 2] = bTmp;
		}
	}

	// done with temp stuff
	SAFE_RELEASE( info );

	return bits;
}

/* ---------------
tga_LoadFromFile
--------------- */
static byte *tga_LoadFromFile( char *name, TARGAFILEHEADER **info )
{
	int ret;
	file_t *file;
	int bitsize;
	byte *bits;
	int infosize;

	// assert
	if( name == NULL )
	{
		con_Print( "<RED>TGA LoadFromFile Assert Failed:  name is NULL" );
		eng_Stop( "50001" );
		return NULL;
	}
	if( info == NULL )
	{
		con_Print( "<RED>TGA LoadFromFile Assert Failed:  info struct is NULL" );
		eng_Stop( "50002" );
		return NULL;
	}

	// open the file
	file = fs_open( name, "r" );
	if( file == NULL ) {
		con_Print( "\t<RED>Targa Load Error:  Could not open file: \"%s\"", name );
		return NULL;
	}

	// allocate for the info header
	infosize = sizeof(TARGAFILEHEADER);
	*info = (TARGAFILEHEADER*)mem_alloc(infosize);
	if( *info == NULL ) {
		con_Print( "\t<RED>Targa Load Error:  Memory allocation failed for targa file header" );
		fs_close(file);
		return NULL;
	}
	memset( *info, 0, infosize );

	// read the image data header
	fs_read( &(*info)->imageIDLength, sizeof(byte), 1, file );
	fs_read( &(*info)->colorMapType, sizeof(byte), 1, file );
	fs_read( &(*info)->imageTypeCode, sizeof(byte), 1, file );
	fs_read( &(*info)->colorMapOrigin, sizeof(short int), 1, file );
	fs_read( &(*info)->colorMapLength, sizeof(short int), 1, file );
	fs_read( &(*info)->colorMapEntrySize, sizeof(byte), 1, file );
	fs_read( &(*info)->imageXOrigin, sizeof(short int), 1, file );
	fs_read( &(*info)->imageYOrigin, sizeof(short int), 1, file );
	fs_read( &(*info)->imageWidth, sizeof(short int), 1, file );
	fs_read( &(*info)->imageHeight, sizeof(short int), 1, file );
	fs_read( &(*info)->bitCount, sizeof(byte), 1, file );
	fs_read( &(*info)->imageDescriptor, sizeof(byte), 1, file );

#ifdef WORDS_BIGENDIAN
	// convert to native format for big-endian architectures
	(*info)->colorMapOrigin = tcReorder16( (*info)->colorMapOrigin );
	(*info)->colorMapLength = tcReorder16( (*info)->colorMapLength );
	(*info)->imageXOrigin = tcReorder16( (*info)->imageXOrigin );
	(*info)->imageYOrigin = tcReorder16( (*info)->imageYOrigin );
	(*info)->imageWidth = tcReorder16( (*info)->imageWidth );
	(*info)->imageHeight = tcReorder16( (*info)->imageHeight );
#endif

	// check the color depth - support three color and grayscale - rle and image plane
	if( ((*info)->imageTypeCode != TGA_COLORMAP) && ((*info)->imageTypeCode != TGA_GRAYSCALE) &&
		((*info)->imageTypeCode != TGA_RLE_COLORMAP) && ((*info)->imageTypeCode != TGA_RLE_GRAYSCALE) )
	{
		con_Print( "\t<RED>Targa Load Error:  Only uncompressed color and grayscale tga files are supported.  Type code is: %d", (*info)->imageTypeCode );
		SAFE_RELEASE(*info);
		fs_close(file);
		return NULL;
	}

	// sanity check for color planes
	if( ((*info)->imageTypeCode == TGA_COLORMAP || (*info)->imageTypeCode == TGA_RLE_COLORMAP) && ((*info)->bitCount != 24) ) 
	{
		con_Print( "\t<RED>Targa Load Error:  Color depth of bitmap is not supported: %d", (*info)->bitCount );
		SAFE_RELEASE(*info);
		fs_close(file);
		return NULL;
	}	

	// sanity check for grayscale planes
	if( ((*info)->imageTypeCode == TGA_GRAYSCALE || (*info)->imageTypeCode == TGA_RLE_GRAYSCALE) && ((*info)->bitCount != 8) ) 
	{
		con_Print( "\t<RED>Targa Load Error:  Color depth of bitmap is not supported: %d", (*info)->bitCount );
		SAFE_RELEASE(*info);
		fs_close(file);
		return NULL;
	}	

	// allocate memory for the bitmap bits
	bitsize = (*info)->imageHeight * (*info)->imageWidth * ( (*info)->bitCount / 8 );
	bits = (byte*)mem_alloc( bitsize );
	if( bits == NULL ) 
	{
		con_Print( "\t<RED>Targa Load Error:  Memory allocation failed for %d bytes.", bitsize );
		SAFE_RELEASE(*info);
		fs_close(file);
		return NULL;
	}
	memset( bits, 0, bitsize );

	// No Encoding
	if( ((*info)->imageTypeCode == TGA_COLORMAP) || ((*info)->imageTypeCode == TGA_GRAYSCALE) )
	{
		ret = fs_read( bits, sizeof(byte), bitsize, file );
		if( fs_error(file) || (ret != bitsize) ) 
		{
			con_Print( "\t<RED>Targa Load Error:  targa image read failed.  Ret:  %d,  Bitsize:  %d", ret, bitsize );
			SAFE_RELEASE(bits);
			SAFE_RELEASE(*info);
			fs_close(file);
			return NULL;
		}
	}
	// RLE Coding
	else
	{
		byte bTmp; // raw repitition count
		byte bCnt; // number of pixels in packet
		byte bPixSize = ((*info)->bitCount / 8);
		short int sRowPos = 0; // all packets break at row ends
		short int sColPos = 0; // so we know when to end
		short int sWidth = (*info)->imageWidth;  // used frequently
		short int sHeight = (*info)->imageHeight; //used frequently
		long int lIndex = 0; // index into bits
		byte bBase[3];
		byte a, b;

		while( sColPos < sHeight )
		{
			while( sRowPos < sWidth )
			{
				// read in the Repetition Count
				ret = fs_read( &bTmp, sizeof(byte), 1, file );
				if( fs_error(file) || (ret < 1) ) 
				{
					con_Print( "\t<RED>Targa Load Error:  targa RLE image read failed for Rep Count at.  X: %d,  Y: %d", sRowPos, sColPos );
					SAFE_RELEASE(bits);
					SAFE_RELEASE(*info);
					fs_close(file);
					return NULL;
				}
				bCnt = (bTmp & 0x7F);
				sRowPos += (short int)bCnt + 1;
		
				// Run Length Packet
				if( bTmp >> 7 )
				{
					// read the data value
					for( a = 0 ; a < bPixSize ; a++ )
					{
						ret = fs_read( &bits[lIndex], sizeof(byte), 1, file );
						if( fs_error(file) || (ret < 1) ) 
						{
							con_Print( "\t<RED>Targa Load Error:  targa RLE image read failed for Base Pixel at.  X: %d,  Y: %d", sRowPos, sColPos );
							SAFE_RELEASE(bits);
							SAFE_RELEASE(*info);
							fs_close(file);
							return NULL;
						}
						// copy the base
						bBase[a] = bits[lIndex];
						// advance
						lIndex++;
					}
					
					// insert repeats
					for( a = 0 ; a < bCnt ; a++ )
					{
						for( b = 0 ; b < bPixSize ; b++ )
						{
							bits[lIndex] = bBase[b];
							lIndex++;
						}
					}
				}
				// Raw Packet
				else
				{
					// add the base to the count, since it's included in the read
					bCnt += 1;
					ret = fs_read( &bits[lIndex], sizeof(byte), (uint32_t)bCnt * (uint32_t)bPixSize, file );
					if( fs_error(file) || (ret < bCnt) ) 
					{
						con_Print( "\t<RED>Targa Load Error:  targa RLE image read failed for Raw Packet at.  X: %d,  Y: %d", sRowPos, sColPos );
						SAFE_RELEASE(bits);
						SAFE_RELEASE(*info);
						fs_close(file);
						return NULL;
					}
					// advance
					lIndex += (uint32_t)bCnt * (uint32_t)bPixSize;
				}	
			}
			// advance a row
			sColPos++;
			sRowPos = 0;
		}
	}

	fs_close(file);

	return bits;
}
