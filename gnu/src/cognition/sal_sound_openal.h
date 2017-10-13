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
// sal_sound_openal.h
// Created 7/24/05 by Terrence Cole

#ifndef _SAL_SOUND_OPENAL_H
#define _SAL_SOUND_OPENAL_H

// Includes
/////////////
#include "cog_global.h"

// Prototypes
////////////////
int s_openal_Initialize(void);
void s_openal_Terminate(void);

void s_openal_Restart(void);
void s_openal_StopAllSound(void);
void s_openal_SpacializeSounds(void);

sndhandle s_openal_PlaySound3D( sound_t *sound, uint8_t bLooping, vec3 *pvPos, vec3 *pvVel );
sndhandle s_openal_PlaySoundBody( sound_t *sound, uint8_t bLooping );
void s_openal_StopSound( sndhandle handle );

uint8_t s_openal_PlayMusic( audioStream_t *stream );
void s_openal_StopMusic( audioStream_t *stream );

void s_openal_CheckStreams(void);
void s_openal_UpdateVolume();

#ifndef HAVE_OPENAL
int s_openal_Initialize(void) 
{
    con_Print( "Sound Init Error: OPENAL mode set, but not compiled with OPENAL!" );
    return 0;
}

void s_openal_Terminate(void) 
{
    con_Print( "Sound Term Error: OPENAL mode set, but not compiled with OPENAL!" );
}

void s_openal_Restart(void)
{
    con_Print( "Sound Restart Error: OPENAL mode set, but not compiled with OPENAL!" );
}

void s_openal_StopAllSound(void)
{
    con_Print( "Sound Stop All Error: OPENAL mode set, but not compiled with OPENAL!" );
}

void s_openal_SpacializeSounds(void)
{
    con_Print( "Sound Spacialize Error: OPENAL mode set, but not compiled with OPENAL!" );
}

sndhandle s_openal_PlaySound3D( sound_t *sound, uint8_t bLooping, vec3 *pvPos, vec3 *pvVel )
{
    con_Print( "Sound Play3D Error: OPENAL mode set, but not compiled with OPENAL!" );
    return -1;
}

sndhandle s_openal_PlaySoundBody( sound_t *sound, uint8_t bLooping )
{
    con_Print( "Sound Play Error: OPENAL mode set, but not compiled with OPENAL!" );
    return -1;
}

void s_openal_StopSound( sndhandle handle )
{
    con_Print( "Sound Stop Error: OPENAL mode set, but not compiled with OPENAL!" );
}

uint8_t s_openal_PlayMusic( audioStream_t *stream )
{
    con_Print( "Music Play Error: OPENAL mode set, but not compiled with OPENAL!" );
    return 0;
}

void s_openal_StopMusic( audioStream_t *stream )
{
    con_Print( "Music Stop Error: OPENAL mode set, but not compiled with OPENAL!" );
}

void s_openal_CheckStreams(void)
{
    con_Print( "Sound Check Error: OPENAL mode set, but not compiled with OPENAL!" );
}

void s_openal_UpdateVolume()
{
    con_Print( "Sound Update Error: OPENAL mode set, but not compiled with OPENAL!" );
}
#endif

#endif // _SAL_SOUND_OPENAL_H
