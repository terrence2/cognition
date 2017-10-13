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
// colormap.c
// Created 2-25-02 @ 1036 by Terrence Cole

// Includes
/////////////
#include "cog_global.h"

// Definitions
////////////////

// Structures
///////////////
/*
typedef struct colormap_s
{
	// tracking
	char *name;
	unsigned int users;

	// openGL parameters
	unsigned int gl_name;
	int iFilter;
	int iWrap;
	int iEnv;
	byte bMipmap;

	// load parameters
	byte bNoRescale;
	int bScaleBias;

	// list stuff
	struct colormap_s *prev;
	struct colormap_s *next;
} colormap_t;

typedef struct colormap_params_s
{
	// loading parameters
	int iFilter;
	int iWrap;
	int iEnv;
	byte bMipmap;

	// load parameters
	byte bNoRescale;
	int bScaleBias;
} colormap_params_t;
*/

// Global Prototypes
//////////////////////
/*
int cm_Initialize(void);
void cm_Terminate(void);
colormap_t *cm_LoadFromFiles( const char *name, const char *name24, const char *name8, const colormap_params_t *params );
colormap_t *cm_LoadConstAlpha( const char *name, const char *name24, const byte alphaClr, const colormap_params_t *params );
colormap_t *cm_LoadFromImage( const char *name, const image_t *img, const colormap_params_t *params );
void cm_Unload( colormap_t *cm );
void cm_PrintInfo( char *null );
int cm_GetGlobalFilter();
*/

// Local Prototypes
/////////////////////
static byte cm_ogl_Load( image_t *img, colormap_t *cm );
static colormap_t *cm_FindColormap( const char *name );
static void cm_LinkTail( colormap_t *cm );
static void cm_Unlink( colormap_t *cm );

// Local Variables
////////////////////
static colormap_t *cmHead = NULL;
static colormap_t *cmTail = NULL;
static varlatch vlTextureDetail = NULL;
static varlatch vlTextureFilter = NULL;

// *********** FUNCTIONALITY ***********
/* ------------
cm_Initialize - Initialize the colormap tracking system
			  - returns 1 on success, 0 on failure
------------ */
int cm_Initialize(void)
{
	// assert that the colormap system is shutdown before continuing
	if( cmHead != NULL )
		cm_Terminate();

	// associate our variable
	if( vlTextureDetail == NULL )
	{
		vlTextureDetail = var_GetVarLatch( "texture_detail" );
		if( vlTextureDetail == NULL )
		{
			con_Print( "<RED>Colormap Initialize Failed!  Could not latch to variable \"texture_detail\"." );
			return 0;
		}
	}

	// associate our other variable
	if( vlTextureFilter == NULL )
	{
		vlTextureFilter = var_GetVarLatch( "texture_filter" );
		if( vlTextureFilter == NULL )
		{
			con_Print( "<RED>Colormap Initialize Failed!  Could not latch to variable \"texture_filter\"." );
			return 0;
		}
	}
	
	return 1;
}

/* ------------
cm_Terminate - shutsdown the colormap system and frees all associted heap and gl memory
			 - returns 1 on success and 0 on failure
------------ */
void cm_Terminate(void)
{
	colormap_t *cmTmp, *cmCur;
	unsigned int stray_cm = 0;

	// assert
	if( cmHead == NULL ) return;

	con_Print( "\nTerminating Colormap System..." );

	cmCur = cmHead;
	while( cmCur != NULL )
	{
		// store next
		cmTmp = cmCur->next;
		
		// remove from list
		cm_Unlink( cmCur );		

		// delete
		glDeleteTextures( 1, &(cmCur->gl_name) );
		SAFE_RELEASE( cmCur->name );
		SAFE_RELEASE( cmCur );
		stray_cm++;

		// next
		cmCur = cmTmp;
	}

	// delete the list
	cmHead = NULL;

	// delete variables
	vlTextureDetail = NULL;

	con_Print( "\tFreed %d stray colormap(s)...", stray_cm );
	con_Print( "\tColormap system successfully terminated." );
}

