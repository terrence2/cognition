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
// .c
// Created  by Terrence Cole

// Includes
/////////////
#include "cog_global.h"
#include "cog_opengl.h"

// Definitions
////////////////
#define NEAR_CLIP_PLANE 1.0f
#define FAR_CLIP_PLANE 10000.0f
#define FOV 35.0f
#define WINDOWED_WIDTH_DEF 640
#define WINDOWED_HEIGHT_DEF 480


// Local Structures
/////////////////////

// Global Prototypes
//////////////////////
/*
int gl_Initialize(void);
void gl_Terminate(void);
void gl_Restart( char *null );
display_t *gl_GetWindow();
void gl_FlipBuffers();
*/

// Local Prototypes
/////////////////////
static void gl_SetupRC(void);

// Local Variables
////////////////////

// *********** FUNCTIONALITY ***********
/* ------------
gl_Initialize - open opengl on this platform and prepare for rendering
			  - handles window related tasks then hands off init to InitOGL
------------ */
int gl_Initialize(void)
{
	con_Print( "\nInitializing Display Parameters...." );

	// get handles to the variables
	if( gl_state.gl_width == NULL ) gl_state.gl_width = var_GetVarLatch( "render_width" );
	if( gl_state.gl_height == NULL ) gl_state.gl_height = var_GetVarLatch( "render_height" );
	if( gl_state.gl_bitdepth == NULL ) gl_state.gl_bitdepth = var_GetVarLatch( "render_bitdepth" );
	if( gl_state.gl_windowed == NULL ) gl_state.gl_windowed = var_GetVarLatch( "render_windowed" );
	if( gl_state.gl_refreshrate == NULL ) gl_state.gl_refreshrate = var_GetVarLatch( "render_max_refreshrate" );
	gl_state.window.sdl_flags = SDL_OPENGL | SDL_HWSURFACE | SDL_DOUBLEBUF;
    
	// setup the viewport and window states
	if( (int)var_GetFloat(gl_state.gl_windowed) )
	{
		gl_state.window.w = WINDOWED_WIDTH_DEF;
		gl_state.window.h = WINDOWED_HEIGHT_DEF;
	}
	else
	{
		gl_state.window.w = (long)var_GetFloat(gl_state.gl_width);
		gl_state.window.h = (long)var_GetFloat(gl_state.gl_height);
		gl_state.window.sdl_flags |= SDL_FULLSCREEN;
	}

  	con_Print( "Initializing OpenGL Rendering System..." );
	con_Print( "\t\tColorBits:   %d", 32 );
	con_Print( "\t\tDepthBits:   %d", 16 );
	con_Print( "\t\tStencilBits: %d", 0 );
	con_Print( "\t\tAccumBits:   %d", 0 );

	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
	SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 0 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    
	// open the window 
	// n.b. that this shouldn't ever get freed
	gl_state.window.sdl_surface = SDL_SetVideoMode( gl_state.window.w, 
	                                                gl_state.window.h, 
	                                                32, 
	                                                gl_state.window.sdl_flags );
    
	con_Print( "\tWindowed: %d", (int)var_GetFloat(gl_state.gl_windowed) );
	con_Print( "\tWidth: %d", gl_state.window.w ); 
	con_Print( "\tHeight: %d", gl_state.window.h );
	con_Print( " " );

	// hide the cursor
	in_HideMouse();

	// init gl state engine
	gl_SetupRC();

	// initialize the viewport
	if( (int)var_GetFloat(gl_state.gl_windowed) )
	{
		// if running in window, the passed size will be slightly bigger than the vp we want 
		glViewport( 0, 0, WINDOWED_WIDTH_DEF, WINDOWED_HEIGHT_DEF );
		gl_state.gl_aspect = (float)WINDOWED_WIDTH_DEF / (float)WINDOWED_HEIGHT_DEF;
	}
	else
	{
		glViewport( 0, 0, gl_state.window.w, gl_state.window.h );
		gl_state.gl_aspect = (float)gl_state.window.w / (float)gl_state.window.h;
	}

	return 1;
}

