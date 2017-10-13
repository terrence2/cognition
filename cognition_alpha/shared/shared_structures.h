// Cognition
// shared_structures.h
// Created 2-25-02 @ 1115 by Terrence Cole

#ifndef _STRUCTURES_H
#define _STRUCTURES_H

#include "shared_misc.h"
#include "shared_queue.h"

// Includes
/////////////
// we need symbols for our definitions here, independent of the global header
#ifdef WIN32
#     define WIN32_LEAN_AND_MEAN
#     include <windows.h>
#     include <mmreg.h> // also for windows
	typedef unsigned long int uint32_t;
	typedef unsigned short int uint16_t;
	typedef unsigned char uint8_t;
	typedef signed long int int32_t;
	typedef signed short int int16_t;
	typedef signed char int8_t;
#else
#	include <stdint.h>
#	ifndef HAVE_WAVEFORMATEX
#	define HAVE_WAVEFORMATEX
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

#endif

#include "../cognition/linked_list.h"

// Definitions
////////////////
#define SHADER_NAME_SIZE 64
#define MODEL_NAME_SIZE 64
#define MOO_NAME_SIZE 64
#define MESH_NAME_SIZE 64
#define ENT_NAME_SIZE 64

#define MAP_FLAG_COLOR 1
#define MAP_FLAG_SUBCOLOR 2
#define MAP_FLAG_ENV 4
#define MAP_FLAG_BUMP 8

#define ENT_FLAG_MENU_ITEM 1
#define ENT_FLAG_SKYBOX 2

#define TEXTEDIT_HEIGHT 24
#define TEXTEDIT_MAX_SIZE 255
#define TEXTEDIT_FONTSIZE 14
#define TEXTEDIT_VERT_OFFSET 1
#define TEXTEDIT_HORZ_OFFSET 7

/****** MATHEMATICAL STRUCTURES ******/
typedef struct plane_s
{
	vec3 V;
	vec3 norm;
	float D;
} plane_t;

typedef struct axisAlignedBoundingBox_s
{
	vec3 hibounds;
	vec3 lobounds;
} aabb_t; // axis aligned bounding box



/****** DRAWING STRUCTURES ******/
typedef struct camera_s
{
	byte type; // projection or orthagonal
	vec3 vPos; // set by whoever using / issuing the camera
	vec3 vAng;
	vec3 vVel; // the camera velocity in the last frame
	int wrldPos[3];

	// for both camera types
	float fnear;
	float ffar;

	// for the orthagonal camera
	float fright;
	float ftop;

	// for projection cameras
	float ffov;

	// computed in d_SetCamera
	plane_t pTop;
	plane_t pBottom;
	plane_t pLeft;
	plane_t pRight;
	plane_t pNear;
	plane_t pFar;
	vec3 vFwd;
	vec3 vRt;
	vec3 vUp;
} camera_t;

typedef struct light_s
{
	byte type; // [0,2] for the type of light
	byte state; // 0 or 1 for on or off
	int lightNum; // [0,GL_MAX_LIGHTS) - reference with GL_LIGHT0 + lightNum

	vec4 position;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;

	float spot_exponent; // [0,128], distribution of light intensity, higher is more focussed
	float spot_cutoff; // [0,90] and 180,  maximum spread angle of light
	vec3 spot_direction;

	float constant_attenuation;
	float linear_attenuation;
	float quadratic_attenuation;
} light_t;



/****** MEDIA TYPES ******/
typedef struct sound_s
{
	char cName[1024]; // the base name of a loaded sound

	// the pcm fmt tag
	// assumed fields
	WAVEFORMATEX wf;
	
	// pointer to the 
	uint32_t iDataSize; // in bytes
	uint32_t iDataBlocks; // total number of sound samples
	uint8_t *bData;

	// internal linkage
	struct sound_s *next;
	struct sound_s *prev;
} sound_t;


typedef struct audioStream_s
{
	int placeholder;
} audioStream_t;

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
	int iScaleBias;

	// list stuff
	struct colormap_s *prev;
	struct colormap_s *next;
} colormap_t;