/* ------------
cm_LoadFromFiles - demand loads a 32 bit colormap from the 24 bit image at name24 and the 8 bit image at name8
				- applies the global scaling from texture_detail and the logal scale bias
				- if the colormap is already loaded, returns a pointer to the existing instance
				- returns a colormap or NULL on an error
------------ */
colormap_t *cm_LoadFromFiles( const char *name, const char *name24, const char *name8, const colormap_params_t *params )
{
	image_t *img, *img24, *img8;
	colormap_t *cm;

	// tell us about it
	con_Print( "<BLUE>Loading</BLUE> <RED>c<ORANGE>o<YELLOW>l<GREEN>o<BLUE>r</BLUE>map from <ORANGE>\"%s\"</ORANGE> and <ORANGE>\"%s\".", name24, name8 );
	eng_LoadingFrame();
	
	// assert
	// assert name in LoadFromImage
	if( name24 == NULL )
	{
		con_Print( "<RED>Colormap LoadFromFiles Assert Failed:  name24 is NULL" );
		eng_Stop( "30001" );
		return NULL;
	}
	if( name8 == NULL )
	{
		con_Print( "<RED>Colormap LoadFromFiles Assert Failed:  name8 is NULL" );
		eng_Stop( "30002" );
		return NULL;
	}
	// assert params in LoadFromImage
	// assert vlTextureDetail in LoadFromImage

	// check for a previous instance
	cm = cm_FindColormap( name );
	if( cm != NULL )
	{
		cm->users++;
		return cm;
	}

	// load and splice the images
	img24 = img_Load( name24, "colormaps" );
	if( img24 == NULL )
	{
		con_Print( "<RED>Colormap LoadFromFiles Failed:  image load for %s failed.", name24 );
		return NULL;
	}

	img8 = img_Load( name8, "colormaps" );
	if( img8 == NULL )
	{
		con_Print( "<RED>Colormap LoadFromFiles Failed:  image load for %s failed.", name8 );
		img_Unload( img24 );
		return NULL;
	}

	// do the splice to get a 32 bit image
	img = img_Splice( img24, img8 );
	if( img == NULL )
	{
		con_Print( "<RED>Colormap LoadFromFiles Failed:  image splice for %s and %s failed.", name24, name8 );
		img_Unload( img24 );
		img_Unload( img8 );
		return NULL;
	}

	// unload the transients
	img_Unload( img24 );
	img_Unload( img8 );

	// do the colormap load
	cm = cm_LoadFromImage( name, img, params );
	if( cm == NULL )
	{
		con_Print( "<RED>Colormap LoadFromFiles Failed:  colormap, load from image failed." );
		img_Unload( img );
		return NULL;
	}

	// unload the loaded image
	img_Unload( img );
	img = NULL;

	return cm;
}

