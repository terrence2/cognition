// Cognition
// model.c
// by Terrence Cole 4/23/02

// PURPOSE:  this functionality tracks, loads, and unloads model files on a needed basis

// Includes
/////////////
#include "linked_list.h"
#include "cog_global.h"

// Definitions
////////////////
#define MODEL_DIRECTORY "models"

// Structures
///////////////
/* 

typedef struct
{
	int length; // |length| = the number of indexes in indicies
				// if length < 0, the list describes a fan, else a strip
	int *indicies;
	int *map_indicies;
} mod_dlist_t; // a packed draw list

typedef struct
{
	// identity
	char name[MESH_NAME_SIZE];

	// counting
	int num_verts;
	int num_mverts;
	int num_tris;
	int num_dlists;

	// mesh data
	vec3 **frame_verts;
	vec3 **frame_norms;
	vec2 *mverts;
	int *mtris;
	int *tris;
	mod_dlist_t *dlists;
} mesh_t;

typedef struct
{
	// identity
	char name[MOO_NAME_SIZE];
	int num_users;

	// counting information
	int num_meshes;
	int num_frames;

	// frame animation
	int cur_frame;
	float sub_frame;

	// the data
	mesh_t *meshes;
} moo_data_t;

typedef struct
{
	// identity
	char name[MODEL_NAME_SIZE];
	int num_users;
	
	// animation
	int frames_per_second;
	double frames_per_millisecond;

	// counting information
	int num_shaders;

	// data
	moo_data_t *moo;
	shader_t **shaders;
} model_t;
*/

// Global Prototypes
//////////////////////
/*
int mod_Initialize(void);
void mod_Terminate(void);
model_t *mod_Load( char *name );
void mod_Unload( model_t *mod );
*/

// Local Prototypes
/////////////////////
static model_t *mod_LoadFromFile( char *name );
static model_t *mod_FindLoadedModel( char *name );
static void mod_UnloadModel( model_t *mod );
static void mod_Release( model_t *mod );

// Local Variables
////////////////////
static LinkedList mod_list = LINKEDLIST_NOT_A_LIST;  // multiple models can reference the same mesh information
													 // the program can reference multiple copies of the same model

// *********** FUNCTIONALITY ***********
/* ----------
mod_Initialize - clear the system of models and prepare the model system to load and track models
			   - returns 1 on success and 0 on failure
---------- */
int mod_Initialize(void)
{
	// reset if a list exists
	if( mod_list != LINKEDLIST_NOT_A_LIST )
	{
		mod_Terminate();
	}

	// initialize
	mod_list = ll_GetNewList();

	if( mod_list <= LINKEDLIST_NOT_A_LIST )
	{
		con_Print( "Model System Initialization Failure:  Get New List failed." );
		return 0;
	}

	return 1;
}

/* ----------
mod_Terminate - releases all models currently being tracked by the model system and releases the tracking lists
			  - if the ents are tracking the models and unloading them on demand, this should usually be empty
---------- */
void mod_Terminate(void)
{
	int count;

	// only alert the user if this call is actually going to do something
	if( mod_list != LINKEDLIST_NOT_A_LIST )
	{
		con_Print( "\nTerminating Model System..." );

		ll_iter_reset(mod_list);
		count = 0;
		while( !ll_iter_isdone(mod_list) )
		{
			mod_Unload( (model_t*)ll_iter_data(mod_list) );
			ll_iter_remove(mod_list);
			ll_iter_next(mod_list);
			count++;
		}
		ll_DeleteList(mod_list);
		mod_list = LINKEDLIST_NOT_A_LIST;

		if( count > 0 )
		{
			con_Print( "\tFound and released %d models...", count );
		}
	}
}

/* ------------
mod_Load - finds and returns the already loaded model or creates a new instance of the requested unloaded model
		 - returns NULL if an error occurs (i.e. the model cannot be found or opened on disk)
------------ */
model_t *mod_Load( char *name )
{
	model_t *mod = NULL;

	// attempt to find the model already loaded
	mod = mod_FindLoadedModel( name );

	if( mod == NULL )
	{
		// we didn't find the model, so load from file
		mod = mod_LoadFromFile( name );
		if( mod == NULL ) return NULL;

		// add the new model to the list
		ll_AddUnordered( mod_list, mod );
	}

	// if we still don't have the model we have failed
	if( mod == NULL ) return NULL;

	// increment the users
	mod->num_users++;

	// return a reference to the model data
	return mod;
}

