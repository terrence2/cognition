// Cognition
// jpeg.c
// Created 5-4-03 @ 1030 by Terrence Cole

// Includes
/////////////
#include "cog_global.h"

// Local Definitions
//////////////////////
#define TABLE_DIRECTORY "tables"

// the maximal size of a 3x downsampled image; prebuffered, converting FF00 -> FF
#define JPEG_MAX_CU_SIZE 2048 // 16 + 16 * 64
#define JPEG_BUFFER_SIZE 2072 // JPEG_MAX_CU_SIZE + 24

#define MARKER 0xFF // tag marker
#define SOI 0xD8 // Start of Image
#define EOI 0xD9 // End of Image
#define SOF 0xC0 // Start of Frame
#define DHT 0xC4 // Define Huffman Table
#define SOS 0xDA // Start of Scan
#define DQT 0xDB // Define Quantization Table
#define APP0 0xE0 // JFIF segment info
#define APP1 0xE1
#define APP2 0xE2
#define APP3 0xE3
#define APP4 0xE4
#define APP5 0xE5
#define APP6 0xE6
#define APP7 0xE7
#define APP8 0xE8
#define APP9 0xE9
#define APP10 0xEA
#define APP11 0xEB
#define APP12 0xEC
#define APP13 0xED
#define APP14 0xEE
#define APP15 0xEF
#define COM 0xFE // Comment
#define DRI 0xDD // Define Restart Interval
#define RST0 0xD0 // reset 0
#define RST1 0xD1 // reset 1
#define RST2 0xD2 // reset 2
#define RST3 0xD3 // reset 3
#define RST4 0xD4 // reset 4
#define RST5 0xD5 // reset 5
#define RST6 0xD6 // reset 6
#define RST7 0xD7 // reset 7
#define DNL 0xDC // define number of lines

// Global Definitions
///////////////////////

// Local Structures
/////////////////////

// the quantization table
typedef struct QT_s
{
	uint8_t bInUse; // starts as 0, 1 after data is loaded to tbl
	uint8_t bNum; // [0,3] // the index of the 4 possible components
	uint8_t bPrecision; // 8 or 16; both supported
	float tbl[64];
} QT_t;

// the huffman nodes
typedef struct HT_node_s
{
	uint8_t bValue;
	uint8_t bIsLeaf;
	struct HT_node_s *left;
	struct HT_node_s *right;
} HT_node_t;

typedef struct jpeg_header_s
{
	/* APP 0 SEGMENT */
	uint8_t bVerMaj;
	uint8_t bVerMin;
	// These are NOT USED
	uint8_t bThumbX; // width pixels
	uint8_t bThumbY; // height pixels
	uint8_t *bThumb; // rgb bytes for thumb

	/* SOF SEGMENT */
	uint8_t bPrecis; // only 8 supported
} jpeg_header_t;

typedef struct jpeg_comp_s
{
	uint8_t bID; // 1=Y, 2=Cb, 3=Cr
	uint8_t bSampFactorVert;
	uint8_t bSampFactorHoriz;

	// the non-upscaled storage plane
	uint8_t *bpPlane;
	uint16_t sX; // the width of the plane in pixels
	uint16_t sY; // the height of the plane in pixels

	// the upscaled storage plane (to 8 aligned (sX/Y, not sImageX/Y)
	// N.B.  THIS IS ONLY VALID IF bUpscaleX or Y ARE > 1 !!!
	uint8_t *bpOutPlane;

	uint8_t bUpscaleX; // the upscale ratio for X
	uint8_t bUpscaleY; // the upscale ratio for Y

	// the spans define the forward travel of the insert pointer to bpPlane as we insert blocks into the image
	uint32_t iUnitSpanX;
	uint32_t iMCUSpanX;
	uint32_t iUnitSpanY;
	uint32_t iMCUSpanY;
	uint16_t sRowSpan;

	// the qt and ht for each component
	uint8_t bQTTblNum;
	uint8_t bHTTreeNumDC;
	uint8_t bHTTreeNumAC;
	QT_t qt; // the quantization table for this component
	HT_node_t *htDC;
	HT_node_t *htAC; // root of the huffman tree
} jpeg_comp_t;

typedef struct jpeg_s
{
	file_t *fp; // the file
	bitstream_t *bs; // the bitstream
	jpeg_header_t header; // misc, static data about this jpeg

	uint16_t sDRI; // the restart tag interval in # of MCU's between tags

	uint16_t sMCUHeight; // num MCU's down
	uint16_t sMCUWidth; // num MCU's across
	uint32_t iNumMCUs; // the total number of MCU's in the image

	uint16_t sX;  // working pixels X
	uint16_t sY;  // working pixels Y

	uint16_t sImageX;  // output pixels X (from file)
	uint16_t sImageY; // output pixels Y (from file)
	uint8_t *imgPlane; // the output image

	uint8_t bMaxSampFactX; // the largest sampling factor in this image
	uint8_t bMaxSampFactY; 

	uint8_t bNumComp;  // 1 (grayscale) or 3 (Y,Cr,Cb)	
	jpeg_comp_t comp[3]; // max of 3 components

	// the buffer: maximal MCU size for 3x-downsampled images:  22528b
	uint8_t bBuffer[JPEG_BUFFER_SIZE];
	uint8_t bBufNeedsInit;
	uint8_t *pBufHead;
	uint8_t *pBufTail;
	uint8_t *pBufFirstByte;
	uint8_t *pBufLastByte;
	uint8_t pBit;
	uint32_t iBufReads;
	uint8_t bBufIsOut; // set when the last file read is made

	// Transfer Tables for the Components
	QT_t QT_Trans[4]; // one for each possible component
	HT_node_t *HT_DC_Trans[4]; // [0-3] DC: Y, Cb, Cr
	HT_node_t *HT_AC_Trans[4]; // [0-3] DC: Y, Cb, Cr
} jpeg_t;


// Global Prototypes
//////////////////////
/*
byte *jpg_Load( char *path, unsigned int *width, unsigned int *height,	byte *bpp );
*/

// Limitations:  	Supports Grayscale and Y,Cr,Cb planes and no others.
//				Will only look for the first segment of the file if there is more than one

// Local Prototypes
/////////////////////
static uint8_t jpg_GetHeaderInfo( jpeg_t *jpeg );
// gets all info for the header and reads to SOS
// returns non-0 if all possible info is read correctly

static uint8_t jpg_FindNextMarker( bitstream_t *bs );
// returns the byte value of the next marker tag

static uint8_t jpg_GetAPP0( jpeg_t *jpeg );
// loads an APP0 segment descriptor from file to app0, returns 0 on fail, !0 on success

static uint16_t jpg_GetDRI( jpeg_t *jpeg );
// loads the specified dri tag into jpeg - sDRI = # of MCU's between RST tags  - return 0 on fail, non 0 on success

static uint8_t jpg_GetDQT( jpeg_t *jpeg );
// - gets the Quantization table from the bitstream; returns 1 on success, 0 on fail
// - QT is an array of 3 Quantization Tables

static void jpg_GetDefaultQT( uint8_t bType, QT_t *QT );
// Loads default quantization tables into all unused tables
// type is in range 1 to 3, 1=Y, 2=Cb = 3 = Cr

 static uint8_t jpg_GetHuffmanTree( bitstream_t *bs, jpeg_t *jpeg, int8_t bPlacement );
// gets the huffman trees under this DHT - returns the number of HT's successfully loaded
// if bPlacement != -1, then loads all subsequent tables into bPlacement for whatever type of table is in the stream

static HT_node_t *jpg_ht_GetNode( uint8_t bLevel, uint8_t bNumCodes[], uint8_t bCodesAvail[], uint8_t **bCodes );
// does the tree generation: checks to see if a value is available for this node, if avail, sets this node
// to that value then returns, else seeks nodes on a lower level to assign

static HT_node_t *jpg_GetDefaultHuffmanTree( uint8_t bComponent, uint8_t bType, jpeg_t *jpeg );
// bComponent: Y=1, C=2,3(,4);  bType: 0=DC, 1=AC;  assigns the new tree to any available transition tree in the
// jpeg block and returns a pointer to the new tree

static void jpg_FreeHuffmanTrees( jpeg_t *jpeg );
static void jpg_FreeHTNode( HT_node_t *node );
// frees all non-null trees

static uint8_t jpg_GetHuffmanValue( HT_node_t *node, uint16_t HCode, uint8_t bNumBits, uint8_t *HDeCode );
// returns 1 if huffman value exists, otherwise 0, if the value exists, HDeCode is set to the value

static uint8_t jpg_GetSOF( jpeg_t *jpeg );
// loads the SOF info to sof from bs; returns 0 on fail non-0 on success

static uint8_t jpg_GetSOS( jpeg_t *jpeg );
// loads the SOS info to sos from bs; returns 0 on fail non-0 on success

static uint8_t jpg_PrepareDecoder( jpeg_t *jpeg );
// returns 0 on fail, non-0 on success;
// gets the MCU size and allocates for image transfer and output data

static uint8_t jpg_Decode( jpeg_t *jpeg );
// do the decode with the given jpeg specs from the given bitstream
// return 0 on fail, non-0 on success

static uint8_t jpg_DecodeUnit( jpeg_t *jpeg, uint8_t comp, int32_t *iDC, uint8_t *bOutUnit );
// brings out and expands one chunk of compressed data (8x8) of a single color
// fDC is 3 floats or longer, bOutUnit is 64 bytes

static int32_t jpg_MinBitsTransform( uint8_t bCategory, uint32_t nBits );
// converts a number of bCategory from the special JPEG representation 
// to a 2's compliment signed 16bit integer

static void jpg_IDCT( jpeg_t *jpeg, uint8_t comp, int16_t *inBlock, uint8_t *outBlock );
// thanks gratefully given to the IJG for the math to do this quickly
// Quantization, IDCT, Level Shift

static void jpg_DeZigZagUnit( uint16_t *bOutUnit, uint16_t *bInUnit );
// takes in 2 64 elmt arrays of shorts and does the jpeg de-zig-zag to make
// the in array into the proper out array

static uint8_t jpg_Upscale( jpeg_t *jpeg, uint8_t comp );
//upscale the component plane to an image size plane, applying software blending 

static uint8_t jpg_ComponentsToOutput( jpeg_t *jpeg );
// copies the YUV component planes to a final output plane in RGB, clipped to the proper size

static uint8_t jpg_Invert( jpeg_t *jpeg );
// inverts the row ordering from bottom up to top down after we are done with the image processing stage

//static void jpg_YUVtoRGB( jpeg_t *jpeg );
// convert the image in YUV to RGB