/* ------------
cm_LoadFromImage - creates a colormap from the data at name24 with an alpha plane with all bits set to alphaClr
				- applies the global scaling from texture_detail and the logal scale bias
				- if the image is already loaded, returns a pointer to the previous instance
				- returns a colormap on success or NULL on error
------------ */
colormap_t *cm_LoadConstAlpha( const char *name, const char *name24, const byte alphaClr, const colormap_params_t *params )
{
	image_t *img, *img24, *img8;
	colormap_t *cm;

	// tell us about it
	con_Print( "<BLUE>Loading</BLUE> <RED>c<ORANGE>o<YELLOW>l<GREEN>o<BLUE>r</BLUE>map from <ORANGE>\"%s\"</ORANGE> with %d alpha.", name24, alphaClr );
	eng_LoadingFrame();
	
	// assert
	// asser name in LoadFromImage
	if( name24 == NULL )
	{
		con_Print( "<RED>Colormap LoadFromFiles Assert Failed:	name24 is NULL" );
		eng_Stop( "30003" );
		return NULL;
	}
	// assert params in LoadFromImage
	// assert vlTextureDetail in LoadFromImage

	// check for a previous instance
	cm = cm_FindColormap( name );
	if( cm != NULL )
	{
		cm->users++;
		return cm;
	}

	// load and splice the images
	img24 = img_Load( name24, "colormaps" );
	if( img24 == NULL )
	{
		con_Print( "<RED>Colormap LoadFromFiles Failed:  image load for %s failed.", name24 );
		return NULL;
	}

	img8 = img_LoadConstant( img24->width, img24->height, 8, &alphaClr );
	if( img8 == NULL )
	{
		con_Print( "<RED>Colormap LoadFromFiles Failed:  image load for constant alpha image failed." );
		img_Unload( img24 );
		return NULL;
	}

	// do the splice to get a 32 bit image
	img = img_Splice( img24, img8 );
	if( img == NULL )
	{
		con_Print( "<RED>Colormap LoadFromFiles Failed:  image splice for %s with a constant alpha failed.", name24 );
		img_Unload( img24 );
		img_Unload( img8 );
		return NULL;
	}

	// unload the transients
	img_Unload( img24 );
	img_Unload( img8 );

	// do the colormap load
	cm = cm_LoadFromImage( name, img, params );
	if( cm == NULL )
	{
		con_Print( "<RED>Colormap LoadFromFiles Failed:  colormap, load from image failed." );
		img_Unload( img );
		return NULL;
	}

	// unload the loaded image
	img_Unload( img );

	return cm;
}

/* ------------
cm_LoadFromImage - creates a colormap from the 32 bit image 'img'
				- applies the global scaling from texture_detail and the logal scale bias
				- if the image is already loaded, returns a pointer to the previous instance
				- returns a colormap on success or NULL on error
------------ */
colormap_t *cm_LoadFromImage( const char *name, const image_t *img, const colormap_params_t *params )
{
	byte bRet;
	image_t *imgRescaled;
	colormap_t *cm;
	int iFinalScale;

	// assert
	if( name == NULL )
	{
		con_Print( "<RED>Colormap LoadFromImage Assert Failed:	name is NULL" );
		eng_Stop( "30004" );
		return NULL;
	}
	if( img == NULL )
	{
		con_Print( "<RED>Colormap LoadFromImage Assert Failed:  img is NULL" );
		eng_Stop( "30005" );
		return NULL;
	}

	// tell us about it
	con_Print( "<BLUE>Loading</BLUE> <RED>c<ORANGE>o<YELLOW>l<GREEN>o<BLUE>r</BLUE>map from image <VIOLET>\"%s\"</VIOLET>.", img->name );
	eng_LoadingFrame();

	if( params == NULL )
	{
		con_Print( "<RED>Colormap LoadFromImage Assert Failed:  params is NULL" );
		eng_Stop( "30006" );
		return NULL;
	}
	if( vlTextureDetail == NULL )
	{
		con_Print( "<RED>Colormap LoadFromImage Assert Failed:  vlTextureDetail is NULL" );
		eng_Stop( "30007" );
		return NULL;
	}
	if( img->bpp != 32 )
	{
		con_Print( "<RED>Colormap LoadFromImage Assert Failed:  image bitdepth is not 32.  bpp:  %d", img->bpp );
		eng_Stop( "30008" );
		return NULL;
	}

	// check for a previous instance
	cm = cm_FindColormap( name );
	if( cm != NULL )
	{
		cm->users++;
		return cm;
	}

	// get a new colormap
	cm = (colormap_t*)mem_alloc( sizeof(colormap_t) );
	if( cm == NULL )
	{
		con_Print( "<RED>Colormap LoadFromImage Failed:  Unable to allocate memory for colormap." );
		return NULL;
	}
	memset( cm, 0, sizeof(colormap_t) );

	// copy over the data
	cm->name = tcDeepStrCpy( name );
	if( cm->name == NULL )
	{
		con_Print( "<RED>Colormap LoadFromImage Failed:  Unable to deep copy colormap name." );
		SAFE_RELEASE( cm );
		return NULL;
	}
	cm->users = 1;
	cm->iFilter = params->iFilter;
	cm->iWrap = params->iWrap;
	cm->iEnv = params->iEnv;
	cm->bMipmap = params->bMipmap;
	cm->bNoRescale = params->bNoRescale;
	cm->iScaleBias = params->iScaleBias;

	// do some logic on our scale base and bias to get us a new scale
	if( !(cm->bNoRescale) )
	{
		// apply bias
		iFinalScale = cm->iScaleBias + (int)var_GetFloat(vlTextureDetail);

		// do the rescale
		imgRescaled = img_Resize( img, iFinalScale );
		if( imgRescaled == NULL )
		{
			con_Print( "<RED>Colormap LoadFromImage Failed:  Image Rescale Failed." );
			SAFE_RELEASE( cm->name );
			SAFE_RELEASE( cm );
			return NULL;
		}
	}
	else
	{
		imgRescaled = img_CreateCopy( img );
		if( imgRescaled == NULL )
		{
			con_Print( "<RED>Colormap LoadFromImage Failed:  Image CreateCopy Failed." );
			SAFE_RELEASE( cm->name );
			SAFE_RELEASE( cm );
			return NULL;
		}
	}

	// do the dependent load
	bRet = cm_ogl_Load( imgRescaled, cm );
	if( !bRet )
	{
		con_Print( "<RED>Colormap LoadFromImage Failed:  OpenGL Dependent Loading Failed." );
		img_Unload( imgRescaled );
		SAFE_RELEASE( cm->name );
		SAFE_RELEASE( cm );
		return NULL;
	}

	// unload the image
	img_Unload( imgRescaled );

	// link in our image to our tracker
	cm_LinkTail( cm );

	return cm;
}

