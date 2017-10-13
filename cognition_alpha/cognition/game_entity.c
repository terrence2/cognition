// Cognition
// entity.c
// by Terrence Cole 9/10/02

// Includes
/////////////
#include "cog_global.h"

// Definitions
////////////////
#define ENT_NAME_SIZE 64

// collision and occlusion flags
#define ENT_VOLUME_SOLID 1
#define ENT_VOLUME_EMPTY 2

// movement flags
#define ENT_MOVE_PHYSICS 4
#define ENT_MOVE_PATH 8
#define ENT_MOVE_ALGORITHM 16
#define ENT_MOVE_FIXED 32

// unspecified flags
#define ENT_FLAG_UNSPECIFIED 64

// Structures
///////////////
/*
typedef struct entity_s
{
	int id;  // the magic tracking number for this entity
	int flags;	// tells us about this entity
	char name[ENT_NAME_SIZE];  // another way to index an entity; not guaranteed to be unique

	// culling info
	float radius; // get from model
	float distSqrd;  // computed each frame to sort
	byte bInFrustum;

	// drawing info
	vec3 position;
	vec2 rotation;
	vec3 scale;
	byte bHintHidden;

	// physics data
	int mass;  // mass in kilograms
	vec3 velocity; // in meters per second
	vec3 accel; // in meters per second * second
	vec2 ang_velocity; // in degrees per second

	// basic info
	model_t *mod;

	// extended properties
	void *ext_data;
	void (*think)(void); // called when the entity receives processor time
	int nextThink;

	// linking
	struct entity_s *next;
	struct entity_s *prev;
} entity_t;
*/

// Global Prototypes
//////////////////////
/*
int ent_Initialize();
void ent_Terminate();
entity_t *ent_Spawn( char *name, int flags, char *model, float *pos, float *rot, float *scale, void *ext_data, void (*think_func)(void), int nextThink );
void ent_Remove( int ent_id );
void ent_PrintInfo();
*/

// Local Prototypes
/////////////////////
static void ent_LinkTail( entity_t *ent );
static void ent_Unlink( entity_t *ent );

// Local Variables
////////////////////
static int ident_index = 0;
static int num_ents = 0;
static entity_t *ent_list = NULL;
static entity_t *ent_tail = NULL;

// *********** FUNCTIONALITY ***********
/* ------------
ent_Initialize - clears the entity list and resets the counter; 1 on success, 0 on fail
			   - sets the state entity list index to the correct list
------------ */
int ent_Initialize(void)
{
	// close down any entities that may be in the list and reset counter
	ent_Terminate();

	// assign the state pointer to head pointer
	state.ent_list = &ent_list;

	return 1;
}

/* ------------
ent_Terminate - frees all list items and removes the list, resets all variables to their initial condition
------------ */
void ent_Terminate(void)
{
	int num_freed = 0;
	entity_t *ent, *prev;

	// reset
	ident_index = 0;
	num_ents = 0;

	// check to see if we need to worry with the list
	if( ent_list == NULL ) return;

	// iterate all items
	ent = ent_list;
	while( ent != NULL )
	{
		// unlink this reference to the model
		mod_Unload( ent->mod );

		// free this ent
		prev = ent;
		ent = ent->next;

		// free the entity itself
		SAFE_RELEASE( prev );
		num_freed++;
	}

	// null the dead list
	ent_list = ent_tail = NULL;

	if( num_freed > 0 )
	{
		con_Print( "\nTerminated Entity System..." );
		con_Print( "\tFreed %d entities.", num_freed );
	}
}

