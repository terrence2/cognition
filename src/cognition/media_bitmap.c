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
// bitmap.c
// Created 2-25-02 @ 1030 by Terrence Cole

// Includes
/////////////
#include "cog_global.h"

// Local Definitions
//////////////////////

// Definitions
//////////////

// Global Structures
//////////////////////
/*
*/

// Global Prototypes
//////////////////////
/*
byte *bmp_Load( char *path, unsigned int *width, unsigned int *height, byte *bpp );
void bmp_ScreenShot( char *null );
int bmp_WriteToFile( const char *abs_path, BITMAPINFO *info, byte *bits );
*/

// Local Prototypes
/////////////////////
static byte *bmp_LoadFromFile( char *name, BITMAPINFO **info );
static byte *bmp_LoadFromScreen( BITMAPINFO **info );
static byte *bmp_LoadRGB( file_t *file, BITMAPINFO **info );
static byte *bmp_LoadRLE8( file_t *file, BITMAPINFO **info );

// Local Variables
////////////////////

// *********** FUNCTIONALITY ***********
/* ------------
bmp_Load - creates a color buffer with the information in path (the fqp of a windows bitmap) with an alpha plane
		 - loaded from the source indicated in 'alpha_type'.  Returns a pointer to the loaded color buffer, or NULL
		 - on errors.  if return is non-null, 'width', 'height', 'gl_type', and 'bpp' discribe the colorbuffer
------------ */
byte *bmp_Load( char *path, unsigned int *width, unsigned int *height, byte *bpp )
{
	byte *bits;
	BITMAPINFO *info = NULL;
	int a, i, iNumPix;
	byte bTmp;

	// assert
	if( path == NULL )
	{
		con_Print( "<RED>BMP Load Assert Failed:  path is NULL" );
		eng_Stop( "60003" );
		return NULL;
	}
	if( width == NULL )
	{
		con_Print( "<RED>BMP Load Assert Failed:  width is NULL" );
		eng_Stop( "60004" );
		return NULL;
	}
	if( height == NULL )
	{
		con_Print( "<RED>BMP Load Assert Failed:  height is NULL" );
		eng_Stop( "60005" );
		return NULL;
	}
	if( bpp == NULL )
	{
		con_Print( "<RED>BMP Load Assert Failed:  bpp is NULL" );
		eng_Stop( "60006" );
		return NULL;
	}

	// load from file
	bits = bmp_LoadFromFile( path, &info );
	if( bits == NULL )
	{
		con_Print( "<RED>BMP Load Failed:  Load From File Failed." );
		SAFE_RELEASE( info );
		return NULL;
	}

	*bpp = (unsigned char)info->bmiHeader.biBitCount;
	*width = info->bmiHeader.biWidth;
	*height = info->bmiHeader.biHeight;
	
	// 24 bit bmp images are stored BGR
	if( *bpp == 24 )
	{
		iNumPix = info->bmiHeader.biWidth * info->bmiHeader.biHeight;
		for( a = 0 ; a < iNumPix ; a++ )
		{
			i = a * 3;
			bTmp = bits[i];
			bits[i] = bits[i + 2];
			bits[i + 2] = bTmp;
		}
	}

	// we are done with our temp layers
	SAFE_RELEASE(info);

	return bits;
}

/* ---------------
bmp_ScreenShot - takes a screenshot of the current opengl color buffer and saves it to an image on the hard-disk
			   - the path for the given image is "executive_path\screenshot_????.bmp' where ???? is the next consecutive
			   - screenshot path available for writing
--------------- */
void bmp_ScreenShot( char *null )
{
	int number;
	char file_name[255];
	char *abs_path;
	byte *bits;
	BITMAPINFO *bmi;

	// create a name for the shot
	number = fs_GetNextOrderedName( "screenshot_????.bmp", "" );
	sprintf( file_name, "screenshot_%04d.bmp", number );
	abs_path = fs_GetBasePath( file_name, "" );

	bits = bmp_LoadFromScreen( &bmi );
	if( bits == NULL ) {
		con_Print( "Bitmap System Error:  Unable to Load ScreenShot Image." );
		return;
	}

	if( !bmp_WriteToFile( abs_path, bmi, bits ) )
	{
		con_Print( "ScreenShot Write Failed." );
		SAFE_RELEASE(bmi);
		return;
	}

	SAFE_RELEASE(bmi);

	con_Print( "Screenshot Saved to Disk as: \"%s\"", abs_path );
}

