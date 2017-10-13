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
// image.c
// Created 2-25-02 @ 1028 by Terrence Cole

// Includes
/////////////
#include "cog_global.h"

// Definitions
////////////////
#define IMG_MIN_SIZE 128

// Structures
///////////////
/*
typedef struct image_s
{
	char *name;
	unsigned int width;
	unsigned int height;
	byte bpp;
	byte *data;

	struct image_s *next;
	struct image_s *prev;
} image_t;
*/

// Global Prototypes
//////////////////////
/*
int img_Initialize(void);
void img_Terminate(void);
image_t *img_Load( const char *name, const char *rel_path );
image_t *img_LoadConstant( const int width, const int height, const byte bpp, const byte *colors );
image_t *img_Splice( const image_t *img24, const image_t *img8 );
image_t *img_CreateCopy( const image_t *img );
image_t *img_Resize( const image_t *img, int size );
void img_Unload( image_t *image );
*/


#ifdef DEBUG
// Local Prototypes
/////////////////////
static void img_LinkTail( image_t *img );
static void img_Unlink( image_t *img );

// Local Variables
////////////////////
static image_t *images_head = NULL;
static image_t *images_tail = NULL;
#endif // DEBUG

// *********** FUNCTIONALITY ***********
/* ------------
img_Initialize
------------ */
int img_Initialize(void)
{
// image tracking should be unneccessary as most images will be immediatly
// unloaded as part of the texture load path
#ifdef DEBUG
	if( (images_head != NULL) || (images_tail != NULL) )
	{
		img_Terminate();
	}
#endif  // DEBUG
	return 1;
}

/* ------------
img_Terminate
------------ */
void img_Terminate(void)
{
#ifdef DEBUG
	image_t *temp;
	unsigned int stray_img = 0;

	con_Print( "\nTerminating Image System..." );

	while( images_head != NULL )
	{
		temp = images_head->next;
		
		if( images_head != NULL )
		{
			SAFE_RELEASE( images_head->data );
			SAFE_RELEASE( images_head->name );
			SAFE_RELEASE( images_head );
			stray_img++;
		}

		images_head = temp;
	}

	images_head = images_tail = NULL;

	con_Print( "\tFreed %d stray image(s)...", stray_img );
	con_Print( "\tImage system terminated successfully." );
#endif  // DEBUG
}

/* ------------
img_Load
------------ */
image_t *img_Load( const char *name, const char *rel_path )
{
	image_t *img;
	char *base_path;
	char pathStr[1024];
	unsigned int path_len;

	// this should never happen - but just in case
	assert( name );
	if( name == NULL ) return NULL;

	// tell us about it
	con_Print( "<BLUE>Loading</BLUE> Image: <VIOLET>\"%s\"</VIOLET>", name );
	eng_LoadingFrame();

	// get some room to put the image
	img = (image_t*)mem_alloc( sizeof(image_t) );
	if( img == NULL )
	{
		con_Print( "Image Load Failed - Unable to allocate memory for image handler" );
		return NULL;
	}
	memset( img, 0, sizeof(image_t) );

	// copy over the title
	img->name = (char*)mem_alloc( tcstrlen(name)+1 );
	if( img->name == NULL )
	{
		con_Print( "Image Load Failed - Unable to allocate memory for image name string" );
		SAFE_RELEASE(img);
		return NULL;
	}
	memset( img->name, 0, tcstrlen(name)+1 );
	memcpy( img->name, name, tcstrlen(name) );
	img->name[tcstrlen(name)] = '\0';

	// get a path and information about the requested load
	base_path = fs_GetMediaPath( name, rel_path );
	path_len = tcstrlen( base_path );

	// try each of the image types in order of preference
	img->data = NULL;

	// TGA
	memset( pathStr, 0, 1024 );
	memcpy( pathStr, base_path, tcstrlen(base_path) );
	tcstrcat( pathStr, ".tga" );
	if( fs_FileExists( pathStr ) )
	{
		img->data = tga_Load( pathStr, &(img->width), &(img->height), &(img->bpp));
	}

	if( img->data == NULL )
	{
		// BMP
		memset( pathStr, 0, 1024 );
		memcpy( pathStr, base_path, tcstrlen(base_path) );
		tcstrcat( pathStr, ".bmp" );
		if( fs_FileExists( pathStr ) )
		{
			img->data = bmp_Load( pathStr, &(img->width), &(img->height), &(img->bpp));
		}
		
		if( img->data == NULL )
		{
			// JPG
			memset( pathStr, 0, 1024 );
			memcpy( pathStr, base_path, tcstrlen(base_path) );
			tcstrcat( pathStr, ".jpg" );
			img->data = jpg_Load( pathStr, &(img->width), &(img->height), &(img->bpp));
		}
	}

	// test the load
	if( img->data == NULL )
	{
		con_Print( "<RED>Image Load Failed - Unable to load Image Data" );
		SAFE_RELEASE( img->name );
		SAFE_RELEASE( img );
		return NULL;
	}

#ifdef DEBUG
	// link it
	img_LinkTail( img );
#endif // DEBUG

	// print the data
#ifdef VERBOSE
	con_Print( "\n<BLUE>Loaded</BLUE> Image from <ORANGE>\"%s\"</ORANGE>", pathStr );
	con_Print( "\tSize: <GREEN>%d x %d x %d</GREEN> -- %d bytes", img->width, img->height, img->bpp, (img->width * img->height * (img->bpp / 8)) + sizeof(image_t) );
#endif  // VERBOSE

	return img;
}

