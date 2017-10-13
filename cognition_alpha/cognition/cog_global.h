// Cognition
// cog_global.h
// created 2-25-02 @ 0017 by Terrence Cole

#ifndef _COG_GLOBAL_H
#define _COG_GLOBAL_H

// Global Includes for Software Abstraction
////////////////////
#include "cog_sysdeps.h"

// Custom Libraries
/////////////////////
#include "../shared/shared_keycodes.h"
#include "../shared/shared_library.h"
#include "../shared/shared_misc.h"
#include "../shared/shared_structures.h"
#include "../shared/shared_mathlib.h"
#include "cog_structures.h"

// Definitions
////////////////
#define SAFE_RELEASE(a) if( (a) != NULL ) { mem_free( (a) ); (a) = NULL; }
#define SAFE_RELEASEv(a) if( (a) != NULL ) { ei->mem_free( (a) ); }
#define SAFE_BIND( a ) if( a != NULL ) { glBindTexture( GL_TEXTURE_2D, a->gl_name ); }

#define SHADER_FUNCT_NONE 0
#define SHADER_FUNCT_SINE 1
#define SHADER_FUNCT_LINEAR 2
#define SHADER_MAX_FUNCTS 8

#define COLORMAP_DIRECTORY "colormaps"
#define SHADER_DIRECTORY "shaders"

#define SERVER_FRAME_TIME 100  // 1/10th second interval, 10 fps
#define CLIENT_FRAME_TIME 33  // 1 / 30th second interval, 30 fps

// Global Variables
/////////////////////
exec_state_t state;
sys_handles_t sys_handles;

engine_interface_t engine_interface;
engine_interface_t *ei;

menu_interface_t *mi;
client_interface_t *ci;
server_interface_t *si;

			///////////////////
// *********** GLOBAL PROTOTYPES ***********
			///////////////////

/******* SOFTWARE ABSTRACTION LAYER *******/


/* ------------
sal_display_*.c
------------ */
int dis_Initialize(void);
void dis_Terminate(void);
int dis_SetMode( int width, int height, int bitdepth, uint32_t maxrefreshrate );
void dis_Resized( int w, int h );
void dis_OpenWindow( display_t *param );
int dis_CloseWindow( display_t *window );

/* ------------
sal_opengl_*.c
------------ */
int gl_Initialize(void);
void gl_Terminate(void);
void gl_Restart( char *null );
display_t *gl_GetWindow();
void gl_FlipBuffers();

/* ------------
sal_input_*.c
------------ */
int in_Initialize(void);
void in_Terminate(void);
void in_ProcessEvents();
void in_HideMouse();
void in_UnhideMouse();

/* ------------
sal_module_*.c
------------ */
int module_Initialize(void);
void module_Terminate(void);
int module_OpenMenu();
void module_CloseMenu();
int module_OpenClient();
void module_CloseClient();
int module_OpenServer();
void module_CloseServer();

/* ------------
sal_network.c
------------ */
int net_Initialize(void);
void net_Terminate(void);

byte net_cl_Connect( char *address, char *port );
void net_cl_Disconnect();
int net_cl_GetServerPacket( byte *type, unsigned short int *size, byte **data );
byte net_cl_SendData( byte *data, unsigned short int size );
byte net_cl_Flush();

byte net_cl_SendByte( byte b );
byte net_cl_SendShort( unsigned short int us );
byte net_cl_SendLong( unsigned long int ul );
byte net_cl_SendString( char *str );

byte net_sv_OpenNetwork( char *port );
byte net_sv_CloseNetwork();
connection_t net_sv_Accept( char **address );
void net_sv_Disconnect( connection_t client );
int net_sv_GetClientPacket( connection_t client, byte *type, unsigned short int *size, byte **data );
byte net_sv_SendData( connection_t client, byte *data, unsigned short int size );
byte net_sv_Flush( connection_t client );

byte net_sv_SendByte( connection_t client, byte b );
byte net_sv_SendShort( connection_t client, unsigned short int us );
byte net_sv_SendLong( connection_t client, unsigned long int ul );
byte net_sv_SendString( connection_t client, char *str );