static uint8_t jpg_FillBuffer( jpeg_t *jpeg );
// the bitstream_t provides 64b of buffer, lookahead, alignment, and transparent file reading;
// however, it can't provide the direct access we need for optimal performance in reading a jpeg stream
// this buffer provides the maximal size of an MCU, stripping FF tags so that we don't have to check
// for them when we do every single read
// The buffer is implemented circularly.
// This should get called once per CU before the call to decode
// all buffer specific members of jpeg should get inited in PrepareDecoder: 
// Needed in PrepareDecoder: bBufNeedsInit = 1;
// NB. this will fail if we try to read all the way around the buffer; that should never happen though with a well sized buffer

static uint8_t jpg_GetBit( jpeg_t *jpeg );
// returns 1 bit from our filtered jpeg stream

// Local Variables
////////////////////

// *********** FUNCTIONALITY ***********
/* ------------
jpg_Load
------------ */
byte *jpg_Load( char *path, unsigned int *width, unsigned int *height, byte *bpp )
{
	jpeg_t jpeg;
	uint8_t a;

	// tell us about it
	con_Print( "<BLUE>Loading</BLUE> jpeg image from <ORANGE>\"%s\"", path );
	eng_LoadingFrame();

	// asserts
	if( path == NULL )
	{
		con_Print( "<RED>JPEG Load Assert Failed:  null path passed to jpg_Load" );
		eng_Stop( "50000" );
		return NULL;
	}
	if( width == NULL )
	{
		con_Print( "<RED>JPEG Load Assert Failed:  null width pointer passed to jpg_Load" );
		eng_Stop( "50001" );
		return NULL;
	}
	if( height == NULL )
	{
		con_Print( "<RED>JPEG Load Assert Failed:  null height pointer passed to jpg_Load" );
		eng_Stop( "50002" );
		return NULL;
	}
	if( bpp == NULL )
	{
		con_Print( "<RED>JPEG Load Assert Failed:  null bpp pointer passed to jpg_Load" );
		eng_Stop( "50003" );
		return NULL;
	}
	
	// clear the jpeg block
	memset( &jpeg, 0, sizeof(jpeg_t) );

	// attempt to open the file
	jpeg.fp = fs_open( path, "r" );
	if( jpeg.fp == NULL ) 
	{
		con_Print( "<RED>JPEG Load Error:  Unable to open path: <ORANGE>\"%s\"", path );
		return NULL;
	}
	
	// get a bitstream for the file
	jpeg.bs = bs_GetBitStream( jpeg.fp );
	if( jpeg.bs == NULL )
	{
		con_Print( "<RED>JPEG Load Error:  Unable to get a bitstream for <ORANGE>\"%s\"" );
		fs_close( jpeg.fp );
		return NULL;
	}

	// ensure we're looking at a jpeg image
	if( bs_Get8Bits(jpeg.bs) != MARKER || bs_Get8Bits(jpeg.bs) != SOI )
	{
		if( bs_error(jpeg.bs) )
			con_Print( "<RED>JPEG Load Error:  an error occured while reading this file." );
		else if( bs_eof(jpeg.bs) )
			con_Print( "<RED>JPEG Load Error:  the end of the file was reached prematurly." );
		else
			con_Print( "<RED>JPEG Load Error:  This file is not a JPEG image." );
		bs_CloseStream(jpeg.bs);
		fs_close(jpeg.fp);
		return NULL;
	}

	// scan to the Start of Scan collecting all info available
	if( !jpg_GetHeaderInfo( &jpeg ) )
	{
		con_Print( "<RED>JPEG Load Error:  GetHeaderInfo failed" );
		bs_CloseStream(jpeg.bs);
		fs_close(jpeg.fp);
		jpg_FreeHuffmanTrees( &jpeg );
		return NULL;
	}

	// fill in default values for unfilled tables and trees
	for( a = 0 ; a < jpeg.bNumComp ; a++ )
	{
		// copy over material for each component
		memcpy( &jpeg.comp[a].qt, &jpeg.QT_Trans[jpeg.comp[a].bQTTblNum], sizeof(QT_t) );
		// load default if not available
		if( !jpeg.comp[a].qt.bInUse ) jpg_GetDefaultQT( jpeg.comp[a].bID, &jpeg.comp[a].qt );

		// copy over the ht from the index
		jpeg.comp[a].htDC = jpeg.HT_DC_Trans[ jpeg.comp[a].bHTTreeNumDC ];
		// load default if not available
		if( jpeg.comp[a].htDC == NULL ) 
		{
			jpeg.comp[a].htDC = jpg_GetDefaultHuffmanTree( jpeg.comp[a].bID, 0, &jpeg ); // DC=0
			if( jpeg.comp[a].htDC == NULL )
			{
				con_Print( "<RED>JPEG Load Error:  Get Default Huffman Tree failed for DC." );
				bs_CloseStream(jpeg.bs);
				fs_close(jpeg.fp);
				jpg_FreeHuffmanTrees( &jpeg );
				return NULL;
			}
		}

		// copy over material for each component
		memcpy( &jpeg.comp[a].qt, &jpeg.QT_Trans[jpeg.comp[a].bQTTblNum], sizeof(QT_t) );
		// load default if not available
		if( !jpeg.comp[a].qt.bInUse ) jpg_GetDefaultQT( jpeg.comp[a].bID, &jpeg.comp[a].qt );

		// copy over the ht from the index
		jpeg.comp[a].htAC = jpeg.HT_AC_Trans[ jpeg.comp[a].bHTTreeNumAC ];
		// load default if not available
		if( jpeg.comp[a].htAC == NULL ) 
		{
			jpeg.comp[a].htAC = jpg_GetDefaultHuffmanTree( jpeg.comp[a].bID, 1, &jpeg ); // AC=1
			if( jpeg.comp[a].htAC == NULL )
			{
				con_Print( "<RED>JPEG Load Error:  Get Default Huffman Tree failed for AC." );
				bs_CloseStream(jpeg.bs);
				fs_close(jpeg.fp);
				jpg_FreeHuffmanTrees( &jpeg );
				return NULL;
			}
		}

	}

	// init some memory and get some data for the decode
	if( !jpg_PrepareDecoder( &jpeg ) )
	{
		con_Print( "<RED>JPEG Load Error:  Decoder Preparation failed." );
		for( a = 0 ; a < jpeg.bNumComp ; a++ ) SAFE_RELEASE(jpeg.comp[a].bpPlane);
		for( a = 0 ; a < jpeg.bNumComp ; a++ ) SAFE_RELEASE(jpeg.comp[a].bpOutPlane);
		SAFE_RELEASE( jpeg.imgPlane );
		jpg_FreeHuffmanTrees( &jpeg );
		return NULL;
	}

	// do the decode
	if( !jpg_Decode( &jpeg ) )
	{
		con_Print( "<RED>JPEG Load Error:  Decode Failed." );
		for( a = 0 ; a < jpeg.bNumComp ; a++ ) SAFE_RELEASE(jpeg.comp[a].bpPlane);
		for( a = 0 ; a < jpeg.bNumComp ; a++ ) SAFE_RELEASE(jpeg.comp[a].bpOutPlane);
		SAFE_RELEASE( jpeg.imgPlane );
		jpg_FreeHuffmanTrees( &jpeg );
		return NULL;
	}

	// cleanup before leaving
	jpg_FreeHuffmanTrees( &jpeg );
	for( a = 0 ; a < jpeg.bNumComp ; a++ ) SAFE_RELEASE(jpeg.comp[a].bpPlane);
	for( a = 0 ; a < jpeg.bNumComp ; a++ ) SAFE_RELEASE(jpeg.comp[a].bpOutPlane);

	// close the bitstream and the file
	bs_CloseStream( jpeg.bs );
	fs_close( jpeg.fp );

	// transfer over our data
	*width = jpeg.sImageX;
	*height = jpeg.sImageY;
	*bpp = 8 * jpeg.bNumComp;

	return jpeg.imgPlane;
}

/* ------------
jpg_GetHeaderInfo - gets all info for the header and reads to SOS
				- returns non-0 if all possible info is read correctly
------------ */
static uint8_t jpg_GetHeaderInfo( jpeg_t *jpeg )
{
	uint8_t bTmp;
	uint8_t bMarker;
	uint8_t bHaveAPP0 = 0;
	uint8_t bHaveSOF = 0;
	uint8_t bLoadedDQT = 0;
	uint8_t bLoadedDHT = 0;
	uint16_t a, sTmp;

	while( (bMarker = jpg_FindNextMarker( jpeg->bs )) != SOS )
	{
		switch( bMarker )
		{
		case 0:
			con_Print( "<RED>JPEG Load Error:  This jpeg file is malformed." );
			return 0;
		case APP0:
			if( bHaveAPP0 ) break; // ignore repeats
			if( !jpg_GetAPP0( jpeg ) )
			{
				con_Print( "<RED>JPEG Load Error:  Could not get APP0 tag." );
				return 0;
			}
			bHaveAPP0 = 1;
			break;
		case APP1:
		case APP2:
		case APP3:
		case APP4:
		case APP5:
		case APP6:
		case APP7:
		case APP8:
		case APP9:
		case APP10:
		case APP11:
		case APP12:
		case APP13:
		case APP14:
		case APP15:
			// read in the length
			sTmp = bs_Get16Bits(jpeg->bs);
			// scan through these tags without reading them
			for( a = 0 ; a < sTmp ; a++ )
			{
				bs_Get8Bits(jpeg->bs);
			}
			break;
		case DQT:
			// don't load more than is acceptable
			if( bLoadedDQT >= 4 )
			{
				con_Print( "<RED>\tJPEG Load Warning:  This jpeg contains more than 4 Quantization Tables" );
				break;
			}
			// attempt the load
			bTmp = jpg_GetDQT( jpeg );
			if( !bTmp )
			{
				con_Print( "<RED>JPEG Load Error:  Could not get Quantization Tables." );
				return 0;
			}
			bLoadedDQT = bLoadedDQT + bTmp;  // stupid compiler
			break;
		case DHT:
			// don't duplicate ourselves
			if( bLoadedDHT >= 8)
			{
				con_Print( "<RED>\tJPEG Load Warning:  This jpeg contains more than 8Huffman Trees" );
				break;
			}
			// attempt the load
			bTmp = jpg_GetHuffmanTree( jpeg->bs, jpeg, -1 );
			if( !bTmp )
			{
				con_Print( "<RED>JPEG Load Error:  Could not get Huffman Table from DHT tag." );
				return 0;
			}
			bLoadedDHT = bLoadedDHT + bTmp;  // stupid compiler
			break;
		case SOF:
			if( bHaveSOF ) 
			{
				con_Print( "<RED>JPEG Load Error:  Start of Frame (SOF) found more than once!" );
 				return 0;
			}
			bTmp = jpg_GetSOF( jpeg );
			if( !bTmp )
			{
				con_Print( "<RED>JPEG Load Error:  Start of Frame (SOF) load failed." );
 				return 0;
			}
			bHaveSOF = 1;
			break;
		case DRI:
			if( !jpg_GetDRI( jpeg ) )
			{
				con_Print( "<RED>JPEG Load Error:  Could not get DRI tag from header." );
				return 0;
			}
			break;
		case DNL:
			con_Print( "DNL (Define Number of Lines) Defined!" );
			return 0;
			break;
		case SOI: // if we see another one of these, reset the decoder and assume what came before was a thumbnail
			bHaveAPP0 = 0;
			bHaveSOF = 0;
			bLoadedDQT = 0;
			bLoadedDHT = 0;
			jpg_FreeHuffmanTrees(jpeg);
			break;
		default:
			// ignore other keys
			break;
		}
	}

	// get the SOS info
	if( !jpg_GetSOS(jpeg) )
	{
		con_Print( "<RED>JPEG Load Error:  could not get the SOS tag." );
		return 0;
	}

	// check for errors
	if( !bHaveAPP0 )
	{
		con_Print( "<RED>JPEG Load Error:  could not find an APP0 tag." );
		return 0;
	}
	if( !bHaveSOF )
	{
		con_Print( "<RED>JPEG Load Error:  could not find an SOF tag." );
		return 0;
	}
	if( jpeg->bNumComp != 1 && jpeg->bNumComp != 3 )
	{
		con_Print( "<RED>JPEG Load Error:  this image has an unsupported number of components:  %d", jpeg->bNumComp );
		return 0;
	}

	return 1;
}

