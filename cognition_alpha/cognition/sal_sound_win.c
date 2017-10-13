// Cognition
// sal_sound_win.c
// Created 1/10/03 by Terrence Cole

// this file controls the loaded sounds and also acts as a gateway to the current sound provider

// Includes
/////////////
#include "cog_global.h"
#include "sal_sound_win_ds.h"

// Definitions
////////////////
#define SND_MODE_NONE 0
#define SND_MODE_DS 1

// Local Structures
/////////////////////

// Global Prototypes
//////////////////////
/*
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
*/

// Local Prototypes
/////////////////////
static sound_t *s_FindSound( char *name );
static void snd_LinkTail( sound_t *snd );
static void snd_Unlink( sound_t *snd );

// Local Variables
////////////////////
static varlatch vlMode = NULL;
static int iMode = SND_MODE_NONE;
static uint8_t bSndAvail = 0;
static sound_t *sndHead = NULL;
static sound_t *sndTail = NULL;

// *********** FUNCTIONALITY ***********
/* ------------
s_Initialize - starts the sound provider
------------ */
int s_Initialize(void)
{
	// latch the mode
	vlMode = var_GetVarLatch( "snd_mode" );

	// pedantic checks
	if( bSndAvail ) return 1;

	// select a method to init
	if( tcstricmp( var_GetString(vlMode), "auto" ) )
	{
		iMode = SND_MODE_DS;
		s_ds_Initialize();
	}
	else if( tcstricmp( var_GetString(vlMode), "dsound" ) )
	{
		iMode = SND_MODE_DS;
		s_ds_Initialize();
	}
	else if( tcstricmp( var_GetString(vlMode), "none" ) )
	{
		iMode = SND_MODE_NONE;
		bSndAvail = 0;
		return 1;
	}
	else if( tcstricmp( var_GetString(vlMode), "off" ) )
	{
		iMode = SND_MODE_NONE;
		bSndAvail = 0;
		return 1;
	}
	else if( tcstricmp( var_GetString(vlMode), "0" ) )
	{
		iMode = SND_MODE_NONE;
		bSndAvail = 0;
		return 1;
	}
	else
	{
		con_Print( "<RED>Warning!</RED>  Variable snd_mode is not a recognized sound type string." );
		con_Print( "<RED>\tSound Not Initialized!" );
		iMode = SND_MODE_NONE;
		bSndAvail = 0;
		return 1;
	}

	bSndAvail = 1;
	return 1;
}

/* ------------
s_Terminate - closes the sound provider
------------ */
void s_Terminate(void)
{
	sound_t *sndTmp;

	if( !bSndAvail ) return;

	// stop all sounds
	s_StopAllSound();

	// free all sounds
	while( sndHead != NULL )
	{
		sndTmp = sndHead->next;

		SAFE_RELEASE( sndHead->bData );
		SAFE_RELEASE( sndHead );
		
		sndHead = sndTmp;
	}
	sndHead = sndTail = NULL;

	// mode select
	switch( iMode )
	{
	case SND_MODE_NONE:
		bSndAvail = 0;
		return;
	case SND_MODE_DS:
		s_ds_Terminate();
		bSndAvail = 0;
		iMode = SND_MODE_NONE;
		return;
	default:
		con_Print( "<RED>Sound System Warning:  Mode is not properly set." );
		return;
	}
}

/* ------------
s_Restart - closes and then starts the sound provider, remixes open samples, stores and restarts play positions
------------ */
void s_Restart(void)
{
	// FIXME:  write this
}


/* ------------
s_Load
// loads a sound for 3d playback
// name is a non-extended name in the sounds directory; eg pass "foo" to load "foo.wav"
------------ */
sound_t *s_Load( char *name )
{
	sound_t *newSnd = NULL;
	uint32_t iTmp;

	// soft exits
	if( !bSndAvail ) return NULL;

	// assert
	assert( name != NULL );
	if( name == NULL )
	{
		con_Print( "<RED>Sound Load Error:  name is NULL" );
		return NULL;
	}
	iTmp = tcstrlen(name);
	assert( iTmp < 1019 );
	assert( iTmp > 0 );
	if( iTmp >= 1019 )
	{
		con_Print( "<RED>Sound Load Error:  name is too long" );
		return NULL;
	}
	if( iTmp == 0 )
	{
		con_Print( "<RED>Sound Load Error:  name is a 0 length string." );
		return NULL;
	}

	// look for an already present sound
	newSnd = s_FindSound( name );
	if( newSnd != NULL ) return newSnd;

	// get memory for the sound
	newSnd = (sound_t *)mem_alloc( sizeof(sound_t) );
	if( newSnd == NULL )
	{
		con_Print( "<RED>Sound Load Failed:  memory allocation failed for a sound structure." );
		return NULL;
	}
	memset( newSnd, 0, sizeof(sound_t) );

	// extend the name
	tcstrcpy( newSnd->cName, name );
	tcstrcat( newSnd->cName, ".wav\0" );

	// load the sound
	newSnd->bData = wav_Load( newSnd->cName, &(newSnd->wf), &(newSnd->iDataSize), &(newSnd->iDataBlocks) );
	if( newSnd->bData == NULL )
	{
		con_Print( "<RED>Sound Load Failed:  wave file load failed." );
		SAFE_RELEASE( newSnd );
		return NULL;
	}

	// link it in
	snd_LinkTail( newSnd );

	return newSnd;
}