/* ------------
img_LoadConstant - loads an image with a constant color bitplane with the requested parameters
------------ */
image_t *img_LoadConstant( const int width, const int height, const byte bpp, const byte *colors )
{
	int a;
	char strName[1024];
	image_t *img;
	int img_size;
	int img_len;
	int elmt_size;
	byte *pdata;

	// tell us about it
	con_Print( "<BLUE>Loading</BLUE> Constant Color Image." );
	eng_LoadingFrame();

	// assert
	if( width <= 0 )
	{
		con_Print( "<RED>Image Load Constant Failed:  Assert failed:  width <= 0.  width: %d", width );
		eng_Stop( "80001" );
		return NULL;
	}
	if( height <= 0 )
	{
		con_Print( "<RED>Image Load Constant Failed:  Assert failed:  height <= 0.  height: %d", height );
		eng_Stop( "80002" );
		return NULL;
	}
	if( (bpp <= 0) || (bpp % 8 != 0) )
	{
		con_Print( "<RED>Image Load Constant Failed:  Assert failed:  bpp <= 0 or not divisible by 8.  bpp: %d", bpp );
		eng_Stop( "80003" );
		return NULL;
	}
	if( colors == NULL )
	{
		con_Print( "<RED>Image Load Constant Failed:  Assert failed:  colors is NULL" );
		eng_Stop( "80004" );
		return NULL;
	}

	// get memory for the image
	img = (image_t*)mem_alloc( sizeof(image_t) );
	if( img == NULL )
	{
		con_Print( "<RED>Image Load Constant Failed:  memory allocation for image failed." );
		return NULL;
	}
	memset( img, 0, sizeof(image_t) );

	// copy parameters
	img->width = width;
	img->height = height;
	img->bpp = bpp;
	snprintf( strName, 1024, "Constant %d, %d, %d", width, height, bpp );
	img->name = tcDeepStrCpy( strName );
	if( img->name == NULL )
	{
		con_Print( "<RED>Image Load Constant Failed:  deep copy of name failed." );
		return NULL;
	}

	// get memory for the color plane
	img_len = width * height;
	elmt_size = bpp / 8;
	img_size = img_len * elmt_size;
	img->data = (byte*)mem_alloc( img_size );
	if( img->data == NULL )
	{
		con_Print( "<RED>Image Load Constant Failed:  memory allocation for image data failed." );
		return NULL;
	}
	memset( img->data, 0, img_size );

	// copy data to the color plane
	pdata = img->data;
	for( a = 0 ; a < img_len ; a++ )
	{
		memcpy( pdata, colors, elmt_size );
		pdata += elmt_size;
	}

#ifdef DEBUG
	// if we're in debug, link the img to the tracker
	img_LinkTail( img );
#endif // DEBUG

#ifdef VERBOSE
	// print the data
	con_Print( "\n<BLUE>Loaded</BLUE> Constant Color Image." );
	con_Print( "\tSize: <GREEN>%d x %d x %d</GREEN> -- %d bytes", img->width, img->height, img->bpp, (img->width * img->height * (img->bpp / 8)) + sizeof(image_t) );
#endif // VERBOSE

	return img;
}