unsigned short int net_GetShort( byte *buf );
unsigned long int net_GetLong( byte *buf );
unsigned long int net_GetString( char *outBuf, byte *buf, int maxLen );

/* ------------
sal_sound_*.c
------------ */
int s_Initialize(void);
void s_Terminate(void);
void s_Restart(void);

sound_t *s_Load( char *name );
void s_Unload( sound_t *snd );

void s_SpacializeSounds(void);
void s_StopAllSound(void);

sndhandle s_PlaySound( sound_t *sound, uint8_t bLooping, vec3 *vpLoc, vec3 *vpVel );
void s_StopSound( sndhandle handle );

uint8_t s_PlayMusic( audioStream_t *stream );
void s_StopMusic( audioStream_t *stream );

void s_CheckStreams(void);
void s_UpdateVolume(void);

/* ------------
sal_system_*.c
------------ */
void sys_ProcessSystemMessages();
void sys_PrintSystemInformation();
void sys_Exit( int errorcode );

/******* BASE LIBRARIES *******/

/* ------------
lib_filesys.c
------------ */
// systems
int fs_Initialize(void);
void fs_Terminate(void);
int fs_PrintFileSystemInformation();

// path structure control
char *fs_GetBaseDirectory();
char *fs_GetMediaDirectory();
char *fs_SetMediaDirectory( const char *name );
char *fs_GetBasePath( const char *filename, const char *subPath );
char *fs_GetMediaPath( const char *filename, const char *subPath );
unsigned long fs_GetFileLength( file_t *file );

// file usage stuff
file_t *fs_open( const char *filename, const char *mode );
int fs_close( file_t *stream );
size_t fs_read( void *buffer, size_t size, size_t count, file_t *stream );
size_t fs_write( const void *buffer, size_t size, size_t count, file_t *stream );
int fs_seek( file_t *stream, long offset, int origin );
long fs_tell( file_t *stream );
int fs_flush( file_t *stream );
void fs_rewind( file_t *stream );
int fs_error( file_t *stream );
int fs_eof( file_t *stream);
int fs_RemoveDirectory( char *rel_dir );
int fs_CreateDirectory( char *rel_dir );

// misc helpers
int fs_GetNextOrderedName( char *name, char *dir );
byte fs_FileExists( char *name );

/* ------------
lib_parse.c
------------ */
int par_ExecFile( char *file_name, char *file_rel_dir );
int par_ExecCmdLine( char *cmdline );
int par_ExecBuffer( char *buffer );
char *par_StripComments( char* buffer );
char *par_StripCarriageReturns( char *buffer );
char *par_StripTabs( char *buffer );
char *par_StripQuotes( char *buffer );
char *par_StripChar( char *buffer, char strip );
char *par_ReplaceChar( char *buffer, char oldchar, char newchar );
int par_ToChar( char *buffer, char end );
int par_ToToken( char* buffer );
int par_ToSpace( char* buffer );
char *par_BufferFile( char *path );
char *par_GetTokenValue( char *src, char *token, int token_length );
int par_GetNextToken( char *buffer, int *toToken, int *tokenLen );
byte par_IsWhiteSpace( const char c );
int par_QuotedStringCopy( char *dest, const char *src, int length );

/* ------------
lib_rng.c
------------ */
int rng_Initialize(void);
void rng_Terminate(void);
double rng_Double( double lo, double hi );
float rng_Float( float lo, float hi );
long rng_Long( long lo, long hi );
short rng_Short( short lo, short hi );
byte rng_Byte( byte lo, byte hi );


/******* BASE ENGINE SERVICES *******/

/* ------------
base_bind.c
------------ */
int bind_Initialize(void);
void bind_Terminate(void);
void bind_TranslateKeyPress( byte key, byte key_state );
int bind_ShiftIsDown();
int bind_AltIsDown();
int bind_CtrlIsDown();
void bind_BindKey( char *buffer );
void bind_SetKeyBind( char *key, char *command );
void bind_UnbindKey( char *buffer );
void bind_UnbindAll();
void bind_FlushKeyStates(void);