/* ------------
s_Unload
// release memory for the sound and unrecord it
------------ */
void s_Unload( sound_t *snd )
{
	// asserts
	assert( snd != NULL );
	assert( tcstrlen(snd->cName) > 0 );
	if( snd == NULL )
	{
		con_Print( "<RED>Sound Unload Failed:  snd is NULL" );
		return;
	}
	if( tcstrlen(snd->cName) == 0 )
	{
		con_Print( "<RED>Sound Unload Failed:  snd's name is not present." );
		return;
	}

	// try to find the sound in our data to make sure it exists
	if( s_FindSound( snd->cName ) == NULL )
	{
		con_Print( "<RED>Sound Unload Failed:  could not find a sound named \"%s\" in our list.", snd->cName );
		return;
	}

	// unlink
	snd_Unlink( snd );

	// do the unload
	SAFE_RELEASE( snd->bData );
	SAFE_RELEASE( snd );
}

/* ------------
s_FindFirstSound 
// returns the first instance of snd named with name in the loaded sound list; null if not present 
------------ */
static sound_t *s_FindSound( char *name )
{
	sound_t *sndTmp = sndHead;

	while( sndTmp != NULL )
	{
		if( tcstrcmp( sndTmp->cName, name ) ) return sndTmp; 
		sndTmp = sndTmp->next;
	}
	
	return NULL;
}

/* ------------
s_StopAllSound - ends all playing sound buffers immediatly
------------ */
void s_StopAllSound(void)
{
	if( bSndAvail ) s_ds_StopAllSound();
}

/* ------------
s_SpacializeSounds - localsizes 3D sounds to the current environment
------------ */
void s_SpacializeSounds(void)
{
	if( bSndAvail ) s_ds_SpacializeSounds();
}

/* ------------
s_PlaySound - plays the world sound 'sound' in a 3D buffer
------------ */
sndhandle s_PlaySound( sound_t *sound, uint8_t bLooping, vec3 *vpLoc, vec3 *vpVel )
{
	if( !bSndAvail ) return -1;
	if( sound == NULL ) return -1;

	// choose the play method based on the number of channels
	if( sound->wf.nChannels == 1 )
		return s_ds_PlaySound3D( sound, bLooping, vpLoc, vpVel );
	else if( sound->wf.nChannels == 2 )
		return s_ds_PlaySoundBody( sound, bLooping );

	// return null handle
	return -1;
}

/* ------------
s_StopSound - stops 'sound' if it is playing and exits otherwise
------------ */
void s_StopSound( sndhandle handle )
{
	if( bSndAvail ) s_ds_StopSound( handle );
}


/* ------------
s_PlayMusic - starts play of the streaming audio in 'stream'; returns > 0 if successful, or 0 if the operation cannot proceed
------------ */
uint8_t s_PlayMusic( audioStream_t *stream )
{
	if( !bSndAvail ) return 0;
	return s_ds_PlayMusic( stream );
}

void s_StopMusic( audioStream_t *stream )
{
	if( bSndAvail ) s_ds_StopMusic( stream );
}

/* ------------
s_CheckStreams - looks at all currently playing streams and polls the buffer position
// if the poll time has arrived.  Fills streams as needed. 
------------ */
void s_CheckStreams(void)
{
	if( bSndAvail ) s_ds_CheckStreams();
}

/* ------------
s_UpdateVolume
// called after a volume change to tell the device to amp the input waveform
------------ */
void s_UpdateVolume(void)
{
	if( bSndAvail ) s_ds_UpdateVolume();
}

/* ------------
snd_LinkTail
------------ */
static void snd_LinkTail( sound_t *snd )
{
	if( sndHead == NULL ) sndHead = snd;

	if( sndTail == NULL )
	{
		sndTail = snd;
		snd->next = NULL;
		snd->prev = NULL;
		return;
	}

	snd->prev = sndTail;
	snd->next = NULL;

	sndTail->next = snd;
	sndTail = snd;
}

/* ------------
snd_Unlink
------------ */
static void snd_Unlink( sound_t *snd )
{
	if( snd->prev != NULL ) snd->prev->next = snd->next;
	if( snd->next != NULL ) snd->next->prev = snd->prev;

	if( sndTail == snd ) sndTail = snd->prev;
	if( sndHead == snd ) sndHead = snd->next;

	snd->prev = NULL;
	snd->next = NULL;
}


