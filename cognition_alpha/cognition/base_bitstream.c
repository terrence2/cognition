// Cognition
// base_bitstream.c
// Created  by Terrence Cole

// Includes
/////////////
#include "cog_global.h"
#ifdef WIN32
#	include <winsock2.h>
#else
#endif // !WIN32

// Definitions
////////////////


// Local Structures
/////////////////////
/*
typedef struct bitstream_s
{
	file_t *fIn;
	byte bBuffer[64]; // store 64 static bytes

	// bit markers
	short bitsMax; // the maximum bits available in the buffer
	short bitsRemaining; // the number of bits currently in the buffer

	// floating position indicators
	byte *bytePtr; // buffer + [0,63] describing the positional offset in buffer (current position, next read)
	byte bitPtr;  // [0,7] describing positional offset in bitBuf (current position, next read)

	// states
	byte bIsEmpty;
	byte bHasLastByte;
	byte bInError;

	// list
	struct bitstream_s *next;
	struct bitstream_s *prev;
} bitstream_t;
*/

// Global Prototypes
//////////////////////
/*
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
*/


// Local Prototypes
/////////////////////
static void bs_FillBuffer( bitstream_t *bs );
static void bs_LinkTail( bitstream_t *bs );
static void bs_Unlink( bitstream_t *bs );

// Local Variables
////////////////////
static bitstream_t *bsHead = NULL;
static bitstream_t *bsTail = NULL;

unsigned long int bs_Get32Bits( bitstream_t *bs );
unsigned short int bs_Get16Bits( bitstream_t *bs );
byte bs_Get8Bits( bitstream_t *bs );
byte bs_Get1Bit( bitstream_t *bs );
unsigned long int bs_GetBits( bitstream_t *bs, byte numBits );

// *********** FUNCTIONALITY ***********
/* ------------
bs_Initialize
------------ */
int bs_Initialize(void)
{
	con_Print( "\n<BLUE>Initializing</BLUE> the bitstream tracker." );
	
	// assert
	if( bsHead != NULL )
	{
		con_Print( "<RED>Bitstream Init Assert Failed:  head is non NULL" );
		eng_Stop( "220001" );
		return 0;
	}
	if( bsTail != NULL )
	{
		con_Print( "<RED>Bitstream Init Assert Failed:  tail is non NULL" );
		eng_Stop( "220002" );
		return 0;
	}
	
	return 1;
}

/* ------------
bs_Terminate
------------ */
void bs_Terminate(void)
{
	int cnt = 0;
	bitstream_t *bsTmp;
	
	con_Print( "\n<RED>Terminating</RED> the bitstream system." );
	
	// systems remain to destroy
	if( bsHead != NULL || bsTail != NULL )
	{
		while( bsHead != NULL )
		{
			bsTmp = bsHead;
			bsHead = bsHead->next;
			bs_CloseStream( bsTmp );
			cnt++;
		}
	
		con_Print( "\tKilled %d particle systems.", cnt );
	}
}

/* ------------
bs_GetBitStream
------------ */
bitstream_t *bs_GetBitStream( file_t *fp )
{
	bitstream_t *bs;

	// assert
	if( fp == NULL )
	{
		con_Print( "<RED>Bitstream GetBitStream Assert Failed:  fp is NULL" );
		eng_Stop( "220003" );
		return 0;
	}

	// get memory for the stream
	bs = (bitstream_t*)mem_alloc( sizeof(bitstream_t) );
	if( bs == NULL )
	{
		con_Print( "<RED>GetBitStream Error: memory allocation failed for new bitstream." );
		return NULL;
	}
	memset( bs, 0, sizeof(bitstream_t) );

	// init to empty
	bs->fIn = fp;
	bs->bitPtr = 0;
	bs->bytePtr = bs->bBuffer + 64;
	bs->bitsMax = 64 * 8;
	bs->bitsRemaining = 0;
	bs->bInError = 0;
	bs->bIsEmpty = 0;
	bs_FillBuffer( bs );

	// link us in
	bs_LinkTail( bs );
	
	return bs;
}

/* ------------
bs_CloseStream
------------ */
void bs_CloseStream( bitstream_t *bs )
{
	if( bs == NULL ) return;

	// remove from the internal list
	bs_Unlink( bs );
	SAFE_RELEASE( bs );
}