typedef struct colormap_params_s
{
	// loading parameters
	int iFilter; // GL_NEAREST, GL_LINEAR
	int iWrap;  // GL_REPEAT, GL_CLAMP
	int iEnv; // GL_MODULATE, GL_DECAL, GL_BLEND, [and GL_ADD, GL_REPLACE - in some later implementations]
	byte bMipmap;  // 0, 1

	// load parameters
	byte bNoRescale;
	int iScaleBias;
} colormap_params_t;

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

typedef struct material_s
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	int shininess ;
} material_t;

typedef struct shader_funct_s
{
	// controller functions
	// fAmp * sine( fRate ) + fT0
	// fRate * t + fT0
	byte bFunct;  // SINE, LINEAR
	byte bAxis; // AXIS_S, AXIS_T
	float fRate;  
	float fT0;  
	float fAmp;
} shader_funct_t;

typedef struct shader_s
{
	// tracking
	char name[SHADER_NAME_SIZE];
	int users;  // the number of users for this shader
	byte bNumMaps;

	// light interation specs
	material_t material;

	// tags
	byte bApplyLights;
	byte bHasTransparency;
	byte bUseMaterial;

	// maps	
	colormap_t *ColorMap;
	colormap_t *subColorMap;
	colormap_t *EnvMap;
	colormap_t *BumpMap;

	// functional controllers
	byte bNumControllers;
	shader_funct_t controller[8];

	// internal linkings
	struct shader_s *next;
	struct shader_s *prev;
} shader_t;

typedef struct mod_dlist_s
{
	int32_t length; // |length| = the number of indexes in indicies
				// if length < 0, the list describes individual tris, else a strip
	uint32_t *indicies;
} mod_dlist_t; // a packed draw list

typedef struct sphere_tree_node_s
{
	vec3 loc;
	float radius;
	
	int numInFaces;
	int *inFaces;

	int numInPoints;
	vec3 *inPoints;

	int numChildren;
	struct sphere_tree_node_s **children;
} stNode_t;

typedef struct mesh_frame_s
{
	vec3 *verts; // numVerts
	vec3 *vNorms;  // numVerts
	vec3 *tNorms;  // numTris
	stNode_t *stRoot;
} mesh_frame_t;

typedef struct mesh_s
{
	// identity
	char name[MESH_NAME_SIZE];

	// counting
	uint32_t num_verts;
	uint32_t num_mverts;
	uint32_t num_tris;
	uint32_t num_dlists;

	// mesh data
	mesh_frame_t *frames;
	vec2 *mverts;  // num_mverts vec2's
	int *mtris;  // numTris * 3 ints;  elmts 0 to num_mverts-1
	int *tris;  // numTris * 3 ints;  elmts 0 to num_verts-1
	mod_dlist_t *dlists;
} mesh_t;

typedef struct tag_s
{
	char *name;
	vec3 pos;
} tag_t;

typedef struct moo_data_s
{
	// identity
	char name[MOO_NAME_SIZE];
	int num_users;
	
	// animation
	int frames_per_second;
	double frames_per_millisecond;

	// counting information
	uint32_t num_meshes;
	uint32_t num_frames;
	uint32_t num_tags;

	// bounds
	aabb_t bounds;
	float radius;

	// frame animation
	uint32_t cur_frame;
	float sub_frame;

	// the data
	mesh_t *meshes;

	// the tags
	tag_t *tags;
} moo_data_t;

typedef struct model_s
{
	// identity
	char name[MODEL_NAME_SIZE];
	int num_users;

	// counting information
	int num_shaders;

	// data
	moo_data_t *moo;
	shader_t **shaders;
} model_t;




/****** GAME DATA STRUCTURES ******/
typedef struct entity_s
{
	int id;  // the magic tracking number for this entity
	int flags;  // tells us about this entity
	char name[ENT_NAME_SIZE];  // another way to index an entity; not guaranteed to be unique
	byte bInFrustum;
	byte bHintHidden;
	int iTransformMode;

	// culling info
	float fRadius; // get from model

	// constants
	float fMass;  // mass in kilograms
	model_t *mod;

	// location
	vec3 vPos;
	vec3 vVel; // in meters per second
	vec3 vAccel; // in meters per second * second
	
	// Euler Angles
	vec3 vRot;
	vec3 vAngVel; // in degrees per second

	// Quaterion Angles
	vec4 vQuat;

	// linking
	struct entity_s *next;
	struct entity_s *prev;
} entity_t;

