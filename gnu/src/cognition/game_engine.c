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

// engine
// generalized system functions for global engine usage
// by Terrence Cole 1/10/03

// Includes
/////////////
#include "cog_global.h"
#include "cog_opengl.h"

// Definitions
////////////////
#define AUTOEXECFILE "autoexec.txt"

// Structures
///////////////

// Global Prototypes
//////////////////////
/*
void eng_Start(); 
void eng_Stop( char *buffer );
void eng_LoadingFrame();
void eng_EscapePress();
*/

// Local Prototypes
/////////////////////
static int eng_Initialize();
static void eng_Terminate();
static void eng_MainLoop();
static int eng_InitializeInterface();

// Local Variables
////////////////////
static byte bIsStopping = 0;

/* ------------
eng_Start - initializes engine services and kicks off the game loop
------------ */
void eng_Start()
{
	// set the loading flag
	state.bInLoading = 1;

	// do the init
	if( !eng_Initialize() )
	{
		eng_Stop( "3" );
		return;
	}

	// mark the init
	state.bInLoading = 0;

	// enter the game loop
	eng_MainLoop();
}

/* ------------
eng_Stop - closes all engine services and tells the system to close execution
------------ */
void eng_Stop( char *buffer )
{
	int exit_code = 0;

	// this function may recurse if a hard failure occurs; DON'T ALLOW IT
	if( bIsStopping ) return;
	bIsStopping = 1;

	// unhide our mouse pointer
	in_UnhideMouse();

	// get the exit code
	if( buffer != NULL ) { exit_code = (int)tcatof(buffer); }

	// close services
	eng_Terminate();

	// exit
	sys_Exit(exit_code);
}

/* ------------
eng_EscapePress
// handle escape key press events sanely
------------ */
void eng_EscapePress()
{
	// if the console is down, raise it
	if( state.bInConsole )
	{
		con_KeyPress( K_ESCAPE, 0 );
	}

	else if( state.bDrawMenu )
	{
		// if a game is active, toggle the menu
		if( state.bGameType != GAME_TYPE_NOGAME )
		{
			state.bDrawMenu ^= 1;
			if( state.bDrawMenu ) ms_Show();
			else ms_Hide();
		}

		// if the menu is there, without a game send the key there	
		else
		{
			mi->menu_KeyPress( K_ESCAPE, 0 );
		}
	}
}

/* ------------
eng_LoadingFrame - runs one frame of updates
------------ */
// this is a main-loop without the loop
void eng_LoadingFrame()
{
    // FIXME: restore this once we are starting up
    //return;
    
	// don't draw if we are trying to close down
	if( bIsStopping ) return;
	if( !state.bInLoading ) return;

	// stay active on the message queue
	in_ProcessEvents();
	sys_ProcessSystemMessages();
	
	// adjust the execution state (in case we want to animate progress later)
	state.last_time = state.frame_time;
	state.frame_time = ts_GetTime();
	state.delta_time = state.frame_time - state.last_time;

	// draw
	d_BeginDraw();

		d_Draw();
		
	d_EndDraw();
}

/* ------------
eng_MainLoop
------------ */
static void eng_MainLoop()
{
	while( 1 )
	{
		// check messages
		in_ProcessEvents();
		sys_ProcessSystemMessages();

		// adjust the execution state
		state.last_time = state.frame_time;
		state.frame_time = ts_GetTime();
		state.delta_time = state.frame_time - state.last_time;

		// update the mouse pointer
		ms_Update();

		// update the background
		bg_Animate();

		// update the particle systems
		ps_UpdateSystems();

		// share time
		t_CheckTimers();

		// update the menu system
		if( mi ) mi->menu_Frame( &state );

		// give the server process time
		if( sys_handles.cServer != NULL )
		{
			if( (state.last_server_frame - state.frame_time) > SERVER_FRAME_TIME )
			{
				si->ser_Frame();
			}
		}

		// give the client everything else
		if( sys_handles.cClient != NULL )
		{
			if( !ci->cl_Frame( &state ) )
			{
				if( state.bGameType == GAME_TYPE_SINGLE ) game_End(NULL);
				else game_Disconnect( NULL);
			}
		}

		// check and fill our sound buffers
		s_CheckStreams();

		// sync the sounds to the camera
		// FIXME:  does this really need to run every frame?
		s_SpacializeSounds();
	
		// draw
		d_BeginDraw();

			d_Draw();

		d_EndDraw();
	}
}