/* ------------
jpg_FindNextMarker - returns the byte value of the next marker tag; 0 on fail
------------ */
static uint8_t jpg_FindNextMarker( bitstream_t *bs )
{
	uint8_t bTmp;

	// static so can skip assert on bs

	// seek to the nearest byte alignment
	bs_AlignToByte(bs);

	// start the search
	bTmp = bs_Get8Bits(bs);
	while( !bs_eof(bs) && !bs_error(bs) )
	{
		if( bTmp == MARKER )
		{
			bTmp = bs_Get8Bits(bs);
			if( bTmp != 0x00 && bTmp != 0xFF ) return bTmp;
		}
		// only get 8 more bits if we skipped above to avoid reading past pairs of FF
		else
		{
			bTmp = bs_Get8Bits(bs);
		}
	}

	return 0;
}

/* ------------
jpg_GetAPP0 - gets the APP0 info for this segment; returns 0 on fail !0 on success
------------ */
static uint8_t jpg_GetAPP0( jpeg_t *jpeg )
{
	uint16_t sLen;
	int32_t a;
	bitstream_t *bs = jpeg->bs;
	
	// static so can skip assert on jpeg

	// get the test values to make sure this is a relevant stream
	sLen = bs_Get16Bits(bs);
	if( sLen < 16 || 
		bs_Get8Bits(bs) != 'J' || 
		bs_Get8Bits(bs) != 'F' || 
		bs_Get8Bits(bs) != 'I' || 
		bs_Get8Bits(bs) != 'F' || 
		bs_Get8Bits(bs) != 0 )
	{
		con_Print( "<RED>JPEG Load Error in GetAPP0:  This stream is not in JFIF format." );
		return 0;
	}

	// get stuff from file
	jpeg->header.bVerMaj = bs_Get8Bits(bs);
	jpeg->header.bVerMin = bs_Get8Bits(bs);

	// junk we don't want or need
	bs_Get8Bits(bs); // UNITS
	bs_Get16Bits(bs); // Pixels per Unit X
	bs_Get16Bits(bs); // Pixels per Unit Y

	// skip the thumbnail
	for( a = bs_Get8Bits(bs) * bs_Get8Bits(bs) * 3 ; a > 0 ; a-- ) bs_Get8Bits(bs);

	return 1;
}

/* ------------
jpg_GetDRI - loads the specified dri tag into jpeg - sDRI = # of MCU's between RST tags  - return 0 on fail, non 0 on success
------------ */
static uint16_t jpg_GetDRI( jpeg_t *jpeg )
{
	uint16_t sTmp;
	
	// length must be 4
	sTmp = bs_Get16Bits(jpeg->bs);
	if( sTmp != 4 )
	{
		con_Print( "<RED>JPEG Load Error:  Encountered badly formed DRI tag: expected length of 4, found %d", sTmp );
		return 0;
	}

	jpeg->sDRI = bs_Get16Bits(jpeg->bs);
	return jpeg->sDRI; // DRI of 0 does not make sense
}

/* ------------
jpg_GetDQT - gets the Quantization table from the bitstream; returns 0 on fail, the number of qt's loaded on success
				- QT is an array of 3 Quantization Tables
------------ */
static uint8_t jpg_GetDQT( jpeg_t *jpeg )
{
	bitstream_t *bs = jpeg->bs;
	uint16_t sLen = bs_Get16Bits(bs);
	uint16_t iOffset = 2; // counts the bytes passed in the stream
	uint8_t a, b;
	uint8_t bTmpNum, bTmpPrecis;
	QT_t *QT = jpeg->QT_Trans;
	
	// load all tables
	for( a = 0 ; iOffset < sLen ; a++ )
	{
		// attempt to get the table info
		bTmpNum = bs_Get8Bits(bs);  
		iOffset++;
		bTmpPrecis = bTmpNum >> 4;
		bTmpNum = bTmpNum & 0x0F;

		// bounds check the table number
		if( bTmpNum > 3 )
		{
			con_Print( "<RED>JPEG Load Error:  DQT index out of range." );
			return 0;
		}

		// get the number and precision of the quantization of the first table
		QT[bTmpNum].bNum = bTmpNum;
		QT[bTmpNum].bPrecision = bTmpPrecis;

		// do the input of the table as a switch for the different table sizes
		switch( bTmpPrecis )
		{
		case 0:  // 8 bit tables
			for( b = 0 ; b < 64 ; b++ )
			{
				QT[bTmpNum].tbl[b] = (float)bs_Get8Bits(bs);
				iOffset++;

				if( bs_eof(bs) || bs_error(bs) ) 
				{
					con_Print( "<RED>JPEG Load Error:  The end of the file has been reached or an I/O error occured." );
					return 0;
				}
			}
			break;
		case 1: // 16 bit tables
			for( b = 0 ; b < 64 ; b++ )
			{
				QT[bTmpNum].tbl[b] = (float)bs_Get16Bits(bs);
				iOffset += 2;

				if( bs_eof(bs) || bs_error(bs) )
				{
					con_Print( "<RED>JPEG Load Error:  The end of the file has been reached or an I/O error occured." );
					return 0;
				}
			}
			break;
		default:
			con_Print( "<RED>JPEG Load Error: DQT precision out of range for table %d", bTmpNum );
			return 0;
		}

		// set the inUse
		QT[bTmpNum].bInUse = 1;
	}

	return a;
}

/* ------------
 jpg_GetDefaultQT - gets default quantization table for all unfilled tables (!bInUse)
 - type is in range 1 to 3, 1=Y, 2=Cb = 3 = Cr
------------ */
static void jpg_GetDefaultQT( uint8_t bType, QT_t *QT )
{
	// precondition
	if( QT->bInUse ) return;

	// set defaults
	QT->bNum;
	QT->bInUse = 1;
	QT->bPrecision = 8;

	if( bType == 1 )
	{
		QT->tbl[0] = 16;
		QT->tbl[1] = 11;
		QT->tbl[2] = 12;
		QT->tbl[3] = 14;
		QT->tbl[4] = 12;
		QT->tbl[5] = 10;
		QT->tbl[6] = 16;
		QT->tbl[7] = 14;
		QT->tbl[8] = 13;
		QT->tbl[9] = 14;
		QT->tbl[10] = 18;
		QT->tbl[11] = 17;
		QT->tbl[12] = 16;
		QT->tbl[13] = 19;
		QT->tbl[14] = 24;
		QT->tbl[15] = 40;
		QT->tbl[16] = 26;
		QT->tbl[17] = 24;
		QT->tbl[18] = 22;
		QT->tbl[19] = 22;
		QT->tbl[20] = 24;
		QT->tbl[21] = 49;
		QT->tbl[22] = 35;
		QT->tbl[23] = 37;
		QT->tbl[24] = 29;
		QT->tbl[25] = 40;
		QT->tbl[26] = 58;
		QT->tbl[27] = 51;
		QT->tbl[28] = 61;
		QT->tbl[29] = 60;
		QT->tbl[30] = 57;
		QT->tbl[31] = 51;
		QT->tbl[32] = 56;
		QT->tbl[33] = 55;
		QT->tbl[34] = 64;
		QT->tbl[35] = 72;
		QT->tbl[36] = 92;
		QT->tbl[37] = 78;
		QT->tbl[38] = 64;
		QT->tbl[39] = 68;
		QT->tbl[40] = 87;
		QT->tbl[41] = 69;
		QT->tbl[42] = 55;
		QT->tbl[43] = 56;
		QT->tbl[44] = 80;
		QT->tbl[45] = 109;
		QT->tbl[46] = 81;
		QT->tbl[47] = 87;
		QT->tbl[48] = 95;
		QT->tbl[49] = 98;
		QT->tbl[50] = 103;
		QT->tbl[51] = 104;
		QT->tbl[52] = 103;
		QT->tbl[53] = 62;
		QT->tbl[54] = 77;
		QT->tbl[55] = 113;
		QT->tbl[56] = 121;
		QT->tbl[57] = 112;
		QT->tbl[58] = 100;
		QT->tbl[59] = 120;
		QT->tbl[60] = 92;
		QT->tbl[61] = 101;
		QT->tbl[62] = 103;
		QT->tbl[63] = 99;
	}
	else
	{
		QT->tbl[0] = 17;
		QT->tbl[1] = 18;
		QT->tbl[2] = 18;
		QT->tbl[3] = 24;
		QT->tbl[4] = 21;
		QT->tbl[5] = 24;
		QT->tbl[6] = 47;
		QT->tbl[7] = 26;
		QT->tbl[8] = 26;
		QT->tbl[9] = 47;
		QT->tbl[10] = 99;
		QT->tbl[11] = 66;
		QT->tbl[12] = 56;
		QT->tbl[13] = 66;
		QT->tbl[14] = 99;
		QT->tbl[15] = 99;
		QT->tbl[16] = 99;
		QT->tbl[17] = 99;
		QT->tbl[18] = 99;
		QT->tbl[19] = 99;
		QT->tbl[20] = 99;
		QT->tbl[21] = 99;
		QT->tbl[22] = 99;
		QT->tbl[23] = 99;
		QT->tbl[24] = 99;
		QT->tbl[25] = 99;
		QT->tbl[26] = 99;
		QT->tbl[27] = 99;
		QT->tbl[28] = 99;
		QT->tbl[29] = 99;
		QT->tbl[30] = 99;
		QT->tbl[31] = 99;
		QT->tbl[32] = 99;
		QT->tbl[33] = 99;
		QT->tbl[34] = 99;
		QT->tbl[35] = 99;
		QT->tbl[36] = 99;
		QT->tbl[37] = 99;
		QT->tbl[38] = 99;
		QT->tbl[39] = 99;
		QT->tbl[40] = 99;
		QT->tbl[41] = 99;
		QT->tbl[42] = 99;
		QT->tbl[43] = 99;
		QT->tbl[44] = 99;
		QT->tbl[45] = 99;
		QT->tbl[46] = 99;
		QT->tbl[47] = 99;
		QT->tbl[48] = 99;
		QT->tbl[49] = 99;
		QT->tbl[50] = 99;
		QT->tbl[51] = 99;
		QT->tbl[52] = 99;
		QT->tbl[53] = 99;
		QT->tbl[54] = 99;
		QT->tbl[55] = 99;
		QT->tbl[56] = 99;
		QT->tbl[57] = 99;
		QT->tbl[58] = 99;
		QT->tbl[59] = 99;
		QT->tbl[60] = 99;
		QT->tbl[61] = 99;
		QT->tbl[62] = 99;
		QT->tbl[63] = 99;
	}
}