/* ---------------
bmp_LoadFromFile - attempts to read a bitmap from file 'name' and fill out 'info' based on that file
				 - returns a heap address to a color buffer on success or NULL on failure
				 - if return is non-null, 'info' will describe the contents of the returned buffer
--------------- */
static byte *bmp_LoadFromFile( char *name, BITMAPINFO **info )
{
	int ret;
	file_t *file;
	BITMAPFILEHEADER header;
	int infosize;
    byte buffer[sizeof(BITMAPFILEHEADER)];
	
	// assert
	if( name == NULL )
	{
		con_Print( "<RED>BMP LoadFromFile Assert Failed:  name is NULL" );
		eng_Stop( "60001" );
		return NULL;
	}
	if( info == NULL )
	{
		con_Print( "<RED>BMP LoadFromFile Assert Failed:  info struct is NULL" );
		eng_Stop( "60002" );
		return NULL;
	}

	// open the file
	file = fs_open( name, "r" );
	if( file == NULL ) {
		con_Print( "\t<RED>Bitmap Load Error:  Could not open file" );
		return NULL;
	}

	// read the header
	ret = fs_read( (void*)&header, sizeof(BITMAPFILEHEADER), 1, file );
	if( ret != 1 ) {
		con_Print( "\t<RED>Bitmap Load Error:  File read failed for bitmap file header.  Expected 1, returned %d.", ret );
		fs_close(file);
		return NULL;
	}

	// check the file type
	if( ((byte*)&(header.bfType))[0] != 'B' || 
        ((byte*)&(header.bfType))[1] != 'M' ) {
		con_Print( "\t<RED>Bitmap Load Error:  File is not a bitmap." );
		fs_close(file);
		return NULL;
	}

	// allocate for the info header
	infosize = header.bfOffBits - sizeof(BITMAPFILEHEADER);
	*info = (BITMAPINFO*)mem_alloc(infosize);
	if( *info == NULL ) {
		con_Print( "\t<RED>Bitmap Load Error:  Memory allocation failed for bitmap info header" );
		fs_close(file);
		return NULL;
	}
	memset( *info, 0, infosize );

	// read in the info header
	ret = fs_read( *info, infosize, 1, file );
	if( ret != 1 ) {
		con_Print( "\t<RED>Bitmap Load Error:  File read failed for bitmap info header.  Expected 1, returned %d.", ret );
		SAFE_RELEASE(*info);
		fs_close(file);
		return NULL;
	}

	// check the color depth - support three color and grayscale
	if( ((*info)->bmiHeader.biBitCount != 24) && ((*info)->bmiHeader.biBitCount != 8) ) {
		con_Print( "\t<RED>Bitmap Load Error:  Color depth of bitmap is not supported: %d", (*info)->bmiHeader.biBitCount );
		SAFE_RELEASE(*info);
		fs_close(file);
		return NULL;
	}	

	// load straight for uncompressed
	if( (*info)->bmiHeader.biCompression == BI_RGB )
	{
		return bmp_LoadRGB( file, info );
	}
	else if( (*info)->bmiHeader.biCompression == BI_RLE8 )
	{
		return bmp_LoadRLE8( file, info );
	}
	else
	{
		con_Print( "\t<RED>Bitmap Load Error:  Bitmap Storage Type is not supported: %d", (*info)->bmiHeader.biBitCount );
		SAFE_RELEASE(*info);
		fs_close(file);
		return NULL;
	}

	return NULL;
}

