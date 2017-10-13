// Cognition
// media_shader.c
// Created 9-12-02 by Terrence Cole

// Includes
/////////////
#include "cog_global.h"

// Definitions
////////////////
// #define SHADER_DIRECTORY "shaders"
// #define SHADER_FUNCT_NONE 0
// #define SHADER_FUNCT_SINE 1
// #define SHADER_FUNCT_LINEAR 2
// #define SHADER_MAX_FUNCTS 8

#define SHADER_TOKEN_SIZE 1024

// Local Structures
/////////////////////
typedef struct shader_file_s
{
	byte unlit; // <Boolean>
//	This flag specifies that lighting should not be performed on these surfaces.
	
	char color1[SHADER_TOKEN_SIZE]; // <string>
//	Specifies a colormap to use for the surface.  Must be a 24-bit plane.
	
	char alpha1[SHADER_TOKEN_SIZE]; // <string>
//	Specifies the alpha plane for use with color1.	If not specified, the alpha plane is taken as 100% opaque.	Only valid if color1 is also specified
	
	char color2[SHADER_TOKEN_SIZE]; // <string>
	char alpha2[SHADER_TOKEN_SIZE]; // <string>
//	Same as color1/alpha1.
	
	int color_size_bias; // <integer>
//	Specifies a linear addition to the global texture size bias in texture_detail.	The majority of images shipped with Cognition are stored as 2048x2048, then downsampled to a reasonable size at load time by texture_detail powers of two.	This bias is added to texture_detail before downsizing.  This may be useful for certain textures that are distributed at less than 2048x2048 and still occupy significant screenspace or for textures that are 'big' and occupy a particularly small screen space (the cursor).  Note well, an image will never be upscaled past its original size or downsampled past	128 as a minimum dimension.
	
	byte color_filter_near; // <Boolean>
//	If this is true, the engine will use nearest neighbor downsample/upsample filtering in GL, rather than mipmapping the image.  This will save memory for images that are only shown at screen resolution or for images where mipmapping is inappropriate. 
	
	byte color1_no_wrap; // <Boolean>
//	By default, a colormap will wrap around at polygon edges.  This causes the map to be applied without wrapping.
	
	byte color2_no_wrap; // <Boolean>
//	Like color1_no_wrap.
	
	byte color_no_rescale; // <Boolean>
//	Specifying this flag will shortcut the downsample stage and pass the image to GL at the loaded file resolution.  This should only be used for special critical images, as the raw size of a 2048x2048x32 image is 12MB, which is fairly cumbersome to deal with, even with compression.
	
	char bump[SHADER_TOKEN_SIZE]; // <string>
//	specifies the normal map for this shader.
	
	int bump_size_bias; // <integer>
//	Same as color_size_bias.
	
	byte bump_no_rescale; // <Boolean>
//	Like color_no_rescale.
	
	char env[SHADER_TOKEN_SIZE]; // <string>
//	Specifies the environment map for this shader.
	
	int env_size_bias; // <integer>
//	Same as color_size_bias.
	
	byte env_no_rescale; // <Boolean>
//	Same as color_no_rescale.
	
	byte use_material; // <Boolean>
//	The following parameters will only work if this flag is set.  Even with this flag, they will only work on certain surfaces.  In general, use of GL materials has been untested, and will remain so given how much better texture mapping looks than plastic.
	
	float ambient_red; // <float [0-1]>
	float ambient_green; // <float [0-1]>
	float ambient_blue; // <float [0-1]>
//	These parameters specify the materials ability to reflect ambient lighting.
	
	float diffuse_red; // <float [0-1]>
	float diffuse_green; // <float [0-1]>
	float diffuse_blue; // <float [0-1]>
//	These parameters specify the materials ability to reflect diffuse lighting.
	
	float specular_red; // <float [0-1]>
	float specular_green; // <float [0-1]>
	float specular_blue; // <float [0-1]>
	int shininess; // <integer [0-128]>
//	These parameters specify the materials ability to reflect specular lighting and the fudge factor for  the highlight size.

	int iNumControllers;
	shader_funct_t controller[8];
//	transS <string [sine, linear] function> <float rate> <float t0> <float A>
//	transT <string [sine, linear] function> <float rate> <float t0> <float A>
//	These parameters specify functions that control the slew rate in each mapping dimension.  In addition to a function, three floats specify the rate, t0, and the amplitude. 
} shader_file_t;

// Global Prototypes
//////////////////////
/*
int sh_Initialize(void);
void sh_Terminate(void);
shader_t *sh_LoadShader( char *name );
void sh_UnloadShader( shader_t *shader );
void sh_PrintInfo( char *null );
uint8_t sh_ReloadAllMaps(void);
*/

// Local Prototypes
/////////////////////
static shader_t *sh_FindShader( char *name );
static shader_t *sh_LoadFromFile( char *name );
static void sh_ReleaseShader( shader_t *shader );
static void sh_LinkTail( shader_t *sh );
static void sh_Unlink( shader_t *sh );

// Local Variables
////////////////////
static shader_t *shHead = NULL;
static shader_t *shTail = NULL;

// *********** FUNCTIONALITY ***********
/* ------------
sh_Initialize - sets up an empty shader list - 1 on success, 0 on fail
------------ */
int sh_Initialize(void)
{
	assert( shHead == NULL );
	assert( shTail == NULL );
	
	return 1;
}

/* ------------
sh_Terminate - clear any remaining shaders in the list and delete the list
------------ */
void sh_Terminate(void)
{
	int count = 0;
	shader_t *shTmp = shHead;

	while( shHead != NULL )
	{
		con_Print( "\nTerminating Shader System..." );
	
		// copy over the next head
		shTmp = shHead->next;

		// free this shader
		sh_ReleaseShader( shHead );

		// set the next head
		shHead = shTmp;

		// count it
		count++;
	}
	if( count > 0 )
	{
		con_Print( "\tFound and released %d shaders...", count );
	}
}