/* ------------
 jpg_GetHuffmanTree - gets the huffman trees under this DHT - returns the number of HT's successfully loaded
 				- if bPlacement != -1, then loads all subsequent tables into bPlacement for whatever type of table is in the stream
------------ */
static uint8_t jpg_GetHuffmanTree( bitstream_t *bs, jpeg_t *jpeg, int8_t bPlacement )
{
	uint16_t sLen = bs_Get16Bits(bs);
	uint16_t iOffset, b, c;
	uint8_t a;
	uint8_t bNumCodes[17];
	uint8_t bCodesAvail[17];
	uint8_t *bCodes[17];
	uint8_t bIndex, bType;

	// get memory for the codes
	memset( bCodes, 0, sizeof(uint8_t*) * 17 );
	for( a = 0 ; a < 17 ; a++ )
	{
		bCodes[a] = (uint8_t*)mem_alloc( sizeof(uint8_t) * 256 );
		if( bCodes[a] == NULL )
		{
			con_Print( "<RED>JPEG Load Failed:  Memory Allocation Failed for Codes at level %d", a );
			for( b = 0 ; b < 17 ; b++ ) SAFE_RELEASE( bCodes[b] );
			return 0;
		}
		memset( bCodes[a], 0, sizeof(uint8_t) * 256 );
	}

	// get all the trees in this tag
	for( iOffset = 2, a = 0 ; iOffset < sLen ; a++ )
	{
		// get the info byte and find an index
		bIndex = bs_Get8Bits(bs);
		bType = (bIndex  >> 4) & 0x01;
		bIndex = bIndex & 0x0F;
		iOffset++;
		if( bIndex > 3 )
		{
			con_Print( "<RED>JPEG Load Error:  Get Huffman Tree: index out of bounds." );
			for( b = 0 ; b < 17 ; b++ ) SAFE_RELEASE( bCodes[b] );
			return 0;
		}

		// if we need to place a single table somewhere specific, modify index accordingly
		if( bPlacement > -1 && bPlacement < 4 ) bIndex = bPlacement;

		// get the number of codes in each level of this tree
		bNumCodes[0] = 0;
		for( b = 1 ; b < 17 ; b++ )
		{
			bNumCodes[b] = bs_Get8Bits(bs);
			iOffset++;
		}

		// read in all the codes for this tree from file
		for( b = 1 ; b < 17 ; b++ )
		{
			for( c = 0 ; c < bNumCodes[b] ; c++ )
			{
				bCodes[b][c] = bs_Get8Bits(bs);
				iOffset++;
			}
		}

		// copy the current to available
		memcpy( bCodesAvail, bNumCodes, sizeof(uint8_t) * 17 );

		// get the root node
		if( !bType ) jpeg->HT_DC_Trans[bIndex] = jpg_ht_GetNode( 0, bNumCodes, bCodesAvail, bCodes );
		else jpeg->HT_AC_Trans[bIndex] = jpg_ht_GetNode( 0, bNumCodes, bCodesAvail, bCodes );
		
		// clear the table
		for( b = 0 ; b < 17 ; b++ )
		{
			for( c = 0 ; c < 256 ; c++ )
			{
				bCodes[b][c] = 0;
			}
		}

		// clear numcodes and codesAvail
		memset( bNumCodes, 0, sizeof(uint8_t) * 17 );
		memset( bCodesAvail, 0, sizeof(uint8_t) * 17 );
	}

	// remove the table memory
	for( b = 0 ; b < 17 ; b++ ) SAFE_RELEASE( bCodes[b] );

	return a;
}

/* ------------
 jpg_ht_GetNode - does the tree generation: checks to see if a value is available for this node, if avail, sets this node
 					- to that value then returns, else seeks nodes on a lower level to assign
------------ */
static HT_node_t *jpg_ht_GetNode( uint8_t bLevel, uint8_t bNumCodes[], uint8_t bCodesAvail[], uint8_t **bCodes )
{
	HT_node_t *node;

	if( bLevel > 16 ) return NULL;

	// get a new node
	node = (HT_node_t*)mem_alloc( sizeof(HT_node_t) );
	if( node == NULL )
	{
		con_Print( "<RED>JPEG Load Failed:  Memory Allocation failed for Huffman Node at level %d", bLevel );
		return NULL;
	}
	memset( node, 0, sizeof(HT_node_t) );

	// fill in remaining nodes from this level
	if( bCodesAvail[bLevel] > 0 )
	{
		node->bValue = bCodes[bLevel][bNumCodes[bLevel] - bCodesAvail[bLevel]];
		bCodesAvail[bLevel]--;
		node->bIsLeaf = 1;
		return node;
	}

	// otherwise, recurse
	node->left = jpg_ht_GetNode( (uint8_t)(bLevel + (uint8_t)1), bNumCodes, bCodesAvail, bCodes );
	node->right = jpg_ht_GetNode( (uint8_t)(bLevel + (uint8_t)1), bNumCodes, bCodesAvail, bCodes );

	if( node->left == NULL && node->right == NULL ) node->bIsLeaf = 1;

	return node;
}

/* ------------
jpg_GetDefaultHuffmanTree - bComponent: Y=1, C=2,3(,4);  bType: 0=DC, 1=AC
------------ */
static HT_node_t *jpg_GetDefaultHuffmanTree( uint8_t bComponent, uint8_t bType, jpeg_t *jpeg )
{
	CHAR *path;
	CHAR cName[64];
	file_t *fTbl;
	bitstream_t *bs;
	uint8_t bRet;
	uint8_t a;
	int8_t bTblIndex;

	// clear the name buffer
	memset( cName, 0, 64 *sizeof(CHAR) );

	// get the file name
	if( bComponent == 1 && bType == 0 )
	{
		tcstrcpy( cName, "jpeg_y_dc.tbl" );
	}
	else if( bComponent > 1 && bComponent < 5 && bType == 0 )
	{
		tcstrcpy( cName, "jpeg_c_dc.tbl" );
	}
	else if( bComponent == 1 && bType == 1 )
	{
		tcstrcpy( cName, "jpeg_y_ac.tbl" );
	}
	else if( bComponent > 1 && bComponent < 5 && bType == 1 )
	{
		tcstrcpy( cName, "jpeg_c_ac.tbl" );
	}
	else
	{
		con_Print( "<RED>JPEG Load Error:  Get Default Huffman Tree -> invalid index" );
		return NULL;
	}

	// get the full path
	path = fs_GetMediaPath( cName, TABLE_DIRECTORY );
	if( path == NULL )
	{
		con_Print( "<RED>JPEG Load Error:  Get Default Huffman Tree could not resolve a full path for table %s", cName );
		return NULL;
	}

	// open the file and a bitstream
	fTbl = fs_open( cName, "r" );
	if( fTbl == NULL )
	{
		con_Print( "<RED>JPEG Load Error:  Get Default Huffman Tree -> unable to open %s for read.", path );
		return NULL;
	}
	bs = bs_GetBitStream( fTbl );
	if( bs == NULL )
	{
		con_Print( "<RED>JPEG Load Error:  Get Default Huffman Tree -> unable to open a bitstream on %s", path );
		fs_close(fTbl);
		return NULL;
	}

	// find an available empty table spot for the appropriate coefficient type
	bTblIndex = -1;
	if( bType == 0 )
	{
		for( a = 0 ; a < 4 ; a++ )
		{	
			if( jpeg->HT_DC_Trans[a] == NULL ) bTblIndex = a;
		}
	}
	else
	{
		for( a = 0 ; a < 4 ; a++ )
		{	
			if( jpeg->HT_AC_Trans[a] == NULL ) bTblIndex = a;
		}
	}

	// check for an error
	if( a >= 4 )
	{
		con_Print( "<RED>JPEG Load Error:  Load Default Huffman Tree:  no empty trees available!" );
		bs_CloseStream(bs);
		fs_close(fTbl);
		return NULL;
	}

	// call the table loader
	bRet = jpg_GetHuffmanTree( bs, jpeg, bTblIndex );

	// close the file
	bs_CloseStream(bs);
	fs_close(fTbl);

	// check for an error on load
	if( bType == 0 && jpeg->HT_DC_Trans[ bTblIndex ] == NULL )
	{
		con_Print( "<RED>JPEG Load Error:  Get Huffman Tree failed for DC default tree." );
		return NULL;
	}
	else if( jpeg->HT_AC_Trans[ bTblIndex ] )
	{
		con_Print( "<RED>JPEG Load Error:  Get Huffman Tree failed for AC default tree." );
		return NULL;
	}

	// return the appropriate new tree
	if( bType == 0 )
	{
		return jpeg->HT_DC_Trans[ bTblIndex ];
	}
	else
	{
		return jpeg->HT_AC_Trans[ bTblIndex ];
	}
}

