// Cognition
// game_forground.c
// Created  by Terrence Cole
// 1st write: 3/7/03

// Includes
/////////////
#include "cog_global.h"

// Definitions
////////////////

// Local Structures
/////////////////////
/*
typedef struct foreground_object_s
{
	// the entity to display
	entity_t *ent;
	float entX, entY;
	float entW, entH;
	
	// the shader to use as background
	shader_t *shader;
	float shX, shY;
	float shW, shH;
	
	// the text string
	char *cpText;
	byte cpColor;
	float cpX, cpY;
	float cpH;
	
	// the integer data pointer
	int *ipData;
	byte ipColor;
	float ipX, ipY;
	float ipH;
	
	// the floating point pointer
	float *fpData;
	byte fpColor;
	float fpX, fpY;
	float fpH;

	struct foreground_object_s *next;
	struct foreground_object_s *prev;
} fgo_t;

typedef struct fgo_params_s
{
	// the entity to display
	entity_t *entity;
	float entX, entY;
	float entW, entH;

	// the shader to use as background
	char *shader_name;
	float shX, shY;
	float shW, shH;

	// the text string
	char *cpText;
	byte cpColor;
	float cpX, cpY;
	float cpH;

	// the integer data pointer
	int *ipData;
	byte ipColor;
	float ipX, ipY;
	float ipH;

	// the floating point pointer
	float *fpData;
	byte fpColor;
	float fpX, fpY;
	float fpH;
} fgo_params_t;

*/

// Global Prototypes
//////////////////////
/*
int fgo_Initialize(void);
void fgo_Terminate(void);
fgo_t *fgo_Load( fgo_params_t *params );
void fgo_Unload( fgo_t *fgo );
*/

// Local Prototypes
/////////////////////
static void fgo_LinkTail( fgo_t *fgo );
static void fgo_Unlink( fgo_t *fgo );

// Local Variables
////////////////////
static fgo_t *fgoHead = NULL;
static fgo_t *fgoTail = NULL;

// *********** FUNCTIONALITY ***********
/* ------------
fgo_Initialize
------------ */
int fgo_Initialize(void)
{
	con_Print( "\n<BLUE>Initializing</BLUE> Forground Object System (FGO)." );

	// test if we need to unload
	if( (fgoHead != NULL) || (fgoTail != NULL) )
	{
		con_Print( "FGO already Active, terminating..." );
		fgo_Terminate();
	}

	// assign the state to our head
	state.fgo_list = &fgoHead;

	return 1;
}

/* ------------
fgo_Terminate
------------ */
void fgo_Terminate(void)
{
	int count = 0;
	fgo_t *fgoTmp;
	
	if( (fgoHead == NULL) && (fgoTail == NULL) ) return;

	con_Print( "\n<RED>Terminating</RED> Forground Object System (FGO)." );

	// iterate
	while( fgoHead != NULL )
	{
		count++;

		// unload dependent memory
		if( fgoHead->shader != NULL ) sh_UnloadShader( fgoHead->shader );

		// advance
		fgoTmp = fgoHead->next;

		// free base memory
		SAFE_RELEASE( fgoHead );

		// regroup
		fgoHead = fgoTmp;
	}

	// null everything
	fgoHead = fgoTail = NULL;

	// tell us what was done
	if( count > 0 )
	{
		con_Print( "\tFound and released %d forground objects." );
	}
}