/* ------------
sh_LoadShader - returns a reference to the named shader, loads it if it is not already available
------------ */
shader_t *sh_LoadShader( char *name )
{
	shader_t *shader = NULL;

	// assert
	if( name == NULL ) 
	{
		con_Print( "<RED>Assertion Failure: passed parameter \'name\' was null in sh_LoadShader." );
		eng_Stop( "70001" );
		return NULL;
	}

	// attempt the find the already loaded shader
	shader = sh_FindShader( name );
	if( shader != NULL )
	{
		shader->users++;
		return shader;
	}

	// attempt to load the shader from file since it wasn't found in the loaded list
	shader = sh_LoadFromFile( name);
	if( shader == NULL )
	{
		con_Print( "Shader Load Failed." );
		return NULL;
	}

	// increment the users
	shader->users++;

	// add the shader to the list of open shaders
	sh_LinkTail( shader );

	return shader;
}

/* ------------
sh_UnloadShader - decriments the usage count of this shader and releases it if it is no longer useful
------------ */
void sh_UnloadShader( shader_t *shader )
{
	if( shader == NULL ) return;

	// decriment the users
	shader->users--;

	// only free if no users remain
	if( shader->users > 0 ) return;

	// Unlink the shader and free it
	sh_Unlink( shader );
	sh_ReleaseShader( shader );
}

/* ------------
sh_FindShader - returns the reference to the already loaded shader named 'name' or NULL if it does not exist.
------------ */
static shader_t *sh_FindShader( char *name )
{
	shader_t *shTmp;

	// assert
	if( name == NULL )
	{
		con_Print( "<RED>Assert Failed in sh_FindShader: name was NULL" );
		eng_Stop( "70002" );
		return NULL;
	}
	if( tcstrlen(name) < 3 )
	{
		con_Print( "<RED>Assert Failed in sh_FindShader: name was too short to be a shader" );
		eng_Stop( "70003" );
		return NULL;
	}

	shTmp = shHead;
	while( shTmp != NULL )
	{
		if( tcstrcmp( shTmp->name, name ) )
		{
			return shTmp;
		}

		shTmp = shTmp->next;
	}

	return NULL;
}