/* ------------
jpg_load_FreeHuffmanTrees - frees all huffman trees
------------ */
static void jpg_FreeHuffmanTrees( jpeg_t *jpeg )
{
	uint8_t a;

	for( a = 0 ; a < 4 ; a++ )
	{
		if( jpeg->HT_DC_Trans[a] != NULL ) 
		{
			jpg_FreeHTNode( jpeg->HT_DC_Trans[a] );
			jpeg->HT_DC_Trans[a] = NULL;
		}
		if( jpeg->HT_AC_Trans[a] != NULL ) 
		{
			jpg_FreeHTNode( jpeg->HT_AC_Trans[a] );
			jpeg->HT_DC_Trans[a] = NULL;
		}
	}
}
static void jpg_FreeHTNode( HT_node_t *node )
{
	if( node->left != NULL ) jpg_FreeHTNode( node->left );
	if( node->right != NULL ) jpg_FreeHTNode( node->right );
	SAFE_RELEASE( node );
}

/* ------------
jpg_GetHuffmanValue - returns 1 if huffman value exists, otherwise 0, if the value exists, HDeCode is set to the value
------------ */
static uint8_t jpg_GetHuffmanValue( HT_node_t *node, uint16_t HCode, uint8_t bNumBits, uint8_t *HDeCode )
{
	// check for leaf conditions when we reach the end of our bits
	if( bNumBits == 0 ) // numbits unsigned
	{
		if( node->bIsLeaf )
		{
			*HDeCode = node->bValue;
			return 1;
		}
		return 0;
	}

	// check against the leftmost bit for the next descend
	bNumBits--;
	if( (HCode >> bNumBits) & 0x00000001 ) return jpg_GetHuffmanValue( node->right, HCode, bNumBits, HDeCode );
	else return jpg_GetHuffmanValue( node->left, HCode, bNumBits, HDeCode );	
}

/* ------------
jpg_load_GetSOF - loads the SOF info to sof from bs; returns 0 on fail non-0 on success
------------ */
static uint8_t jpg_GetSOF( jpeg_t *jpeg )
{
	bitstream_t *bs = jpeg->bs;
	uint8_t a;
	uint8_t bCompID;
	uint8_t bSampFact;
	
	bs_Get16Bits(bs); // dump the length of segment since it's not needed
	jpeg->header.bPrecis = bs_Get8Bits(bs);
	jpeg->sImageY = bs_Get16Bits(bs);
	jpeg->sImageX = bs_Get16Bits(bs);
	jpeg->bNumComp = bs_Get8Bits(bs);

	// check for jpegs that we can't decode
	if( jpeg->header.bPrecis != 8 )
	{
		con_Print( "<RED>JPEG Load Error:  file has data precision of %d bits.  We can only handle 8 bits.", jpeg->header.bPrecis );
		return 0;
	}
	if( jpeg->sImageY == 0 )
	{
		con_Print( "<RED>JPEG Load Error:  Image Height (in MCU's) not found in the SOF tag." );
		return 0;
	}
	if( jpeg->sImageX == 0 )
	{
		con_Print( "<RED>JPEG Load Error:  Image Width (in MCU's) not found in the SOF tag." );
		return 0;
	}

	for( a = 0 ; a < jpeg->bNumComp ; a++ )
	{
		// get and test the ID
		bCompID = bs_Get8Bits(bs);
		if( bCompID > 3 )
		{
			con_Print( "<RED>JPEG Load Error:  Get Start of Frame (SOF) found a component with ID > 3" );
			return 0;
		}
		jpeg->comp[bCompID-1].bID = bCompID;

		// get and decode the sampling factors
		bSampFact = bs_Get8Bits(bs);
		jpeg->comp[bCompID-1].bSampFactorVert = bSampFact & 0x0F;
		jpeg->comp[bCompID-1].bSampFactorHoriz = bSampFact >> 4;

		// get and test the qt table number
		jpeg->comp[bCompID-1].bQTTblNum = bs_Get8Bits(bs);
		if( jpeg->comp[bCompID-1].bQTTblNum > 3 )
		{
			con_Print( "<RED>JPEG Load Error:  Get Start of Frame (SOF) found a quantization table index out of range." );
			return 0;
		}
	}	

	return 1;
}

/* ------------
jpg_load_GetSOS - loads the SOS info to sos from bs; returns 0 on fail non-0 on success
------------ */
static uint8_t jpg_GetSOS( jpeg_t *jpeg )
{
	bitstream_t *bs = jpeg->bs;
	uint16_t a;

	// the size of this block	
	bs_Get16Bits(bs); // length

	// check the components to make sure we're still sane
	if( bs_Get8Bits(bs) != jpeg->bNumComp ) // # of components
	{
		con_Print( "<RED>JPEG Load Error:  SOS number of components != to jpeg number of components" );
		return 0;
	}
	
	for( a = 0 ; a < jpeg->bNumComp ; a++ ) 
	{
		// check for mangled ids
		if( bs_Get8Bits(bs) != jpeg->comp[a].bID ) // component ID
		{
			con_Print( "<RED>JPEG Load Error:  The Quantization Tables have mangled ordering with respect to the Huffman Trees." );
			return 0;
		}

		// get the tree indicies
		jpeg->comp[a].bHTTreeNumDC = bs_Get8Bits(bs); // huffman tbl to use
		jpeg->comp[a].bHTTreeNumAC = jpeg->comp[a].bHTTreeNumDC & 0x0F;
		jpeg->comp[a].bHTTreeNumDC = jpeg->comp[a].bHTTreeNumDC >> 4;

		// bounds check these
		if( jpeg->comp[a].bHTTreeNumDC > 3 )
		{
			con_Print( "<RED>JPEG Load Error:  The Huffman Tree DC Index is out of bounds" );
			return 0;
		}
		if( jpeg->comp[a].bHTTreeNumAC > 3  )
		{
			con_Print( "<RED>JPEG Load Error:  The Huffman Tree AC Index is out of bounds" );
			return 0;
		}

	}

	// a block of ignores?
	bs_Get8Bits(bs); // ignore?
	bs_Get8Bits(bs); // ignore?
	bs_Get8Bits(bs); // ignore?

	return 1;
}

/* ------------
jpg_load_PrepareDecoder - returns 0 on fail, non-0 on success;
					- gets the MCU size and allocates for image transfer and output data
------------ */
static uint8_t jpg_PrepareDecoder( jpeg_t *jpeg )
{
	uint32_t a, b;
	uint32_t iXferSize;

	// prepare the runtime prebuffer/filter
	memset( jpeg->bBuffer, 0, sizeof(uint8_t) * JPEG_BUFFER_SIZE );
	jpeg->bBufNeedsInit = 1;

	// get the minimum compressed unit in 8x8 blocks
	// this will define the required upscale for each image plane
	jpeg->bMaxSampFactY = 0;
	jpeg->bMaxSampFactX = 0;
	for( a = 0 ; a < jpeg->bNumComp ; a++ )
	{
		if( jpeg->comp[a].bSampFactorVert > jpeg->bMaxSampFactY ) jpeg->bMaxSampFactY = jpeg->comp[a].bSampFactorVert;
		if( jpeg->comp[a].bSampFactorHoriz > jpeg->bMaxSampFactX ) jpeg->bMaxSampFactX = jpeg->comp[a].bSampFactorHoriz;
	}
	if( jpeg->bMaxSampFactX == 0 || jpeg->bMaxSampFactX == 0 )
	{
		con_Print( "<RED>JPEG Load Error:  Computed MCU is 0" );
		return 0;
	}

	// compute the stored image sX and sY (the size for all of our pre-output computations)
	// sX/Y are the divisible by 8 size of the image
	// sImageX/Y should be the actual (source/output) image size already, or we're out of business here
	jpeg->sX = jpeg->sImageX; // this is slow, but I understand it
	while( jpeg->sX % (8 * jpeg->bMaxSampFactX) != 0 ) jpeg->sX++;
	jpeg->sY = jpeg->sImageY;
	while( jpeg->sY % (8 * jpeg->bMaxSampFactY) != 0 ) jpeg->sY++;
	jpeg->sMCUWidth =  jpeg->sX / (8 * jpeg->bMaxSampFactX);
	jpeg->sMCUHeight = jpeg->sY / (8 * jpeg->bMaxSampFactX);
	jpeg->iNumMCUs = jpeg->sMCUWidth * jpeg->sMCUHeight;

	// compute the spans for rows on MCU's and units for each component
	for( a = 0 ; a < jpeg->bNumComp ; a++ )
	{
		// pixels in component plane
		jpeg->comp[a].sX = jpeg->comp[a].bSampFactorHoriz * jpeg->sMCUWidth * 8;
		jpeg->comp[a].sY = jpeg->comp[a].bSampFactorVert * jpeg->sMCUHeight * 8;

		// spans in X
		jpeg->comp[a].iUnitSpanX = 8;
		jpeg->comp[a].iMCUSpanX = 8 * jpeg->comp[a].bSampFactorHoriz;

		// spans in Y
		jpeg->comp[a].iUnitSpanY = (jpeg->comp[a].sX * 8) - jpeg->comp[a].iMCUSpanX;
		jpeg->comp[a].iMCUSpanY = (jpeg->comp[a].sX * 8 * jpeg->comp[a].bSampFactorVert) - jpeg->comp[a].sX;

		// the delta row span
		jpeg->comp[a].sRowSpan = jpeg->comp[a].sX - 8;
	}

	// compute the upscale:  the max sampFace / current sampFace
	for( a = 0 ; a < jpeg->bNumComp ; a++ )
	{
		jpeg->comp[a].bUpscaleX = jpeg->bMaxSampFactX / jpeg->comp[a].bSampFactorHoriz;
		jpeg->comp[a].bUpscaleY = jpeg->bMaxSampFactY / jpeg->comp[a].bSampFactorVert;
	}

	// allocate the final output plane
	iXferSize = sizeof(uint8_t) * jpeg->sImageX * jpeg->sImageY * jpeg->bNumComp;
	jpeg->imgPlane = (uint8_t*)mem_alloc( iXferSize );
	if( jpeg->imgPlane == NULL )
	{
		con_Print( "<RED>JPEG Load Error:  memory allocation failed for final Image plane allocation in Prepare Decoder for %d bytes.", iXferSize );
		return 0;
	}
	memset( jpeg->imgPlane, 0, iXferSize );

	// allocate the component transfer planes
	for( a = 0 ; a < jpeg->bNumComp ; a++ )
	{
		iXferSize = sizeof(uint8_t) * jpeg->comp[a].sX * jpeg->comp[a].sY;
		jpeg->comp[a].bpPlane = (uint8_t*)mem_alloc( iXferSize );
		if( jpeg->comp[a].bpPlane == NULL )
		{
			con_Print( "<RED>JPEG Load Error: memory allocation failed for component transfer plane for component ID: %d", jpeg->comp[a].bID );
			SAFE_RELEASE( jpeg->imgPlane );
			return 0;
		}
		memset( jpeg->comp[a].bpPlane, 0, iXferSize );
	}

	// allocate the component image processing planes
	for( a = 0 ; a < jpeg->bNumComp ; a++ )
	{
		iXferSize = sizeof(uint8_t) * jpeg->sX * jpeg->sY;
		jpeg->comp[a].bpOutPlane = (uint8_t*)mem_alloc( iXferSize );
		if( jpeg->comp[a].bpOutPlane == NULL )
		{
			con_Print( "<RED>JPEG Load Error:  memory allocation failed for component image processing plane for component ID: %d", jpeg->comp[a].bID );
			for( b = 0 ; b < jpeg->bNumComp ; b++ ) SAFE_RELEASE( jpeg->comp[b].bpPlane );
			SAFE_RELEASE( jpeg->imgPlane );
			return 0;
		}
		memset( jpeg->comp[a].bpOutPlane, 0, iXferSize );
	}
	
	return 1;
}

