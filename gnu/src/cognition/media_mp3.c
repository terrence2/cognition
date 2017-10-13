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
// .c
// Created  by Terrence Cole

// Includes
/////////////
#include "cog_global.h"

// Definitions
////////////////

// Local Structures
/////////////////////

// Global Prototypes
//////////////////////
/*
int mp3_Initialize(void);
void mp3_Terminate(void);
audioStream_t *mp3_GetStream( char *name );
void mp3_ReleaseStream( audioStream_t *stream );
byte *mp3_GetAudioSample( int amt );
*/

// Local Prototypes
/////////////////////

// Local Variables
////////////////////

// *********** FUNCTIONALITY ***********
/* ------------
mp3_Initialize - prepares the mp3 streamer to decode audio data
------------ */
int mp3_Initialize(void)
{
	return 1;
}

/* ------------
mp3_Terminate - frees all memory associated with the stream decoder and the currently decoding stream
------------ */
void mp3_Terminate(void)
{
}

/* ------------
mp3_GetStream - opens the stream and prepares to read audio data
------------ */
audioStream_t *mp3_GetStream( char *name )
{
	return NULL;
}

/* ------------
mp3_ReleaseStream - frees all memory associated with the audio stream
------------ */
void mp3_ReleaseStream( audioStream_t *stream )
{
}

/* ------------
mp3_GetAudioSample - returns amt milliseconds of PCM audio samples
------------ */
byte *mp3_GetAudioSample( int amt )
{
	return NULL;
}