/* ------------
img_Splice - this function takes a 24 bit image and an 8 bit image and quickly creates a 32 bit image from them
		- returns NULL on fail or a new image on success
------------ */
image_t *img_Splice( const image_t *img24, const image_t *img8 )
{
	image_t *img;
	char buffer[1024];
	int img_len;
	byte *pbits, *pcm, *pam;
	int a;

	// tell us about it
	con_Print( "<BLUE>Splicing</BLUE> 24 bit Image <VIOLET>\"%s\"</VIOLET> with 8 bit Image <VIOLET>\"%s\"</VIOLET>.", img24->name, img8->name );
	eng_LoadingFrame();

	// assert
	if( img24 == NULL )
	{
		con_Print( "<RED>Image Splice Error:  Assert Failed  img24 is NULL" );
		eng_Stop( "80005" );
		return NULL;
	}
	if( img8 == NULL )
	{
		con_Print( "<RED>Image Splice Error:  Assert Failed  img8 is NULL" );
		eng_Stop( "80006" );
		return NULL;
	}
	if( img24->bpp != 24 )
	{
		con_Print( "<RED>Image Splice Error:  Assert Failed  img24 is not a 24 bit color plane" );
		eng_Stop( "80007" );
		return NULL;
	}
	if( img8->bpp != 8 )
	{
		con_Print( "<RED>Image Splice Error:  Assert Failed  img24 is not an 8 bit color plane" );
		eng_Stop( "80008" );
		return NULL;
	}
	
	// test to see if this is a valid operation
	if( (img24->width != img8->width) || (img24->height != img8->height) )
	{
		con_Print( "<RED>Image Splice Failed:  Dimensions of input images do not match" );
		return NULL;
	}

	// get a new image
	img = mem_alloc( sizeof(image_t) );
	if( img == NULL )
	{
		con_Print( "<RED>Image Splice Failed:  Memory allocation for image failed." );
		return NULL;
	}
	memset( img, 0, sizeof(image_t) );

	// init image
	memset( buffer, 0, 1024 );
	sprintf( buffer, "%s_SPLICE_%s", img24->name, img8->name );
	img->name = tcDeepStrCpy( buffer );
	if( img->name == NULL )
	{
		con_Print( "<RED>Image Splice Failed:  tcDeepStrCpy for image name failed." );
		SAFE_RELEASE( img );
		return NULL;
	}

	// copy the rest of the parameters
	img->bpp = 32;
	img->width = img24->width;
	img->height = img24->height;

	// allocate a 32 bit plane
	img_len = img->width * img->height;
	img->data = (byte*)mem_alloc( 4 * img_len );
	if( img->data == NULL )
	{
		con_Print( "<RED>Image Splice Failed:  Memory allocation for image data failed." );
		SAFE_RELEASE( img->name );
		SAFE_RELEASE( img );
		return NULL;
	}
	memset( img->data, 0, 4 * img_len );

	// copy the images into the new plane
	pbits = img->data;
	pcm = img24->data;
	pam = img8->data;
	for( a=0 ; a < img_len ; a++ )
	{
		pbits[0] = pcm[0];
		pbits[1] = pcm[1];
		pbits[2] = pcm[2];
		pbits[3] = pam[0];

		pbits += 4;
		pcm += 3;
		pam += 1;
	}

#ifdef VERBOSE
	// print the data
	con_Print( "\nSpliced 24 bit Image \"%s\" with 8 bit Image \"%s\"", img24->name, img8->name );

	// print the rest of the data
	con_Print( "\tSize: %d x %d x %d -- %d bytes", img->width, img->height, img->bpp, img_len * 4 + sizeof(image_t) );
#endif // VERBOSE

	return img;
}