typedef struct particle_s
{
	// scheduled time of death in ms
	uint32_t ulEndTime; 

	// the look and feel
	colormap_t *shape;
	color color;

	// the system descriptor
	vec3 vPos;
	vec3 vVel;
	float fAng; // radians
	float fAngVel; // radians per second
	float fSize;
	float fSizeVel;
} particle_t; // 13 * 4 bytes = 52

typedef struct psystem_s
{
	// tracking and maintenance
	uint32_t uiNumParticles; // the current number of particles
	float fPartialParts; // the fractional parts from the last update
	int bType; // attachment point / animation type:  PS_ENTITY, PS_WORLD
	uint32_t uiLiveTime; // time in ms for this system to live
	uint32_t uiSysStartTime; // frame time of creation
	uint32_t uiSysKillTime;  // scheduled death time for this system

	// if bType is PS_ENTITY, this is the entity attachment
	entity_t *ent; // the entity attached
	tag_t *tStart; // the start point tag
	tag_t *tEnd; // the end point tag
	
	// the system descriptor
	vec3 vPos; // absolute world
	vec3 vVel; // movement in world
	vec3 vAng; // the emission direction (angles notation)
	vec2 vAngVel; // the rotation (world only)

	// the emission descriptor
	// (Probabalistic rates should be Normal Distribution;
	//  they are actually implemented as Constant Distribution
	//  for the sake of speed and sanity of implementation.)
	float fStartAng;  // particles shoot randomly in start angle to end angle
	float fEndAng;
	float fMeanRate; // the average speed of emitted particles
	float fMeanSize;
	float fMeanGrowthRate; // the average growth rate for emitted particles
	uint32_t uiPartLiveTime; // particles die after this much time passes

	// output function
	uint32_t uiSysFunction; // the parameterization of the particle emission rate
	float fA, fB, fC;

	// shape descriptor
	color cMeanColor;
	uint32_t iMapIndexLow;
	uint32_t iMapIndexHigh;

	// the particles
	queue_t qParts;

	// internal linking
	struct psystem_s *next;
	struct psystem_s *prev;
} psystem_t;

typedef struct wrld_cube_s
{
	byte bHidden;
	byte bFlagged;
	byte numAround;  // 0 -> 6, WRLD_CUBE_MINED is mined
	vec3 verts[8];
	entity_t *entCenter;
} wrld_cube_t;

typedef struct ser_world_3d_s
{
	int size[3]; // the cube/world array size
	wrld_cube_t ***cubes;  // a 3-d array of cubes that specify states
	float cubeSize;  // the length of one side (s)
	aabb_t wrldBnds;
	plane_t planes[6];
	vec3 quads[6][4];
	int numCubes;
	int numMines;
	varlatch world_color_r;
	varlatch world_color_g;
	varlatch world_color_b;
	shader_t *shEnd;
	shader_t *shFace;
	shader_t *shFlag;

	struct ser_world_3d_s *next;
	struct ser_world_3d_s *prev;
} world_t;



/****** GLOBAL MANAGEMENT STRUCTURES ******/
typedef struct exec_state_s
{
	// timing information
	unsigned long int frame_time;
	unsigned long int last_time;
	unsigned long int delta_time;
	unsigned long int last_server_frame;  // last time server frame was run
	unsigned long int last_client_frame;  // last time client frame was run

	// axis updates
	short int mAxis[2];
	short int mAxisPrev[2];
	short int mAxisDelta[2];
	short int mWheel;

	// menu state
	byte bDrawMenu;

	// console state
	byte bInConsole;

	// loading state
	byte bInLoading;
	
	// special states
	byte bInInit;
	byte bGameType;

	// the big entity update list
	entity_t **ent_list;  // a pointer to the list head
//	psystem_t **ps_list;
} exec_state_t;


/****** INTERFACE STRUCTURES ******/
typedef struct menu_interface_s
{
	int apiversion;

	int (*menu_Initialize)(void);
	void (*menu_Terminate)(void);
	void (*menu_KeyPress)( byte key, byte key_state );
	void (*menu_Frame)( exec_state_t *state );
	void (*menu_Draw)( exec_state_t *state );
} menu_interface_t;

