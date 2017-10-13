// Cognition
// opengl.h
// by Terrence Cole 12/11/01

#ifndef _OPENGL_H
#define _OPENGL_H

// Inclusions
////////////////
#include <GL/gl.h>
#include <GL/glext.h>

// Definitions
/////////////
#define RENDER_BASE 1
#define RENDER_EXTENDED 2
#define RENDER_NVIDIA 3
#define RENDER_ATI 4

// Structures
///////////////

#ifdef WIN32
typedef struct gl_extensions_s
{
	// Vertex Programs
	PFNGLGENPROGRAMSARBPROC glGenProgramsARB;
	PFNGLPROGRAMSTRINGARBPROC glProgramStringARB;
	PFNGLBINDPROGRAMARBPROC glBindProgramARB;
	PFNGLDELETEPROGRAMSARBPROC glDeleteProgramsARB;

//	 VertexAttrib...ARB
//ProgramEnvParameter...ARB
//ProgramLocalParameter...ARB

	// Multitexturing
	PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB;
	PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;
	PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB;

	// Vertex Buffer Objects
	PFNGLBINDBUFFERARBPROC glBindBufferARB;
	PFNGLGENBUFFERSARBPROC glGenBuffersARB;
	PFNGLBUFFERDATAARBPROC glBufferDataARB;
	PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB;

} gl_extensions_t;
#else
typedef struct gl_extensions_s
{
	int placeholder;
} gl_extensions_t;
#endif


typedef struct
{
	// the system dependent window
	display_t window;
	float gl_aspect;

	// variables / settings
	varlatch gl_width;
	varlatch gl_height;
	varlatch gl_bitdepth;
	varlatch gl_windowed;
	varlatch gl_refreshrate;
	
	// gl implementation specifics
	char *gli_vendor;
	char *gli_renderer;
	char *gli_version;
	char *gli_extensions;
	
	// extensions
	gl_extensions_t extensions;
	int renderPath;
	int useVP;
	int useMultiTex;
	int useVBO;

	// color mapping
	int num_cm_units;
	int tex_size_max;

	// point sizes
	float point_size_range[2];
	float point_size_step;

	// line sizes
	float line_size_range[2];
	float line_size_step;

	// material
	material_t matDefault;
} gl_status_t;

// Header Variables
/////////////////////
gl_status_t gl_state;

#endif // _OPENGL_H