/* ------------
bmp_LoadRGB
------------ */
static byte *bmp_LoadRGB( file_t *file, BITMAPINFO **info )
{
	int a;
	int scanlinebytes;
	int padbytes;
	int linebytes;
	int aligned;
	int bytesize;
	byte *bits, *pbits = 0;
	int ret;

	// check the bitmap alignment and get all relevant sizes
	if( ((*info)->bmiHeader.biWidth * (*info)->bmiHeader.biBitCount) % 32 != 0 ) 
	{
		int scanlinebits;
		int padbits;
	
		// how many 32's does this scan line fit on?
		scanlinebits = ( (*info)->bmiHeader.biWidth * (*info)->bmiHeader.biBitCount ) / 32;
		scanlinebits++;
		// the total size of one scanline
		scanlinebits *= 32;
		
		// the end of scanline pad to fit the 32 bit alignment
		padbits = scanlinebits - ( (*info)->bmiHeader.biWidth * (*info)->bmiHeader.biBitCount );
		
		// reduce to bytes
		scanlinebytes = scanlinebits / 8;
		padbytes = padbits / 8;

		// get the actual line size
		linebytes = scanlinebytes - padbits;

		// set flag
		aligned = 0;
	}
	else
	{
		scanlinebytes = ( (*info)->bmiHeader.biWidth * (*info)->bmiHeader.biBitCount ) / 8;
		padbytes = 0;
		linebytes = scanlinebytes;
		aligned = 1;
	}

	// get the needed memory footprint
	bytesize = ( (*info)->bmiHeader.biWidth * abs( (*info)->bmiHeader.biHeight ) * (*info)->bmiHeader.biBitCount ) / 8;

	// allocate memory for the bitmap bits
	bits = (byte*)mem_alloc( bytesize );
	if( bits == NULL ) 
	{
		con_Print( "\t<RED>Bitmap Load Error:  Memory allocation failed for bitmap bits." );
		SAFE_RELEASE(*info);
		return NULL;
	}
	memset( bits, 0, bytesize );

	// read the bits from file
	if( aligned )
	{
		fs_read( bits, 1, bytesize, file );
		if( fs_error(file) ) 
		{
			con_Print( "\t<RED>Bitmap Load Error:  bitmap read failed." );
			SAFE_RELEASE(bits);
			SAFE_RELEASE(*info);
			return NULL;
		}
	}
	else
	{
		// read the bits by scanline, skipping the end-of-line pads
		for( a=0 ; a < ABS( (*info)->bmiHeader.biHeight ) ; a++ )
		{
			fs_read( pbits, 1, linebytes, file );
			if( fs_error(file) ) 
			{
				con_Print( "\t<RED>Error encountered loading bitmap - bitmap read failed on scanline %d", a );
				SAFE_RELEASE(bits);
				SAFE_RELEASE(*info);
				return NULL;
			}

			// set the bit pointer to the next read location
			pbits += linebytes;

			// skip the pad
			ret = fs_seek( file, padbytes, SEEK_CUR );
			if( ret )
			{
				con_Print( "\t<RED>Error encountered loading bitmap - seek failed on scanline %d for %d pad bytes", a, padbytes );
				SAFE_RELEASE(bits);
				SAFE_RELEASE(*info);
				return NULL;
			}
		}
	}

	fs_close(file);
	return bits;
}

