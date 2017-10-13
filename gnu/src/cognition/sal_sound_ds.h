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

#ifndef WIN32
int s_ds_Initialize(void) 
{
    con_Print( "Sound Init Error: DirectSound mode set, but not in Windows!" );
    return 0;
}

void s_ds_Terminate(void) 
{
    con_Print( "Sound Term Error: DirectSound mode set, but not in Windows!" );
}

void s_ds_Restart(void)
{
    con_Print( "Sound Restart Error: DirectSound mode set, but not in Windows!" );
}

void s_ds_StopAllSound(void)
{
    con_Print( "Sound Stop All Error: DirectSound mode set, but not in Windows!" );
}

void s_ds_SpacializeSounds(void)
{
    con_Print( "Sound Spacialize Error: DirectSound mode set, but not in Windows!" );
}

sndhandle s_ds_PlaySound3D( sound_t *sound, uint8_t bLooping, vec3 *pvPos, vec3 *pvVel )
{
    con_Print( "Sound Play3D Error: DirectSound mode set, but not in Windows!" );
    return -1;
}

sndhandle s_ds_PlaySoundBody( sound_t *sound, uint8_t bLooping )
{
    con_Print( "Sound Play Error: DirectSound mode set, but not in Windows!" );
    return -1;
}

void s_ds_StopSound( sndhandle handle )
{
    con_Print( "Sound Stop Error: DirectSound mode set, but not in Windows!" );
}

uint8_t s_ds_PlayMusic( audioStream_t *stream )
{
    con_Print( "Music Play Error: DirectSound mode set, but not in Windows!" );
    return 0;
}

void s_ds_StopMusic( audioStream_t *stream )
{
    con_Print( "Music Stop Error: DirectSound mode set, but not in Windows!" );
}

void s_ds_CheckStreams(void)
{
    con_Print( "Sound Check Error: DirectSound mode set, but not in Windows!" );
}

void s_ds_UpdateVolume()
{
    con_Print( "Sound Update Error: DirectSound mode set, but not in Windows!" );
}
#endif

#endif // _SAL_SOUND_WIN_DS_H