/* ------------
jpg_Decode - do the decode with the given jpeg specs from the given bitstream
				- return 0 on fail, non-0 on success
------------ */
static uint8_t jpg_Decode( jpeg_t *jpeg )
{
	uint16_t a, b;
	uint8_t c, d;
	uint8_t e, f, g;
	uint8_t bTmp;
	uint16_t cnt;
	int32_t iDC[3];
	uint16_t sRowSpan;
	uint8_t *bpMCU[3];
	uint8_t *bpUnit;
	uint8_t *bpSegment;
	uint8_t bOutUnit[64]; // the final output unit
	uint16_t sMCUCount = 0;

	// clear the init DC
	memset( iDC, 0, sizeof(int32_t) * 3 );

	// init the pointers to the data base
	bpMCU[0] = jpeg->comp[0].bpPlane;
	bpMCU[1] = jpeg->comp[1].bpPlane;
	bpMCU[2] = jpeg->comp[2].bpPlane;

	// for set of MCU's across
	for( a = 0 ; a < jpeg->sMCUHeight ; a++ )
	{
		// for each MCU
		for( b = 0 ; b < jpeg->sMCUWidth ; b++ )
		{
			// for each color component
			for( c = 0 ; c < jpeg->bNumComp ; c++ )
			{
				// set the starting unit pointer to the current base mcu pointer
				bpUnit = bpMCU[c];
					
				// there are SampFactorVert * SampFactorHoriz units in this mcu
				for( d = 0 ; d < jpeg->comp[c].bSampFactorVert ; d++ )
				{
					for( e = 0 ; e < jpeg->comp[c].bSampFactorHoriz ; e++ )
					{
						// clear output buffer
						memset( bOutUnit, 0, sizeof(uint8_t) * 64 );

						// fill the jpeg buffer
						jpg_FillBuffer( jpeg );
						
						// get the next unit
						if( !jpg_DecodeUnit( jpeg, c, iDC, bOutUnit ) ) return 0;

						// copy the unit to the iterator
						bpSegment = bpUnit;
						sRowSpan = jpeg->comp[c].sRowSpan;
						cnt = 0;
						for( f = 0 ; f < 8 ; f++ )
						{
							for( g = 0 ; g < 8 ; g++ )
							{
								*bpSegment = bOutUnit[cnt];
								cnt++;
								bpSegment++;
							}
							bpSegment += sRowSpan;
						}

						bpUnit += jpeg->comp[c].iUnitSpanX;
					}

					bpUnit += jpeg->comp[c].iUnitSpanY;
				}
				
			}

			// advance the MCU counter and check for an RST if needed
			// make sure to skip the check after the last MCU is read in
			sMCUCount++;
			if( (jpeg->sDRI) && (sMCUCount % jpeg->sDRI == 0) && ((sMCUCount + 1UL) != jpeg->iNumMCUs) ) 
			{
				// advance to the next byte alignment
				while( jpeg->pBit != 0 )
				{
					if( !jpg_GetBit(jpeg) )
					{
						con_Print( "<RED>JPEG Load Warning: Pad Prior to RST is not all 1's" ); 
					}
				}
				
				// read the RST marker
				bTmp = 0;
				for( c = 7 ; c < 255 ; c-- ) // don't do this at home, kids
				{
					bTmp |= jpg_GetBit(jpeg) << c;
				}
				if( bTmp != 0xFF )
				{
					// something went horribly wrong
					con_Print( "<RED>JPEG Load Failed:  Expected RST tag, found non-0xFF at read time:  0x%X", bTmp );
					return 0;
				}

				// read the marker number
				bTmp = 0;
				for( c = 7 ; c < 255 ; c-- )
				{
					bTmp |= jpg_GetBit(jpeg) << c;
				}
				if( bTmp < 0xD0 || bTmp > 0xD7 )
				{
					con_Print( "<RED>JPEG Load Failed:  Expected RST tag to have Number from D0 to D7, found %X", bTmp ); 
					return 0;
				}

				// con_Print( "Miraculously found the RST in the right place: %X, %d", bTmp, sMCUCount );
			}

			// advance 1 MCU column
			bpMCU[0] += jpeg->comp[0].iMCUSpanX;
			bpMCU[1] += jpeg->comp[1].iMCUSpanX;
			bpMCU[2] += jpeg->comp[2].iMCUSpanX;			
		}

		// span 1 row of mcu down for each component
		bpMCU[0] += jpeg->comp[0].iMCUSpanY;
		bpMCU[1] += jpeg->comp[1].iMCUSpanY;
		bpMCU[2] += jpeg->comp[2].iMCUSpanY;
	}

	// we now have 3 image planes in each of three components
	// we need to upscale all to the final resolution, chop the right and bottom
	// edges from multiples of 8 to the actual image size and convert them
	// from YUV to RGB format

	// first do the upscaling for each plane
	for( a = 0 ; a < jpeg->bNumComp ; a++ )
	{
		jpg_Upscale( jpeg, (uint8_t)a );
	}

	// throw together the components onto an RGB plane
	jpg_ComponentsToOutput( jpeg );

	// invert the output plane
	jpg_Invert( jpeg );

	return 1;
}

/* ------------
jpg_DecodeUnit - brings out and expands one chunk of compressed data (8x8) of a single color
			- fDC is 3 floats or longer, bOutUnit is 64 bytes
------------ */
static uint8_t jpg_DecodeUnit( jpeg_t *jpeg, uint8_t comp, int32_t *iDC, uint8_t *bOutUnit )
{
	uint8_t a;
	int16_t b; 
	uint8_t bRet;
	uint8_t bEOB;
	uint8_t bHDecode;
	uint8_t bCategory;
	uint8_t bNumPrevZero;
	uint8_t bAC_Cnt;
	uint16_t HCode;
	uint16_t nBits; // category is always <= 16
	uint32_t nBitsDC; // category could be huge
	int16_t sInUnit[64];
	int16_t sDeZigUnit[64];

	// get the DC value
	a = 0; // [1,16] -> 16 total bits
	HCode = 0;
	do 
	{
		a++;
		HCode = (HCode << 1) |jpg_GetBit(jpeg);
		bRet = jpg_GetHuffmanValue( jpeg->comp[comp].htDC, HCode, a, &bCategory );
	} while( a < 17 && !bRet );

	// check for the error
	if( !bRet ) return 0;

	// get the n bits indicated
	b = (int16_t)bCategory;
	nBitsDC = 0;
	while( b > 0 )
	{
		b--;
		nBitsDC |= (uint32_t)jpg_GetBit(jpeg) << b;
	}
	
	// special unsigned JPEG representation to a signed float
	sInUnit[0] =  (int16_t)(jpg_MinBitsTransform( bCategory, nBitsDC ) + iDC[comp]);
	iDC[comp] = (int32_t)sInUnit[0]; // store i-1 for next i

	// get the AC values
	bAC_Cnt = 1;
	bEOB = 0;
	while( !bEOB && bAC_Cnt < 64 )
	{
		// fetch a valid huffman code from the AC table
		a = 0;
		HCode = 0;
		bHDecode = 0;
		do
		{
			a++;
			HCode = (HCode << 1) | jpg_GetBit(jpeg);
			bRet = jpg_GetHuffmanValue( jpeg->comp[comp].htAC, HCode, a, &bHDecode ); 
		} while( a < 17 && !bRet );

		// check for the error
		if( !bRet ) return 0;

		// check for EOB
		if( !bHDecode )
		{
			bEOB = 1;
		}
		else
		{
			// break up the fetched byte
			bNumPrevZero = bHDecode >> 4;
			bCategory = bHDecode & 0x0F;

			// put in the previous zeros
			while( bNumPrevZero > 0 )
			{
				if( bAC_Cnt > 62 )
				{
					con_Print( "<RED>JPEG Decode Error: Encountered weird data -> bNumPrevZero Counted Past Input Unit." );
					con_Print( "<RED>\tbNumPrevZero: %d, bCategory %d, bHDecode: 0x%x, bHCode: 0x%x", bNumPrevZero, bCategory, bHDecode, HCode );
					return 0;
				}
				sInUnit[bAC_Cnt] = 0;
				bNumPrevZero--;
				bAC_Cnt++;
			}

			// get the n bits indicated
			b = (int16_t)bCategory;
			nBits = 0;
			while( b > 0 )
			{
				b--;
				nBits |= (uint16_t)jpg_GetBit(jpeg) << b;
			}

			// special unsigned JPEG representation to a signed float
			sInUnit[bAC_Cnt] = (int16_t)jpg_MinBitsTransform( bCategory, nBits );

			// count forward an AC_cnt
			bAC_Cnt++;
		}		
	}

	// now we need to fill in the back from EOB to 64
	while( bAC_Cnt < 64 )
	{
		sInUnit[bAC_Cnt] = 0;
		bAC_Cnt++;
	}

	// we now have a zig-zagged array of 64 samples, DC and AC for component comp

	// dezigzag the input samples
	jpg_DeZigZagUnit( sDeZigUnit, sInUnit );

	// idct and dequantization, level shift and range limiting
	jpg_IDCT( jpeg, comp, sDeZigUnit, bOutUnit );

	return 1;
}


/* ------------
jpg_MinBitsTransform - converts a number of bCategory from the special JPEG representation 
				- to a 2's compliment signed 32bit integer
------------ */
static int32_t jpg_MinBitsTransform( uint8_t bCategory, uint32_t nBits )
{
	int32_t iOut;
	
	// the jpeg standard encodes bits in a least bit's pattern
	if( !bCategory ) return 0;

	// if the high bit is 0, the number is negative: in form of -(2^n - 1) 
	if( !((nBits >> (bCategory - 1)) & 0x01) )
	{
		// this signed transform is okay because the high bit == 0 by def.
		iOut = (int32_t)nBits - ( (0x01L << bCategory) - 1L);
	}
	else
	{
		iOut = (int32_t)nBits;
	}

	return iOut;
}