/* ------------
img_CreateCopy - creates an exact duplicate of img or NULL on failure
------------ */
image_t *img_CreateCopy( const image_t *img )
{
	image_t *newImg;
	int iDataSize;

	// tell us about it
	con_Print( "<BLUE>Copying</BLUE> image <VIOLET>\"%s\"</VIOLET>.", img->name );
	eng_LoadingFrame();

	// assert
	if( img == NULL )
	{
		con_Print( "<RED>Image CreateCopy Assert Failed:  img is NULL" );
		return NULL;
	}

	// get memory for the new image
	newImg = (image_t*)mem_alloc( sizeof(image_t) );
	if( newImg == NULL )
	{
		con_Print( "<RED>Image CreateCopy Failed:  memory allocation failed for newImg." );
		return NULL;
	}
	memset( newImg, 0, sizeof(image_t) );

	// copy the name
	newImg->name = tcDeepStrCpy( img->name );
	if( newImg->name == NULL )
	{
		con_Print( "<RED>Image CreateCopy Failed:  deep string copy for name failed." );
		SAFE_RELEASE( newImg );
		return NULL;
	}

	// copy data
	newImg->bpp = img->bpp;
	newImg->width = img->width;
	newImg->height = img->height;

	// copy image data
	iDataSize = newImg->width * newImg->height * (newImg->bpp / 8);
	newImg->data = (byte*)mem_alloc( iDataSize );
	if( newImg->data == NULL )
	{
		con_Print( "<RED>Image CreateCopy Failed:  memory allocation for failed." );
		SAFE_RELEASE( newImg->name );
		SAFE_RELEASE( newImg );
		return NULL;
	}
	memset( newImg->data, 0, iDataSize );
	memcpy( newImg->data, img->data, iDataSize );

#ifdef DEBUG
	// link if debug
	img_LinkTail( newImg );
#endif // DEBUG

	return newImg;
}