/* ------------
eng_Initialize -  starts up all engine services
------------ */
static int eng_Initialize()
{
	// flag us as initing for the draw loop
	state.bInInit = 1;
	
	// base lib stuff, independent of other init, may or may not be required later
	if( !q_Initialize() ) return 0;
    
	// lib and base prelibs to get up the screen and file logs
	if( !eng_InitializeInterface() ) return 0; // shared base stuff dereferences through this
	if( !mem_Initialize() ) return 0; // required by everything else
	if( !fs_Initialize() ) return 0; // needed for log; start asap
	if( !f_Initialize() ) return 0; // start logging almost first
	if( !ts_Initialize() ) return 0; // so that calls to mem_alloc will have times in debug builds
	if( !t_Initialize() ) return 0; 
	if( !rng_Initialize() ) return 0; // seed random numbers
	
	// init command system
	if( !com_Initialize() ) return 0;
	if( !bind_Initialize() ) return 0;  // we can now interact
	
	// load variable values from file
	if( !var_Initialize() ) return 0; 

	// update vars from command line
	par_ExecCmdLine( sys_handles.cmdLine );

	// get a screen up asap
	if( !dis_Initialize() ) return 0; // allow us to open window asap
	if( !gl_Initialize() ) return 0;
	if( !trans_Initialize() ) return 0; // ready to frame

	// prepare to load media
	if( !bs_Initialize() ) return 0; // needed for jpeg loads
	if( !img_Initialize() ) return 0;
	if( !cm_Initialize() ) return 0;

	// load media
	if( !con_Initialize() ) return 0;

	// we now have enough to render loading frames
	// post the first farme
	eng_LoadingFrame();

	// print out some timely announcements
	con_Print( "\nCommand Line: %s", sys_handles.cmdLine );
	sys_PrintSystemInformation();
	fs_PrintFileSystemInformation();
		eng_LoadingFrame();

	// base system stuff
	if( !m_Initialize() ) return 0; // fast trig table init
		eng_LoadingFrame();
	if( !net_Initialize() ) return 0;

	// direct sound needs the window open in WIN32
	if( !s_Initialize() ) return 0;
	if( !in_Initialize() ) return 0; // needs display window

	// now we can set mouse states
	in_HideMouse();

	// model system init medias
	if( !sh_Initialize() ) return 0;
	if( !moo_Initialize() ) return 0;
	if( !mod_Initialize() ) return 0;

	// music init
	if( !mp3_Initialize() ) return 0;
	if( !mus_Initialize() ) return 0;

	// high level abstractions
	if( !ms_Initialize() ) return 0;
	if( !ent_Initialize() ) return 0;
	if( !ps_Initialize() ) return 0;
	if( !wrld_Initialize() ) return 0;
	if( !module_Initialize() ) return 0;

	// drawing stuff
	if( !d_Initialize() ) return 0;

	// high level implementations
	if( !bg_Initialize() ) return 0;
	if( !game_Initialize() ) return 0;

	// DONE:  open the menu
	if( !module_OpenMenu() ) return 0;

	// load user preferences from autoexec
	con_Print( "\n<BLUE>Running</BLUE> Custom User Commands:" );
	par_ExecFile( AUTOEXECFILE, "" );

	// the engine is prepared for operation
	con_Print( "\n-----------------------" );
	con_Print( "Initialization Complete" );
	con_Print( "-----------------------\n" );

	// mark us done and return
	state.bInInit = 0;
	return 1;
}

