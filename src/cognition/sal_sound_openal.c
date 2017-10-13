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
// sal_sound_openal.c
// Created 8/24/05 by Terrence Cole

// Includes
/////////////
#include "cog_global.h"
#include "sal_sound_openal.h"
#include <AL/al.h>
#include <AL/alut.h>

// Definitions
////////////////
#define DS_DEFAULT_CHANNELS 16
#define BODY_CHANNEL 1
#define MUSIC_CHANNEL 0

// Local Structures
/////////////////////
typedef struct
{
	uint8_t bPlaying; // currently playing
	sound_t *snd; // the current bound sound; valid if bPlaying
	audioStream_t *stream; // only used by the music channel
    ALuint buffer; // the al sound buffer index
	ALuint source; // the al source index
    
	// spacialization data
	uint8_t bLooping;
	vec3 *pvLoc; // points to the location of the location data or NULL of (0,0,0)
	vec3 *pvVel; // points to the location of the velocity data or NULL for stationary

	// buffer streaming data
	uint32_t iBufferSize; // bytes in the play buffer
	uint32_t iBufferLen; // time in milliseconds (default is 200) in the play buffer
	uint32_t iStartTime;
	uint32_t iStopTime;

	// update me every frame
	uint32_t iLastFillTime; // set to current frame time after a successful poll
	uint32_t iNextFillTime; // a time soon after which the buffer should be polled successivly until filled; iLastFillTime + iBufferLen - 50 (20fps min)
	uint32_t iSndPos; // the offset of played data in the DATA buffer; the next read position
	uint32_t iFillPos; // the next write location in the HW buffer
} snd_channel_t;
// on init we create num channels of these + music channel + body channel


// Local Variables
////////////////////
static varlatch vlSndRate = NULL;
static varlatch vlSndDepth = NULL;
static varlatch vlSndChannels = NULL;
static varlatch vlGlobalVolume = NULL;
static varlatch vlSfxVolume = NULL;
static varlatch vlMusicVolume = NULL;
static varlatch vlSfxPreload = NULL;
static varlatch vlMusicPreload = NULL;
static varlatch vlSfxQuality = NULL;

static uint8_t bSndInited = 0;

// channels of audio
static snd_channel_t *channels = NULL;
static int32_t iNumChannels = DS_DEFAULT_CHANNELS;
static ALuint *buffers = 0;
static ALuint *sources = 0;
static snd_channel_t musicChannel;

/* ------------
s_openal_Initialize
------------ */
int s_openal_Initialize(void) 
{	
    int a;
    double tStart, tEnd;
    
    con_Print( "\n<BLUE>Initializing</BLUE> OpenAL..." );
	eng_LoadingFrame();
	tStart = ts_GetSaneTime();

	// do basic sanity checks
	if( bSndInited )
	{
		con_Print( "<RED>OpenAL Init Warning:  OpenAL is already initialized." );
		return 1;
	}

	// latch our sound variables
	vlSndRate = var_GetVarLatch( "snd_rate" );
	vlSndDepth = var_GetVarLatch( "snd_depth" );
	vlSndChannels = var_GetVarLatch( "snd_channels" );
	vlGlobalVolume = var_GetVarLatch( "snd_volume" );
	vlSfxVolume = var_GetVarLatch( "snd_sfx_volume" );
	vlMusicVolume = var_GetVarLatch( "snd_music_volume" );
	vlSfxPreload = var_GetVarLatch( "snd_sfx_preload" );
	vlMusicPreload = var_GetVarLatch( "snd_music_preload" );
	vlSfxQuality = var_GetVarLatch( "snd_sfx_quality" );

	// check our latches
	if( vlSndRate == NULL ||
		vlSndDepth == NULL ||
		vlSndChannels == NULL ||
		vlGlobalVolume == NULL ||
		vlSfxVolume == NULL ||
		vlMusicVolume == NULL ||
		vlSfxPreload == NULL ||
		vlMusicPreload == NULL ||
		vlSfxQuality == NULL )
	{
		con_Print( "<RED>OpenAL Init Failed:  A variable latch operation failed." );
		return 0;
	}
	
	// get al
    alutInit(0,0);

	// allocate sound channels
	iNumChannels = (int32_t)var_GetFloat( vlSndChannels );
	if( iNumChannels < 0 ) iNumChannels = DS_DEFAULT_CHANNELS;
	else if( iNumChannels < 8 ) iNumChannels = 8;
	channels = (snd_channel_t*)mem_alloc( sizeof(snd_channel_t) * iNumChannels );
	if( channels == NULL )
	{
		con_Print( "<RED>OpenAL Init Failed:  Memory allocation failed for %d Channels.", iNumChannels );
		return 0;
	}
	memset( channels, 0, sizeof(snd_channel_t) * iNumChannels ); 
	memset( &musicChannel, 0, sizeof(snd_channel_t) );
    
    // get sources for everyone
    sources = mem_alloc( sizeof(ALint) * iNumChannels );
    assert( sources );
    alGetError(); /* clear error */
    alGenSources( iNumChannels, sources );
    if( alGetError() != AL_NO_ERROR ) 
    {	
        con_Print( "<RED>OpenAL Init Failed:  Source allocation failed for %d Sources.", iNumChannels );
		return 0;
    }
    
    // get buffers for everyone
    buffers = mem_alloc( sizeof(ALint) * iNumChannels );
    assert( buffers );
    alGetError(); /* clear error */
    alGenBuffers( iNumChannels, buffers );
    if( alGetError() != AL_NO_ERROR ) 
    {	
        con_Print( "<RED>OpenAL Init Failed:  Buffer allocation failed for %d Sources.", iNumChannels );
		return 0;
    }
    
    // copy over and setup the buffers and sources
    for( a = 0 ; a < iNumChannels ; a++ ) {
        channels[a].buffer = buffers[a];
        channels[a].source = sources[a];
    }

	// set ourselves to "inited"
	tEnd = ts_GetSaneTime();
	bSndInited = 1;
	con_Print( "\tDone:  %.3f sec", tEnd - tStart );

    return 1;
}