typedef struct client_interface_s
{
	int apiversion;

	int (*cl_Initialize)(void);
	void (*cl_Terminate)(void);
	int (*cl_Connect)( char *address, char *port );
	int (*cl_Frame)( exec_state_t *state );
	void (*cl_Draw2D)( exec_state_t *state );
} client_interface_t;

typedef struct server_interface_s
{
	int apiversion;
	int (*ser_Initialize)(void);
	void (*ser_Terminate)(void);
	int (*ser_Start)( int iMaxClients, byte bPublicServer, int *size, int dimension, int numMines );
	void (*ser_Frame)(void);
} server_interface_t;

typedef struct engine_interface_s
{
	int apiversion;

// Engine Functionality
	void (*con_Print)( char *fmt, ... );
	void* (*mem_alloc)( unsigned int size );
	void (*mem_free)( void *memblock );
	void (*eng_Stop)( char *stop_code );
	void (*eng_LoadingFrame)();

// Command Processor
	command_t (*com_AddCommand)( char *name, void (*function)( char *buffer ) );
	void (*com_RemoveCommand)( command_t command );

// list management
	void *(*ll_iter_data)( LinkedList list );
	int (*ll_iter_isdone)( LinkedList list );
	void (*ll_iter_next)( LinkedList list );
	void (*ll_iter_reset)( LinkedList list );

// Network interfaces
	byte (*net_cl_Connect)( char *address, char *port );
	void (*net_cl_Disconnect)();
	int (*net_cl_GetServerPacket)( byte *type, unsigned short int *size, byte **data );
	byte (*net_cl_Flush)();
	
	byte (*net_cl_SendByte)( byte b );
	byte (*net_cl_SendShort)( unsigned short int us );
	byte (*net_cl_SendLong)( unsigned long int ul );
	byte (*net_cl_SendString)( char *str );
	
	byte (*net_sv_OpenNetwork)( char *port );
	byte (*net_sv_CloseNetwork)();
	connection_t (*net_sv_Accept)( char **address );
	void (*net_sv_Disconnect)( connection_t client );
	int (*net_sv_GetClientPacket)( connection_t client, byte *type, unsigned short int *size, byte **data );
	byte (*net_sv_Flush)( connection_t client );

	byte (*net_sv_SendByte)( connection_t client, byte b );
	byte (*net_sv_SendShort)( connection_t client, unsigned short int us );
	byte (*net_sv_SendLong)( connection_t client, unsigned long int ul );
	byte (*net_sv_SendString)( connection_t client, char *str );

	unsigned short int (*net_GetShort)( byte *buf );
	unsigned long int (*net_GetLong)( byte *buf );
	unsigned long int (*net_GetString)( char *outBuf, byte *buf, int maxLen );

// game services
	world_t *(*wrld_Create)( int *size, int dimension, int numMines );
	void (*wrld_Delete)( world_t *world );
	int (*wrld_ClearSquare)( world_t *world, int x, int y, int z );
	byte (*wrld_RayIntersect)( world_t *world, vec3 vStart, vec3 vDir, int *vCube );

// game management
	void (*game_ShowLoading)();
	void (*game_ShowMenu)();
	void (*game_ShowGame)();
	void (*game_Start)( char *type );
	void (*game_End)( char *null );
	void (*game_Connect)( char *address );
	void (*game_Disconnect)( char *null );
	void (*game_StartServer)( char *type );
	void (*game_EndServer)( char *null );

// particle systems
	psystem_t *(*ps_Spawn)( vec3 loc, vec3 dir, vec3 vel, vec2 ang_vel, 
						int live_time, int part_live_time,
						color mean_color, int shape,
						float inner_theta, float outer_theta, 
						float mean_speed, float mean_size, float mean_growth, 
						int function, float fA, float fB, float fC );
	psystem_t *(*ps_SpawnOnEnt)( entity_t *ent, tag_t *start, tag_t *end,
						int live_time, int part_live_time,
						color mean_color, int shape,
						float inner_theta, float outer_theta, 
						float mean_speed, float mean_size, float mean_growth, 
						int function, float fA, float fB, float fC );
	void (*ps_Kill)( psystem_t *ps );

// Lighting System
	light_t *(*l_Spawn)( byte type, float *position, float *ambient, float *diffuse, float *specular, float spot_exp, float spot_cutoff, float *spot_dir, float const_atten, float linear_atten, float quad_atten );
	void (*l_Kill)( light_t *l );

// Camera / Environment
	void (*d_SetCamera)( camera_t *cam );
	void (*d_LoadSkybox)( char *name );
	void (*d_SetWorld)( world_t *world );
	camera_t *(*d_GetCamera)();
	void (*d_MoveCamera)( float movespeed, float *vAng,  float *vMvReq, float *vOut );

// Text Services 
	void (*d_TextLineBlack)( char *line, float font_size, float pos_x, float pos_y );
	void (*d_TextLineWhite)( char *line, float font_size, float pos_x, float pos_y );
	void (*d_TextLineColor)( char *line, float font_size, byte color[4], float pos_x, float pos_y );
	float (*d_GetTextWidth)( char *text, float font_size );
	float (*d_GetTextHeight)( float font_size );
	float (*d_GetCharWidth)( char c, float font_size );

// Forground Rectangle
	void (*d_MenuRect)( float left, float right, float bottom, float top, color clr, colormap_t *cm );

// Sound Management
	sound_t *(*s_Load)( char *name );
	void (*s_Unload)( sound_t *snd );
	sndhandle (*s_PlaySound)( sound_t *sound, uint8_t bLooping, vec3 *vpLoc, vec3 *vpVel );
	void (*s_StopSound)( sndhandle handle );
	uint8_t (*s_PlayMusic)( audioStream_t *stream );
	void (*s_StopMusic)( audioStream_t *stream );

// Media Management
	model_t *(*mod_Load)( char *name );
	void (*mod_Unload)( model_t *mod );
	entity_t* (*ent_Spawn)( char *name, int flags, char *model, vec3 pos, vec3 rot );
	void (*ent_Remove)( entity_t *rmEnt );

// Colormap Management
	colormap_t *(*cm_LoadFromFiles)( const char *name, const char *name24, const char *name8, const colormap_params_t *params );
	colormap_t *(*cm_LoadConstAlpha)( const char *name, const char *name24, const byte alphaClr, const colormap_params_t *params );
	colormap_t *(*cm_LoadFromImage)( const char *name, const image_t *img, const colormap_params_t *params );
	void (*cm_Unload)( colormap_t *cm );
	int (*cm_GetGlobalFilter)();
	void (*cm_PrintInfo)( char *null );

// Collision Detection
	byte (*col_CheckEnts)( float *hit, entity_t *ent1, entity_t *ent2, byte priority );
	void (*col_Respond)( entity_t *ent, vec3 hit, vec3 vNorm );

// Random Number Generator
	double (*rng_Double)( double lo, double hi );
	float (*rng_Float)( float lo, float hi );
	long (*rng_Long)( long lo, long hi );
	short (*rng_Short)( short lo, short hi );
	byte (*rng_Byte)( byte lo, byte hi );

// Timers and Timing Data
	unsigned long (*t_GetTime)(void);
	double (*t_GetSaneTime)(void);
	cog_timer_t (*t_SetTimer)( void (*funct)( char *buffer ), char *command, int milli, int repeating );
	void (*t_KillTimer)( cog_timer_t timer );

// Persistant Variable Management
	varlatch (*var_GetVarLatch)( char *varname );
	float (*var_GetFloat)( varlatch handle );
	char *(*var_GetString)( varlatch handle );
	float (*var_GetFloatFromName)( char *var_name );
	char *(*var_GetStringFromName)( char *var_name );
	void (*var_UpdateFloat)( varlatch handle, float value );
	void (*var_UpdateString)( varlatch handle, char *value );

// mouse
	int (*ms_GetPosition)( int *x, int *y );
	void (*ms_Show)();
	void (*ms_Hide)();
	int (*ms_MouseIsVisible)();
	int (*ms_MouseIsHidden)();

// keyboard
	int (*bind_ShiftIsDown)();
	int (*bind_AltIsDown)();
	int (*bind_CtrlIsDown)();

} engine_interface_t;


#endif // _STRUCTURES_H