/* ------------
base_bitstream.c
------------ */
int bs_Initialize(void);
void bs_Terminate(void);
bitstream_t *bs_GetBitStream( file_t *fp );
void bs_CloseStream( bitstream_t *bs );
unsigned long int bs_Get32Bits( bitstream_t *bs );
unsigned short int bs_Get16Bits( bitstream_t *bs );
byte bs_Get8Bits( bitstream_t *bs );
byte bs_Get1Bit( bitstream_t *bs );
unsigned long int bs_GetBits( bitstream_t *bs, byte numBits );
byte bs_error( bitstream_t *bs );
byte bs_eof( bitstream_t *bs );
byte bs_IsEnding( bitstream_t *bs );
void bs_AlignToByte( bitstream_t *bs);
byte bs_IsAlignedToByte( bitstream_t *bs );
byte bs_Lookahead8Bits( bitstream_t *bs );
unsigned short int bs_Lookahead16Bits( bitstream_t *bs );

/* ------------
base_command.c
------------ */
int com_Initialize(void);
void com_Terminate(void);
void com_Process( char *buffer );
int com_VerifyCommand( char *command_name );
command_t com_AddCommand( char *name, void (*function)( char *buffer ) );
void com_RemoveCommand( command_t command );

/* ------------
base_filelog.c
------------ */
int f_Initialize(void);
void f_Terminate(void);
void f_LogVA( char *fmt, ... );
void f_Log( char *str );

/* ------------
base_memory.c
------------ */
int mem_Initialize(void);
void mem_Terminate(void);
void* mem_alloc( unsigned int size );
void mem_free( void *memblock );
void mem_PrintInfo( char *null );

/* ------------
base_timesys.c
------------ */
int t_Initialize(void);
void t_Terminate(void);
unsigned long int t_GetTime(void);
double t_GetSaneTime(void);
cog_timer_t t_SetTimer( void (*funct)( char *buffer ), char *command, int milli, int repeating );
void t_KillTimer( cog_timer_t timer );
void t_CheckTimers(void);

/* ------------
base_variable.c
------------ */
int var_Initialize(void);
void var_Terminate(void);
varlatch var_GetVarLatch( char *varname );
float var_GetFloat( varlatch handle );
char *var_GetString( varlatch handle );
float var_GetFloatFromName( char *var_name );
char *var_GetStringFromName( char *var_name );
void var_UpdateFloat( varlatch handle, float value );
void var_UpdateString( varlatch handle, char *value );
void var_SetValue( char *buffer );
void var_SetValueSync( char *buffer );
void var_UpdateVar( char *varname, char *varvalue );
void var_ValueOf( char *varName );


/******* HIGH LEVEL ENGINE SERVICES *******/

/* ------------
sv_collision.c
------------ */
byte col_CheckEnts( float *hit, entity_t *ent1, entity_t *ent2, byte priority );
void col_Respond( entity_t *ent, vec3 hit, vec3 vNorm );

/* ------------
sv_console.c
------------ */
int con_Initialize(void);
void con_Terminate(void);
void con_Print( char *fmt, ... );
void con_Toggle( char *null );
void con_KeyPress( unsigned char key, int st );

/* ------------
sv_engine.c
------------ */
void eng_Start(); 
void eng_Stop( char *buffer );
void eng_LoadingFrame();
void eng_EscapePress();

/* ------------
sv_mouse.c
------------ */
int ms_Initialize();
void ms_Terminate();
void ms_Update();
int ms_GetPosition( int *x, int *y );
void ms_Show();
void ms_Hide();
int ms_MouseIsVisible();
int ms_MouseIsHidden();
void ms_Draw();

/* ------------
sv_music.c
------------ */
int mus_Initialize(void);
void mus_Terminate(void);
void mus_SetDirectory( char *dir );
void mus_RescanDirectory();

void mus_Start();
void mus_Stop();
void mus_NextTrack();

void mus_PlaySpecific( char *name );


/******* ABSTRACT MEDIA HANDLING SERVICES *******/

