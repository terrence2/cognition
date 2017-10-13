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
// sal_sound_win_ds.c
// Created 6/27/03 by Terrence Cole

// Includes
/////////////
#include "cog_global.h"
#include "cog_opengl.h"
#include "sal_sound_win_ds.h"
#include <mmreg.h>
#define DIRECTSOUND_VERSION 0x0900
#include <dsound.h>

// Definitions
////////////////
#define SND_DISTANCE_FACTOR 0.06f // in meters per unit

#define DS_DEFAULT_RATE 22050
#define DS_DEFAULT_DEPTH 8
#define DS_DEFAULT_CHANNELS 16

#define BODY_CHANNEL 1
#define MUSIC_CHANNEL 0

#define BUFFER_POLL_GRANULARITY 50
// 50 = 20fps
// 100 = 10fps
// etc

// Local Structures
/////////////////////
typedef struct
{
	uint8_t bPlaying; // currently playing
	sound_t *snd; // the current bound sound; valid if bPlaying
	audioStream_t *stream; // only used by the music channel
	LPDIRECTSOUND3DBUFFER8 ds3DBuffer;
	LPDIRECTSOUNDBUFFER8 dsBuffer;
	
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

// Headered Prototypes
//////////////////////
/*
int s_ds_Initialize(void);
void s_ds_Terminate(void);

void s_ds_Restart(void);
void s_ds_StopAllSound(void);
void s_ds_SpacializeSounds(void);

sndhandle s_ds_PlaySound3D( sound_t *sound, uint8_t bLooping, vec3 *pvPos, vec3 *pvVel );
sndhandle s_ds_PlaySoundBody( sound_t *sound, uint8_t bLooping );
void s_ds_StopSound( sndhandle handle );

uint8_t s_ds_PlayMusic( audioStream_t *stream );
void s_ds_StopMusic( audioStream_t *stream );

void s_ds_CheckStreams(void);
void s_ds_UpdateVolume();
*/


// Local Prototypes
/////////////////////
static void s_ds_FillBuffer( snd_channel_t *ch, sndhandle handle );

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

// dsound stuff
static LPDIRECTSOUND8 pDS8 = NULL;
static LPDIRECTSOUNDBUFFER pPrimary = NULL;
static LPDIRECTSOUND3DLISTENER pListener = NULL;

// channels of audio
static snd_channel_t *channels = NULL;
static int32_t iNumChannels = DS_DEFAULT_CHANNELS;
static snd_channel_t musicChannel;

// *********** FUNCTIONALITY ***********
/* ------------
s_ds_Initialize
// brings up ds with the requirested parameters; returns 0 on failure
------------ */
int s_ds_Initialize(void)
{
	WORD wTmp;
	HRESULT hr;
	DSBUFFERDESC dsbd;
	WAVEFORMATEX wf;
	double tStart, tEnd;

	con_Print( "\n<BLUE>Initializing</BLUE> Direct Sound..." );
	eng_LoadingFrame();
	tStart = t_GetSaneTime();

	// do basic sanity checks
	if( bSndInited )
	{
		con_Print( "<RED>Direct Sound Init Warning:  Direct Sound is already initialized." );
		return 1;
	}
	
	if( gl_state.window.hWnd == NULL )
	{
		con_Print( "<RED>Direct Sound Init Failed:  Direct Sound Needs the Render Window to set Cooperative Level." );
		return 0;
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
		con_Print( "<RED>Direct Sound Init Failed:  A variable latch operation failed." );
		return 0;
	}
	
	// get direct sound
	hr = DirectSoundCreate8( &DSDEVID_DefaultPlayback, &pDS8, NULL );
	if( hr != DS_OK )
	{
		con_Print( "<RED>Direct Sound Initialization Failed:  Could not create the Direct Sound Device." );
		return 0;
	}

	// set the coop level to priority
	hr = pDS8->lpVtbl->SetCooperativeLevel( pDS8, gl_state.window.hWnd, DSSCL_PRIORITY );
	if( hr != DS_OK )
	{
		con_Print( "<RED>Direct Sound Initialization Failed:  Could not Set Cooperative Level." );
		s_ds_Terminate();
		return 0;
	}

	// compact memory
	hr = pDS8->lpVtbl->Compact( pDS8 );

	// get the primary buffer
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME;
	dsbd.dwBufferBytes = 0;
	dsbd.dwReserved = 0;
	dsbd.lpwfxFormat = NULL;
	dsbd.guid3DAlgorithm = GUID_NULL;
	hr = pDS8->lpVtbl->CreateSoundBuffer( pDS8, &dsbd, &pPrimary, NULL );
	if( hr != DS_OK )
	{
		con_Print( "<RED>Direct Sound Initialization Failed:  Could not Create Primary Buffer." );
		s_ds_Terminate();
		return 0;
	}

	// try to set the buffer format to what the user wants
	wf.wFormatTag = WAVE_FORMAT_PCM;
	wf.nChannels = 2;
	wf.nSamplesPerSec = (DWORD)var_GetFloat( vlSndRate );
	// FIXME: lock the sound depth to 8, 16, 32, 64, 128 and up
	wTmp = (WORD)var_GetFloat( vlSndDepth );
	if( wTmp <= 8 ) wTmp = 8;
	else wTmp = 16;
	wf.wBitsPerSample = wTmp;
	wf.nBlockAlign = (wf.wBitsPerSample * wf.nChannels) / 8;
	wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
	wf.cbSize = 0;
	hr = pPrimary->lpVtbl->SetFormat( pPrimary, &wf );
	if( hr == DSERR_BADFORMAT )
	{
		// try doing this the default way
		con_Print( "\t<RED>Warning:  Set Format Failed for Rate: %d, Depth:  %d", wf.nSamplesPerSec, wf.wBitsPerSample );
		con_Print( "\t<RED>Using Default Values:  Rate:  22050,  Depth:  8" );

		wf.nSamplesPerSec = DS_DEFAULT_RATE;
		wf.wBitsPerSample = DS_DEFAULT_DEPTH;
		wf.nBlockAlign = (wf.wBitsPerSample * wf.nChannels) / 8;
		wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
		
		hr = pPrimary->lpVtbl->SetFormat( pPrimary, &wf );
	}
	if( hr != DS_OK )
	{
		con_Print( "<RED>Direct Sound Initialization Failed:  Could not Create Primary Buffer." );
		s_ds_Terminate();
		return 0;
	}

	// get the 3d listener
	hr = pPrimary->lpVtbl->QueryInterface( pPrimary, &IID_IDirectSound3DListener8, (VOID**)&pListener );
	if( hr != DS_OK )
	{
		con_Print( "<RED>Direct Sound Initialization Failed:  Could not Query Primary Buffer for Listener." );
		s_ds_Terminate();
		return 0;
	}

	// setup the listener for our world
	pListener->lpVtbl->SetDistanceFactor( pListener, SND_DISTANCE_FACTOR, DS3D_IMMEDIATE );
	pListener->lpVtbl->SetRolloffFactor( pListener, 0.5f, DS3D_IMMEDIATE );

	// allocate sound channels
	iNumChannels = (int32_t)var_GetFloat( vlSndChannels );
	if( iNumChannels < 0 ) iNumChannels = DS_DEFAULT_CHANNELS;
	else if( iNumChannels < 8 ) iNumChannels = 8;
	channels = (snd_channel_t*)mem_alloc( sizeof(snd_channel_t) * iNumChannels );
	if( channels == NULL )
	{
		con_Print( "<RED>Direct Sound Init Failed:  Memory allocation failed for %d Channels.", iNumChannels );
		s_ds_Terminate();
		return 0;
	}
	memset( channels, 0, sizeof(snd_channel_t) * iNumChannels ); 
	memset( &musicChannel, 0, sizeof(snd_channel_t) );

	// set ourselves to "inited"
	tEnd = t_GetSaneTime();
	bSndInited = 1;
	con_Print( "\tDone:  %.3f sec", tEnd - tStart );
	return 0;
}

/* ------------
s_ds_Terminate
// takes down ds
------------ */
void s_ds_Terminate(void)
{
	con_Print( "\n<RED>Terminating</RED> Direct Sound..." );

	// this *should* already have been called at a higher level
	s_ds_StopAllSound();

	// cleanup memory
	SAFE_RELEASE( channels );

	// cleanup objects
	// n.b. Stop should handle closing out the secondary buffers
	if( pListener != NULL ) pListener->lpVtbl->Release( pListener );
	pListener = NULL;
	if( pPrimary != NULL ) pPrimary->lpVtbl->Release( pPrimary );
	pPrimary = NULL;
	if( pDS8 != NULL ) pDS8->lpVtbl->Release( pDS8 );
	pDS8 = NULL;
	
	bSndInited = 0;

	con_Print( "\tSuccess.." );
}

void s_ds_Restart(void)
{
	// FIXME:  write this, bind it to a command
    assert(1==0);
}

/* ------------
s_ds_StopAllSounds
// iterates all possibly playing sounds, calling stop if playing
------------ */
void s_ds_StopAllSound(void)
{
	int a;

	if( !bSndInited ) return;

	for( a = 0 ; a < iNumChannels ; a++ )
	{
		if( channels[a].bPlaying ) s_ds_StopSound( a );
	}
	if( channels[MUSIC_CHANNEL].bPlaying ) s_ds_StopMusic( channels[a].stream );
}

/* ------------
s_ds_SpacializeSounds
// sync the listener to the camera; direct x docs say this is fairly laborious; sync at 10fps?
------------ */
void s_ds_SpacializeSounds(void)
{
	camera_t *cam = d_GetCamera();
	vec3 vAxisFwd = { 0.0f, 0.0f, -1.0f };
	LPDIRECTSOUND3DBUFFER dsb3d;
	int32_t a;

	if( pListener == NULL ) return;

	// remix all 
	for( a = 0 ; a < iNumChannels ; a++ )
	{
		if( channels[a].bPlaying && channels[a].snd != NULL && channels[a].snd->wf.nChannels < 2 ) 
		{
//			con_Print( "Spacializing Channel %d: ( %f, %f, %f )", a, (*(channels[a].pvLoc))[0], (*(channels[a].pvLoc))[1], (*(channels[a].pvLoc))[2] );
		
			dsb3d = channels[a].ds3DBuffer;
			if( channels[a].pvLoc != NULL )
			{
				dsb3d->lpVtbl->SetPosition( dsb3d, (*(channels[a].pvLoc))[X_AXIS], 
											(*(channels[a].pvLoc))[Y_AXIS], 
											-((*(channels[a].pvLoc))[Z_AXIS]), DS3D_DEFERRED );
			}
			if( channels[a].pvVel != NULL )
			{
				dsb3d->lpVtbl->SetVelocity( dsb3d, (*(channels[a].pvVel))[X_AXIS], 
											(*(channels[a].pvVel))[Y_AXIS], 
											-((*(channels[a].pvVel))[Z_AXIS]), DS3D_DEFERRED );
			}
		}
	}

	// set the values for the listener
	pListener->lpVtbl->SetVelocity( pListener, cam->vVel[X_AXIS], cam->vVel[Y_AXIS], -(cam->vVel[Z_AXIS]), DS3D_DEFERRED );
	pListener->lpVtbl->SetPosition( pListener, cam->vPos[X_AXIS], cam->vPos[Y_AXIS], -(cam->vPos[Z_AXIS]), DS3D_DEFERRED );
	pListener->lpVtbl->SetOrientation( pListener, cam->vFwd[0], cam->vFwd[1], -(cam->vFwd[2]), 
									cam->vUp[0], cam->vUp[1], -(cam->vUp[2]), DS3D_DEFERRED );

	// commit
	pListener->lpVtbl->CommitDeferredSettings( pListener );
}

/* ------------
s_ds_PlaySoundBody
// called for stereo sounds; less expensive since plays w/out ds3d buffer
// starts the play, sets up states, allocates a channel, etc
// returns the handle on success, or -1 on fail
------------ */
sndhandle s_ds_PlaySoundBody( sound_t *sound, uint8_t bLooping )
{
	DSBUFFERDESC dsbd;
	LPDIRECTSOUNDBUFFER dsb; // part of the path to dsb8
	LPDIRECTSOUNDBUFFER8 dsb8; // temp for ease of use
	HRESULT hr;
	uint8_t *pData1, *pData2;
	uint32_t iSize1, iSize2;
	int32_t a, iChIndex;
	uint32_t iTmp;
	
	if( !bSndInited ) return -1;
	if( sound == NULL ) 
	{
		con_Print( "Direct Sound Warning:  Play Sound Called for a NULL sound." );
		return -1;
	}
	
	// find an empty channel
	a = 0;
	iChIndex = -1;
	while( iChIndex == -1 && a < iNumChannels )
	{
		if( !channels[a].bPlaying ) 
		{
			iChIndex = a;
		}
		else
		{
			a++;
		}
	}

	// get a new secondary buffer
	memset( &dsbd, 0, sizeof(DSBUFFERDESC) );
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_STATIC | DSBCAPS_CTRLVOLUME;
	dsbd.dwBufferBytes = (DWORD)(var_GetFloat( vlSfxPreload ) * sound->wf.nAvgBytesPerSec);
	dsbd.dwReserved = 0;
	dsbd.lpwfxFormat = &(sound->wf);
	dsbd.guid3DAlgorithm = GUID_NULL;

	// get the dsb
	hr = pDS8->lpVtbl->CreateSoundBuffer( pDS8, &dsbd, &dsb, NULL );
	if( hr != DS_OK )
	{
		con_Print( "<RED>Direct Sound Warning:  Play Sound (Body) Failed, could not create secondary buffer." );
		return -1;
	}

	// query for the dsb8
	hr = dsb->lpVtbl->QueryInterface( dsb, &IID_IDirectSoundBuffer8, &dsb8 );
	if( hr != DS_OK )
	{
		con_Print( "<RED>Direct Sound Warning:  Play Sound (Body) Failed, could not query for secondary buffer version 8." );
		dsb->lpVtbl->Release( dsb );
		return -1;
	}

	// release dsb, keep dsb8
	dsb->lpVtbl->Release( dsb );

	// setup info for the streamer
	channels[iChIndex].iBufferSize = dsbd.dwBufferBytes;

	// lock the buffer
	hr = dsb8->lpVtbl->Lock( dsb8, 0, 0, &pData1, (DWORD*)&iSize1, &pData2, (DWORD*)&iSize2, DSBLOCK_ENTIREBUFFER );
	if( hr == DSERR_BUFFERLOST )
	{
		dsb8->lpVtbl->Restore( dsb8 );
		hr = dsb8->lpVtbl->Lock( dsb8, 0, 0, &pData1, (DWORD*)&iSize1, &pData2, (DWORD*)&iSize2, DSBLOCK_ENTIREBUFFER );
	}
	if( hr != DS_OK || pData1 == NULL )
	{
		con_Print( "<RED>Direct Sound Warning:  Cannot play sfx (3D):  Could not lock buffer." );
		dsb8->lpVtbl->Release( dsb8 );
		return -1;
	}
	
	// copy data to the buffer; we can use only the first pointer since it's the full buffer
	if( sound->iDataSize < iSize1 ) iTmp = sound->iDataSize;
	else iTmp = iSize1;
	memcpy( pData1, sound->bData, iTmp );

	// unlock the buffer
	hr = dsb8->lpVtbl->Unlock( dsb8, pData1, iTmp, pData2, 0 );

	// kick off the sound asap
	if( bLooping ) dsb8->lpVtbl->Play( dsb8, 0, 0, DSBPLAY_LOOPING ); 
	else dsb8->lpVtbl->Play( dsb8, 0, 0, 0 );

	// record how much was copied, so we know where to pick up
	channels[iChIndex].iSndPos = iTmp;
	channels[iChIndex].iFillPos = iTmp;
	while( channels[iChIndex].iFillPos > channels[iChIndex].iBufferLen ) channels[iChIndex].iFillPos -= channels[iChIndex].iBufferSize; 

	// set states
	channels[iChIndex].iBufferLen = (DWORD)(var_GetFloat( vlSfxPreload ) * 1000.0f); // sec->millisec
	channels[iChIndex].iLastFillTime = state.frame_time;
	channels[iChIndex].iNextFillTime = channels[iChIndex].iLastFillTime + channels[iChIndex].iBufferLen - BUFFER_POLL_GRANULARITY;
	channels[iChIndex].iStartTime = state.frame_time;

	if( bLooping )
	{
		channels[iChIndex].bLooping = 1;
		channels[iChIndex].iStopTime = 0xFFFFFFFF;
	}
	else
	{
		channels[iChIndex].bLooping = 0;
		channels[iChIndex].iStopTime = channels[iChIndex].iStartTime + (uint32_t)(((double)sound->iDataBlocks / (double)sound->iDataBlocks) * 1000.0);
	}

	channels[iChIndex].bPlaying = 1;
	channels[iChIndex].snd = sound;
	channels[iChIndex].stream = NULL;
	channels[iChIndex].dsBuffer = dsb8;
	channels[iChIndex].ds3DBuffer = NULL;
	channels[iChIndex].pvLoc = NULL;
	channels[iChIndex].pvVel = NULL;

	return iChIndex;
}

/* ------------
s_ds_PlaySound3D
// called for mono sounds;  more expensive, since needs extra buffer and continual spacialization
// starts the play, sets up states, allocates a channel, etc
// returns the handle on success, or -1 on fail
------------ */
sndhandle s_ds_PlaySound3D( sound_t *sound, uint8_t bLooping, vec3 *pvPos, vec3 *pvVel )
{
	DSBUFFERDESC dsbd;
	LPDIRECTSOUNDBUFFER dsb; // part of the path to dsb8
	LPDIRECTSOUNDBUFFER8 dsb8; // temp for ease of use
	LPDIRECTSOUND3DBUFFER8 dsb3d; // temp for ease of use
	HRESULT hr;
	int32_t a, iChIndex;
	uint8_t *pData1, *pData2;
	uint32_t iSize1, iSize2;
	uint32_t iTmp;

	if( !bSndInited ) return -1;
	if( sound == NULL ) 
	{
		con_Print( "Direct Sound Warning:  Play Sound (3D) Called for a NULL sound." );
		return -1;
	}
	
	// find an empty channel
	a = 0;
	iChIndex = -1;
	while( iChIndex == -1 && a < iNumChannels )
	{
		if( !channels[a].bPlaying ) 
		{
			iChIndex = a;
		}
		else
		{
			a++;
		}
	}
	
	// check for 'out of channels'
	if( iChIndex < 0 ) return -1;

	// get a new secondary buffer
	memset( &dsbd, 0, sizeof(DSBUFFERDESC) );
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_STATIC | DSBCAPS_CTRLVOLUME;
	dsbd.dwBufferBytes = (DWORD)(var_GetFloat( vlSfxPreload ) * sound->wf.nAvgBytesPerSec);
	dsbd.dwReserved = 0;
	dsbd.lpwfxFormat = &(sound->wf);
	if( tcstricmp( var_GetString(vlSfxQuality), "high" ) ) dsbd.guid3DAlgorithm = DS3DALG_HRTF_FULL;
	else if( tcstricmp( var_GetString(vlSfxQuality), "medium" ) ) dsbd.guid3DAlgorithm = DS3DALG_HRTF_LIGHT;
	else dsbd.guid3DAlgorithm = DS3DALG_NO_VIRTUALIZATION;

	// get the dsb
	hr = pDS8->lpVtbl->CreateSoundBuffer( pDS8, &dsbd, &dsb, NULL );
	if( hr != DS_OK && hr != DS_NO_VIRTUALIZATION )
	{
		con_Print( "<RED>Direct Sound Warning:  Play Sound (3D) Failed, could not create secondary buffer." );
		return -1;
	}

	// query for the dsb8
	hr = dsb->lpVtbl->QueryInterface( dsb, &IID_IDirectSoundBuffer8, &dsb8 );
	if( hr != DS_OK )
	{
		con_Print( "<RED>Direct Sound Warning:  Play Sound (3D) Failed, could not query for secondary buffer version 8." );
		dsb->lpVtbl->Release( dsb );
		return -1;
	}

	// query for the dsb3d
	hr = dsb->lpVtbl->QueryInterface( dsb, &IID_IDirectSound3DBuffer8, &dsb3d );
	if( hr != DS_OK )
	{
		con_Print( "<RED>Direct Sound Warning:  Play Sound (3D) Failed, could not query for secondary 3D buffer." );
		dsb8->lpVtbl->Release( dsb8 );
		dsb->lpVtbl->Release( dsb );
		return -1;
	}

	// release dsb, keep dsb8
	dsb->lpVtbl->Release( dsb );

	// setup info for the streamer
	channels[iChIndex].iBufferSize = dsbd.dwBufferBytes;

	// lock the buffer
	hr = dsb8->lpVtbl->Lock( dsb8, 0, 0, &pData1, (DWORD*)&iSize1, &pData2, (DWORD*)&iSize2, DSBLOCK_ENTIREBUFFER );
	if( hr == DSERR_BUFFERLOST )
	{
		dsb8->lpVtbl->Restore( dsb8 );
		hr = dsb8->lpVtbl->Lock( dsb8, 0, 0, &pData1, (DWORD*)&iSize1, &pData2, (DWORD*)&iSize2, DSBLOCK_ENTIREBUFFER );
	}
	if( hr != DS_OK || pData1 == NULL )
	{
		con_Print( "<RED>Direct Sound Warning:  Cannot play sfx (3D):  Could not lock buffer." );
		dsb8->lpVtbl->Release( dsb8 );
		dsb3d->lpVtbl->Release( dsb3d );
		return -1;
	}

	// copy data to the buffer; we can use only the first pointer since it's the full buffer
	if( sound->iDataSize < iSize1 ) iTmp = sound->iDataSize;
	else iTmp = iSize1;
	memcpy( pData1, sound->bData, iTmp );

	// unlock the buffer
	hr = dsb8->lpVtbl->Unlock( dsb8, pData1, iTmp, pData2, 0 );

	// kick off the sound
	dsb8->lpVtbl->Play( dsb8, 0, 0, DSBPLAY_LOOPING ); 

	// record how much was copied, so we know where to pick up
	channels[iChIndex].iSndPos = iTmp;
	channels[iChIndex].iFillPos = iTmp;
	while( channels[iChIndex].iFillPos > channels[iChIndex].iBufferLen ) channels[iChIndex].iFillPos -= channels[iChIndex].iBufferSize; 

	// set 3d settings for the buffer
	// values are all stored in a right hand system, dx is left handed; invert z
	if( pvPos != NULL )
	{
		dsb3d->lpVtbl->SetPosition( dsb3d, *(pvPos)[X_AXIS], 
									 *(pvPos)[Y_AXIS], 
								      -(*(pvPos)[Z_AXIS]), DS3D_DEFERRED );
	}
	if( pvVel != NULL )
	{
		dsb3d->lpVtbl->SetVelocity( dsb3d, *(pvVel)[X_AXIS], 
									*(pvVel)[Y_AXIS], 
									-(*(pvVel)[Z_AXIS]), DS3D_DEFERRED );
	}
	dsb3d->lpVtbl->SetConeAngles( dsb3d, 360, 360, DS3D_DEFERRED );
	dsb3d->lpVtbl->SetConeOutsideVolume( dsb3d, DSBVOLUME_MAX, DS3D_DEFERRED ); 
	dsb3d->lpVtbl->SetMinDistance( dsb3d, 7.0f, DS3D_DEFERRED );
//	dsb3d->lpVtbl->SetMaxDistance( dsb3d, 100.0f, DS3D_DEFERRED );
	pListener->lpVtbl->CommitDeferredSettings( pListener );

	// set states
	channels[iChIndex].iBufferLen = (DWORD)(var_GetFloat( vlSfxPreload ) * 1000.0f); // sec->millisec
	channels[iChIndex].iLastFillTime = state.frame_time;
	channels[iChIndex].iNextFillTime = channels[iChIndex].iLastFillTime + channels[iChIndex].iBufferLen - BUFFER_POLL_GRANULARITY;
	channels[iChIndex].iStartTime = state.frame_time;

	if( bLooping )
	{
		channels[iChIndex].bLooping = 1;
		channels[iChIndex].iStopTime = 0xFFFFFFFF;
	}
	else
	{
		channels[iChIndex].bLooping = 0;
		channels[iChIndex].iStopTime = channels[iChIndex].iStartTime + (uint32_t)(((double)sound->iDataBlocks / (double)sound->iDataBlocks) * 1000.0);
	}

	channels[iChIndex].bPlaying = 1;
	channels[iChIndex].snd = sound;
	channels[iChIndex].stream = NULL;
	channels[iChIndex].dsBuffer = dsb8;
	channels[iChIndex].ds3DBuffer = dsb3d;
	channels[iChIndex].pvLoc = pvPos;
	channels[iChIndex].pvVel = pvVel;

	return iChIndex;
}

/* ------------
s_ds_StopSound
// this is called to stop any 3d or body sound
------------ */
void s_ds_StopSound( sndhandle handle )
{
	snd_channel_t *ch;

	// check to avoid shooting ourselves in the foot
	if( handle < 0 || handle >= iNumChannels ) return;
	ch = &channels[handle];
	if( !ch->bPlaying ) return;

	// stop the sound
	ch->dsBuffer->lpVtbl->Stop( ch->dsBuffer );

	// close out dsb's
	if( ch->ds3DBuffer != NULL ) ch->ds3DBuffer->lpVtbl->Release( ch->ds3DBuffer );
	if( ch->dsBuffer != NULL ) ch->dsBuffer->lpVtbl->Release( ch->dsBuffer );
	ch->ds3DBuffer = NULL;
	ch->dsBuffer = NULL;

	// unassociate this channel with anything to do with the sound just played
	ch->bPlaying = 0;
	ch->snd = NULL;
	ch->stream = NULL; // jic
	ch->iBufferLen = 0;
	ch->iBufferSize = 0;
	ch->iFillPos = 0;
	ch->iLastFillTime = 0;
	ch->iNextFillTime = 0xFFFFFFFF;
	ch->pvLoc = NULL;
	ch->pvVel = NULL;
	ch->iStartTime = 0;
	ch->iStopTime = 0;
	ch->iSndPos = 0;
}

uint8_t s_ds_PlayMusic( audioStream_t *stream )
{
	return 0;
}

void s_ds_StopMusic( audioStream_t *stream )
{
}


/* ------------
s_ds_CheckStreams 
// iterate all of the currently playing streams, copying new content to their buffers as necessary
------------ */
void s_ds_CheckStreams(void)
{
	int32_t a;
	snd_channel_t *ch;

	if( !bSndInited ) return;

	// iterate all channels
	for( a = 0 ; a < iNumChannels ; a++ )
	{
		// check for a playing channel that needs filling
		ch = &channels[a]; 
		if( ch->bPlaying && state.frame_time > ch->iNextFillTime )
		{		
			// fill the buffer
			s_ds_FillBuffer( &channels[a], a );
		}
	}
}

/* ------------
s_ds_FillBuffer
// updates the volume on the primary buffer
------------ */
void s_ds_UpdateVolume()
{
	float fGlobal;
	float fSfx;
	float fMusic;
	int32_t iGlobal;
	int32_t iSfx;
	int32_t iMusic;
	int32_t a;

	// check some stuff for sanity
	if( !bSndInited ) return;
	if( vlGlobalVolume == NULL ) return;
	if( vlSfxVolume == NULL ) return;
	if( vlMusicVolume == NULL ) return;

	// get values
	fGlobal = var_GetFloat( vlGlobalVolume );
	fSfx = var_GetFloat( vlSfxVolume );
	fMusic = var_GetFloat( vlMusicVolume );

	// range limit
	if( fGlobal > 100.0f )
	{
		var_UpdateFloat( vlGlobalVolume, 100.0f );
		fGlobal = 100.0f;
	}
	if( fGlobal < 0.0f )
	{
		var_UpdateFloat( vlGlobalVolume, 0.0f );
		fGlobal = 0.0f;
	}
	if( fSfx > 100.0f )
	{
		var_UpdateFloat( vlSfxVolume, 100.0f );
		fSfx = 100.0f;
	}
	if( fSfx < 0.0f )
	{
		var_UpdateFloat( vlSfxVolume, 0.0f );
		fSfx = 0.0f;
	}
	if( fMusic > 100.0f )
	{
		var_UpdateFloat( vlMusicVolume, 100.0f );
		fMusic = 100.0f;
	}
	if( fMusic < 0.0f )
	{
		var_UpdateFloat( vlMusicVolume, 0.0f );
		fMusic = 0.0f;
	}

	// scale the inputs from [0,100] on R to [-5000,0] on Z or -10000 if 0
	iGlobal = (int32_t)((fGlobal * 50.0f) - 5000.0f);
	iSfx = (int32_t)((fSfx * 50.0f) - 5000.0f);
	iMusic = (int32_t)((fMusic * 50.0f) - 5000.0f);
	if( fGlobal == 0.0f ) iGlobal = -10000;
	if( fSfx == 0.0f ) iSfx = -10000;
	if( fMusic == 0.0f ) iMusic = -10000;

//	con_Print( "Global:  %d, Sfx:  %d,  Music:  %d", iGlobal, iSfx, iMusic );

	// set volume for all sub-channels
	for( a = 0 ; a < iNumChannels ; a++ )
	{
		if( channels[a].bPlaying )
		{
			if( channels[a].snd != NULL && channels[a].dsBuffer != NULL )
			{
				channels[a].dsBuffer->lpVtbl->SetVolume( channels[a].dsBuffer, iSfx );
			}
			else if( channels[a].stream != NULL && channels[a].dsBuffer != NULL )
			{
				channels[a].dsBuffer->lpVtbl->SetVolume( channels[a].dsBuffer, iMusic );
			}
		}
	}
}


/* ------------
s_ds_FillBuffer
// streams data to a channel
------------ */
static void s_ds_FillBuffer( snd_channel_t *ch, sndhandle handle )
{
	uint32_t iCurPlay, iCurWrite;
	int32_t iAmtToWrite;
	int32_t iTmp;
	uint8_t *pData1, *pData2;
	uint32_t iSize1, iSize2;
	HRESULT hr;

	// poll the buffer to find out if we're ready for more info
	ch->dsBuffer->lpVtbl->GetCurrentPosition( ch->dsBuffer, (LPDWORD)&iCurPlay, (LPDWORD)&iCurWrite );

	// check to see if we are finished filling this buffer
	if( ch->iSndPos >= ch->snd->iDataSize ) 
	{
		if( !ch->bLooping )
		{
			// check to see if the sound has finished playing
			if( state.frame_time > ch->iStopTime )
			{
				s_ds_StopSound( handle );
				return;
			}
			ch->iNextFillTime = ch->iStopTime;
			return;
		}
		else
		{
			// we need to setup to loop the sound
			ch->iSndPos = 0;
		}
	}
	
	// if we overplay ourselves too much, we won't be able to tell where we are
	// however, if we are only a little behind, we can tell and move the write forward to compensate
	if( ch->iFillPos > iCurPlay && ch->iFillPos < iCurWrite ) 
	{
		con_Print( "<RED>Sound Warning:  Check Streams lagged behind sound buffer %d.", handle );
		con_Print( "<RED>\tIncrease the buffer size or improve the framerate to prevent" );
		con_Print( "<RED>\tthe sound from skipping." );
		ch->iFillPos = iCurWrite;
	}
	
	// compute the maximum amount of data we can write to the buffer now
	iAmtToWrite = ch->iBufferSize - (ch->iFillPos - iCurPlay); // invert the distance between them
	// if we have circled, the overlap will be a multiple of the length
	while( iAmtToWrite > (int32_t)ch->iBufferSize ) iAmtToWrite -= ch->iBufferSize; 
	if( iAmtToWrite <= 0 ) return; // lost, polled too quick or broken
	
	// the amount we can copy might be more than we need to copy
	iTmp = ch->snd->iDataSize - ch->iSndPos; // the data remaining
	if( iAmtToWrite > iTmp ) iAmtToWrite = iTmp; // the rest of the buffer can be written 

	// lock the entirity of the played part of the buffer up to what we need
	hr = ch->dsBuffer->lpVtbl->Lock( ch->dsBuffer, ch->iFillPos, iAmtToWrite, &pData1, (LPDWORD)&iSize1, &pData2, (LPDWORD)&iSize2, 0 );
	if( hr == DSERR_BUFFERLOST )
	{
		ch->dsBuffer->lpVtbl->Restore( ch->dsBuffer );
		hr = ch->dsBuffer->lpVtbl->Lock( ch->dsBuffer, ch->iFillPos, iAmtToWrite, &pData1, (LPDWORD)&iSize1, &pData2, (LPDWORD)&iSize2, 0 );
	}
	if( hr == DSERR_BUFFERLOST ) // can't reclaim our space yet, hold off a bit
	{
		ch->dsBuffer->lpVtbl->Restore( ch->dsBuffer );
		ch->iNextFillTime = state.frame_time + BUFFER_POLL_GRANULARITY - 10;
		return;
	}

	// final test for a lock error
	if( hr != DS_OK )
	{		
		con_Print( "<RED>Direct Sound Warning: Cannot stream to audio device:  Could not lock buffer." );

		if( hr == DSERR_BUFFERLOST )
		{
			con_Print( "\tDSERR_BUFFERLOST" );
			ch->dsBuffer->lpVtbl->Restore( ch->dsBuffer );
			ch->iNextFillTime = state.frame_time + BUFFER_POLL_GRANULARITY - 10;
			return;
		}
		else if( hr == DSERR_INVALIDCALL )
		{
			con_Print( "\tDSERR_INVALIDCALL" );
			s_ds_StopSound( handle );
			return;
		}
		else if( hr == DSERR_INVALIDPARAM )
		{
			con_Print( "\tDSERR_INVALIDPARAM:  ch->iFillPos: %d, iAmtToWrite: %d", ch->iFillPos, iAmtToWrite );
			s_ds_StopSound( handle );
			return;
		}
		else if( hr == DSERR_PRIOLEVELNEEDED )
		{
			con_Print( "\tDSERR_PRIOLEVELNEEDED" );
			s_ds_StopSound( handle );
			return;
		}
		
		return;
	}

	// no guarantee we'll be given what we ask for
	// but we'll never have more than we need
	if( iAmtToWrite > (int32_t)(iSize1 + iSize2) ) iAmtToWrite = iSize1 + iSize2;

	// write into the first position
	memcpy( pData1, &(ch->snd->bData[ch->iSndPos]), iSize1 );
	iAmtToWrite -= iSize1;
	ch->iSndPos += iSize1;

	// do the second copy if needed
	if( pData2 != NULL )
	{
		memcpy( pData2, &(ch->snd->bData[ch->iSndPos]), iSize2 );
		iAmtToWrite -= iSize2;
		ch->iSndPos += iSize2;
	}

	// unlock the buffer
	hr = ch->dsBuffer->lpVtbl->Unlock( ch->dsBuffer, pData1, iTmp, pData2, 0 );

	// update the sound to reflect the update
	ch->iLastFillTime = state.frame_time;
	ch->iNextFillTime = (uint32_t)((double)(iSize1 + iSize2) / (double)ch->snd->wf.nAvgBytesPerSec * 1000.0 - BUFFER_POLL_GRANULARITY + ch->iLastFillTime);
	ch->iFillPos += iSize1 + iSize2;
	while( ch->iFillPos >= ch->iBufferSize ) ch->iFillPos -= ch->iBufferSize;
}