/* ------------
img_Resize - downsamples img to the scale of size - 0 is IMG_MIN_SIZE, each larger increment is a pow2 increase
			- will not upsample, will not downsample past 0 MIN_SIZE, for a dimension
			- all downsamples work in both axes
			- returns a new image on success, and NULL on failure
------------ */
image_t *img_Resize( const image_t *img, int size )
{
	int a;
	image_t *imgNew;
	int iDestResX, iDestResY;
	int iScaleFactX, iScaleFactY;
	int iNumPlanes;
	int iSrcJmpSpanY;
	unsigned int destX, destY;
	int srcX, srcY, pl;
	int iSrcIndex;
	double dDestPool[4];
	double dNumDestPoolPix;

	// assert
	if( img == NULL )
	{
		con_Print( "<RED>Image Resize Assert Failed:  img was NULL" );
		eng_Stop( "40001" );
		return NULL;
	}	

	// constrain the size
	if( size < 0 ) size = 0;

	// count up to the destination resolution
	a = 0;
	if( img->width < img->height )
	{
		iDestResX = IMG_MIN_SIZE;
		iDestResY = IMG_MIN_SIZE * img->height / img->width;
	}
	else
	{
		iDestResY = IMG_MIN_SIZE;
		iDestResX = IMG_MIN_SIZE * img->width / img->height;
	} 

	// iterate through size iterations of pow2 increase from base
	while( a < size )
	{
		iDestResX *= 2;
		iDestResY *= 2;
		a++;
	}

	// find the scale factor
	iScaleFactX = img->width / iDestResX;
	iScaleFactY = img->height / iDestResY;
	iNumPlanes = (img->bpp / 8);
//	iSrcJmpSpanY = iScaleFactY * img->height;
	iSrcJmpSpanY = iScaleFactY * img->width;
	dNumDestPoolPix = iScaleFactX * iScaleFactY;
	dDestPool[0] = dDestPool[1] = dDestPool[2] = dDestPool[3] = 0.0;

	// never upscale images (0) and don't bother if we aren't scaling (1)
	if( (iScaleFactX <= 1) || (iScaleFactY <= 1) )
	{
		return img_CreateCopy(img);
	}

	// tell us about it
	con_Print( "<BLUE>Resizing</BLUE> image <VIOLET>\"%s\"</VIOLET>: (%d, %d) -> (%d, %d)", img->name, img->width, img->height, iDestResX, iDestResY );
	eng_LoadingFrame();
	
	// create  a new image
	imgNew = (image_t*)mem_alloc( sizeof(image_t) );
	if( imgNew == NULL )
	{
		con_Print( "<RED>Image Resize Failed:  unable to allocate memory for new image." );
		return NULL;
	}
	memset( imgNew, 0, sizeof(image_t) );

	// get the name
	imgNew->name = tcDeepStrCpy( img->name );
	if( imgNew->name == NULL )
	{
		con_Print( "<RED>Image Resize Failed:  deep string copy for name failed." );
		return NULL;
	}

	// copy over our parameters
	imgNew->bpp = img->bpp;
	imgNew->width = iDestResX;
	imgNew->height = iDestResY;

	// allocate the new plane
	imgNew->data = (byte*)mem_alloc( iNumPlanes * imgNew->width * imgNew->height );
	if( imgNew->data == NULL )
	{
		con_Print( "<RED>Image Resize Failed:  memory allocation failed for new data." );
		return NULL;
	}
	memset( imgNew->data, 0, iNumPlanes * imgNew->width * imgNew->height );

	// downsize:
	// iterate each pixel of the destination
	for( destY = 0 ; destY < imgNew->height ; destY++ )
	{
		for( destX = 0 ; destX < imgNew->width ; destX++ )
		{
			// internal iteration of the src image pixels
			for( srcY = 0 ; srcY < iScaleFactY ; srcY++ )
			{
				for( srcX = 0 ; srcX < iScaleFactX ; srcX++ )
				{
					// get the src Index for this pixel
					iSrcIndex = (iSrcJmpSpanY * destY) + (img->width * srcY) + (iScaleFactX * destX) + srcX;
					iSrcIndex *= iNumPlanes;
				
					// iterate each plane of the source, adding part to the pool
					for( pl = 0 ; pl < iNumPlanes ; pl++ )
					{
						dDestPool[pl] += (double)img->data[iSrcIndex + pl] / dNumDestPoolPix;
					}
					
				}
			}

			// iterate and collapse our pool into each plane
			for( pl = 0 ; pl < iNumPlanes ; pl++ )
			{
				imgNew->data[ (((imgNew->width * destY) + destX) * iNumPlanes) + pl ] = (byte)dDestPool[pl];
				dDestPool[pl] = 0.0;				
			}
		}
	}

#ifdef DEBUG
	// add to the list
	img_LinkTail( imgNew );
#endif // DEBUG

	return imgNew;
}

/* ------------
img_Unload - unloads the data in  image
------------ */
void img_Unload( image_t *image )
{
	if( image != NULL )
	{
		con_Print( "<RED>Unloading</RED> image <VIOLET>\"%s\"</VIOLET>.", image->name ); 
		eng_LoadingFrame();

#ifdef DEBUG
		img_Unlink( image );
#endif // DEBUG
		SAFE_RELEASE( image->name );
		SAFE_RELEASE( image->data );
		SAFE_RELEASE( image );
	}
}

#ifdef DEBUG
/* ------------
img_LinkTail
------------ */
static void img_LinkTail( image_t *img )
{
	if( images_head == NULL ) images_head = img;

	if( images_tail == NULL )
	{
		images_tail = img;
		img->next = NULL;
		img->prev = NULL;
		return;
	}

	img->prev = images_tail;
	img->next = NULL;

	images_tail->next = img;
	images_tail = img;
}

/* ------------
img_Unlink
------------ */
static void img_Unlink( image_t *img )
{
	if( img->prev != NULL ) img->prev->next = img->next;
	if( img->next != NULL ) img->next->prev = img->prev;

	if( images_tail == img ) images_tail = img->prev;
	if( images_head == img ) images_head = img->next;

	img->prev = NULL;
	img->next = NULL;
}
#endif // DEBUG