/* ------------
gl_Terminate - close opengl on this system and free all associated render memory
------------ */
void gl_Terminate(void)
{
	con_Print( "\nTerminating OpenGL..." );

    // SDL_Quit takes care of shutting down the display
    
	// redisplay the cursor
	in_UnhideMouse();

	// free descriptive memory
	SAFE_RELEASE( gl_state.gli_extensions );
	SAFE_RELEASE( gl_state.gli_renderer );
	SAFE_RELEASE( gl_state.gli_version );
	SAFE_RELEASE( gl_state.gli_vendor );
}

/* ------------
gl_Restart - close and reopen opengl on this platform and refresh all opengl dependent items
------------ */
void gl_Restart( char *null )
{	
#if 0
    /* FIXME: i haven't looked at this in ages and it never worked to start with */
    
	// this operation will mess up input control on window systems
	in_Terminate();

	// tell bind to flush it's keystates, since we just closed input
	bind_FlushKeyStates();

	// the console has it's own texture issues
	con_Terminate();

	// flush all colormaps
	cm_Terminate();

	// shutdown
	gl_Terminate();

	// re-start
	if( !gl_Initialize() ) eng_Stop( "15" );

	// let there be color
	cm_Initialize();

	// open the console on our fresh gl
	if( !con_Initialize() ) eng_Stop( "17" );

	// shutting down ogl flushed all of our color maps
	if( !sh_ReloadAllMaps() ) eng_Stop( "16" );

	// restart the controls
	if( !in_Initialize() ) eng_Stop( "18" );
#endif
    con_Print("Opengl restart doesn't work yet, sorry");
    assert(1==0);
}


/* ------------
gl_GetWindow - returns the window associated with this process's OpenGL context
------------ */
display_t *gl_GetWindow()
{
	return &gl_state.window;
}

/* ------------
gl_SetupRC 
------------ */
static void gl_SetupRC(void)
{
	vec4 ambiance = { 0.0f, 0.0f, 0.0f, 1.0f };

	con_Print( "\nInitializing the rendering context..." );

	// set the clear color default to black
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

	// clear the screen buffer
	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );

	// init the transform state
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	// drawing
	glRenderMode( GL_RENDER );
	glDrawBuffer( GL_BACK );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glDisable( GL_TEXTURE_GEN_S );
	glDisable( GL_TEXTURE_GEN_T );

	// Geometry rendering controls
	glEnable( GL_DEPTH_TEST );
	glDepthMask( GL_TRUE );
	glDepthFunc( GL_LESS );
	glEnable( GL_CULL_FACE );
	glFrontFace( GL_CCW );

	// gl implementation
	gl_state.gli_vendor = tcDeepStrCpy( (signed char*)glGetString( GL_VENDOR ) );
	gl_state.gli_renderer = tcDeepStrCpy( (signed char*)glGetString( GL_RENDERER ) );
	gl_state.gli_version = tcDeepStrCpy( (signed char*)glGetString( GL_VERSION ) );
	gl_state.gli_extensions = tcDeepStrCpy( (signed char*)glGetString( GL_EXTENSIONS ) );
	con_Print( "\tGL Vendor: %s", gl_state.gli_vendor );
	con_Print( "\tGL Renderer: %s", gl_state.gli_renderer );
	con_Print( "\tGL Version: %s", gl_state.gli_version );
	con_Print( "\tGL Extensions: %s", gl_state.gli_extensions ); // this is just a little to long and indeterminate to trust

	// find the appropriate renderer for this system
	gl_state.renderPath = RENDER_BASE;

	// vertex programs