/* ------------
cm_ogl_Load - loads a colormap into opengl from a given image structure, with the parameters in cm
			- returns 0 on failure, 1 on success, stores the gl texture name to cm->gl_name on success
------------ */
static byte cm_ogl_Load( image_t *img, colormap_t *cm )
{
	int ret;

	// assert
	if( img == NULL )
	{
		con_Print( "<RED>Colormap ogl_Load error:  Assert Failed:  img == NULL" );
		eng_Stop( "30009" );
		return 0;
	}

	// reset the error state
	glGetError(); // call this b4 checking for an unrelated error after a draw, etc

	// get a gl name for our texture
	glGenTextures( 1, &(cm->gl_name) ); // OGL 1.1 and later
	if( (ret = glGetError()) != GL_NO_ERROR )
	{
		con_Print( "<RED>Load Colormap Failed at glGenTextures: %s", gluErrorString(ret) );
		return 0;
	}

	// initialize the texture
	glBindTexture( GL_TEXTURE_2D, cm->gl_name );

	// set the wrapping parameter GL_REPEAT, GL_CLAMP
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, cm->iWrap );
	if( (ret = glGetError()) != GL_NO_ERROR )
	{
		con_Print( "<RED>Load Colormap Failed at glTexParameter (wrap S): %s", gluErrorString(ret) );
		glDeleteTextures( 1, &(cm->gl_name) );
		return 0;
	}
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, cm->iWrap );
	if( (ret = glGetError()) != GL_NO_ERROR )
	{
		con_Print( "<RED>Load Colormap Failed at glTexParameter (wrap T): %s", gluErrorString(ret) );
		glDeleteTextures( 1, &(cm->gl_name) );
		return 0;
	}

	// set the environment GL_MODULATE, GL_DECAL, GL_BLEND, [and GL_ADD, GL_REPLACE - in some later implementations]
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, cm->iEnv );
	if( (ret = glGetError()) != GL_NO_ERROR )
	{
		con_Print( "<RED>Load Colormap Failed at glTexEnv: %s", gluErrorString(ret) );
		glDeleteTextures( 1, &(cm->gl_name) );
		return 0;
	}

	if( cm->iFilter != GL_NEAREST )
	{
#ifdef VERBOSE
		con_Print( "\tLoading with linear filtering" );
#endif // VERBOSE	
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		if( (ret = glGetError()) != GL_NO_ERROR )
		{
			con_Print( "<RED>Load Colormap Failed at glTexParameter (linear filter MAG): %s", gluErrorString(ret) );
			glDeleteTextures( 1, &(cm->gl_name) );
			return 0;
		}
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		if( (ret = glGetError()) != GL_NO_ERROR )
		{
			con_Print( "<RED>Load Colormap Failed at glTexParameter (linear filter MIN): %s", gluErrorString(ret) );
			glDeleteTextures( 1, &(cm->gl_name) );
			return 0;
		}
	}
	else
	{
#ifdef VERBOSE
		con_Print( "\tLoading with nearest pix-map element filtering" );
#endif // VERBOSE
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		if( (ret = glGetError()) != GL_NO_ERROR )
		{
			con_Print( "<RED>Load Colormap Failed at glTexParameter (nearest filter MAG): %s", gluErrorString(ret) );
			glDeleteTextures( 1, &(cm->gl_name) );
			return 0;
		}
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		if( (ret = glGetError()) != GL_NO_ERROR )
		{
			con_Print( "<RED>Load Colormap Failed at glTexParameter (nearest filter MIN): %s", gluErrorString(ret) );
			glDeleteTextures( 1, &(cm->gl_name) );
			return 0;
		}
	}

 
	 // loading function is based around the mipmap state
	 if( cm->bMipmap )
	 {
#ifdef VERBOSE
		con_Print( "\tLoading mipmaped image" );
#endif // VERBOSE
		gluBuild2DMipmaps(  GL_TEXTURE_2D,
							img->bpp / 8,
							img->width,
							img->height,
							GL_RGBA,
							GL_UNSIGNED_BYTE,
							(void*)img->data );
#ifdef VERBOSE							
		con_Print( "\tLoaded ( %d, %d, %d )", img->width, img->height, img->bpp );
#endif // VERBOSE
		if( (ret = glGetError()) != GL_NO_ERROR )
		{
			con_Print( "<RED>Colormap Load Failed:  Build2DMipmaps failed." );
			con_Print( "<RED>GL error: %s", gluErrorString(glGetError()) );
			glDeleteTextures( 1, &(cm->gl_name) );
			return 0;
		}
	}
	else
	{
#ifdef VERBOSE
		con_Print( "\tLoading single image (no mipmaps)" );
#endif // VERBOSE
		glTexImage2D(	GL_TEXTURE_2D,
						0,   // image L-O-D
						img->bpp / 8,   // color components 
						img->width,
						img->height,
						0,   // border pixels  ? 0 : 1
						GL_RGBA,
						GL_UNSIGNED_BYTE,
						img->data );
#ifdef VERBOSE
		con_Print( "\tLoaded ( %d, %d, %d )", img->width, img->height, img->bpp );
#endif // VERBOSE
		if( (ret = glGetError()) != GL_NO_ERROR )
		{
			con_Print( "<RED>Load Colormap Failed at glTexImage2D: %s", gluErrorString(ret) );
			glDeleteTextures( 1, &(cm->gl_name) );
			return 0;
		}
	}
	
	return 1;
}