/* ------------
ent_Spawn - creates a new entity with the specified attributes, adds it to the main entity list and returns a pointer to the new entity
		  - returns NULL on failure
------------ */
entity_t *ent_Spawn( char *name, int flags, char *model, vec3 pos, vec3 rot )
{
	entity_t *ent;
	int len;

	// get memory for the new entity
	ent = (entity_t*)mem_alloc( sizeof(entity_t) );
	if( ent == NULL )
	{
		con_Print( "Entity Spawn Failed:  Unable to get memory for entity holder" );
		return NULL;
	}
	memset( ent, 0, sizeof(entity_t) );

	// setup the main attributes
	// id
	ent->id = ident_index;
	ident_index++;

	// name
	len = tcstrlen( name );
	if( len > ENT_NAME_SIZE ) len = ENT_NAME_SIZE;
	tcstrncpy( ent->name, name, len );

	// flags
	ent->flags = flags;
	if( model == NULL ) ent->flags |= ENT_VOLUME_EMPTY;
	else if( model != NULL && !(ent->flags & ENT_VOLUME_EMPTY) ) ent->flags |= ENT_VOLUME_SOLID;  // set to solid if not explicitly set
	
	// the model
	if( (model != NULL) && (tcstrlen(model) > 0) )
	{
		ent->mod = mod_Load( model );
		if( ent->mod == NULL )
		{
			con_Print( "Entity Spawn Failed:  Unable to load model \"%s\".", model );
			return NULL;
		}
		ent->fRadius = ent->mod->moo->radius;
	}
	else
	{
		ent->fRadius = 0.0f;
		ent->mod = NULL;
	}
	
	// position, orientation, scale
	memcpy( ent->vPos, pos, sizeof(vec3) );
	memcpy( ent->vRot, rot, sizeof(vec3) );
	ent->bHintHidden = 0;

	// add the entity to the list
	ent_LinkTail( ent );

	return ent;
}


/* ------------
ent_Remove - removes the entity from the list that has an id of ent_id
------------ */
void ent_Remove( entity_t *rmEnt )
{
	entity_t *ent;
	
	// look through the main list of entities
	ent = ent_list;
	while( ent != NULL )
	{
		// make the comparison
		if( ent == rmEnt )
		{
			// unlink this reference to the model
			mod_Unload( ent->mod );

			// unlink this entity
			ent_Unlink( ent );

			// free the entity itself
			SAFE_RELEASE( ent );

			// we are done
			return;
		}

		// next
		ent = ent->next;
	}
}

/* ------------
ent_PrintInfo - prints info about all currently loaded entities to the console
------------ */
void ent_PrintInfo( char *null )
{
	entity_t *ent;

	// print the header
	con_Print( "\nHuman Name | Tracking ID | Model Name | Position | Rotation | Hidden" );
	con_Print( "-------------------------------------------------------------" );

	// look through the main list of entities
	ent = ent_list;
	while( ent != NULL )
	{
		if( ent->mod != NULL )
		{
			con_Print( "%s | %d | %s | ( %f, %f, %f ) | ( %f, %f ) | %d", ent->name, ent->id, ent->mod->name,
						ent->vPos[X_AXIS], ent->vPos[Y_AXIS], ent->vPos[Z_AXIS],
						ent->vRot[PITCH], ent->vRot[YAW], ent->bHintHidden );
		}
		else
		{
			con_Print( "%s | %d | <NULL> | ( %f, %f, %f ) | ( %f, %f ) | %d", ent->name, ent->id,
						ent->vPos[X_AXIS], ent->vPos[Y_AXIS], ent->vPos[Z_AXIS],
						ent->vRot[PITCH], ent->vRot[YAW], ent->bHintHidden );
		}

		// next
		ent = ent->next;
	}
}


/* ------------
ent_LinkTail - adds entity to list at the tail
------------ */
static void ent_LinkTail( entity_t *ent )
{
	// check first item case
	if( ent_tail == NULL )
	{
		ent_tail = ent_list = ent;
		return;
	}

	ent->prev = ent_tail;
	ent->next = NULL;

	ent_tail->next = ent;
	ent_tail = ent;
}

/* ------------
ent_Unlink - removes entity from list
------------ */
static void ent_Unlink( entity_t *ent )
{
	if( ent->prev != NULL ) ent->prev->next = ent->next;
	if( ent->next != NULL ) ent->next->prev = ent->prev;

	if( ent_tail == ent ) ent_tail = ent->prev;
	if( ent_list == ent ) ent_list = ent->next;

	ent->prev = NULL;
	ent->next = NULL;
}