/* ------------
media_bitmap.c
------------ */
byte *bmp_Load( char *path, unsigned int *width, unsigned int *height, byte *bpp );
void bmp_ScreenShot( char *null );
int bmp_WriteToFile( const char *abs_path, BITMAPINFO *info, byte *bits );

/* ------------
media_colormap.c
------------ */
int cm_Initialize(void);
void cm_Terminate(void);
colormap_t *cm_LoadFromFiles( const char *name, const char *name24, const char *name8, const colormap_params_t *params );
colormap_t *cm_LoadConstAlpha( const char *name, const char *name24, const byte alphaClr, const colormap_params_t *params );
colormap_t *cm_LoadFromImage( const char *name, const image_t *img, const colormap_params_t *params );
void cm_Unload( colormap_t *cm );
int cm_GetGlobalFilter();
void cm_PrintInfo( char *null );

/* ------------
media_image.c
------------ */
int img_Initialize(void);
void img_Terminate(void);
image_t *img_Load( const char *name, const char *rel_path );
image_t *img_LoadConstant( const int width, const int height, const byte bpp, const byte *colors );
image_t *img_Splice( const image_t *img24, const image_t *img8 );
image_t *img_CreateCopy( const image_t *img );
image_t *img_Resize( const image_t *img, int size );
void img_Unload( image_t *image );

/* ------------
media_jpeg.c
------------ */
byte *jpg_Load( char *path, unsigned int *width, unsigned int *height,  byte *bpp );

/* ------------
media_model.c
------------ */
int mod_Initialize(void);
void mod_Terminate(void);
model_t *mod_Load( char *name );
void mod_Unload( model_t *mod );

/* ------------
media_moo.c
------------ */
int moo_Initialize(void);
void moo_Terminate(void);
moo_data_t *moo_Load( char *name );
void moo_Unload( moo_data_t *moo );

/* ------------
media_mp3.c
------------ */
int mp3_Initialize(void);
void mp3_Terminate(void);
audioStream_t *mp3_GetStream( char *name );
void mp3_ReleaseStream( audioStream_t *stream );
byte *mp3_GetAudioSample( int amt );

/* ------------
media_shader.c
------------ */
int sh_Initialize(void);
void sh_Terminate(void);
shader_t *sh_LoadShader( char *name );
void sh_UnloadShader( shader_t *shader );
void sh_PrintInfo( char *null );
uint8_t sh_ReloadAllMaps(void);

/* ------------
media_targa.c
------------ */
byte *tga_Load( char *path, unsigned int *width, unsigned int *height, byte *bpp );

/* ------------
media_wave.c
------------ */
uint8_t *wav_Load( char *name, WAVEFORMATEX *wf, uint32_t *iDataSize, uint32_t *iDataBlocks );

/******* GAME CONTROL/DATA SERVICES *******/

/* ------------
game_background.c
------------ */
int bg_Initialize(void);
void bg_Terminate(void);
void bg_Animate();

/* ------------
game_entity.c
------------ */
int ent_Initialize(void);
void ent_Terminate(void);
entity_t *ent_Spawn( char *name, int flags, char *model, vec3 pos, vec3 rot );
void ent_Remove( entity_t *rmEnt );
void ent_PrintInfo( char *null );

/* ------------
game_game.c
------------ */
int game_Initialize(void);
void game_Terminate(void);
void game_ShowLoading();
void game_ShowMenu();
void game_ShowGame();
void game_Start( char *type );
void game_End( char *null );
void game_Connect( char *address );
void game_Disconnect( char *null );
void game_StartServer( char *type );
void game_EndServer( char *null );

/* ------------
game_particlesys.c
------------ */
int ps_Initialize(void);
void ps_Terminate(void);
psystem_t *ps_Spawn( vec3 loc, vec3 dir, vec3 vel, vec2 ang_vel, 
						int live_time, int part_live_time,
						color mean_color, int shape,
						float inner_theta, float outer_theta, 
						float mean_speed, float mean_size, float mean_growth, 
						int function, float fA, float fB, float fC );