void s_openal_Terminate(void) 
{
    if( !bSndInited ) return;

    // stop
    s_openal_StopAllSound();
    
    // free our resources
    alDeleteSources( iNumChannels, sources );
    SAFE_RELEASE( sources );
    alDeleteBuffers( iNumChannels, buffers );
    SAFE_RELEASE( buffers );
    SAFE_RELEASE( channels );
    
    // close down al    
    //alutExit();
    bSndInited = 0;
}

void s_openal_Restart(void)
{
    con_Print("OPENAL Sound Support Unimplemented");
    assert(1==0);
}

void s_openal_StopAllSound(void)
{
	int a;

	if( !bSndInited ) return;

	for( a = 0 ; a < iNumChannels ; a++ )
	{
		if( channels[a].bPlaying ) s_openal_StopSound( a );
	}
	if( channels[MUSIC_CHANNEL].bPlaying ) s_openal_StopMusic( channels[a].stream );
}

void s_openal_SpacializeSounds(void)
{
	camera_t *cam = d_GetCamera();
	vec3 vAxisFwd = { 0.0f, 0.0f, -1.0f };
	int32_t a;
    ALuint src;
    
	// remix all 
	for( a = 0 ; a < iNumChannels ; a++ )
	{
		if( channels[a].bPlaying && channels[a].snd != NULL && channels[a].snd->wf.nChannels < 2 ) 
		{
//			con_Print( "Spacializing Channel %d: ( %f, %f, %f )", a, (*(channels[a].pvLoc))[0], (*(channels[a].pvLoc))[1], (*(channels[a].pvLoc))[2] );
		
            src = channels[a].source;
			if( channels[a].pvLoc != NULL )
			{
//                alSetSource3f( src, POSITION, /*
//				dsb3d->lpVtbl->SetPosition( dsb3d, */(*(channels[a].pvLoc))[X_AXIS], 
//											(*(channels[a].pvLoc))[Y_AXIS], 
//											-((*(channels[a].pvLoc))[Z_AXIS]) );//, DS3D_DEFERRED );
			}
			if( channels[a].pvVel != NULL )
			{
 //               alSetSource3f( src, VELOCIY, /* 
//				dsb3d->lpVtbl->SetVelocity( dsb3d,*/ (*(channels[a].pvVel))[X_AXIS], 
//											(*(channels[a].pvVel))[Y_AXIS], 
//											-((*(channels[a].pvVel))[Z_AXIS]) );//, DS3D_DEFERRED );
			}
		}
	}

/*	// set the values for the listener
    alListener3f( POSITION
	pListener->lpVtbl->SetVelocity( pListener, cam->vVel[X_AXIS], cam->vVel[Y_AXIS], -(cam->vVel[Z_AXIS]), DS3D_DEFERRED );
	pListener->lpVtbl->SetPosition( pListener, cam->vPos[X_AXIS], cam->vPos[Y_AXIS], -(cam->vPos[Z_AXIS]), DS3D_DEFERRED );
	pListener->lpVtbl->SetOrientation( pListener, cam->vFwd[0], cam->vFwd[1], -(cam->vFwd[2]), 
									cam->vUp[0], cam->vUp[1], -(cam->vUp[2]), DS3D_DEFERRED );

	// commit
	pListener->lpVtbl->CommitDeferredSettings( pListener );
*/}

sndhandle s_openal_PlaySound3D( sound_t *sound, uint8_t bLooping, vec3 *pvPos, vec3 *pvVel )
{
    con_Print("OPENAL Sound Support Unimplemented");
/*    assert(1==0); */
    return -1;
}

sndhandle s_openal_PlaySoundBody( sound_t *sound, uint8_t bLooping )
{
    con_Print("OPENAL Sound Support Unimplemented");
/*    assert(1==0); */
    return -1;
}

void s_openal_StopSound( sndhandle handle )
{
    con_Print("OPENAL Sound Support Unimplemented");
/*    assert(1==0); */
}

uint8_t s_openal_PlayMusic( audioStream_t *stream )
{
    con_Print("OPENAL Sound Support Unimplemented");
/*    assert(1==0); */
    return 0;
}

void s_openal_StopMusic( audioStream_t *stream )
{
    con_Print("OPENAL Sound Support Unimplemented");
/*    assert(1==0); */
}

void s_openal_CheckStreams(void)
{
// Too Noisy!
//    con_Print("OPENAL Sound Support Unimplemented");
/*    assert(1==0); */
}

void s_openal_UpdateVolume()
{
    con_Print("OPENAL Sound Support Unimplemented");
/*    assert(1==0); */
}