/* ------------
cm_GetGlobalFilter - returns the ogl value of the global filter string;  the two options are GL_NEAREST,
				- represented as "nearest" and GL_LINEAR, represented as "linear" or "auto"
				- returns GL_NEAREST if the string "nearest" is specified, GL_LINEAR for everything else
------------ */
int cm_GetGlobalFilter()
{
	// assert
	if( vlTextureFilter == NULL )
	{
		con_Print( "<RED>Colormap GetGlobalFilter Assert Failed:  vlTextureFilter is NULL" );
		eng_Stop( "30012" );
		return GL_LINEAR;
	}

	// compare the filter value to nearest
	if( tcstrincmp( var_GetString(vlTextureFilter), "nearest", 7 ) )
	{
		return GL_NEAREST;
	}

	return GL_LINEAR;
}

/* ------------
cm_Unload - removes the indicated colormap from the colormap list and frees all heap and gl memory of the map
------------ */
void cm_Unload( colormap_t *cm )
{
	// assert
	if( cm == NULL )
	{
		con_Print( "<RED>Colormap Unload Assert Failed:  cm is NULL." );
		eng_Stop( "30010" );
		return;
	}
	// remove a user
	cm->users--;

	if( cm->users > 0 ) return;

	con_Print( "<RED>Unloading c<ORANGE>o<YELLOW>l<GREEN>o<BLUE>r</BLUE>map <VIOLET>\"%s\"</VIOLET>.", cm->name );
	eng_LoadingFrame();

	// remove the colormap from the list
	cm_Unlink( cm );

	// free its memory
	glDeleteTextures( 1, &(cm->gl_name) );
	SAFE_RELEASE( cm->name );
	SAFE_RELEASE( cm );
}