/* ------------
sh_LoadFromFile - opens the file at path and parses the contents into a new shader.
				- returns the new shader when successful, or NULL on failure
------------ */
static shader_t *sh_LoadFromFile( char *name )
{
	CHAR *buffer; // the buffered file
	CHAR *cCurPos; // the current buffer position
	CHAR *path;
	CHAR cCurToken[1024];
	shader_t *shader; // the address of the new shader
	int32_t iBufLen;
	int32_t iToToken;
	int32_t iTokenLen;
	shader_file_t sh;
	char cmName[1024];
	colormap_params_t cmParams;

#ifdef DEBUG
	assert( name != NULL );
	assert( tcstrlen(name) > 3 );
#endif
	// assert
	if( name == NULL )
	{
		con_Print( "<RED>Assert Failed in sh_LoadFromFile: name was NULL" );
		eng_Stop( "70004" );
		return NULL;
	}
	if( tcstrlen(name) < 3 )
	{
		con_Print( "<RED>Assert Failed in sh_LoadFromFile: name was too short to be a shader" );
		eng_Stop( "70005" );
		return NULL;
	}

	// resolve a full path for the shader
	path = fs_GetMediaPath( name, SHADER_DIRECTORY );
	if( path == NULL )
	{
		con_Print( "Shader Load Error:	Unable to Resolve a path to shader \"%s\".", name );
		return NULL;
	}

	// tell us about it
	con_Print( "<BLUE>Loading</BLUE> shader from <ORANGE>\"%s\"</ORANGE>.", path );
	eng_LoadingFrame();

	// buffer the contents of the shader
	buffer = par_BufferFile( path );
	if( buffer == NULL )
	{
		con_Print( "<RED>Shader Load Error:  Unable to buffer file at \"%s\".", path );
		return NULL;
	}
	iBufLen = tcstrlen(buffer);

	// clear shader comments
	par_StripComments( buffer );

	// fill in the shader_file with (non-zero) default values for all components
	memset( &sh, 0, sizeof(shader_file_t) );
	sh.ambient_red = 1.0f; // <float [0-1]>
	sh.ambient_green = 1.0f; // <float [0-1]>
	sh.ambient_blue = 1.0f; // <float [0-1]>
	sh.diffuse_red = 1.0f; // <float [0-1]>
	sh.diffuse_green = 1.0f; // <float [0-1]>
	sh.diffuse_blue = 1.0f; // <float [0-1]>
	sh.specular_red = 1.0f; // <float [0-1]>
	sh.specular_green = 1.0f; // <float [0-1]>
	sh.specular_blue = 1.0f; // <float [0-1]>

	// iterate the tokens
	cCurPos = buffer;
	while( par_GetNextToken( cCurPos, &iToToken, &iTokenLen ) )
	{
		// check for weird errors /malicious overflows
		if( iTokenLen > 1023 )
		{
			con_Print( "<RED>Shader Load Error:  Found an exceptionally long token in %s", name );
			SAFE_RELEASE( buffer );
			return 0;
		}

		// check for other weird errors
		if( iTokenLen < 1 ) 
		{
			con_Print( "<RED>Shader Load Error:  Found a 0 length token(!) in %s", name );
			SAFE_RELEASE( buffer );
			return 0;
		}

		// advance to the token
		cCurPos += iToToken;

		// copy the token
		memset( cCurToken, 0, SHADER_TOKEN_SIZE );
		cCurPos += par_QuotedStringCopy( cCurToken, cCurPos, 1024 );

		// check the token against our possible values:
		// n.b. This will be LONG, PAINFUL, and largly FRUITLESS
		// but also will execute faster and safer than greping for individual token pairs
		if( tcstrcmp( cCurToken, "unlit" ) )
		{
			// get the boolean value
			if( !par_GetNextToken( cCurPos, &iToToken, &iTokenLen ) || iTokenLen >= SHADER_TOKEN_SIZE || iTokenLen < 1 )
			{
				con_Print( "<RED>Shader Load Error:  Found a malformed token in %s", name );
				SAFE_RELEASE( buffer );
				return 0;
			}
			cCurPos += iToToken;
			memset( cCurToken, 0, SHADER_TOKEN_SIZE );
			cCurPos += par_QuotedStringCopy( cCurToken, cCurPos, iTokenLen );

			// set the flag if needed
			if( tcstrcmp( cCurToken, "true" ) ) sh.unlit = 1; 
			else sh.unlit = 0;
		}
		else if( tcstrcmp( cCurToken, "color1" ) )
		{
			if( !par_GetNextToken( cCurPos, &iToToken, &iTokenLen ) || iTokenLen >= SHADER_TOKEN_SIZE || iTokenLen < 1 )
			{
				con_Print( "<RED>Shader Load Error:  Found a malformed token in %s", name );
				SAFE_RELEASE( buffer );
				return 0;
			}
			cCurPos += iToToken;
			memset( sh.color1, 0, SHADER_TOKEN_SIZE );
			cCurPos += par_QuotedStringCopy( sh.color1, cCurPos, iTokenLen );
		}
		else if( tcstrcmp( cCurToken, "alpha1" ) )
		{
			if( !par_GetNextToken( cCurPos, &iToToken, &iTokenLen ) || iTokenLen >= SHADER_TOKEN_SIZE || iTokenLen < 1 )
			{
				con_Print( "<RED>Shader Load Error:  Found a malformed token in %s", name );
				SAFE_RELEASE( buffer );
				return 0;
			}
			cCurPos += iToToken;
			memset( sh.alpha1, 0, SHADER_TOKEN_SIZE );
			cCurPos += par_QuotedStringCopy( sh.alpha1, cCurPos, iTokenLen );
		}
		else  if( tcstrcmp( cCurToken, "color2" ) )
		{
			if( !par_GetNextToken( cCurPos, &iToToken, &iTokenLen ) || iTokenLen >= SHADER_TOKEN_SIZE || iTokenLen < 1 )
			{
				con_Print( "<RED>Shader Load Error:  Found a malformed token in %s", name );
				SAFE_RELEASE( buffer );
				return 0;
			}
			cCurPos += iToToken;
			memset( sh.color2, 0, SHADER_TOKEN_SIZE );
			cCurPos += par_QuotedStringCopy( sh.color2, cCurPos, iTokenLen );
		}
		else if( tcstrcmp( cCurToken, "alpha2" ) )
		{
			if( !par_GetNextToken( cCurPos, &iToToken, &iTokenLen ) || iTokenLen >= SHADER_TOKEN_SIZE || iTokenLen < 1 )
			{
				con_Print( "<RED>Shader Load Error:  Found a malformed token in %s", name );
				SAFE_RELEASE( buffer );
				return 0;
			}
			cCurPos += iToToken;
			memset( sh.alpha2, 0, SHADER_TOKEN_SIZE );
			cCurPos += par_QuotedStringCopy( sh.alpha2, cCurPos, iTokenLen );
		}
		else if( tcstrcmp( cCurToken, "color_size_bias" ) )
		{
			if( !par_GetNextToken( cCurPos, &iToToken, &iTokenLen ) || iTokenLen >= SHADER_TOKEN_SIZE || iTokenLen < 1 )
			{
				con_Print( "<RED>Shader Load Error:  Found a malformed token in %s", name );
				SAFE_RELEASE( buffer );
				return 0;
			}
			cCurPos += iToToken;
			memset( cCurToken, 0, SHADER_TOKEN_SIZE );
			cCurPos += par_QuotedStringCopy( cCurToken, cCurPos, iTokenLen );

			sh.color_size_bias = tcatoi( cCurToken );
		}
		else if( tcstrcmp( cCurToken, "color_filter_near" ) )
		{
			// get the boolean value
			if( !par_GetNextToken( cCurPos, &iToToken, &iTokenLen ) || iTokenLen >= SHADER_TOKEN_SIZE || iTokenLen < 1 )
			{
				con_Print( "<RED>Shader Load Error:  Found a malformed token in %s", name );
				SAFE_RELEASE( buffer );
				return 0;
			}
			cCurPos += iToToken;
			memset( cCurToken, 0, SHADER_TOKEN_SIZE );
			cCurPos += par_QuotedStringCopy( cCurToken, cCurPos, iTokenLen );

			// set the flag if needed
			if( tcstrcmp( cCurToken, "true" ) ) sh.color_filter_near = 1; 
			else sh.color_filter_near = 0;
		}
		else if( tcstrcmp( cCurToken, "color1_no_wrap" ) )
		{
			// get the boolean value
			if( !par_GetNextToken( cCurPos, &iToToken, &iTokenLen ) || iTokenLen >= SHADER_TOKEN_SIZE || iTokenLen < 1 )
			{
				con_Print( "<RED>Shader Load Error:  Found a malformed token in %s", name );
				SAFE_RELEASE( buffer );
				return 0;
			}
			cCurPos += iToToken;
			memset( cCurToken, 0, SHADER_TOKEN_SIZE );
			cCurPos += par_QuotedStringCopy( cCurToken, cCurPos, iTokenLen );

			// set the flag if needed
			if( tcstrcmp( cCurToken, "true" ) ) sh.color1_no_wrap = 1; 
			else sh.color1_no_wrap = 0;
		}
		else if( tcstrcmp( cCurToken, "color2_no_wrap" ) )
		{
			// get the boolean value
			if( !par_GetNextToken( cCurPos, &iToToken, &iTokenLen ) || iTokenLen >= SHADER_TOKEN_SIZE || iTokenLen < 1 )
			{
				con_Print( "<RED>Shader Load Error:  Found a malformed token in %s", name );
				SAFE_RELEASE( buffer );
				return 0;
			}
			cCurPos += iToToken;
			memset( cCurToken, 0, SHADER_TOKEN_SIZE );
			cCurPos += par_QuotedStringCopy( cCurToken, cCurPos, iTokenLen );

			// set the flag if needed
			if( tcstrcmp( cCurToken, "true" ) ) sh.color2_no_wrap = 1; 
			else sh.color2_no_wrap = 0;
		}
		else if( tcstrcmp( cCurToken, "color_no_rescale" ) )
		{
			// get the boolean value
			if( !par_GetNextToken( cCurPos, &iToToken, &iTokenLen ) || iTokenLen >= SHADER_TOKEN_SIZE || iTokenLen < 1 )
			{
				con_Print( "<RED>Shader Load Error:  Found a malformed token in %s", name );
				SAFE_RELEASE( buffer );
				return 0;
			}
			cCurPos += iToToken;
			memset( cCurToken, 0, SHADER_TOKEN_SIZE );
			cCurPos += par_QuotedStringCopy( cCurToken, cCurPos, iTokenLen );

			// set the flag if needed
			if( tcstrcmp( cCurToken, "true" ) ) sh.color_no_rescale = 1; 
			else sh.color_no_rescale = 0;
		}
		else if( tcstrcmp( cCurToken, "bump" ) )
		{
			if( !par_GetNextToken( cCurPos, &iToToken, &iTokenLen ) || iTokenLen >= SHADER_TOKEN_SIZE || iTokenLen < 1 )
			{
				con_Print( "<RED>Shader Load Error:  Found a malformed token in %s", name );
				SAFE_RELEASE( buffer );
				return 0;
			}
			cCurPos += iToToken;
			memset( sh.bump, 0, SHADER_TOKEN_SIZE );
			cCurPos += par_QuotedStringCopy( sh.bump, cCurPos, iTokenLen );
		}
		else if( tcstrcmp( cCurToken, "bump_size_bias" ) )
		{
			if( !par_GetNextToken( cCurPos, &iToToken, &iTokenLen ) || iTokenLen >= SHADER_TOKEN_SIZE || iTokenLen < 1 )
			{
				con_Print( "<RED>Shader Load Error:  Found a malformed token in %s", name );
				SAFE_RELEASE( buffer );
				return 0;
			}
			cCurPos += iToToken;
			memset( cCurToken, 0, SHADER_TOKEN_SIZE );
			cCurPos += par_QuotedStringCopy( cCurToken, cCurPos, iTokenLen );

			sh.bump_size_bias = tcatoi( cCurToken );
		}
		else if( tcstrcmp( cCurToken, "bump_no_rescale" ) )
		{
			// get the boolean value
			if( !par_GetNextToken( cCurPos, &iToToken, &iTokenLen ) || iTokenLen >= SHADER_TOKEN_SIZE || iTokenLen < 1 )
			{
				con_Print( "<RED>Shader Load Error:  Found a malformed token in %s", name );
				SAFE_RELEASE( buffer );
				return 0;
			}
			cCurPos += iToToken;
			memset( cCurToken, 0, SHADER_TOKEN_SIZE );
			cCurPos += par_QuotedStringCopy( cCurToken, cCurPos, iTokenLen );

			// set the flag if needed
			if( tcstrcmp( cCurToken, "true" ) ) sh.bump_no_rescale = 1; 
			else sh.bump_no_rescale = 0;
		}
		else if( tcstrcmp( cCurToken, "env" ) )
		{
			if( !par_GetNextToken( cCurPos, &iToToken, &iTokenLen ) || iTokenLen >= SHADER_TOKEN_SIZE || iTokenLen < 1 )
			{
				con_Print( "<RED>Shader Load Error:  Found a malformed token in %s", name );
				SAFE_RELEASE( buffer );
				return 0;
			}
			cCurPos += iToToken;
			memset( sh.env, 0, SHADER_TOKEN_SIZE );
			cCurPos += par_QuotedStringCopy( sh.env, cCurPos, iTokenLen );
		}
		else if( tcstrcmp( cCurToken, "env_size_bias" ) )
		{
			if( !par_GetNextToken( cCurPos, &iToToken, &iTokenLen ) || iTokenLen >= SHADER_TOKEN_SIZE || iTokenLen < 1 )
			{
				con_Print( "<RED>Shader Load Error:  Found a malformed token in %s", name );
				SAFE_RELEASE( buffer );
				return 0;
			}
			cCurPos += iToToken;
			memset( cCurToken, 0, SHADER_TOKEN_SIZE );
			cCurPos += par_QuotedStringCopy( cCurToken, cCurPos, iTokenLen );

			sh.env_size_bias = tcatoi( cCurToken );
		}
		else if( tcstrcmp( cCurToken, "env_no_rescale" ) )
		{
			if( !par_GetNextToken( cCurPos, &iToToken, &iTokenLen ) || iTokenLen >= SHADER_TOKEN_SIZE || iTokenLen < 1 )
			{
				con_Print( "<RED>Shader Load Error:  Found a malformed token in %s", name );
				SAFE_RELEASE( buffer );
				return 0;
			}
			cCurPos += iToToken;
			memset( cCurToken, 0, SHADER_TOKEN_SIZE );
			cCurPos += par_QuotedStringCopy( cCurToken, cCurPos, iTokenLen );

			if( tcstrcmp( cCurToken, "true" ) ) sh.env_no_rescale = 1; 
			else sh.env_no_rescale = 0;
		}

		else if( tcstrncmp( cCurToken, "trans", 5 ) )
		{
			// check for too many controllers: decriment and report a warning
			if( sh.iNumControllers >= 8 )
			{
				con_Print( "\t<RED>Shader Load Warning:  More than 8 Controllers are specified on this shader." );
				sh.iNumControllers--;
			}
		
			// set the axis
			if( cCurToken[5] == 'S' ) sh.controller[sh.iNumControllers].bAxis = S_AXIS;
			else sh.controller[sh.iNumControllers].bAxis = T_AXIS;
			
			// get and set the function
			if( !par_GetNextToken( cCurPos, &iToToken, &iTokenLen ) || iTokenLen >= SHADER_TOKEN_SIZE || iTokenLen < 1 )
			{
				con_Print( "<RED>Shader Load Error:  Found a malformed token in %s", name );
				SAFE_RELEASE( buffer );
				return 0;
			}
			cCurPos += iToToken;
			memset( cCurToken, 0, SHADER_TOKEN_SIZE );
			cCurPos += par_QuotedStringCopy( cCurToken, cCurPos, iTokenLen );
			if( tcstrcmp( cCurToken, "sine" ) ) sh.controller[sh.iNumControllers].bFunct = SHADER_FUNCT_SINE;
			else if( tcstrcmp( cCurToken, "linear" ) ) sh.controller[sh.iNumControllers].bFunct = SHADER_FUNCT_LINEAR;
			else  sh.controller[sh.iNumControllers].bFunct = SHADER_FUNCT_NONE;

			// get and set the slew rate
			if( !par_GetNextToken( cCurPos, &iToToken, &iTokenLen ) || iTokenLen >= SHADER_TOKEN_SIZE || iTokenLen < 1 )
			{
				con_Print( "<RED>Shader Load Error:  Found a malformed token in %s", name );
				SAFE_RELEASE( buffer );
				return 0;
			}
			cCurPos += iToToken;
			memset( cCurToken, 0, SHADER_TOKEN_SIZE );
			cCurPos += par_QuotedStringCopy( cCurToken, cCurPos, iTokenLen );
			sh.controller[sh.iNumControllers].fRate = tcatof( cCurToken );

			// get and set t0
			if( !par_GetNextToken( cCurPos, &iToToken, &iTokenLen ) || iTokenLen >= SHADER_TOKEN_SIZE || iTokenLen < 1 )
			{
				con_Print( "<RED>Shader Load Error:  Found a malformed token in %s", name );
				SAFE_RELEASE( buffer );
				return 0;
			}
			cCurPos += iToToken;
			memset( cCurToken, 0, SHADER_TOKEN_SIZE );
			cCurPos += par_QuotedStringCopy( cCurToken, cCurPos, iTokenLen );
			sh.controller[sh.iNumControllers].fT0 = tcatof( cCurToken );

			// get and set A
			if( !par_GetNextToken( cCurPos, &iToToken, &iTokenLen ) || iTokenLen >= SHADER_TOKEN_SIZE || iTokenLen < 1 )
			{
				con_Print( "<RED>Shader Load Error:  Found a malformed token in %s", name );
				SAFE_RELEASE( buffer );
				return 0;
			}
			cCurPos += iToToken;
			memset( cCurToken, 0, SHADER_TOKEN_SIZE );
			cCurPos += par_QuotedStringCopy( cCurToken, cCurPos, iTokenLen );
			sh.controller[sh.iNumControllers].fAmp = tcatof( cCurToken );

			// add a controller
			sh.iNumControllers++;
		}

		else if( tcstrcmp( cCurToken, "use_material" ) )
		{
			// get the boolean value
			if( !par_GetNextToken( cCurPos, &iToToken, &iTokenLen ) || iTokenLen >= SHADER_TOKEN_SIZE || iTokenLen < 1 )
			{
				con_Print( "<RED>Shader Load Error:  Found a malformed token in %s", name );
				SAFE_RELEASE( buffer );
				return 0;
			}
			cCurPos += iToToken;
			memset( cCurToken, 0, SHADER_TOKEN_SIZE );
			cCurPos += par_QuotedStringCopy( cCurToken, cCurPos, iTokenLen );

			// set the flag if needed
			if( tcstrcmp( cCurToken, "true" ) ) sh.use_material = 1; 
			else sh.use_material = 0;
		}
		else if( tcstrcmp( cCurToken, "ambient_red" ) )
		{
			if( !par_GetNextToken( cCurPos, &iToToken, &iTokenLen ) || iTokenLen >= SHADER_TOKEN_SIZE || iTokenLen < 1 )
			{
				con_Print( "<RED>Shader Load Error:  Found a malformed token in %s", name );
				SAFE_RELEASE( buffer );
				return 0;
			}
			cCurPos += iToToken;
			memset( cCurToken, 0, SHADER_TOKEN_SIZE );
			cCurPos += par_QuotedStringCopy( cCurToken, cCurPos, iTokenLen );

			sh.ambient_red = tcatof( cCurToken );
			if( sh.ambient_red < 0.0f ) sh.ambient_red = 0.0f;
			else if( sh.ambient_red > 1.0f ) sh.ambient_red = 1.0f;
		}
		else if( tcstrcmp( cCurToken, "ambient_green" ) )
		{
			if( !par_GetNextToken( cCurPos, &iToToken, &iTokenLen ) || iTokenLen >= SHADER_TOKEN_SIZE || iTokenLen < 1 )
			{
				con_Print( "<RED>Shader Load Error:  Found a malformed token in %s", name );
				SAFE_RELEASE( buffer );
				return 0;
			}
			cCurPos += iToToken;
			memset( cCurToken, 0, SHADER_TOKEN_SIZE );
			cCurPos += par_QuotedStringCopy( cCurToken, cCurPos, iTokenLen );

			sh.ambient_green = tcatof( cCurToken );
			if( sh.ambient_green < 0.0f ) sh.ambient_green = 0.0f;
			else if( sh.ambient_green > 1.0f ) sh.ambient_green = 1.0f;
		}
		else if( tcstrcmp( cCurToken, "ambient_blue" ) )
		{
			if( !par_GetNextToken( cCurPos, &iToToken, &iTokenLen ) || iTokenLen >= SHADER_TOKEN_SIZE || iTokenLen < 1 )
			{
				con_Print( "<RED>Shader Load Error:  Found a malformed token in %s", name );
				SAFE_RELEASE( buffer );
				return 0;
			}
			cCurPos += iToToken;
			memset( cCurToken, 0, SHADER_TOKEN_SIZE );
			cCurPos += par_QuotedStringCopy( cCurToken, cCurPos, iTokenLen );

			sh.ambient_blue = tcatof( cCurToken );
			if( sh.ambient_blue < 0.0f ) sh.ambient_blue = 0.0f;
			else if( sh.ambient_blue > 1.0f ) sh.ambient_blue = 1.0f;
		}
		else if( tcstrcmp( cCurToken, "diffuse_red" ) )
		{
			if( !par_GetNextToken( cCurPos, &iToToken, &iTokenLen ) || iTokenLen >= SHADER_TOKEN_SIZE || iTokenLen < 1 )
			{
				con_Print( "<RED>Shader Load Error:  Found a malformed token in %s", name );
				SAFE_RELEASE( buffer );
				return 0;
			}
			cCurPos += iToToken;
			memset( cCurToken, 0, SHADER_TOKEN_SIZE );
			cCurPos += par_QuotedStringCopy( cCurToken, cCurPos, iTokenLen );

			sh.diffuse_red = tcatof( cCurToken );
			if( sh.diffuse_red < 0.0f ) sh.diffuse_red = 0.0f;
			else if( sh.diffuse_red > 1.0f ) sh.diffuse_red = 1.0f;
		}
		else if( tcstrcmp( cCurToken, "diffuse_green" ) )
		{
			if( !par_GetNextToken( cCurPos, &iToToken, &iTokenLen ) || iTokenLen >= SHADER_TOKEN_SIZE || iTokenLen < 1 )
			{
				con_Print( "<RED>Shader Load Error:  Found a malformed token in %s", name );
				SAFE_RELEASE( buffer );
				return 0;
			}
			cCurPos += iToToken;
			memset( cCurToken, 0, SHADER_TOKEN_SIZE );
			cCurPos += par_QuotedStringCopy( cCurToken, cCurPos, iTokenLen );

			sh.diffuse_green = tcatof( cCurToken );
			if( sh.diffuse_green < 0.0f ) sh.diffuse_green = 0.0f;
			else if( sh.diffuse_green > 1.0f ) sh.diffuse_green = 1.0f;
		}
		else if( tcstrcmp( cCurToken, "diffuse_blue" ) )
		{
			if( !par_GetNextToken( cCurPos, &iToToken, &iTokenLen ) || iTokenLen >= SHADER_TOKEN_SIZE || iTokenLen < 1 )
			{
				con_Print( "<RED>Shader Load Error:  Found a malformed token in %s", name );
				SAFE_RELEASE( buffer );
				return 0;
			}
			cCurPos += iToToken;
			memset( cCurToken, 0, SHADER_TOKEN_SIZE );
			cCurPos += par_QuotedStringCopy( cCurToken, cCurPos, iTokenLen );

			sh.diffuse_blue = tcatof( cCurToken );
			if( sh.diffuse_blue < 0.0f ) sh.diffuse_blue = 0.0f;
			else if( sh.diffuse_blue > 1.0f ) sh.diffuse_blue = 1.0f;
		}
		else if( tcstrcmp( cCurToken, "specular_red" ) )
		{
			if( !par_GetNextToken( cCurPos, &iToToken, &iTokenLen ) || iTokenLen >= SHADER_TOKEN_SIZE || iTokenLen < 1 )
			{
				con_Print( "<RED>Shader Load Error:  Found a malformed token in %s", name );
				SAFE_RELEASE( buffer );
				return 0;
			}
			cCurPos += iToToken;
			memset( cCurToken, 0, SHADER_TOKEN_SIZE );
			cCurPos += par_QuotedStringCopy( cCurToken, cCurPos, iTokenLen );

			sh.specular_red = tcatof( cCurToken );
			if( sh.specular_red < 0.0f ) sh.specular_red = 0.0f;
			else if( sh.specular_red > 1.0f ) sh.specular_red = 1.0f;
		}
		else if( tcstrcmp( cCurToken, "specular_green" ) )
		{
			if( !par_GetNextToken( cCurPos, &iToToken, &iTokenLen ) || iTokenLen >= SHADER_TOKEN_SIZE || iTokenLen < 1 )
			{
				con_Print( "<RED>Shader Load Error:  Found a malformed token in %s", name );
				SAFE_RELEASE( buffer );
				return 0;
			}
			cCurPos += iToToken;
			memset( cCurToken, 0, SHADER_TOKEN_SIZE );
			cCurPos += par_QuotedStringCopy( cCurToken, cCurPos, iTokenLen );

			sh.specular_green = tcatof( cCurToken );
			if( sh.specular_green < 0.0f ) sh.specular_green = 0.0f;
			else if( sh.specular_green > 1.0f ) sh.specular_green = 1.0f;
		}
		else if( tcstrcmp( cCurToken, "specular_blue" ) )
		{
			if( !par_GetNextToken( cCurPos, &iToToken, &iTokenLen ) || iTokenLen >= SHADER_TOKEN_SIZE || iTokenLen < 1 )
			{
				con_Print( "<RED>Shader Load Error:  Found a malformed token in %s", name );
				SAFE_RELEASE( buffer );
				return 0;
			}
			cCurPos += iToToken;
			memset( cCurToken, 0, SHADER_TOKEN_SIZE );
			cCurPos += par_QuotedStringCopy( cCurToken, cCurPos, iTokenLen );

			sh.specular_blue = tcatof( cCurToken );
			if( sh.specular_blue < 0.0f ) sh.specular_blue = 0.0f;
			else if( sh.specular_blue > 1.0f ) sh.specular_blue = 1.0f;
		}
		else if( tcstrcmp( cCurToken, "shininess" ) )
		{
			if( !par_GetNextToken( cCurPos, &iToToken, &iTokenLen ) || iTokenLen >= SHADER_TOKEN_SIZE || iTokenLen < 1 )
			{
				con_Print( "<RED>Shader Load Error:  Found a malformed token in %s", name );
				SAFE_RELEASE( buffer );
				return 0;
			}
			cCurPos += iToToken;
			memset( cCurToken, 0, SHADER_TOKEN_SIZE );
			cCurPos += par_QuotedStringCopy( cCurToken, cCurPos, iTokenLen );

			sh.shininess = tcatoi( cCurToken );
			if( sh.shininess < 0 ) sh.shininess = 0;
			else if( sh.shininess > 255 ) sh.shininess = 255;
		}
		else
		{
			con_Print( "\t<RED>Found an unrecognized token \"%s\" in shader, skipping." );
		}
	}

	// if we made it here, we have a shader_file_t that specifies how and what to load
	// get memory for the shader
	shader = (shader_t*)mem_alloc( sizeof(shader_t) );
	if( shader == NULL )
	{
		con_Print( "<RED>Shader Load Error:  Unable to allocate memory for the shader object." );
		SAFE_RELEASE( buffer );
		return NULL;
	}
	memset( shader, 0, sizeof(shader_t) );

	// clear the load parameters
	memset( &cmParams, 0, sizeof(colormap_params_t) );

	// copy the name
	tcstrncpy( shader->name, name, SHADER_NAME_SIZE );
#ifdef VERBOSE
	con_Print( "\tShader Name:	%s", shader->name );
#endif // VERBOSE
	if( sh.unlit ) shader->bApplyLights = 0;
	else shader->bApplyLights = 1;

	// load the 1st color map
	if( tcstrlen(sh.color1) > 0 )
	{
#ifdef VERBOSE
		con_Print( "\tLoading colormap1 from \"%s\"", sh.color1 );
#endif // VERBOSE		
		// setup parameters
		cmParams.bMipmap = 1;
		cmParams.iEnv = GL_MODULATE;
		cmParams.bNoRescale = sh.color_no_rescale;
		cmParams.iScaleBias = sh.color_size_bias;

		// filtering
		if( sh.color_filter_near > 0 ) cmParams.iFilter = GL_NEAREST;
		else cmParams.iFilter = cm_GetGlobalFilter();
		
		// wrapping
		if( sh.color1_no_wrap ) cmParams.iWrap = GL_CLAMP;
		else cmParams.iWrap = GL_REPEAT;
		
		// check for an alpha map to add
		if( tcstrlen(sh.alpha1) > 0 )
		{
#ifdef VERBOSE
			con_Print( "\tLoading alphamap1 from \"%s\"", sh.alpha1 );
#endif // VERBOSE
			// get a good name for this composite image
			memset( cmName, 0, 1024 );
			sprintf( cmName, "%s_SPLICE_%s", sh.color1, sh.alpha1 );
		
			// load with alpha			
			shader->ColorMap = cm_LoadFromFiles( cmName, sh.color1, sh.alpha1, &cmParams );
			shader->bHasTransparency = 1;
		}
		else
		{
			// load singularly
			shader->ColorMap = cm_LoadConstAlpha( sh.color1, sh.color1, IMG_ALPHA_SOLID, &cmParams );
		}
	}
	
	// load the 2nd color map
	if( tcstrlen(sh.color2) > 0 )
	{
#ifdef VERBOSE
		con_Print( "\tLoading colormap2 from \"%s\"", sh.color2 );
#endif // VERBOSE
		// setup parameters
		cmParams.bMipmap = 1;
		cmParams.iEnv = GL_MODULATE;
		cmParams.bNoRescale = sh.color_no_rescale;
		cmParams.iScaleBias = sh.color_size_bias;

		// filtering
		if( sh.color_filter_near > 0 ) cmParams.iFilter = GL_NEAREST;
		else cmParams.iFilter = cm_GetGlobalFilter();
		
		// wrapping
		if( sh.color2_no_wrap ) cmParams.iWrap = GL_CLAMP;
		else cmParams.iWrap = GL_REPEAT;
		
		// check for an alpha map to add
		if( tcstrlen(sh.alpha2) > 0 )
		{
#ifdef VERBOSE
			con_Print( "\tLoading alphamap2 from \"%s\"", sh.alpha2 );
#endif // VERBOSE
			// get a good name for this composite image
			memset( cmName, 0, 1024 );
			sprintf( cmName, "%s_SPLICE_%s", sh.color2, sh.alpha2 );
		
			// load with alpha			
			shader->subColorMap = cm_LoadFromFiles( cmName, sh.color2, sh.alpha2, &cmParams );
			shader->bHasTransparency = 1;
		}
		else
		{
			// load singularly
			shader->subColorMap = cm_LoadConstAlpha( sh.color2, sh.color2, IMG_ALPHA_SOLID, &cmParams );
		}
	}

	// load the bump
	if( tcstrlen(sh.bump) > 0 )
	{
#ifdef VERBOSE
		con_Print( "\tLoading bumpmap from \"%s\"", sh.bump );
#endif // VERBOSE
		// setup parameters
		cmParams.bMipmap = 0;
		cmParams.iEnv = GL_MODULATE;
		cmParams.iFilter = cm_GetGlobalFilter();
		cmParams.iWrap = GL_REPEAT;
		cmParams.bNoRescale = sh.bump_no_rescale;
		cmParams.iScaleBias = sh.bump_size_bias;

		// load the map
		shader->BumpMap = cm_LoadConstAlpha( sh.bump, sh.bump, IMG_ALPHA_SOLID, &cmParams );
	}

	// load the environment map
	if( tcstrlen(sh.env) > 0 )
	{
#ifdef VERBOSE
		con_Print( "\tLoading environment map from \"%s\"", sh.env );
#endif // VERBOSE
		// setup parameters
		cmParams.bMipmap = 1;
		cmParams.iEnv = GL_MODULATE;
		cmParams.iFilter = cm_GetGlobalFilter();
		cmParams.iWrap = GL_REPEAT;
		cmParams.bNoRescale = sh.env_no_rescale;
		cmParams.iScaleBias = sh.env_size_bias;

		// load singularly
		shader->EnvMap = cm_LoadConstAlpha( sh.env, sh.env, IMG_ALPHA_SOLID, &cmParams );
	}

	// the function stuff
	shader->bNumControllers = (uint8_t)sh.iNumControllers;
	memcpy( shader->controller, sh.controller, sizeof(shader_funct_t) * SHADER_MAX_FUNCTS );

	// the material stuff
	shader->bUseMaterial = sh.use_material;
	shader->material.ambient[0] = sh.ambient_red;
	shader->material.ambient[1] = sh.ambient_green;
	shader->material.ambient[2] = sh.ambient_blue;
	shader->material.ambient[3] = 1.0f;
	shader->material.diffuse[0] = sh.diffuse_red;
	shader->material.diffuse[1] = sh.diffuse_green;
	shader->material.diffuse[2] = sh.diffuse_blue;
	shader->material.diffuse[3] = 1.0f;
	shader->material.specular[0] = sh.specular_red;
	shader->material.specular[1] = sh.specular_green;
	shader->material.specular[2] = sh.specular_blue;
	shader->material.specular[3] = 1.0f;
	shader->material.shininess = sh.shininess;

	// check for some exceptional states
	if( shader->ColorMap == NULL && shader->subColorMap != NULL )
	{
		shader->ColorMap = shader->subColorMap;
		shader->subColorMap = NULL;
	}

	// count the maps
	if( shader->ColorMap != NULL ) shader->bNumMaps++;
	if( shader->subColorMap != NULL ) shader->bNumMaps++;
	if( shader->EnvMap != NULL ) shader->bNumMaps++;
	if( shader->BumpMap != NULL ) shader->bNumMaps++;

	// release the buffer before exiting
	SAFE_RELEASE( buffer );
	return shader;
}

