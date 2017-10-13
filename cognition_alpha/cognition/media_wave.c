// Cognition
// media_wave.c
// Created by Terrence Cole

// Includes
/////////////
#include "cog_global.h"

// Definitions
////////////////
#define SOUND_DIRECTORY "sounds"

// Local Structures
/////////////////////

// Global Prototypes
//////////////////////
/*
uint8_t *wav_Load( char * name, WAVEFORMATEX *wf, uint32_t *iDataSize, uint32_t *iDataBlocks );
*/

// Local Prototypes
/////////////////////
static uint8_t wav_FindChunk( char *cID, uint32_t travel, file_t *fp );

// Local Variables
////////////////////

// *********** FUNCTIONALITY ***********
/* ------------
wav_LoadFromFile
// loads the path into wave/sound structures from RIFF; returns a pointer to sound data or NULL on failure
// sets wf to sound parameters, iDataSize to the length of the returned buffer in bytes and iDataBlocks
// to the number of blocks in the buffer according to wf
------------ */
uint8_t *wav_Load( char *name, WAVEFORMATEX *wf, uint32_t *iDataSize, uint32_t *iDataBlocks )
{
#ifdef WORDS_BIGENDIAN
	uint16_t sTmp;
#endif
	uint32_t iTmp;
	char ID[4];
	char *path;
	file_t *fp;
	uint8_t *bData;

	// assume name is a complete name
	path = fs_GetMediaPath( name, SOUND_DIRECTORY );
	if( path == NULL )
	{
		con_Print( "<RED>Wave Load Failed:  Could not get full path." );
		return NULL;
	}

	con_Print( "<BLUE>Loading</BLUE> wave <ORANGE>\"%s\"</ORANGE>.", path );
	eng_LoadingFrame();

	// open and continue
	fp = fs_open( path, "r" );
	if( fp == NULL )
	{
		con_Print( "<RED>Wave Load Failed:  Could not open file." );
		return NULL;
	}

	// In theory i should create a chunk descend/ascend to handle RIFF properly.
	// but i don't really care to waste the cycles on the super-simple stuff i'm doing.

	// find the RIFF tag somewhere a sane distance from the start of the file
	ID[0] = 'R'; ID[1] = 'I'; ID[2] = 'F'; ID[3] = 'F';
	if( !wav_FindChunk( ID, 20, fp ) )
	{
		con_Print( "<RED>Wave Load Failed:	This is not a RIFF wav file." );
		fs_close(fp);
		return NULL;
	}

	// find the WAVE tag
	ID[0] = 'W'; ID[1] = 'A'; ID[2] = 'V'; ID[3] = 'E';
	if( !wav_FindChunk( ID, 20, fp ) )
	{
		con_Print( "<RED>Wave Load Failed:	Could not find a WAVE section." );
		fs_close(fp);
		return NULL;
	}

	// find the fmt tag
	ID[0] = 'f'; ID[1] = 'm'; ID[2] = 't'; ID[3] = ' ';
	if( !wav_FindChunk( ID, 20, fp ) )
	{
		con_Print( "<RED>Wave Load Failed:	Could not find a fmt section." );
		fs_close(fp);
		return NULL;
	}

	// get the chunk length; should be 16 bytes
	fs_read( &iTmp, sizeof(uint32_t), 1, fp );
#ifdef WORDS_BIGENDIAN
	iTmp = tcReorder32(iTmp);	
#endif
	if( iTmp != 16 )
	{
		con_Print( "<RED>Wave Load Failed:	The format tag is %d bytes, not 16 as expected", iTmp );
		fs_close( fp );
		return NULL;
	}

	// read in some relevant values to ensure this is a readable wave
	fs_read( &wf->wFormatTag, sizeof(uint16_t), 1, fp );
#ifdef WORDS_BIGENDIAN
	wf->wFormatTag = tcReorder16(sTmp);
#endif
	if( wf->wFormatTag != 1 ) // PCM format wave
	{
		con_Print( "<RED>Wave Load Failed:  This is not a PCM type wav." );
		fs_close( fp );
		return NULL;
	}

	// read in the normal header stuff
#ifdef WORDS_BIGENDIAN 
	fs_read( &sTmp, sizeof(uint16_t), 1, fp );
	wf->nChannels = tcReorder16(sTmp);
	fs_read( &iTmp, sizeof(uint32_t), 1, fp );
	wf->nSamplesPerSec = tcReorder32(iTmp);
	fs_read( &iTmp, sizeof(uint32_t), 1, fp );
	wf->nAvgBytesPerSec = tcReorder32(iTmp);
	fs_read( &sTmp, sizeof(uint16_t), 1, fp );
	wf->nBlockAlign = tcReorder16(sTmp);
	fs_read( &sTmp, sizeof(uint16_t), 1, fp );
	wf->wBitsPerSample = tcReorder16(sTmp);
#else  // LITTLE_ENDIAN
	fs_read( &wf->nChannels, sizeof(uint16_t), 1, fp );
	fs_read( &wf->nSamplesPerSec, sizeof(uint32_t), 1, fp );
	fs_read( &wf->nAvgBytesPerSec, sizeof(uint32_t), 1, fp );
	fs_read( &wf->nBlockAlign, sizeof(uint16_t), 1, fp );
	fs_read( &wf->wBitsPerSample, sizeof(uint16_t), 1, fp );
#endif

	// we only want to handle "normal" bits per sample; shouldn't be an issue with most encoders
	if( wf->wBitsPerSample != 8 && wf->wBitsPerSample != 16 )
	{
		con_Print( "<RED>Wave Load Failed:  %d bits per sample is not supported.", wf->wBitsPerSample );
		fs_close( fp );
		return NULL;
	}

	// find the wave data chunk
	ID[0] = 'd'; ID[1] = 'a'; ID[2] = 't'; ID[3] = 'a';
	if( !wav_FindChunk( ID, 20, fp ) )
	{
		con_Print( "<RED>Wave Load Failed:	Could not find a data section." );
		fs_close( fp );
		return NULL;
	}

	// read the data length
#ifdef WORDS_BIGENDIAN
	fs_read( &iTmp, sizeof(uint32_t), 1, fp );
	*iDataSize = tcReorder32(iTmp);
#else // LITTLE ENDIAN
	fs_read( iDataSize, sizeof(uint32_t), 1, fp );
#endif

	// compute the number of blocks of audio data in the stream
	*iDataBlocks = (*iDataSize / (wf->wBitsPerSample / 8)) / wf->nChannels;

	// allocate a holder for the data
	bData = (uint8_t*)mem_alloc( *iDataSize );
	if( bData == NULL )
	{
		con_Print( "<RED>Wave Load Failed:  memory allocation failed for sound data:  %d bytes", *iDataSize );
		fs_close( fp );
		return NULL;
	}
	memset( bData, 0, *iDataSize );

#ifndef WORDS_BIGENDIAN  // LITTLE ENDIAN 
	// we can do this directly no matter the size
	iTmp = fs_read( bData, sizeof(byte), *iDataSize, fp );
	if( iTmp != *iDataSize )
	{
		con_Print( "<RED>Wave Load Failed:  file read failed for sound data:  got %d of %d bytes", iTmp, *iDataSize );
		SAFE_RELEASE( bData );
		fs_close( fp );
		return NULL;
	}
#else // BIG_ENDIAN
	// individual bytes we can always do directly
	if( wf->wBitsPerSample == 8 )
	{
		iTmp = fs_read( bData, sizeof(byte), *iDataSize, fp );
		if( iTmp != *iDataSize )
		{
			con_Print( "<RED>Wave Load Failed:  file read failed for sound data:  got %d of %d bytes", iTmp, *iDataSize );
			SAFE_RELEASE( bData );
			fs_close( fp );
			return NULL;
		}
	}
	else
	{
		uint32_t a, b;
		uint16_t sSamp;
		uint32_t iSamp;

		switch( wf->wBitsPerSample )
		{
		case 16:
			for( a = 0 ; a < *iDataBlocks ; a++ )
			{
				for( b = 0 ; b < wf->wChannels ; b++ )
				{
					// read one part of a sample
					if( fs_read( &sSamp, sizeof(uint16_t), 1, fp ) != 1 )
					{
						con_Print( "<RED>Wave Load Failed:  file read failed for sound data:  fs_read not 1 in 16 bit sample read" );
						SAFE_RELEASE( bData );
						fs_close( fp );
						return NULL;
					}

					// I haven't tested this;  I know it's slow, but it *should* work
					sTmp = tcReorder16(sSamp);
					((int16_t*)(bData))[a][b] = (int16_t)sTmp;
				}
			}
			break;
		case 32:
			for( a = 0 ; a < *iDataBlocks ; a++ )
			{
				for( b = 0 ; b < wf->wChannels ; b++ )
				{
					if( fs_read( &iSamp, sizeof(uint32_t), 1, fp ) != 1 )
					{
						con_Print( "<RED>Wave Load Failed:  file read failed for sound data:  fs_read not 1 in 32 bit sample read" );
						SAFE_RELEASE( bData );
						fs_close( fp );
						return NULL;
					}

					iTmp = tcReorder32(iSamp);
					((int32_t*)(bData))[a][b] = (int32_t)iTmp;
				}
			}
			break;
		default:
			con_Print( "<RED>Wave Load Failed:  file read failed for sound data:  wBitsPerSample is Not a multiple of 8:  %d", wf->wBitsPerSample );
			SAFE_RELEASE( bData );
			fs_close( fp );
			return NULL;
		}
	}
#endif

#ifdef VERBOSE
	// print out stats
	con_Print( "\tDepth:  %d", wf->wBitsPerSample );
	con_Print( "\tChannels:  %d", wf->nChannels );
	con_Print( "\tRate:  %d", wf->nSamplesPerSec );
#endif // VERBOSE

	// close the file and return the data
	fs_close(fp);
	return bData;
}

/* ------------
wav_FindChunk
// searches down into fp for a FOURCC value equal to ID
// searches a maximum of travel bytes into the file
------------ */
static uint8_t wav_FindChunk( char *cID, uint32_t travel, file_t *fp )
{
	uint32_t a;
	char ID[4];
	
	memset( ID, 0, sizeof(char) * 4 );
	fs_read( ID, sizeof(char), 4, fp );
	a = 0;
	while( ID[0] != cID[0] && ID[1] != cID[1] && ID[2] != cID[2] && ID[3] != cID[3] && a < travel + 1 )
	{
		ID[0] = ID[1];
		ID[1] = ID[2];
		ID[2] = ID[3];
		fs_read( &ID[3], sizeof(char), 1, fp );
		a++;
	}

	// check to see if this is really a wav
	if( a == travel + 1 )
	{
		return 0;
	}

	return 1;
}