/* ------------
cm_FindColormap - returns the first item (and by def. the only item) in the list with 'name'
------------ */
static colormap_t *cm_FindColormap( const char *name )
{
	colormap_t *cmTmp;

	// assert
	if( name == NULL )
	{
		con_Print( "<RED>Colormap FindColormap Assert Failed:  name is NULL" );
		eng_Stop( "30011" );
		return NULL;
	}

	// init
	cmTmp = cmHead;
	while( cmTmp != NULL )
	{
		// safe comparison
		if( (cmTmp->name != NULL) && (tcstrncmp(cmTmp->name, name, tcstrlen(name))) )
		{
			return cmTmp;
		}

		// next
		cmTmp = cmTmp->next;
	}
	
	return NULL;
}

/* ------------
cm_PrintInfo - iterates the colormap list and prints relevant info about each loaded map
------------ */
void cm_PrintInfo( char *null )
{
	colormap_t *cmTmp;

	// print a header
	con_Print( "\nHuman Name | Width x Height x Bpp | GL Name | Users | Mipmapped" );
	con_Print( "------------------------------------------" );

	cmTmp = cmHead;
	while( cmTmp != NULL )
	{
		if( cmTmp->name != NULL )
		{
			// print each info
			con_Print( "%s | %d | %d | %d", cmTmp->name, cmTmp->gl_name, cmTmp->users, cmTmp->bMipmap );
		}

		cmTmp = cmTmp->next;
	}
}

// LIST MAINTAINENCE
/* ------------
cm_LinkTail - internal list maintainence, adds cm to the list at the tail
------------ */
static void cm_LinkTail( colormap_t *cm )
{
	if( cmHead == NULL ) cmHead = cm;

	if( cmTail == NULL )
	{
		cmTail = cm;
		cm->next = NULL;
		cm->prev = NULL;
		return;
	}

	cm->prev = cmTail;
	cm->next = NULL;

	cmTail->next = cm;
	cmTail = cm;
}

/* ------------
cm_Unlink - removes cm from the colormap list - repairs the list for cm's absence
------------ */
static void cm_Unlink( colormap_t *cm )
{
	if( cm->prev != NULL ) cm->prev->next = cm->next;
	if( cm->next != NULL ) cm->next->prev = cm->prev;

	if( cmTail == cm ) cmTail = cm->prev;
	if( cmHead == cm ) cmHead = cm->next;

	cm->prev = NULL;
	cm->next = NULL;
}