/* ------------
sh_ReleaseShader - releases all memory associated with the shader
------------ */
static void sh_ReleaseShader( shader_t *shader )
{
	// assert
	if( shader == NULL )
	{
		con_Print( "<RED>Assert Failed sh_ReleaseShader:  shader was NULL" );
		eng_Stop( "70006" );
		return;
	}

	con_Print( "\nUnloading shader \"%s\"", shader->name );

	if( shader->ColorMap != NULL ) cm_Unload( shader->ColorMap ); 
	if( shader->subColorMap != NULL ) cm_Unload( shader->subColorMap );
	if( shader->EnvMap != NULL ) cm_Unload( shader->EnvMap );
	if( shader->BumpMap != NULL ) cm_Unload( shader->BumpMap );

	SAFE_RELEASE( shader );
}

/* ------------
sh_PrintInfo - push info about the shader system to the console
------------ */
void sh_PrintInfo( char *null )
{
	int count = 0;
	shader_t *sh;

	con_Print( "Shader System Information:" );
	con_Print( "----------" ); 

	sh = shHead;
	while( sh != NULL )
	{
		con_Print( "%s:", sh->name );
		if( sh->ColorMap != NULL ) con_Print( "\tColor1:  %s", sh->ColorMap->name );
		if( sh->subColorMap != NULL ) con_Print( "\tColor2:  %s", sh->subColorMap->name );
		if( sh->EnvMap != NULL ) con_Print( "\tEnvironment:  %s", sh->EnvMap->name );
		if( sh->BumpMap != NULL ) con_Print( "\tBump:  %s", sh->BumpMap->name );
		
		sh = sh->next;
	}
	con_Print( "%d Recorded Shaders.", count );
}