/* ------------
bs_Get32Bits
------------ */
unsigned long int bs_Get32Bits( bitstream_t *bs )
{
	unsigned long int newLong = 0;

	// assert
	if( bs == NULL )
	{
		con_Print( "<RED>Bitstream Get32Bits Assert Failed: bs is NULL" );
		eng_Stop( "220004" );
		return 0;
	}

	// check remaining bits
	if( bs->bitsRemaining < 40 ) 
	{
		// get 8 bits would do this, but we prefer lumped reads
		bs_FillBuffer(bs);

		// check for errors
		if( bs->bInError ) return 0;

		// check overflow
		if( bs->bIsEmpty || bs->bitsRemaining < 32 ) 
		{
			// mark the error for overflow reads
			bs->bInError = 1;
			return 0;
		}
		
		// catch eof
		if( bs->bitsRemaining == 32 ) bs->bIsEmpty = 1; 
	}

	// we can accelerate byte aligned reads easily
	if( bs->bitPtr == 0 )
	{
		newLong = (unsigned long int)*bs->bytePtr << 24;
		bs->bytePtr++;
		newLong |= (unsigned long int)*bs->bytePtr << 16;
		bs->bytePtr++;
		newLong |= (unsigned long int)*bs->bytePtr << 8;
		bs->bytePtr++;
		newLong |= (unsigned long int)*bs->bytePtr;
		bs->bytePtr++;
		bs->bitsRemaining -= 32;
		return newLong;
	}
	
	// else give an abstract treatment with byte reads
	newLong = (unsigned long int)bs_Get16Bits(bs) << 16;
	newLong |= (unsigned long int)bs_Get16Bits(bs);

	return ntohl( newLong );
}

/* ------------
bs_Get16Bits
------------ */
unsigned short int bs_Get16Bits( bitstream_t *bs )
{
	unsigned short int newShort = 0;
	
	// assert
	if( bs == NULL )
	{
		con_Print( "<RED>Bitstream Get16Bits Assert Failed: bs is NULL" );
		eng_Stop( "220005" );
		return 0;
	}

	// check remaining bits
	if( bs->bitsRemaining < 24 ) 
	{
		// get 8 bits would do this, but we prefer lumped reads
		bs_FillBuffer(bs);

		// check for errors
		if( bs->bInError ) return 0;

		// check for overflow
		if( bs->bIsEmpty || bs->bitsRemaining < 16 ) 
		{
			// mark the error for overflow reads
			bs->bInError = 1;
			return 0;
		}
		
		// catch eof
		if( bs->bitsRemaining == 16 ) bs->bIsEmpty = 1; 
	}

	// we can accelerate byte aligned reads easily
	if( bs->bitPtr == 0 )
	{
		newShort = (unsigned short int)*bs->bytePtr << 8;
		bs->bytePtr++;
		newShort |= (unsigned short int)*bs->bytePtr;
		bs->bytePtr++;
		bs->bitsRemaining -= 16;
		return newShort;
	}

	// else give an abstract treatment with byte reads
	newShort = (unsigned short int)bs_Get8Bits(bs) << 8;
	newShort |= (unsigned short int)bs_Get8Bits(bs);

	return ntohs( newShort );
}

/* ------------
bs_Get8Bits
------------ */
byte bs_Get8Bits( bitstream_t *bs )
{
	byte newByte = 0;

	// assert
	if( bs == NULL )
	{
		con_Print( "<RED>Bitstream Get8Bits Assert Failed: bs is NULL" );
		eng_Stop( "220006" );
		return 0;
	}
	
	// check remaining bits
	if( bs->bitsRemaining < 16 ) 
	{
		bs_FillBuffer(bs);

		// check for errors
		if( bs->bInError ) return 0;

		// check overflow
		if( bs->bIsEmpty || bs->bitsRemaining < 8 ) 
		{
			// mark the error for overflow reads
			bs->bInError = 1;
			return 0;
		}
		
		// catch eof
		if( bs->bitsRemaining == 8 ) bs->bIsEmpty = 1; 
	}
	
	// if we are byte aligned, we can accelerate the read dramatically
	if( bs->bitPtr == 0 )
	{
		newByte = *bs->bytePtr;
		bs->bytePtr++;
		bs->bitsRemaining -= 8;
		return newByte;
	}

	// else, do the read in 2 parts
	newByte = *bs->bytePtr <<  bs->bitPtr;
	bs->bytePtr++;
	newByte |= *bs->bytePtr  >> (8 - bs->bitPtr);

	// byte and bit ptrs are still synched
	bs->bitsRemaining -= 8;

	return newByte;
}