#if 0
	// vertex buffer objects
	if( tcstrnfind( gl_state.gli_extensions, "ARB_vertex_buffer_object", 24, tcstrlen(gl_state.gli_extensions) - 24 ) )
	{
	
		gl_state.extensions.glBindBufferARB = (PFNGLBINDBUFFERARBPROC)wglGetProcAddress( "glBindBufferARB" );
		gl_state.extensions.glGenBuffersARB = (PFNGLGENBUFFERSARBPROC)wglGetProcAddress( "glGenBuffersARB" );
		gl_state.extensions.glBufferDataARB = (PFNGLBUFFERDATAARBPROC)wglGetProcAddress( "glBufferDataARB" );
		gl_state.extensions.glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC)wglGetProcAddress( "glDeleteBuffersARB" );
		gl_state.useVBO = 1;
	}
	else
	{
		gl_state.useVB0 = 0;
	}

	// color mapping
	if( tcstrnfind( gl_state.gli_extensions, "GL_ARB_multitexture", 19, tcstrlen(gl_state.gli_extensions) - 19) )
	{
		glGetIntegerv( GL_MAX_TEXTURE_UNITS_ARB, &gl_state.num_cm_units );
		gl_state.extensions.glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC)wglGetProcAddress( "glMultiTexCoord2fARB" );
		gl_state.extensions.glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress( "glActiveTextureARB" );
		gl_state.extensions.glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC)wglGetProcAddress( "glClientActiveTextureARB" );
		gl_state.useMultiTex = 1;
	}
	else
	{
		gl_state.num_cm_units = 1;
		gl_state.useMultiTex = 0;
	}
	con_Print( "\tTexture Units: %d", gl_state.num_cm_units );

	// enable texturing on all texture units
	if( gl_state.num_cm_units > 1 )
	{
		for( a = 0 ; a < gl_state.num_cm_units ; a++ )
		{
			gl_state.extensions.glActiveTextureARB( GL_TEXTURE0_ARB + a );
			glEnable( GL_TEXTURE_2D );
		}
	}
	else
	{
		glEnable( GL_TEXTURE_2D );
	}
#endif
	// point sizes
	glGetFloatv( GL_POINT_SIZE_RANGE, gl_state.point_size_range );
	glGetFloatv( GL_POINT_SIZE_GRANULARITY, &gl_state.point_size_step );
	con_Print( "\tPoint Size Range:  [ %.3f, %.3f ] with a granularity of %.3f", 
				gl_state.point_size_range[0],  gl_state.point_size_range[1], gl_state.point_size_step );

	// line sizes
	glGetFloatv( GL_LINE_WIDTH_RANGE, gl_state.line_size_range );
	glGetFloatv( GL_LINE_WIDTH_GRANULARITY, &gl_state.line_size_step );
	con_Print( "\tLine Width Range:  [ %.3f, %.3f ] with a granularity of %.3f", 
				gl_state.line_size_range[0], gl_state.line_size_range[1], gl_state.line_size_step );

	// find the max texture size
	glGetIntegerv( GL_MAX_TEXTURE_SIZE, &(gl_state.tex_size_max) );
	con_Print( "\tMaximum Texture Size:  %d", gl_state.tex_size_max );

	// setup the light states
	glEnable( GL_LIGHTING );
	glLightModelfv( GL_LIGHT_MODEL_LOCAL_VIEWER, ambiance );
	glShadeModel( GL_SMOOTH );

	// setup the default material
	gl_state.matDefault.ambient[0] = 1.0f;
	gl_state.matDefault.ambient[1] = 1.0f;
	gl_state.matDefault.ambient[2] = 1.0f;
	gl_state.matDefault.ambient[3] = 0.0f;
	gl_state.matDefault.diffuse[0] = 1.0f;
	gl_state.matDefault.diffuse[1] = 1.0f;
	gl_state.matDefault.diffuse[2] = 1.0f;
	gl_state.matDefault.diffuse[3] = 0.0f;
	gl_state.matDefault.specular[0] = 1.0f;
	gl_state.matDefault.specular[1] = 1.0f;
	gl_state.matDefault.specular[2] = 1.0f;
	gl_state.matDefault.specular[3] = 0.0f;
	gl_state.matDefault.shininess = 0;
	// FIXME:  verify that these states are good, then call them here

	// coloration
	glEnable( GL_TEXTURE_2D );

	// enable vertex arrays
	glEnableClientState( GL_NORMAL_ARRAY );
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
}

/* ------------
gl_FlipBuffers - the call to flip the back buffer is system dependend, so it is branched here from the draw_end
------------ */
void gl_FlipBuffers()
{
    SDL_GL_SwapBuffers();
}