/* ------------
bmp_LoadRLE8
------------ */
static byte *bmp_LoadRLE8( file_t *file, BITMAPINFO **info )
{
	#define FAIL( a ) { con_Print( (a) ); SAFE_RELEASE(*info); SAFE_RELEASE(bits); return NULL; }

	int ret;
	int bytesize;
	int bDone;
	byte *bits = NULL;
	byte *pbits;
	byte a;
	byte bCnt, bVal;

	// get the needed memory footprint
	bytesize = ( (*info)->bmiHeader.biWidth * abs( (*info)->bmiHeader.biHeight ) * (*info)->bmiHeader.biBitCount ) / 8;

	// allocate memory for the bitmap bits
	bits = (byte*)mem_alloc( bytesize );
	if( bits == NULL ) FAIL( "\t<RED>Bitmap Load Error:  Memory allocation failed for bitmap bits." );
	memset( bits, 0, bytesize );
	pbits = bits;

	// loop it
	bDone = 0;
	while( !bDone )
	{
		// read the count and value
		ret = fs_read( &bCnt, 1, 1, file );
		if( !ret || fs_error(file) || fs_eof(file) ) FAIL( "<RED>Bitmap Load Error: Read Failed on Cnt." );
		ret = fs_read( &bVal, 1, 1, file );
		if( !ret || fs_error(file) || fs_eof(file) ) FAIL( "<RED>Bitmap Load Error: Read Failed on Val" );

		
		con_Print( "Decoding Packet: %d, %d", bCnt, bVal );

		// do the normal rle fill
		for( a = 0 ; a < bCnt ; a++ )
		{
			*pbits = bVal;
			pbits++;
		}

		// check for eol
		if( !bCnt && !bVal )
		{
			// go forward until we are at the end of a line
			int pos = (pbits - bits);
			int w = (*info)->bmiHeader.biWidth;
			while( pos > 0 ) pos -= w;
			pbits += -pos;
		}

		// check for eoi
		else if( !bCnt && bVal == 1 )
		{
			bDone = 1;
		}

		// check for delta pos
		else if( !bCnt && bVal == 2 )
		{
			// get offsets
			byte x, y, off;
			ret = fs_read( &x, 1, 1, file );
			if( !ret || fs_error(file) || fs_eof(file) ) FAIL( "<RED>Bitmap Load Error: Read Failed on Delta X." );
			ret = fs_read( &y, 1, 1, file );
			if( !ret || fs_error(file) || fs_eof(file) ) FAIL( "<RED>Bitmap Load Error: Read Failed on Delta Y" );

			// compute linear offset
			off = y * (*info)->bmiHeader.biWidth + x;
			pbits += off;
		}

		// check for absolute mode
		else if( !bCnt && bVal >= 3 )
		{
			// read absolute pixels
			ret = fs_read( pbits, 1, bVal, file );
			if( !ret || fs_error(file) || fs_eof(file) ) FAIL( "<RED>Bitmap Load Error: Read Failed on Delta X." );

			// align to word boundary
			if( bVal & 1 ) fs_seek( file, 1, SEEK_CUR );
		}

		// sanity check
		assert( (pbits - bits) <= bytesize );
		
		// check for end conditions
		if( (pbits - bits) > (bytesize - 2) ) bDone = 1;
		if( fs_eof(file) ) bDone = 1;
	} 

	// finished
	fs_close(file);
	return bits;
}