/* ------------
bs_Lookahead8Bits - returns the value of the next 8 bits of the stream, does not take them out of the stream though
------------ */
byte bs_Lookahead8Bits( bitstream_t *bs )
{
	byte nextByte = 0;

	// assert
	if( bs == NULL )
	{
		con_Print( "<RED>Bitstream Lookahead8Bits Assert Failed: bs is NULL" );
		eng_Stop( "2200013" );
		return 0;
	}
	
	// check remaining bits
	if( bs->bitsRemaining < 16 ) 
	{
		bs_FillBuffer(bs);

		// check for errors
		if( bs->bInError ) return 0;

		// check eof
		if( bs->bIsEmpty || bs->bitsRemaining < 8 ) 
		{
			// mark the error for overflow reads
			bs->bInError = 1;
			return 0;
		}
	}
	
	// if we are byte aligned, we can accelerate the read dramatically
	if( bs->bitPtr == 0 )
	{
		nextByte = *bs->bytePtr;
		return nextByte;
	}

	// else, do the read in 2 parts
	nextByte = *bs->bytePtr <<  bs->bitPtr;
	bs->bytePtr++;
	nextByte |= *bs->bytePtr  >> (8 - bs->bitPtr);
	bs->bytePtr--;

	return nextByte;
}

/* ------------
bs_Lookahead16Bits
------------ */
unsigned short int bs_Lookahead16Bits( bitstream_t *bs )
{
	unsigned short int nextShort = 0;
	
	// assert
	if( bs == NULL )
	{
		con_Print( "<RED>Bitstream Get16Bits Assert Failed: bs is NULL" );
		eng_Stop( "2200014" );
		return 0;
	}

	// check remaining bits
	if( bs->bitsRemaining < 24 ) 
	{
		// get 8 bits would do this, but we prefer lumped reads
		bs_FillBuffer(bs);

		// check for errors
		if( bs->bInError ) return 0;

		// check eof
		if( bs->bIsEmpty || bs->bitsRemaining < 16 ) 
		{
			// mark the error for overflow reads
			bs->bInError = 1;
			return 0;
		}
	}

	// we can accelerate byte aligned reads easily
	if( bs->bitPtr == 0 )
	{
		nextShort = (unsigned short int)*bs->bytePtr << 8;
		bs->bytePtr++;
		nextShort |= (unsigned short int)*bs->bytePtr;
		bs->bytePtr--;
		return nextShort;
	}

	// else give an abstract treatment with byte reads
	nextShort = (unsigned short int)bs_Lookahead8Bits(bs) << 8;
	bs->bytePtr++;
	nextShort |= (unsigned short int)bs_Get8Bits(bs);
	bs->bytePtr--;
	
	return ntohs( nextShort );
}

/* ------------
bs_Get1Bit
------------ */
byte bs_Get1Bit( bitstream_t *bs )
{
	byte newByte = 0;

	if( bs == NULL )
	{
		con_Print( "<RED>Bitstream Get1Bit Assert Failed: bs is NULL" );
		eng_Stop( "220007" );
		return 0;
	}

	// check remaining bits
	if( bs->bitsRemaining < 9 ) 
	{
		bs_FillBuffer(bs);
		
		// check for errors
		if( bs->bInError ) return 0;

		// check for overflow
		if( bs->bIsEmpty || bs->bitsRemaining < 1 ) 
		{
			// mark the error for overflow reads
			bs->bInError = 1;
			return 0;
		}

		// catch eof
		if( bs->bitsRemaining == 1 ) bs->bIsEmpty = 1; 
	}
	
	// do the read
	newByte = (*(bs->bytePtr) >> (7 - bs->bitPtr)) & 1;

	// adjust our states
	bs->bitPtr++;
	bs->bitsRemaining--;
	if( bs->bitPtr > 7 )
	{
		bs->bitPtr = 0;

		// our check for 8 spare bits ensures we'll have a spare byte to move here
		bs->bytePtr++;
	}
	
	return newByte;
}

/* ------------
bs_GetBits
------------ */
unsigned long int bs_GetBits( bitstream_t *bs, byte numBits )
{
	unsigned long int newLong = 0;
	signed short int a;

	if( bs == NULL )
	{
		con_Print( "<RED>Bitstream GetBits Assert Failed: bs is NULL" );
		eng_Stop( "220009" );
		return 0;
	}

	// we can only get 32 or fewer bits
	if( numBits > 32 ) 
	{
		con_Print( "<RED>Bitstream GetBits Assert Failed: numBits > 32" );
		eng_Stop( "220010" );
		return 0;
	}
	
	// check remaining bits
	if( bs->bitsRemaining < numBits + 8 ) 
	{
		bs_FillBuffer(bs);
		
		// check for errors
		if( bs->bInError ) return 0;

		// check for overflow
		if( bs->bIsEmpty || bs->bitsRemaining < numBits ) 
		{
			// mark the error for overflow reads
			bs->bInError = 1;
			return 0;
		}

		// catch eof
		if( bs->bitsRemaining == numBits ) bs->bIsEmpty = 1;
	}

	// this is painfully slow and painfully necessary
	for( a = numBits - 1 ; a >= 0 ; a-- )
	{
		newLong |= (unsigned long int)bs_Get1Bit(bs) << a;
	}

	return newLong;
}