/* ------------
mod_LoadFromFile - loads a model definition and all relevant data from file name
				 - returns the new reference to the model or NULL if an error occurs
------------ */
static model_t *mod_LoadFromFile( char *name )
{
	uint32_t a;
	char *path;
	char *buffer;
	char *value;
	char moo_name[MOO_NAME_SIZE];
	int length;
	model_t *mod;

	// get the path
	path = fs_GetMediaPath( name, MODEL_DIRECTORY );
	if( path == NULL )
	{
		con_Print( "Model Load Error:  Unable to resolve full path." );
		return NULL;
	}

	// buffer the file
	buffer = par_BufferFile( path );
	if( buffer == NULL )
	{
		con_Print( "Model Load Error:  Unable to buffer the file." );
		return NULL;
	}

	// find the tokens
	value = par_GetTokenValue( buffer, "mesh", 4 );
	if( value == NULL )
	{
		con_Print( "Model Load Error:  Unable to find a \"mesh\" token in the model definition." );
		SAFE_RELEASE(buffer);
		return NULL;
	}

	// copy over the name
	length = tcstrlen(value);
	if( length > MOO_NAME_SIZE ) length = MOO_NAME_SIZE;
	tcstrncpy( moo_name, value, length );
	moo_name[length] = '\0';
	SAFE_RELEASE(value);

	// get the model memory
	mod = (model_t*)mem_alloc( sizeof(model_t) );
	if( mod == NULL )
	{
		con_Print( "Model Load Error:  Unable to allocate memory for model." );
		SAFE_RELEASE(buffer);
		return NULL;
	}
	memset( mod, 0, sizeof(model_t) );

	// copy the name, leaving behind the extension
	length = tcstrlen(name) - 4;
	if( length > MODEL_NAME_SIZE ) length = MODEL_NAME_SIZE;
	tcstrncpy( mod->name, name, length );

	// get the moo
	mod->moo = moo_Load( moo_name );
	if( mod->moo == NULL )
	{
		con_Print( "Model Load Error:  Load Moo Data failed." );
		SAFE_RELEASE(buffer);
		SAFE_RELEASE(mod);
		return NULL;
	}

	// counting
	mod->num_users = 0;
	mod->num_shaders = mod->moo->num_meshes;

	// allocate a spot for the shader references
	if( mod->num_shaders > 0 )
	{
		mod->shaders = (shader_t**)mem_alloc( sizeof(shader_t*) * mod->moo->num_meshes );
		if( mod->shaders == NULL )
		{
			con_Print( "Model Load Error:  Unable to allocate memory for shader pointer block." );
			SAFE_RELEASE(buffer);
			moo_Unload( mod->moo );
			SAFE_RELEASE(mod);
			return NULL;
		}
		memset( mod->shaders, 0, sizeof(shader_t*) * mod->moo->num_meshes );
	}

	// find and initialize the shaders
	for( a = 0 ; a < mod->moo->num_meshes ; a++ )
	{
		// get the shader names
		value = par_GetTokenValue( buffer, mod->moo->meshes[a].name, tcstrlen(mod->moo->meshes[a].name) );
		if( value == NULL )
		{
			con_Print( "Model Load Error:  Unable to find a shader for mesh \"%s\" in the model definition.", mod->moo->meshes[a].name );
			SAFE_RELEASE(buffer);
			moo_Unload( mod->moo );
			SAFE_RELEASE(mod->shaders);
			SAFE_RELEASE(mod);
			return NULL;
		}

		// demand load the shader
		mod->shaders[a] = sh_LoadShader( value );
		SAFE_RELEASE(value);
		if( mod->shaders[a] == NULL )
		{
			con_Print( "Model Load Error:  Unable to find a shader for mesh \"%s\" in the model definition.", mod->moo->meshes[a].name );
			SAFE_RELEASE(buffer);
			moo_Unload( mod->moo );
			SAFE_RELEASE(mod->shaders);
			SAFE_RELEASE(mod);
			return NULL;
		}
	}

	// free and return
	SAFE_RELEASE( buffer );
	return mod;
}

/* ---------
mod_UnloadModel - decrements the references to mod; if mod is no longer in use, all memory related to mod is freed and the list reference to mod is removed
--------- */
void mod_Unload( model_t *mod )
{
	// assert
	if( mod == NULL ) return;

	mod->num_users--;

	// if the model is still being used elsewhere, we are done
	if( mod->num_users > 0 ) return;

	// find the list reference to mod
	ll_iter_reset(mod_list);
	while( !ll_iter_isdone(mod_list) )
	{
		if( mod == ll_iter_data(mod_list) )
		{
			ll_iter_remove(mod_list);
		}
		ll_iter_next(mod_list);
	}

	// contract the work here to the convenient function we use to terminate the lists quickly
	mod_Release( mod );
}

/* ------------
mod_FindLoadedModel - searches the list of loaded models for one with name and returns it. returns NULL if not present
------------ */
static model_t *mod_FindLoadedModel( char *name )
{
	model_t *mod;

	ll_iter_reset( mod_list );

	while( !ll_iter_isdone( mod_list ) )
	{
		mod = (model_t*)ll_iter_data( mod_list );

		// do the comparison
		if( tcstrncmp( mod->name, name, tcstrlen(mod->name) ) )
		{
			return mod;
		}

		ll_iter_next( mod_list );
	}

	// the model was not found
	return NULL;
}

/* ---------
mod_Release - free the mod and all associated memories;  tell the demand loading moo system that this model is no longer using its moo
--------- */
static void mod_Release( model_t *mod )
{
	int a;

	// unload all of the model data
	for( a = 0 ; a < mod->num_shaders ; a++ )
	{
		// undemand each shader reference
		sh_UnloadShader( mod->shaders[a] );
	}
	SAFE_RELEASE( mod->shaders );

	// decrement the uses of this moo and possibly free it entirly
	moo_Unload( mod->moo );

	// free this
	SAFE_RELEASE( mod );
}