/* ------------
bmp_LoadFromScreen - executes gl commands to pack the current colorbuffer into a heap address and
				   - fills out 'info' based on the current gl properties and color buffer state
				   - returns the address of the heap memory of the copied buffer or NULL on errors
				   - if byte is non-null, info contains a bitmap info header describing returned bytes
------------ */
static byte *bmp_LoadFromScreen( BITMAPINFO **info )
{
	int vp[4];  // the current viewport
	int width;
	int bitsize;
	byte *bits;

	glGetIntegerv( GL_VIEWPORT, vp );

	*info = (BITMAPINFO*)mem_alloc( sizeof(BITMAPINFOHEADER) );
	if( *info == NULL )
	{
		con_Print( "Bitmap System Error:  Memory Allocation Failed for Bitmap Info Header in Load From Screen" );
		return NULL;
	}
	memset( *info, 0, sizeof(BITMAPINFOHEADER) );

	width = vp[2] * 3;  // scanline size
	width = (width + 3) & -3;  // align to 4 bytes
	bitsize = width * vp[3];  // total size of bits in the map

	bits = (byte*)mem_alloc( bitsize );
	if( bits == NULL )
	{	
		con_Print( "Bitmap System Error:  Memory Allocation Failed for Bitmap bits in Load From Screen" );
		SAFE_RELEASE(*info);
		return NULL;
	}
	memset( bits, 0, bitsize );

	// make sure that the opengl states are okay to copy
	glFinish();
	glPixelStorei( GL_PACK_ALIGNMENT, 4 );
	glPixelStorei( GL_PACK_ROW_LENGTH, 0 );
	glPixelStorei( GL_PACK_SKIP_ROWS, 0 );
	glPixelStorei( GL_PACK_SKIP_PIXELS, 0 );

	// copy over the screen buffer
	glReadPixels( 0, 0, vp[2], vp[3], GL_BGR_EXT, GL_UNSIGNED_BYTE, bits );
	
	// initialize the info header
	(*info)->bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
	(*info)->bmiHeader.biWidth			= vp[2];
	(*info)->bmiHeader.biHeight			= vp[3];
	(*info)->bmiHeader.biPlanes			= 1;
	(*info)->bmiHeader.biBitCount		= 24;
	(*info)->bmiHeader.biCompression	= BI_RGB;
	(*info)->bmiHeader.biSizeImage		= bitsize;
	(*info)->bmiHeader.biXPelsPerMeter	= 2952; // 75 dpi
	(*info)->bmiHeader.biYPelsPerMeter	= 2952; // 75 dpi
	(*info)->bmiHeader.biClrUsed		= 0;
	(*info)->bmiHeader.biClrImportant	= 0;
	
	return bits;
}

/* ------------
bmp_WriteToFile - writes a windows style bitmap file at the given path, with the given data
				- returns 1 on success, 0 on failure
------------ */
int bmp_WriteToFile( const char *abs_path, BITMAPINFO *info, byte *bits )
{
	file_t *fp;
	int bitsize;
	int infosize;
	int headersize;
	int size;
	BITMAPFILEHEADER header;

	// open the file in binary write mode
	fp = fs_open( abs_path, "w" );
	if( fp == NULL ) {
		con_Print( "Bitmap System Error:  Open Failed for Screenshot File \"%s\"", abs_path );
		return 0;
	}

	// calculate various sized
	if( info->bmiHeader.biSizeImage == 0 )
	{
		bitsize = ( info->bmiHeader.biWidth * info->bmiHeader.biBitCount + 7 ) /
				  ( 8 * ABS( info->bmiHeader.biHeight ) );
	}
	else
	{
		bitsize = info->bmiHeader.biSizeImage;
	}

	infosize = sizeof(BITMAPINFOHEADER);
	headersize = sizeof(BITMAPFILEHEADER);

	size = headersize + infosize + bitsize;

	// set up the file header
    ((byte*)&(header.bfType))[0] = 'M';
    ((byte*)&(header.bfType))[1] = 'B';
	header.bfSize		= size;
	header.bfReserved1	= 0;
	header.bfReserved2	= 0;
	header.bfOffBits	= headersize + infosize;

	// do the physical writing
	if( fs_write( &header, 1, headersize, fp ) < (unsigned)headersize )
	{
		con_Print( "Bitmap System Error:  File Write Failed for Bitmap File Header!" );
		fs_close(fp);
		return 0;
	}

	if( fs_write( info, 1, infosize, fp ) < (unsigned)infosize )
	{
		con_Print( "Bitmap System Error:  File Write Failed for Bitmap Info Header!" );
		fs_close(fp);
		return 0;
	}

	if( fs_write( bits, 1, bitsize, fp ) < (unsigned)bitsize )
	{
		con_Print( "Bitmap System Error:  File Write Failed for Bitmap Bits!" );
		fs_close(fp);
		return 0;
	}

	fs_close(fp);
	return 1;
}