/* ------------
sh_ReloadAllMaps
// needs to be called after changes to ogl that need to get reflected here
// on fail, leaves new stuff in a clean state, old stuff still present
------------ */
uint8_t sh_ReloadAllMaps(void)
{
	shader_t *sh;
	shader_t *newSh = NULL;
/*
	// unload all maps
	sh = shHead;
	while( sh != NULL )
	{
		if( sh->ColorMap != NULL ) cm_Unload( sh->ColorMap );
		if( sh->subColorMap != NULL ) cm_Unload( sh->subColorMap );
		if( sh->EnvMap != NULL ) cm_Unload( sh->EnvMap );
		if( sh->BumpMap != NULL ) cm_Unload( sh->BumpMap );

		sh = sh->next;
	}
*/
	// reload all maps
	sh = shHead;
	while( sh != NULL )
	{
		newSh = sh_LoadFromFile( sh->name );
		if( newSh == NULL )
		{
			con_Print( "<RED>Shader System, ReloadAllMaps Failed:  Load from file failed for \"%s\"", sh->name );
			return 0;
		}

		// copy this shader onto the old shader, so the objects
		// that use it will still be able to find it
		// n.b. we could also use this to change shader properties on the fly
		sh->bNumMaps = newSh->bNumMaps;
		memcpy( &(sh->material), &(newSh->material), sizeof(material_t) );
		sh->bApplyLights = newSh->bApplyLights;
		sh->bHasTransparency = newSh->bHasTransparency;
		sh->bUseMaterial = newSh->bUseMaterial;
		sh->ColorMap = newSh->ColorMap;
		sh->subColorMap = newSh->subColorMap;
		sh->EnvMap = newSh->EnvMap;
		sh->BumpMap = newSh->BumpMap;
		sh->bNumControllers = newSh->bNumControllers;
		memcpy( &(sh->controller), &(newSh->controller), sizeof(shader_funct_t) * 8 );

		// delete only what we don't need
		SAFE_RELEASE( newSh );
		
		// next
		sh = sh->next;
	}

	return 1;
}

/* ------------
sh_LinkTail - internal list maintainence, adds to the list at the tail
------------ */
static void sh_LinkTail( shader_t *sh )
{
	if( shHead == NULL ) shHead = sh;

	if( shTail == NULL )
	{
		shTail = sh;
		sh->next = NULL;
		sh->prev = NULL;
		return;
	}

	sh->prev = shTail;
	sh->next = NULL;

	shTail->next = sh;
	shTail = sh;
}

/* ------------
sh_Unlink - removes item / repairs the list
------------ */
static void sh_Unlink( shader_t *sh )
{
	if( sh->prev != NULL ) sh->prev->next = sh->next;
	if( sh->next != NULL ) sh->next->prev = sh->prev;

	if( shTail == sh ) shTail = sh->prev;
	if( shHead == sh ) shHead = sh->next;

	sh->prev = NULL;
	sh->next = NULL;
}

