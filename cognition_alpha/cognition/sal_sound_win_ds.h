// Cognition
// sal_sound_win_ds.h
// Created 6/29/03 by Terrence Cole

#ifndef _SAL_SOUND_WIN_DS_H
#define SAL_SOUND_WIN_DS_H

// Includes
/////////////
#include "cog_global.h"

// Prototypes
////////////////
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

#endif // _SAL_SOUND_WIN_DS_H