/* ------------
eng_Terminate - shuts down all system services
------------ */
static void eng_Terminate()
{
	// term pathway (opposite of):  lib (initless), base, lib (needs init), sal, sv, media, game, draw
	game_Terminate();
	module_Terminate();

	// Game Stuff
	bg_Terminate();
	d_Terminate();
	wrld_Terminate();
	ps_Terminate();
	ent_Terminate();

	// Media Stuff
	mp3_Terminate();
	mod_Terminate();
	moo_Terminate();
	sh_Terminate();

	// Service Stuff
	mus_Terminate();
	con_Terminate();
	ms_Terminate();

	// Pre Media
	cm_Terminate();
	img_Terminate();

	// we are about to leave window land
	in_UnhideMouse();

	// System stuff
	s_Terminate();
	in_Terminate();
	gl_Terminate();
	net_Terminate();
	trans_Terminate();

	// Base Service Stuff
	var_Terminate();
	com_Terminate();
	bs_Terminate();

	// these don't do anything at the moment
	bind_Terminate();
	rng_Terminate();
	m_Terminate();

	// last out
	dis_Terminate();
	t_Terminate();
    ts_Terminate();
	fs_Terminate();
	mem_Terminate();
	f_Terminate();
	q_Terminate();
}

/* ------------
eng_InitializeInterface
------------ */
static int eng_InitializeInterface()
{
	ei = &engine_interface;
	ei->apiversion = API_VERSION;

// Engine Functionality
	ei->con_Print = con_Print;
	ei->mem_alloc = mem_alloc;
	ei->mem_free = mem_free;
	ei->eng_Stop = eng_Stop;
	ei->eng_LoadingFrame = eng_LoadingFrame;

// Command Processor
	ei->com_AddCommand = com_AddCommand;
	ei->com_RemoveCommand = com_RemoveCommand;

// List Management
	ei->ll_iter_data = ll_iter_data;
	ei->ll_iter_isdone = ll_iter_isdone;
	ei->ll_iter_next = ll_iter_next;
	ei->ll_iter_reset = ll_iter_reset;
	
// Network interfaces
	ei->net_cl_Connect = net_cl_Connect;
	ei->net_cl_Disconnect = net_cl_Disconnect;
	ei->net_cl_GetServerPacket = net_cl_GetServerPacket;
	ei->net_cl_Flush = net_cl_Flush;
	
	ei->net_cl_SendByte = net_cl_SendByte;
	ei->net_cl_SendShort = net_cl_SendShort;
	ei->net_cl_SendLong = net_cl_SendLong;
	ei->net_cl_SendString = net_cl_SendString;

	ei->net_sv_OpenNetwork = net_sv_OpenNetwork;
	ei->net_sv_CloseNetwork = net_sv_CloseNetwork;
	ei->net_sv_Accept = net_sv_Accept;
	ei->net_sv_Disconnect = net_sv_Disconnect;
	ei->net_sv_GetClientPacket = net_sv_GetClientPacket;
	ei->net_sv_Flush = net_sv_Flush;

	ei->net_sv_SendByte = net_sv_SendByte;
	ei->net_sv_SendShort = net_sv_SendShort;
	ei->net_sv_SendLong = net_sv_SendLong;
	ei->net_sv_SendString = net_sv_SendString;

	
	ei->net_GetShort = net_GetShort;
	ei->net_GetLong = net_GetLong;
	ei->net_GetString = net_GetString;
	
// game services
	ei->wrld_Create = wrld_Create;
	ei->wrld_Delete = wrld_Delete;
	ei->wrld_ClearSquare = wrld_ClearSquare;
	ei->wrld_RayIntersect = wrld_RayIntersect;

// game management
	ei->game_ShowLoading = game_ShowLoading;
	ei->game_ShowMenu = game_ShowMenu;
	ei->game_ShowGame = game_ShowGame;
	ei->game_Start = game_Start;
	ei->game_End = game_End;
	ei->game_Connect = game_Connect;
	ei->game_Disconnect = game_Disconnect;
	ei->game_StartServer = game_StartServer;
	ei->game_EndServer = game_EndServer;

// particle systems
 	ei->ps_Spawn = ps_Spawn;
	ei->ps_SpawnOnEnt = ps_SpawnOnEnt;
	ei->ps_Kill = ps_Kill;
	
// Lighting System
	ei->l_Spawn = l_Spawn;
	ei->l_Kill = l_Kill;

// Camera / Environment
	ei->d_LoadSkybox = d_LoadSkybox;
	ei->d_SetCamera = d_SetCamera;
	ei->d_GetCamera = d_GetCamera;
	ei->d_SetWorld = d_SetWorld;
	ei->d_MoveCamera = d_MoveCamera;

// Text Services 
	ei->d_TextLineBlack = d_TextLineBlack;
	ei->d_TextLineWhite = d_TextLineWhite;
	ei->d_TextLineColor = d_TextLineColor;
	ei->d_GetTextWidth = d_GetTextWidth;
	ei->d_GetTextHeight = d_GetTextHeight;
	ei->d_GetCharWidth = d_GetCharWidth;

// Forground Rectangle
	ei->d_MenuRect = d_MenuRect;

// Sound Management
	ei->s_Load = s_Load;
	ei->s_Unload = s_Unload;
	ei->s_PlaySound = s_PlaySound;
	ei->s_StopSound = s_StopSound;
	ei->s_PlayMusic = s_PlayMusic;
	ei->s_StopMusic = s_StopMusic;

// Media Management
	ei->mod_Load = mod_Load;
	ei->mod_Unload = mod_Unload;
	ei->ent_Spawn = ent_Spawn;
	ei->ent_Remove = ent_Remove;

// Colormap Management
	ei->cm_LoadFromFiles = cm_LoadFromFiles;
	ei->cm_LoadConstAlpha = cm_LoadConstAlpha;
	ei->cm_LoadFromImage = cm_LoadFromImage;
	ei->cm_Unload = cm_Unload;
	ei->cm_GetGlobalFilter = cm_GetGlobalFilter;
	ei->cm_PrintInfo = cm_PrintInfo;

// Collision Detection
	ei->col_CheckEnts = col_CheckEnts;
	ei->col_Respond = col_Respond;
	
// Random Number Generator
	ei->rng_Double = rng_Double;
	ei->rng_Float = rng_Float;
	ei->rng_Long = rng_Long;
	ei->rng_Short = rng_Short;
	ei->rng_Byte = rng_Byte;
	
// Timers and Timing Data
	ei->ts_GetTime = ts_GetTime;
	ei->ts_GetSaneTime = ts_GetSaneTime;
	ei->t_SetTimer = t_SetTimer;
	ei->t_KillTimer = t_KillTimer;
	
// Persistant Variable Management
	ei->var_GetVarLatch = var_GetVarLatch;
	ei->var_GetFloat = var_GetFloat;
	ei->var_GetString = var_GetString;
	ei->var_GetFloatFromName = var_GetFloatFromName;
	ei->var_GetStringFromName = var_GetStringFromName;
	ei->var_UpdateFloat = var_UpdateFloat;
	ei->var_UpdateString = var_UpdateString;

// mouse
	ei->ms_GetPosition = ms_GetPosition;
	ei->ms_Show = ms_Show;
	ei->ms_Hide = ms_Hide;
	ei->ms_MouseIsVisible = ms_MouseIsVisible;
	ei->ms_MouseIsHidden = ms_MouseIsHidden;

// keyboard
	ei->bind_ShiftIsDown = bind_ShiftIsDown;
	ei->bind_AltIsDown = bind_AltIsDown;
	ei->bind_CtrlIsDown = bind_CtrlIsDown;

	return 1;
}

/* THESE ARE STUBS FOR COMPILING WITHOUT DX */
/*
int in_di_Initialize(void) { return 0; }
void in_di_Terminate(void) {}
void in_di_ProcessEvents() {}
int s_ds_Initialize(void) { return 0; }
void s_ds_Terminate(void) {}

void s_ds_Restart(void) {}
void s_ds_StopAllSound(void) {}
void s_ds_SpacializeSounds(void) {}

sndhandle s_ds_PlaySound3D( sound_t *sound, uint8_t bLooping, vec3 *pvPos, vec3 *pvVel ) { return 0; }
sndhandle s_ds_PlaySoundBody( sound_t *sound, uint8_t bLooping ) { return 0; }
void s_ds_StopSound( sndhandle handle ) {}

uint8_t s_ds_PlayMusic( audioStream_t *stream ) { return 0; }
void s_ds_StopMusic( audioStream_t *stream ) {}

void s_ds_CheckStreams(void) {}
void s_ds_UpdateVolume() {}
*/