psystem_t *ps_SpawnOnEnt( entity_t *ent, tag_t *start, tag_t *end,
						int live_time, int part_live_time,
						color mean_color, int shape,
						float inner_theta, float outer_theta, 
						float mean_speed, float mean_size, float mean_growth, 
						int function, float fA, float fB, float fC );
void ps_Kill( psystem_t *ps );
void ps_UpdateSystems();
void ps_Draw();

/* ------------
game_world.c
------------ */
int wrld_Initialize(void);
void wrld_Terminate(void);
world_t *wrld_Create( int *size, int dimension, int numMines );
void wrld_Delete( world_t *world );
int wrld_ClearSquare( world_t *world, int x, int y, int z );
byte wrld_RayIntersect( world_t *world, vec3 vStart, vec3 vDir, int *vCube );

/******* DRAWING SERVICES *******/

/* ------------
dr_draw.c
------------ */
int d_Initialize(void);
void d_BeginDraw(void);
void d_EndDraw(void);
void d_Draw(void);
void d_Terminate(void);

int d_SetMapState1( shader_t *shader, int pass );
void d_UnsetMapState1( shader_t *shader );

/* ------------
dr_camera.c
------------ */
int d_CamInitialize(void);
void d_CamTerminate(void);
void d_SetCamera( camera_t *cam );
camera_t *d_GetCamera();
void d_MoveCamera( float movespeed, float *vAng,  float *vMvReq, float *vOut );
void d_ComputeFrameFrustum( camera_t *cam );
void d_CameraIn();
void d_CameraOut();

/* ------------
dr_console.c
------------ */
void d_DrawConsole();
void d_DrawConsoleFullscreen();

/* ------------
dr_entity.c
------------ */
void d_DrawEnts();
void d_DrawMesh( mesh_t *mesh, moo_data_t *moo );

/* ------------
dr_light.c
------------ */
int l_Initialize(void);
void l_Terminate(void);
void l_KillAll();
light_t *l_Spawn( byte type, float *position, float *ambient, float *diffuse, float *specular, 
							 float spot_exp, float spot_cutoff, float *spot_dir, 
							 float const_atten, float linear_atten, float quad_atten );
void l_Kill( light_t *l );
void l_Draw();
void l_PrintInfo( char *null );

/* ------------
dr_menu.c
------------ */
void d_MenuRect( float left, float right, float bottom, float top, color clr, colormap_t *cm );

/* ------------
dr_misc.c
------------ */
void d_EnterOrthoMode();
void d_LeaveOrthoMode();

/* ------------
dr_skybox.c
------------ */
int d_LoadSkybox( char *name );
void d_UnloadSkybox(void);
void d_DrawSkybox();
	
/* ------------
dr_text.c
------------ */
void d_TextLineBlack( char *line, float font_size, float pos_x, float pos_y );
void d_TextLineWhite( char *line, float font_size, float pos_x, float pos_y );
void d_TextLineColor( char *line, float font_size, byte color[4], float pos_x, float pos_y );
float d_GetTextWidth( char *text, float font_size );
float d_GetTextHeight( float font_size );
float d_GetCharWidth( char c, float font_size );

/* ------------
dr_transparent.c
------------ */
int trans_Initialize(void);
void trans_Terminate(void);
void d_DrawTransparent();
void trans_Quad( vec3 v1, vec3 v2, vec3 v3, vec3 v4, vec3 vNorm, 
					vec2 tc1, vec2 tc2, vec2 tc3, vec2 tc4, 
					uint8_t r, uint8_t g, uint8_t b, uint8_t a, shader_t *sh );
void trans_Mesh( entity_t *ent, mesh_t *mesh, uint8_t r, uint8_t g, uint8_t b, uint8_t a, shader_t *sh );
void trans_Sprite( vec3 vPos, float fSize, float fAng, color clr, colormap_t *cm );

/* ------------
dr_world.c
------------ */
void d_SetWorld( world_t *world );
aabb_t *d_GetWorldBounds();
void d_DrawWorld( void );
void wrld_SetColor( char *color );

#endif // _COG_GLOBAL_H