/* ------------
bs_AlignToByte - reads to the nearest byte alignment; read bits disappear
------------ */
void bs_AlignToByte( bitstream_t *bs )
{
	if( bs == NULL )
	{
		con_Print( "<RED>Assert failed in bitstream bs_AlignToByte:  bs == NULL" );
		eng_Stop( "2200011" );
		return;
	}

	if( bs->bitPtr == 0 ) return;
	bs_GetBits( bs, (byte)(8L - (int32_t)bs->bitPtr) );
}

/* ------------
bs_IsAlignedToByte - returns non-0 if the bitstream is aligned to a byte boundary, 0 otherwise
------------ */
byte bs_IsAlignedToByte( bitstream_t *bs )
{
	if( bs == NULL )
	{
		con_Print( "<RED>Assert failed in bitstream bs_IsAlignedToByte:  bs == NULL" );
		eng_Stop( "2200012" );
		return 0;
	}

	return !bs->bitPtr;
}

/* ------------
bs_FillBuffer - reads bits from the file into the buffer, moves buffer contents to the front
------------ */
static void bs_FillBuffer( bitstream_t *bs )
{
	byte bTransLen; // num bytes to copy to front
	byte *bpRead;  // for the file read
	int iReadLen;
	int a;
	int iInCnt; // bytes read from file
	
	// assert
	if( bs == NULL )
	{
		con_Print( "<RED>Assert failed in bitstream bs_FillBuffer:  bs == NULL" );
		eng_Stop( "220008" );
		return;
	}

	// no need to read conditions
	if( bs->bBuffer == bs->bytePtr ) return;

	// error or end conditions
	if( bs->bInError || bs->bHasLastByte || bs->bIsEmpty ) return;

	// now do the math for the read
	bTransLen = BS_BUFFER_SIZE - (byte)(bs->bytePtr - bs->bBuffer);
	bpRead = bs->bBuffer + bTransLen;
	iReadLen = BS_BUFFER_SIZE - bTransLen;
	
	// move bytes to begining
	for( a = 0 ; a < bTransLen ; a++ )
	{
		bs->bBuffer[a] = *bs->bytePtr;
		bs->bytePtr++;
	}
	bs->bytePtr = bs->bBuffer;

	// read in what we need
	iInCnt = fs_read( bpRead, 1, iReadLen, bs->fIn );

	// check for error / eof conditions
	if( iInCnt < iReadLen )
	{
		if( fs_error(bs->fIn) ) bs->bInError = 1;
		if( fs_eof(bs->fIn) ) bs->bHasLastByte = 1;
	}

	// adjust our bit markers to the new set
	bs->bitsRemaining = (BS_BUFFER_SIZE * 8) - bs->bitPtr;
}

byte bs_error( bitstream_t *bs )
{
	if( bs == NULL ) return 1;
	return bs->bInError;
}
byte bs_eof( bitstream_t *bs )
{
	if( bs == NULL ) return 1;
	return bs->bIsEmpty;
}

byte bs_IsEnding( bitstream_t *bs )
{
	if( bs == NULL ) return 1;
	return bs->bHasLastByte;
}

static void bs_LinkTail( bitstream_t *bs )
{
	if( bsHead == NULL ) bsHead = bs;

	if( bsTail == NULL )
	{
		bsTail = bs;
		bs->next = NULL;
		bs->prev = NULL;
		return;
	}

	bs->prev = bsTail;
	bs->next = NULL;

	bsTail->next = bs;
	bsTail = bs;
}

static void bs_Unlink( bitstream_t *bs )
{
	if( bs->prev != NULL ) bs->prev->next = bs->next;
	if( bs->next != NULL ) bs->next->prev = bs->prev;

	if( bsTail == bs ) bsTail = bs->prev;
	if( bsHead == bs ) bsHead = bs->next;

	bs->prev = NULL;
	bs->next = NULL;
}