// thanks gratefully given to the IJG for the math to do this quickly
// Quantization, IDCT, Level Shift
static void jpg_IDCT( jpeg_t *jpeg, uint8_t comp, int16_t *inBlock, uint8_t *outBlock )
{
	float ft0, ft1, ft2, ft3, ft4, ft5, ft6, ft7;
	float ft10, ft11, ft12, ft13;
	float z5, z10, z11, z12, z13;
	int16_t *spIn = inBlock;
	uint8_t *bpOut = outBlock;
	float *fpQT = jpeg->comp[comp].qt.tbl;
	float fXfer[64];
	float *fpXfer = fXfer;
	int32_t a;	
	int32_t iTmp;

	// Pass 1: columns from in to work
	for( a = 0 ; a < 8 ; a++ )
	{
		if( 	spIn[8] == 0  && spIn[16] == 0 && spIn[24] == 0 && spIn[32] == 0 && 
			spIn[40] == 0 && spIn[48] == 0 && spIn[56] == 0 )
		{
			float fDC = (float)spIn[0] * fpQT[0];

			fpXfer[0] = fDC;
			fpXfer[8] = fDC;
			fpXfer[16] = fDC;
			fpXfer[24] = fDC;
			fpXfer[32] = fDC;
			fpXfer[40] = fDC;
			fpXfer[48] = fDC;
			fpXfer[56] = fDC;
		
			// advance
			fpQT++;
			spIn++;
			fpXfer++;
			continue;
		}

		// even: non-zero column
		ft0 = (float)spIn[0]  * fpQT[0]; 
		ft1 = (float)spIn[16] * fpQT[16];
		ft2 = (float)spIn[32] * fpQT[32];
		ft3 = (float)spIn[48] * fpQT[48];
		ft10 = ft0 + ft2;

		// phase 3	 
		ft11 = ft0 - ft2;
		ft13 = ft1 + ft3;

		// phases 5-3	
		ft12 = ((ft1 - ft3) * 1.414213562f) -ft13;
		ft0 = ft10 + ft13;

		// phase 2
		ft3 = ft10 - ft13;
		ft1 = ft11 + ft12;
		ft2 = ft11 - ft12;

		// Odd part: non-zero column	  
		ft4 = (float)spIn[8]  * fpQT[8];	
		ft5 = (float)spIn[24] * fpQT[24];
		ft6 = (float)spIn[40] * fpQT[40];
		ft7 = (float)spIn[56] * fpQT[56];
		z13 = ft6 + ft5;

		// phase 6
		z10 = ft6 - ft5;	  
		z11 = ft4 + ft7;	  
		z12 = ft4 - ft7;	  
		ft7 = z11 + z13;		

		// phase 5
		ft11 = (z11 - z13) * 1.414213562f;
		z5 = (z10 + z12) * 1.847759065f;
		ft10 = 1.082392200f * z12 - z5;
		ft12 = -2.613125930f * z10 + z5;
		ft6 = ft12 - ft7;

		// phase 2
		ft5 = ft11 - ft6;
		ft4 = ft10 + ft5;
		fpXfer[0] = ft0 + ft7;
		fpXfer[56] = ft0 - ft7;
		fpXfer[8] = ft1 + ft6;
		fpXfer[48] = ft1 - ft6;
		fpXfer[16] = ft2 + ft5;
		fpXfer[40] = ft2 - ft5;
		fpXfer[32] = ft3 + ft4;
		fpXfer[24] = ft3 - ft4;

		// advance
		fpQT++;
		spIn++;
		fpXfer++;
	}

	// Pass 2: from work array to output array
	fpXfer = fXfer;
	// TC
	bpOut = outBlock;
	for( a = 0 ; a < 8 ; a++ )
	{    
		// Even part     
		ft10 = fpXfer[0] + fpXfer[4];
		ft11 = fpXfer[0] - fpXfer[4];
		ft13 = fpXfer[2] + fpXfer[6];
		ft12 = (fpXfer[2] - fpXfer[6]) * 1.414213562f - ft13;
		ft0 = ft10 + ft13;
		ft3 = ft10 - ft13;
		ft1 = ft11 + ft12;
		ft2 = ft11 - ft12;

		// Odd part
		z13 = fpXfer[5] + fpXfer[3];
		z10 = fpXfer[5] - fpXfer[3];
		z11 = fpXfer[1] + fpXfer[7];
		z12 = fpXfer[1] - fpXfer[7];
		ft7 = z11 + z13;
		ft11 = (z11 - z13) * 1.414213562f;
		z5 = (z10 + z12) * 1.847759065f;
		ft10 = 1.082392200f * z12 - z5;
		ft12 = -2.613125930f * z10 + z5;
		ft6 = ft12 - ft7;
		ft5 = ft11 - ft6;
		ft4 = ft10 + ft5;

		// Final output stage: scale down by a factor of 8, level shift, and range-limit
		iTmp = (((int32_t)(ft0 + ft7) + 4) >> 3) + 128;
		if( iTmp < 0 ) bpOut[0] = 0;
		else if( iTmp > 255 ) bpOut[0] = 255;
		else bpOut[0] = (uint8_t)iTmp;
		
		iTmp = (((int32_t)(ft1 + ft6) + 4) >> 3) + 128;
		if( iTmp < 0 ) bpOut[1] = 0;
		else if( iTmp > 255 ) bpOut[1] = 255;
		else bpOut[1] = (uint8_t)iTmp;

		iTmp = (((int32_t)(ft2 + ft5) + 4) >> 3) + 128;
		if( iTmp < 0 ) bpOut[2] = 0;
		else if( iTmp > 255 ) bpOut[2] = 255;
		else bpOut[2] = (uint8_t)iTmp;

		iTmp = (((int32_t)(ft3 - ft4) + 4) >> 3) + 128;
		if( iTmp < 0 ) bpOut[3] = 0;
		else if( iTmp > 255 ) bpOut[3] = 255;
		else bpOut[3] = (uint8_t)iTmp;

		iTmp = (((int32_t)(ft3 + ft4) + 4) >> 3) + 128;
		if( iTmp < 0 ) bpOut[4] = 0;
		else if( iTmp > 255 ) bpOut[4] = 255;
		else bpOut[4] = (uint8_t)iTmp;

		iTmp = (((int32_t)(ft2 - ft5) + 4) >> 3) + 128;
		if( iTmp < 0 ) bpOut[5] = 0;
		else if( iTmp > 255 ) bpOut[5] = 255;
		else bpOut[5] = (uint8_t)iTmp;

		iTmp = (((int32_t)(ft1 - ft6) + 4) >> 3) + 128;
		if( iTmp < 0 ) bpOut[6] = 0;
		else if( iTmp > 255 ) bpOut[6] = 255;
		else bpOut[6] = (uint8_t)iTmp;

		iTmp = (((int32_t)(ft0 - ft7) + 4) >> 3) + 128;
		if( iTmp < 0 ) bpOut[7] = 0;
		else if( iTmp > 255 ) bpOut[7] = 255;
		else bpOut[7] = (uint8_t)iTmp;

		// advance
		fpXfer += 8;
		bpOut += 8;
	}
	
}

/* ------------
jpg_DeZigZagUnit - takes in 2 64 elmt arrays of bytes and does the jpeg de-zig-zag to make
				- the in array into the proper out array
------------ */
static void jpg_DeZigZagUnit( uint16_t *bOutUnit, uint16_t *bInUnit )
{
	bOutUnit[0] = bInUnit[0];
	bOutUnit[1] = bInUnit[1];
	bOutUnit[2] = bInUnit[5];
	bOutUnit[3] = bInUnit[6];
	bOutUnit[4] = bInUnit[14];
	bOutUnit[5] = bInUnit[15];
	bOutUnit[6] = bInUnit[27];
	bOutUnit[7] = bInUnit[28];
	bOutUnit[8] = bInUnit[2];
	bOutUnit[9] = bInUnit[4];
	bOutUnit[10] = bInUnit[7];
	bOutUnit[11] = bInUnit[13];
	bOutUnit[12] = bInUnit[16];
	bOutUnit[13] = bInUnit[26];
	bOutUnit[14] = bInUnit[29];
	bOutUnit[15] = bInUnit[42];
	bOutUnit[16] = bInUnit[3];
	bOutUnit[17] = bInUnit[8];
	bOutUnit[18] = bInUnit[12];
	bOutUnit[19] = bInUnit[17];
	bOutUnit[20] = bInUnit[25];
	bOutUnit[21] = bInUnit[30];
	bOutUnit[22] = bInUnit[41];
	bOutUnit[23] = bInUnit[43];
	bOutUnit[24] = bInUnit[9];
	bOutUnit[25] = bInUnit[11];
	bOutUnit[26] = bInUnit[18];
	bOutUnit[27] = bInUnit[24];
	bOutUnit[28] = bInUnit[31];
	bOutUnit[29] = bInUnit[40];
	bOutUnit[30] = bInUnit[44];
	bOutUnit[31] = bInUnit[53];
	bOutUnit[32] = bInUnit[10];
	bOutUnit[33] = bInUnit[19];
	bOutUnit[34] = bInUnit[23];
	bOutUnit[35] = bInUnit[32];
	bOutUnit[36] = bInUnit[39];
	bOutUnit[37] = bInUnit[45];
	bOutUnit[38] = bInUnit[52];
	bOutUnit[39] = bInUnit[54];
	bOutUnit[40] = bInUnit[20];
	bOutUnit[41] = bInUnit[22];
	bOutUnit[42] = bInUnit[33];
	bOutUnit[43] = bInUnit[38];
	bOutUnit[44] = bInUnit[46];
	bOutUnit[45] = bInUnit[51];
	bOutUnit[46] = bInUnit[55];
	bOutUnit[47] = bInUnit[60];
	bOutUnit[48] = bInUnit[21];
	bOutUnit[49] = bInUnit[34];
	bOutUnit[50] = bInUnit[37];
	bOutUnit[51] = bInUnit[47];
	bOutUnit[52] = bInUnit[50];
	bOutUnit[53] = bInUnit[56];
	bOutUnit[54] = bInUnit[59];
	bOutUnit[55] = bInUnit[61];
	bOutUnit[56] = bInUnit[35];
	bOutUnit[57] = bInUnit[36];
	bOutUnit[58] = bInUnit[48];
	bOutUnit[59] = bInUnit[49];
	bOutUnit[60] = bInUnit[57];
	bOutUnit[61] = bInUnit[58];
	bOutUnit[62] = bInUnit[62];
	bOutUnit[63] = bInUnit[63];
}