/* ------------
fgo_Load - creates a shader in the given position, loads a background if requested, copies the pointer to the data to print on a framly basis
		- returns the new forground object on success and NULL on fail.
------------ */
fgo_t *fgo_Load( fgo_params_t *params )
{
	fgo_t *fgoNew;

	// assert
	if( (params == NULL) )
	{
		con_Print( "Forground Object System Assert Failed:  Load:  params was NULL" );
		eng_Stop( "11002" );
		return NULL;
	}

	con_Print( "\n<BLUE>Loading</BLUE> Foreground Object: " ); 
	if( params->shader_name != NULL )
		con_Print( "\tShader \"%s\": ( %f, %f, %f, %f )", params->shader_name, params->shX, params->shY, params->shW, params->shH );
	if( params->entity != NULL )
		con_Print( "\tEntity \"%s\": ( %f, %f, %f, %f )", params->entity->name, params->entX, params->entY, params->entW, params->entH );
	if( params->cpText != NULL )
		con_Print( "\tText Data Ptr: ( %f, %f, %f )", params->cpX, params->cpY, params->cpH );
	if( params->ipData != NULL )
		con_Print( "\tInteger Data Ptr: ( %f, %f, %f )", params->ipX, params->ipY, params->ipH );
	if( params->fpData != NULL )
		con_Print( "\tFloating Data Ptr: ( %f, %f, %f )", params->fpX, params->fpY, params->fpH );

	// allocate memory for the new fgo
	fgoNew = (fgo_t*)mem_alloc( sizeof(fgo_t) );
	if( fgoNew == NULL )
	{
		con_Print( "<RED>Forground Object System Error:  Load:  memory allocate failed for new fgo." );
		return NULL;
	}
	memset( fgoNew, 0, sizeof(fgo_t) );

	// copy over data
	// load the shader if requested
	if( params->shader_name != NULL )
	{
		fgoNew->shader = sh_LoadShader( params->shader_name );
		if( fgoNew->shader == NULL )
		{
			con_Print( "<RED>Forground Object System Error:  Load:  Shader Load Failed for Shader \"%s\".", params->shader_name );
			SAFE_RELEASE( fgoNew );
			return NULL;
		}
	}
	else
	{
		fgoNew->shader = NULL;
	}
	fgoNew->shX = params->shX;
	fgoNew->shY = params->shY;
	fgoNew->shW = params->shW;
	fgoNew->shH = params->shH;

	// copy over the ent pointer
	fgoNew->ent = params->entity;
	fgoNew->entX = params->entX;
	fgoNew->entY = params->entY;
	fgoNew->entW = params->entW;
	fgoNew->entH = params->entH;

	// copy over the text pointer
	fgoNew->cpText = params->cpText;
	fgoNew->cpColor = params->cpColor;
	fgoNew->cpX = params->cpX;
	fgoNew->cpY = params->cpY;
	fgoNew->cpH = params->cpH;

	// track an int if requested
	fgoNew->ipData = params->ipData;
	fgoNew->ipColor = params->ipColor;
	fgoNew->ipX = params->ipX;
	fgoNew->ipY = params->ipY;
	fgoNew->ipH = params->ipH;

	fgoNew->fpData = params->fpData;
	fgoNew->fpColor = params->fpColor;
	fgoNew->fpX = params->fpX;
	fgoNew->fpY = params->fpY;
	fgoNew->fpH = params->fpH;

	// track the new fgo
	fgo_LinkTail( fgoNew );

	return fgoNew;
}

/* ------------
fgo_Unload - free all memory associated with the indicated fgo
------------ */
void fgo_Unload( fgo_t *fgo )
{
	// assert
	if( fgo == NULL )
	{
		con_Print( "<RED>Forground Object System Assert Failed:  Unload:  fgo was NULL" );
		eng_Stop( "11001" );
		return;
	}

	con_Print( "\n<RED>Unloading</RED> Forground Object..." );

	// unlatch it from the list
	fgo_Unlink( fgo );

	// release associated memories
	if( fgo->shader != NULL ) sh_UnloadShader( fgo->shader );
	SAFE_RELEASE( fgo );
}

/* ------------
fgo_LinkTail - internal list maintainence, adds fgo to the list at the tail
------------ */
static void fgo_LinkTail( fgo_t *fgo )
{
	if( fgoHead == NULL ) fgoHead = fgo;

	if( fgoTail == NULL )
	{
		fgoTail = fgo;
		fgo->next = NULL;
		fgo->prev = NULL;
		return;
	}

	fgo->prev = fgoTail;
	fgo->next = NULL;

	fgoTail->next = fgo;
	fgoTail = fgo;
}

/* ------------
fgo_Unlink - removes fgo from the forground object list - repairs the list for fgo's absence
------------ */
static void fgo_Unlink( fgo_t *fgo )
{
	if( fgo->prev != NULL ) fgo->prev->next = fgo->next;
	if( fgo->next != NULL ) fgo->next->prev = fgo->prev;

	if( fgoTail == fgo ) fgoTail = fgo->prev;
	if( fgoHead == fgo ) fgoHead = fgo->next;

	fgo->prev = NULL;
	fgo->next = NULL;
}