/* ------------
jpg_Upscale
//upscale the component plane to an image size plane, applying software blending 
------------ */
static uint8_t jpg_Upscale( jpeg_t *jpeg, uint8_t comp )
{
	uint32_t a, b, c, d;
	uint32_t ix, iy, ox, oy, cx, cy, usx, usy, tx, ty;
	uint8_t *inPlane, *outPlane;
	
	// don't bother processing images that are already at output size, just copy over
	if( jpeg->comp[comp].bUpscaleX == 1 && jpeg->comp[comp].bUpscaleY == 1 )
	{
		memcpy( jpeg->comp[comp].bpOutPlane, jpeg->comp[comp].bpPlane, sizeof(uint8_t) * jpeg->comp[comp].sX * jpeg->comp[comp].sY );
		return 1;
	}

	// these are constant across the component and used in inner loops	
	iy =  jpeg->comp[comp].sY;
	ix = jpeg->comp[comp].sX;
	oy = jpeg->sY;
	ox = jpeg->sX;
	usy = jpeg->comp[comp].bUpscaleY;
	usx = jpeg->comp[comp].bUpscaleX;
	inPlane = jpeg->comp[comp].bpPlane;
	outPlane = jpeg->comp[comp].bpOutPlane;

	// for each row of this image
	for( a = 0 ; a < iy ; a++ )
	{
		// the output row is the number of rows per component * the current input row
		cy = usy * a;

		// for each elmt in this image
		// subelmts for a * compX + b
		for( b = 0 ; b < ix ; b++ )
		{
			// the output column in the number of columns per component * the current input column
			cx = usx * b;
				
			for( c = 0 ; c < usy ; c++ )
			{
				for( d = 0 ; d < usx ; d++ )
				{
					// compute relative rows
					tx = cx + d;
					ty = cy + c;

					// write the input color to the output buffer
					*(outPlane + ty * ox + tx) = *(inPlane + a * ix + b);
				}
			}
		}
	}

	return 1;
}


/* ------------
jpg_ComponentsToOutput
// copies the YUV component planes to a final output plane in RGB, clipped to the proper size
------------ */
static uint8_t jpg_ComponentsToOutput( jpeg_t *jpeg )
{
	uint16_t a, b;
	int16_t sXfer[3];
	uint8_t bXfer[3];
	uint16_t sX = jpeg->sX, sY = jpeg->sY;
	uint16_t sImageX = jpeg->sImageX, sImageY = jpeg->sImageY;
	uint8_t *bpOut = jpeg->imgPlane;
	uint8_t *bpIn;
	uint32_t iTmp;

	// if we only have a 1 color image the copy can proceed quickly
	if( jpeg->bNumComp <= 1 )
	{
		bpIn = jpeg->comp[0].bpOutPlane;

		// if the input and output have the same number of pixels, we can accerate this dramatically
		if( sImageX == sX && sImageY == sY )
		{
			memcpy( bpOut, bpIn, sizeof(uint8_t) * sX * sY );
		}
		else
		{
			for( a = 0 ; a < sY ; a++ )
			{
				for( b = 0 ; b < sX ; b++ )
				{
					// clip off any oversize borders
					if( a < sImageX && b < sImageY )
					{
						// Y == Intensity in Grayscale, so no conversion here
						bpOut[ a * sImageX + b ] = bpIn[ a * sX + b ];
					}
				}
			}
		}
	}
	else // jpeg->bNumComp == 3
	{

		//	R = Y					 + 1.402  *(Cr-128)
		//	G = Y - 0.34414*(Cb-128) - 0.71414*(Cr-128)
		//	B = Y + 1.772  *(Cb-128)

		for( a = 0 ; a < sY ; a++ )
		{
			for( b = 0 ; b < sX ; b++ )
			{
				// clip off any oversize borders
				if( a < sImageX && b < sImageY )
				{
					iTmp = a * sX + b;

					// get 1 part of each component
					bXfer[0] = jpeg->comp[0].bpOutPlane[ iTmp ];
					bXfer[1] = jpeg->comp[1].bpOutPlane[ iTmp ];
					bXfer[2] = jpeg->comp[2].bpOutPlane[ iTmp ];

					// perform the transform to RGB in short 
					sXfer[0] = (int16_t)bXfer[0] + (int16_t)(1.402f * ((float)bXfer[2] - 128.0f));
					sXfer[1] = (int16_t)bXfer[0] - (int16_t)(0.34414f * ((float)bXfer[1] - 128.0f)) - (int16_t)(0.71414f * ((float)bXfer[2] - 128.0f));
					sXfer[2] = (int16_t)bXfer[0] + (int16_t)(1.772f * ((float)bXfer[1] - 128.0f));

					// FIXME:  use fast clipping here if possible
					iTmp = (a * sImageX + b) * 3;

					if( sXfer[0] < 0 ) bpOut[iTmp] = 0;
					else if( sXfer[0] > 255 ) bpOut[iTmp] = 255;
					else	bpOut[iTmp] = (uint8_t)sXfer[0];

					if( sXfer[1] < 0 ) bpOut[iTmp + 1] = 0;
					else if( sXfer[1] > 255 ) bpOut[iTmp + 1] = 255;
					else	bpOut[iTmp + 1] = (uint8_t)sXfer[1];

					if( sXfer[2] < 0 ) bpOut[iTmp + 2] = 0;
					else if( sXfer[2] > 255 ) bpOut[iTmp + 2] = 255;
					else	bpOut[iTmp + 2] = (uint8_t)sXfer[2];
				}
			}
		}
	}

	return 1;
}

/* ------------
jpg_Invert
// inverts the row ordering from bottom up to top down after we are done with the image processing stage
------------ */
static uint8_t jpg_Invert( jpeg_t *jpeg )
{
	uint16_t sX = jpeg->sImageX;
	uint16_t sY = jpeg->sImageY;
	uint8_t *bpPlane = jpeg->imgPlane;
	uint8_t *bpRow;
	uint32_t iRowBytes = sX * jpeg->bNumComp;
	uint32_t iB, iA;
	uint16_t a, b;

	// allocate a temporary transfer row
	bpRow = (uint8_t*)mem_alloc( iRowBytes );
	if( bpRow == NULL )
	{
		con_Print( "<RED>JPEG Load Failed:  memory allocation failed in jpg_Invert for a temporary row." );
		return 0;
	}
	memset( bpRow, 0, iRowBytes );

	// iterate top and bottom toward center of image
	for( a = 0, b = (sY - 1) ; a < b ; a++, b-- )
	{
		iA = a * iRowBytes;
		iB = b * iRowBytes;
		
		// copy out the 'a' row to temp
		memcpy( bpRow, &bpPlane[ iA ], iRowBytes );

		// copy 'b' to 'a'
		memcpy( &bpPlane[ iA ], &bpPlane[ iB ], iRowBytes );

		// copy temp to 'b'
		memcpy( &bpPlane[ iB ], bpRow, iRowBytes );
	}

	// cleanup the row byes
	SAFE_RELEASE( bpRow );

	return 1;
}

/* ------------
jpg_FillBuffer
// the bitstream_t provides 64b of buffer, lookahead, alignment, and transparent file reading;
// however, it can't provide the direct access we need for optimal performance in reading a jpeg stream
// this buffer provides the maximal size of an MCU, stripping FF tags so that we don't have to check
// for them when we do every single read
// The buffer is implemented circularly.
// This should get called once per CU before the call to decode
// all buffer specific members of jpeg should get inited in PrepareDecoder: 
// Needed in PrepareDecoder: bBufNeedsInit = 1;
// NB. this will fail if we try to read all the way around the buffer; that should never happen though with a well sized buffer
------------ */
static uint8_t jpg_FillBuffer( jpeg_t *jpeg )
{
	// special case #1:  this is the first call to FillBuffer
	if( jpeg->bBufNeedsInit )
	{
		jpeg->pBufTail = jpeg->pBufHead = jpeg->pBufFirstByte = jpeg->bBuffer;
		jpeg->iBufReads = 0;
		jpeg->pBit = 0;
		jpeg->pBufLastByte = jpeg->bBuffer + JPEG_BUFFER_SIZE;
		*jpeg->pBufTail = bs_Get8Bits(jpeg->bs);
		jpeg->pBufTail++;
		jpeg->bBufNeedsInit = 0;
	}

	// special case #2:  no significant reads have been made since the last FillBuffer
	if( (jpeg->pBufTail == jpeg->pBufHead) && (!jpeg->iBufReads) ) return 1;

	// special case #3:  we have read all the way around
	if( jpeg->iBufReads > JPEG_BUFFER_SIZE ) return 0;

	// special case #4:  the file is finished
	if( jpeg->bBufIsOut ) return 1;

	// Normal Case #1:  the standard buffer fill
	// fill the buffer from the tail to the head, advancing the tail to the head
	while( jpeg->pBufTail != jpeg->pBufHead )
	{
		// read in a byte
		*jpeg->pBufTail = bs_Get8Bits(jpeg->bs);

		// check for the last read
		if( bs_eof(jpeg->bs) ) jpeg->bBufIsOut;

		// Special JPEG Cases
		if( *jpeg->pBufTail == 0xFF )
		{
			uint8_t bTmp = bs_Lookahead8Bits(jpeg->bs);
			
			// #1:  0xFF00 = 0xFF
			if( bTmp == 0x00 )
			{
				bs_Get8Bits(jpeg->bs);
			}
			// #2-8: RST0-7 need to stay in the stream so we can sync our decoder
			else if( bTmp >= 0xD0 && bTmp <= 0xD7 )
			{
				// these stay in the stream
			}

		}

		// advance
		jpeg->pBufTail++;
	
		// check for the wrap-around
		if( jpeg->pBufTail >= jpeg->pBufLastByte )
		{
			jpeg->pBufTail = jpeg->pBufFirstByte;
		}
	}

	// reset the reads from this buffer
	// this should get incremented every time the buffer advances to a new byte
	jpeg->iBufReads = 0;

	return 1;
}

/* ------------
jpg_GetBit - returns 1 bit from our filtered jpeg stream
------------ */
static uint8_t jpg_GetBit( jpeg_t *jpeg )
{
	byte newByte = 0;

	// do the read
	newByte = (*(jpeg->pBufHead) >> (7 - jpeg->pBit)) & 1;

	// adjust our states
	jpeg->pBit++;
	if( jpeg->pBit > 7 )
	{
		jpeg->pBit = 0;

		// advance the head and read bytes
		jpeg->pBufHead++;
		jpeg->iBufReads++;

		// check the head for wrap
		if( jpeg->pBufHead >= jpeg->pBufLastByte )
		{
			jpeg->pBufHead = jpeg->pBufFirstByte;
		}
	}
	
	return newByte;
}


