// Cognition
// moo.c
// by Terrence Cole 4/23/02

// PURPOSE:  this functionality tracks, loads, and unloads model files on a needed basis

// Includes
/////////////
#include "cog_global.h"
#include "linked_list.h"

// Definitions
////////////////
#define MOO_DIRECTORY "meshes"

// Structures
///////////////
/* 
typedef struct
{
	int length; // |length| = the number of indexes in indicies
				// if length < 0, the list describes individual tris, else a strip
	int *indicies;
} mod_dlist_t; // a packed draw list

typedef struct sphere_tree_node_s
{
	vec3 loc;
	float radius;
	
	int numInFaces;
	int *inFaces;

	int numInPoints;
	vec3 *inPoints;

	int numChildren;
	struct sphere_tree_node_s **children;
} stNode_t;

typedef struct
{
	vec3 *verts; // numVerts
	vec3 *vNorms;  // numVerts
	vec3 *tNorms;  // numTris
	stNode_t *stRoot;
} mesh_frame_t;

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
	mesh_frame_t *frames;
	vec2 *mverts;  // num_mverts vec2's
	int *mtris;  // numTris * 3 ints;  elmts 0 to num_mverts-1
	int *tris;  // numTris * 3 ints;  elmts 0 to num_verts-1
	mod_dlist_t *dlists;
} mesh_t;

typedef struct tag_s
{
	int index;
	int type;
	vec3 pos;
	vec3 dir;
	float theta;
} tag_t;

typedef struct
{
	// identity
	char name[MOO_NAME_SIZE];
	int num_users;
	
	// animation
	int frames_per_second;
	double frames_per_millisecond;

	// counting information
	int num_meshes;
	int num_frames;

	// bounds
	aabb_t bounds;
	float radius;

	// frame animation
	int cur_frame;
	float sub_frame;

	// the data
	mesh_t *meshes;

	// the tags
	tag_t *tags;
} moo_data_t;

typedef struct
{
	// identity
	char name[MODEL_NAME_SIZE];
	int num_users;

	// counting information
	int num_shaders;

	// data
	moo_data_t *moo;
	shader_t **shaders;
} model_t;
*/

#define MOO_FILE_VERSION 9
#define MOO_FILE_MAGIC_NUMBER 0x1FF10880

typedef struct
{
	int magic;
	byte version;
	char name[MOO_NAME_SIZE];
	int num_tags;
	int num_meshes;
	int num_frames;
	int frames_per_second;
	double frames_per_millisecond;
	float hibounds[3];
	float lobounds[3];
	float radius;
	int file_size;
	int header_size;
} moo_file_header_t;

typedef struct
{
	char name[MESH_NAME_SIZE];
	int num_verts; // and # norms, and # map coords
	int num_tris;
	int num_mverts;
	int num_strip_lists;
	int mesh_size;	// size from first byte of header to end of sphere tree data
	int mesh_header_size;
} moo_file_mesh_header_t;

// THE FILE
/*
 - model_file_header_t
	 - model_file_tag_header_t (1)
	 - ....
	 - model_file_tag_header_t (n)
	 - model_file_mesh_header_t
		 - verts frame 1
		 - ....
		 - verts frame n
		 - vertex norms frame 1
		 - ....
		 - vertex norms frame n
		 - tri norms frame 1
		 - ....
		 - tri norms frame n
		 - map coords
		 - map tris
		 - tris
		 - strips
			- int length (sign bit is type)
			- and a list of geometry indicies
		 - sphere tree (frame 0)
			- root node
				-child 0
					children...
				-child 1
				-child n...
		 - sphere tree (frame 1)
			...
		 - sphere tree (frame n)

	 -model_file_mesh_header_t
		 - verts frame 1
		 - ....
		 - verts frame n
		 - vertex norms frame 1
		 - ....
		 - vertex norms frame n
		 - tri norms frame 1
		 - ....
		 - tri norms frame n
		 - map coords
		 - map tris
		 - tris
		 - strips
			- int length (sign bit is type)
			- and a list of geometry indicies
		 - sphere tree (frame 0)
			- root node
				-child 0
					children...
				-child 1
				-child n...
		 - sphere tree (frame 1)
			...
		 - sphere tree (frame n)

	 - etc.
*/


// Global Prototypes
//////////////////////
/*
int moo_Initialize(void);
void moo_Terminate(void);
moo_data_t *moo_Load( char *name );
void moo_Unload( moo_data_t *moo );
*/

// Local Prototypes
/////////////////////
static moo_data_t *moo_LoadMooFromFile( char *moo_name );
static moo_data_t *moo_lm_GetMooHeader( moo_file_header_t *header, file_t *fp );
static int moo_lm_GetTag( tag_t *tag, file_t *fp );
static int moo_lm_GetMeshHeader( mesh_t *mesh, moo_file_mesh_header_t *mesh_header, file_t *fp );
static int moo_lm_GetMeshMemory( mesh_t *mesh, moo_file_mesh_header_t *mesh_header, moo_file_header_t *moo_header, file_t *fp );
static int moo_lm_ReadMeshData( mesh_t *mesh, int num_frames, file_t *fp );
static stNode_t *moo_ReadSphereTreeNode( int *error_state, file_t *fp );

static moo_data_t *moo_FindLoadedMoo( char *moo_name );
static void moo_ReleaseMoo( moo_data_t *moo );
static void moo_ReleaseSphereTreeNode( stNode_t *root );

// Local Variables
////////////////////
static LinkedList moo_list = LINKEDLIST_NOT_A_LIST;  // multiple models can reference the same mesh information
													 // the program can reference multiple copies of the same model

// *********** FUNCTIONALITY ***********
/* ----------
moo_Initialize - clear the system of moos and prepare the moo system to load and track models
			   - returns 1 on success and 0 on failure
---------- */
int moo_Initialize(void)
{
	// reset if a list exists
	if( moo_list != LINKEDLIST_NOT_A_LIST )
	{
		moo_Terminate();
	}

	// initialize
	moo_list = ll_GetNewList();

	if( moo_list <= LINKEDLIST_NOT_A_LIST )
	{
		con_Print( "Moo System Initialization Failure:  Get New List failed." );
		return 0;
	}

	return 1;
}

/* ----------
moo_Terminate - releases all models currently being tracked by the model system and releases the tracking lists
			  - if the ents are tracking the models and unloading them on demand, this should usually be empty
---------- */
void moo_Terminate(void)
{
	int count;

	// only alert the user if this call is actually going to do something
	if( moo_list != LINKEDLIST_NOT_A_LIST )
	{
		con_Print( "\nTerminating Moo System..." );

		ll_iter_reset(moo_list);
		count = 0;
		while( !ll_iter_isdone(moo_list) )
		{
			moo_ReleaseMoo( (moo_data_t*)ll_iter_data(moo_list) );
			ll_iter_remove(moo_list);
			ll_iter_next(moo_list);
			count++;
		}
		ll_DeleteList(moo_list);
		moo_list = LINKEDLIST_NOT_A_LIST;

		if( count > 0 )
		{
			con_Print( "\tFound and released %d moo mesh datas...", count );
		}
	}
}

/* ---------
moo_Load - finds the moo with moo_name in the loaded list or loads the moo from file
--------- */
moo_data_t *moo_Load( char *moo_name )
{
	moo_data_t *moo = NULL;

	// attempt to find the model already loaded
	moo = moo_FindLoadedMoo( moo_name );

	if( moo == NULL )
	{
		// we didn't find the model, so load from file
		moo = moo_LoadMooFromFile( moo_name );
		if( moo == NULL ) return NULL;

		// add the new model to the list of models
		ll_AddUnordered( moo_list, moo );
	}

	// if we still don't have the model we have failed
	if( moo == NULL ) return NULL;

	// increment the users
	moo->num_users++;

	// return a reference to the model data
	return moo;
}

/* ------------
moo_Unload - decrements the usage count on moo and frees the moo_data if it is no longer in use
------------ */
void moo_Unload( moo_data_t *moo )
{
	// assert
	if( moo == NULL ) return;

	moo->num_users--;

	// if the moo still has users we are done
	if( moo->num_users > 0 ) return;

	// else we need to pull out the moo
	
	// find the list reference
	ll_iter_reset(moo_list);
	while( !ll_iter_isdone(moo_list) )
	{
		if( moo == ll_iter_data(moo_list) )
		{
			ll_iter_remove(moo_list);
		}
		ll_iter_next(moo_list);
	}

	// we export the work here, so that we can fail on load w/out crashing out the other models that use this moo
	moo_ReleaseMoo( moo );
}


/* ------------
moo_LoadMooFromFile - loads the moo file at moo_name in the mesh directory and returns a pointer to the data
					- the data tracked in the moo list and it is originally given 0 users, NULL is returned on errors
------------ */
static moo_data_t *moo_LoadMooFromFile( char *moo_name )
{
	int a;
	int ret;
	file_t *fp;
	char *path;
	moo_file_header_t moo_header;
	moo_file_mesh_header_t mesh_header;
	moo_data_t *moo;

	// resolve a full path
	path = fs_GetMediaPath( moo_name, MOO_DIRECTORY );
	if( path == NULL )
	{
		con_Print( "Moo Load Error:  Unable to resolve path for \"%s\"", moo_name );
		return NULL;
	}

	// open the file
	fp = fs_open( path, "r" );
	if( fp == NULL )
	{
		con_Print( "Moo Load Error:  Unable to open file \"%s\" for reading.", path );
		return NULL;
	}

	// tell the audience why they are having to wait
	con_Print( "<BLUE>Loading</BLUE> Mesh from file <ORANGE>\"%s\"</ORANGE>.", path );
	eng_LoadingFrame();

	// 1) get a moo based on the header
	moo = moo_lm_GetMooHeader( &moo_header, fp );
	if( moo == NULL )
	{
		// the error was logged in get moo header
		fs_close(fp);
		return NULL;
	}

	// get the tags
	for( a = 0 ; a < moo_header.num_tags ; a++ )
	{
		// 2) read the tag
		ret = moo_lm_GetTag( &moo->tags[a], fp );
		if( !ret )
		{
			fs_close(fp);
			moo_ReleaseMoo( moo );
			return NULL;
		}
	}

	// get the meshes
	for( a = 0 ; a < moo_header.num_meshes ; a++ )
	{
		// 3) read the mesh header
		ret = moo_lm_GetMeshHeader( &moo->meshes[a], &mesh_header, fp );
		if( !ret )
		{
			// logged in get mesh header
			fs_close(fp);
			moo_ReleaseMoo( moo );
			return NULL;
		}

		// 3) prepare the mesh
		ret = moo_lm_GetMeshMemory( &moo->meshes[a], &mesh_header, &moo_header, fp );
		if( !ret )
		{
			// logged in get mesh memory
			fs_close(fp);
			moo_ReleaseMoo(moo);
			return NULL;
		}

		// 4) read data to the mesh
		ret = moo_lm_ReadMeshData( &moo->meshes[a], moo->num_frames, fp );
		if( !ret )
		{
			// logged in read mesh data
			fs_close(fp);
			moo_ReleaseMoo(moo);
			return NULL;
		}
	}

	// close the file and exit
	fs_close(fp);
	return moo;
}

/* ------------
moo_lm_GetMooHeader - reads the header from a moo file and reads the data into a new moo data object
------------ */
static moo_data_t *moo_lm_GetMooHeader( moo_file_header_t *header, file_t *fp )
{
	moo_data_t *moo;

	// allocate a new moo
	moo = (moo_data_t*)mem_alloc( sizeof(moo_data_t) );
	if( moo == NULL )
	{
		con_Print( "Moo Load Error:  Unable to allocate memory for moo data structure." );
		return NULL;
	}
	memset( moo, 0, sizeof(moo_data_t) );

	// read the header
	fs_read( header, sizeof(moo_file_header_t), 1, fp );
	if( (header->version != MOO_FILE_VERSION) ||
		(header->header_size != sizeof(moo_file_header_t)) ||
		(header->magic != MOO_FILE_MAGIC_NUMBER) )
	{
		con_Print( "Moo Load Error:  Mismatched file version or header size." );
		SAFE_RELEASE( moo );
		return NULL;
	}

	// copy over the data
	moo->num_meshes = header->num_meshes;
	moo->num_frames = header->num_frames;
	moo->num_tags = header->num_tags;
	moo->frames_per_second = header->frames_per_second;
	moo->frames_per_millisecond = header->frames_per_millisecond;
	memcpy( &moo->bounds.hibounds, &header->hibounds, sizeof(vec3) ); 
	memcpy( &moo->bounds.lobounds, &header->lobounds, sizeof(vec3) ); 
	moo->radius = header->radius;
	moo->num_users = 0;
	tcstrncpy( moo->name, header->name, tcstrlen(header->name) );

	// allocate memory for the tags
	if( moo->num_tags > 0 )
	{
		moo->tags = (tag_t*)mem_alloc( sizeof(tag_t) * moo->num_tags );
		if( moo->tags == NULL )
		{
			con_Print( "<RED>Moo Load Error:  Unable to allocate memory for moo tags.</RED>" );
			SAFE_RELEASE( moo );
			return NULL;
		}
		memset( moo->tags, 0, sizeof(tag_t) * moo->num_tags );
	}
	else
	{
		moo->tags = NULL;
	}

	// allocate memory for the meshes
	if( moo->num_meshes > 0 )
	{
		moo->meshes = (mesh_t*)mem_alloc( sizeof(mesh_t) * moo->num_meshes );
		if( moo->meshes == NULL )
		{
			con_Print( "Moo Load Error:  Unable to allocate memory for moo meshes." );
			SAFE_RELEASE( moo );
			SAFE_RELEASE( moo->tags );
			return NULL;
		}
		memset( moo->meshes, 0, sizeof(mesh_t) * moo->num_meshes );
	}
	else
	{
		moo->meshes = NULL;
	}

	return moo;
}

/* ------------
moo_lm_GetMeshHeader - tag is already malloced, but not inited
------------ */
static int moo_lm_GetTag( tag_t *tag, file_t *fp )
{
	int iNumRead = fs_read( tag, sizeof(tag_t), 1, fp );
	if( iNumRead < 1 ) return 0;
	return 1;
}

/* ------------
moo_lm_GetMeshHeader - mesh and mesh_header are already malloced, but not initialized
					 - moo header is complete and accurate, and fp is open 
------------ */
static int moo_lm_GetMeshHeader( mesh_t *mesh, moo_file_mesh_header_t *mesh_header, file_t *fp )
{
	// initialize
	memset( mesh, 0, sizeof(mesh_t) );
	memset( mesh_header, 0, sizeof(moo_file_mesh_header_t) );

	// read the next mesh header
	fs_read( mesh_header, sizeof(moo_file_mesh_header_t), 1, fp );
	if( mesh_header->mesh_header_size != sizeof(moo_file_mesh_header_t) )
	{
		con_Print( "Moo Load Error:  Mesh Header size mismatch." );
		return 0;
	}

	// copy over the mesh data
	tcstrncpy( mesh->name, mesh_header->name, tcstrlen(mesh_header->name) );
	mesh->num_verts = mesh_header->num_verts;
	mesh->num_tris = mesh_header->num_tris;
	mesh->num_mverts = mesh_header->num_mverts;
	mesh->num_dlists = mesh_header->num_strip_lists;

	return 1;
}

/* ------------
moo_lm_GetMeshMemory - mesh and mesh_header are already malloced and initialized
					 - moo header is complete and accurate, and fp is open 
------------ */
static int moo_lm_GetMeshMemory( mesh_t *mesh, moo_file_mesh_header_t *mesh_header, moo_file_header_t *moo_header, file_t *fp )
{
	int a;

	// assert
	if( mesh == NULL ) return 0;
	if( mesh_header == NULL ) return 0;
	if( moo_header == NULL ) return 0;
	if( fp == NULL ) return 0;

	// get frame pointers
	mesh->frames = (mesh_frame_t*)mem_alloc( sizeof(mesh_frame_t) * moo_header->num_frames );
	if( mesh->frames == NULL )
	{
		con_Print( "Moo Load Failure:  memory allocation failed for frame pointers in mesh %s.", mesh_header->name );
		return 0;
	}
	memset( mesh->frames, 0, sizeof(mesh_frame_t) * moo_header->num_frames );

	// get memory in the frame pointers
	for( a = 0 ; a < moo_header->num_frames ; a++ )
	{
		// verts
		mesh->frames[a].verts = (vec3*)mem_alloc( sizeof(vec3) * mesh->num_verts );
		// norms
		mesh->frames[a].vNorms = (vec3*)mem_alloc( sizeof(vec3) * mesh->num_verts );
		// triangle norms
		mesh->frames[a].tNorms = (vec3*)mem_alloc( sizeof(vec3) * mesh->num_tris );

		// test our memory allocations
		if( (mesh->frames[a].verts == NULL) ||
			(mesh->frames[a].vNorms == NULL) ||
			(mesh->frames[a].tNorms == NULL) )
		{
			con_Print( "Moo Load Failure:  A memory allocation failed in mesh %s in frame %d.", mesh_header->name, a );
			return 0;
		}
		
		// clear the newly allocated memory
		memset( mesh->frames[a].verts, 0, sizeof(vec3) * mesh->num_verts );
		memset( mesh->frames[a].vNorms, 0, sizeof(vec3) * mesh->num_verts );
		memset( mesh->frames[a].tNorms, 0, sizeof(vec3) * mesh->num_tris );
	}

	// get non frame memory

	// mapping coordinates
	mesh->mverts = (vec2*)mem_alloc( sizeof(vec2) * mesh->num_mverts );
	// mapping coordinate triangle associations
	mesh->mtris = (int*)mem_alloc( sizeof(int) * mesh->num_tris * 3 );
	// triangle vertex indicies
	mesh->tris = (int*)mem_alloc( sizeof(int) * mesh->num_tris * 3 );
	// the dlist blocks
	mesh->dlists = (mod_dlist_t*)mem_alloc( sizeof(mod_dlist_t) * mesh->num_dlists );

	// test the new memory
	if( (mesh->mverts == NULL) ||
		(mesh->mtris == NULL) ||
		(mesh->tris == NULL) ||
		(mesh->dlists == NULL) )
	{
		con_Print( "Moo Load Failed:  Unable to allocate memory for non frame objects in mesh %s.", mesh_header->name );
		return 0;
	}

	// clear the memory

	memset( mesh->mverts, 0, sizeof(vec2) * mesh->num_mverts );
	memset( mesh->mtris, 0, sizeof(int) * mesh->num_tris * 3 );
	memset( mesh->tris, 0, sizeof(int) * mesh->num_tris * 3 );
	memset( mesh->dlists, 0, sizeof(mod_dlist_t) * mesh->num_dlists );

	return 1;
}

/* ------------
moo_lm_ReadMeshData - gets the data for mesh from fp, >0 on success, 0 on fail
					- mesh is alloced and initialized with memory waiting
------------ */
static int moo_lm_ReadMeshData( mesh_t *mesh, int num_frames, file_t *fp )
{
	int32_t a;
	int ret;

	// assert
	if( mesh == NULL ) return 0;
	if( fp == NULL ) return 0;

	// READ THE DATA
	for( a = 0 ; a < num_frames ; a++ )
	{
		fs_read( mesh->frames[a].verts, sizeof(vec3), mesh->num_verts, fp );
	}
	for( a = 0 ; a < num_frames ; a++ )
	{
		fs_read( mesh->frames[a].vNorms, sizeof(vec3), mesh->num_verts, fp );
	}
	for( a = 0 ; a < num_frames ; a++ )
	{
		fs_read( mesh->frames[a].tNorms, sizeof(vec3), mesh->num_tris, fp );
	}
	fs_read( mesh->mverts, sizeof(vec2), mesh->num_mverts, fp );
	fs_read( mesh->mtris, sizeof(int) * 3, mesh->num_tris, fp );
	fs_read( mesh->tris, sizeof(int) * 3, mesh->num_tris, fp );
	
	// read the dlists sequentially
	for( a = 0 ; a < (signed)mesh->num_dlists ; a++ )
	{
		// get the list length
		fs_read( &mesh->dlists[a].length, sizeof(int), 1, fp );

		// allocate memory for the geometry indicies
		mesh->dlists[a].indicies = (int*)mem_alloc( sizeof(int) * ABS(mesh->dlists[a].length) );
		if( mesh->dlists[a].indicies == NULL )
		{
			con_Print( "Moo Load Error:  Unable to allocate memory for geometry index block in display list %d in mesh %s.", a, mesh->name );
			return 0;
		}
		memset( mesh->dlists[a].indicies, 0, sizeof(int) * ABS(mesh->dlists[a].length) );

		// read in the indicies
		fs_read( mesh->dlists[a].indicies, sizeof(int), ABS(mesh->dlists[a].length), fp );
	}

	// read the sphere collision tree
	for( a = 0 ; a < num_frames ; a++ )
	{
		mesh->frames[a].stRoot = moo_ReadSphereTreeNode( &ret, fp );
		if( !ret )
		{
			con_Print( "Moo Load Error:  Sphere tree load failure caught at root." );
			return 0;
		}
	}

	return 1;
}

/* ------------
moo_ReadSphereTreeNode - reads one tree node from fp (including it's children)
------------ */
static stNode_t *moo_ReadSphereTreeNode( int *error_state, file_t *fp )
{
	int a, ret;
	stNode_t *node;

	// assert
	if( fp == NULL ) return 0;

	// assume error is bad unless we say otherwise
	*error_state = 0;

	// get a new node to hold the data
	node = (stNode_t*)mem_alloc( sizeof(stNode_t) );
	if( node == NULL )
	{
		con_Print( "MOO Load Failed:  Unable to allocate memory for sphere tree node." );
		return NULL;
	}
	memset( node, 0, sizeof(stNode_t) );

	// read in the always present data
	fs_read( node->loc, sizeof(vec3), 1, fp );
	fs_read( &(node->radius), sizeof(float), 1, fp );
	fs_read( &(node->numChildren), sizeof(int), 1, fp );
	fs_read( &(node->numInFaces), sizeof(int), 1, fp );
	fs_read( &(node->numInPoints), sizeof(int), 1, fp );

	// if this is a leaf we'll have data about the faces near this sphere
	if( node->numInFaces > 0 )
	{
		// get memory
		node->inFaces = (int*)mem_alloc( sizeof(int) * node->numInFaces );
		if( node->inFaces == NULL )
		{
			con_Print( "MOO Load Failed:  Unable to allocate memory for sphere tree face indicies." );
			SAFE_RELEASE( node );
			return NULL;
		}
		memset( node->inFaces, 0, sizeof(int) * node->numInFaces );

		// read from file
		fs_read( node->inFaces, sizeof(int), node->numInFaces, fp );

		// get memory for the points
		node->inPoints = (vec3*)mem_alloc( sizeof(vec3) * node->numInPoints );
		if( node->inPoints == NULL ) 
		{
			con_Print( "MOO Load Failed:  Unable to allocate memory for sphere tree face indicies." );
			SAFE_RELEASE( node->inFaces );
			SAFE_RELEASE( node );
			return NULL;
		}
		memset( node->inPoints, 0, sizeof(vec3) * node->numInPoints );

		// read the points
		fs_read( node->inPoints, sizeof(vec3), node->numInPoints, fp );
	}
	
	// read the children for all non-leaf nodes
	if( node->numChildren > 0 )
	{
		// get memory for the children pointers
		node->children = (stNode_t**)mem_alloc( sizeof(stNode_t*) * node->numChildren );
		if( node->children == NULL )
		{
			con_Print( "MOO Load Failed:  Unable to allocate memory for sphere tree child hash table." );
			SAFE_RELEASE( node->inFaces );
			SAFE_RELEASE( node->inPoints );
			SAFE_RELEASE( node );
			return NULL;
		}
		memset( node->children, 0, sizeof(stNode_t*) * node->numChildren );

		// read all of the children of this node, in order
		for( a = 0 ; a < node->numChildren ; a++ )
		{
			node->children[a] = moo_ReadSphereTreeNode( &ret, fp );
			if( !ret )
			{
				SAFE_RELEASE( node->children );
				SAFE_RELEASE( node->inFaces );
				SAFE_RELEASE( node->inPoints );
				SAFE_RELEASE( node );
				return NULL;
			}
		}
	}

	*error_state = 1;
	return node;
}

/* ------------
mod_FindLoadedMoo - searches the list of loaded moos for one with name and returns it. returns NULL if not present
------------ */
static moo_data_t *moo_FindLoadedMoo( char *moo_name )
{
	moo_data_t *moo;

	ll_iter_reset( moo_list );

	while( !ll_iter_isdone( moo_list ) )
	{
		moo = (moo_data_t*)ll_iter_data( moo_list );

		// do the comparison
		if( tcstrcmp( moo->name, moo_name ) )
		{
			return moo;
		}

		ll_iter_next( moo_list );
	}

	// the model was not found
	return NULL;
}


/* ------------
moo_ReleaseMoo - frees the moo_data and all associated memory
------------ */
static void moo_ReleaseMoo( moo_data_t *moo )
{
	uint32_t a, b;

	// assert
	if( moo == NULL ) return;

	con_Print( "\nUnloading Mesh \"%s\"", moo->name );

	// unload all of the model data
	for( a = 0 ; a < moo->num_meshes ; a++ )
	{
		for( b = 0 ; b < moo->num_frames ; b++ )
		{
			SAFE_RELEASE( moo->meshes[a].frames[b].verts );
			SAFE_RELEASE( moo->meshes[a].frames[b].vNorms );
			SAFE_RELEASE( moo->meshes[a].frames[b].tNorms );
			moo_ReleaseSphereTreeNode( moo->meshes[a].frames[b].stRoot );
		}

		SAFE_RELEASE( moo->meshes[a].frames );

		for( b = 0 ; b < moo->meshes[a].num_dlists ; b++ )
		{
			SAFE_RELEASE( moo->meshes[a].dlists[b].indicies );
		}
		SAFE_RELEASE( moo->meshes[a].dlists );

		SAFE_RELEASE( moo->meshes[a].mverts );
		SAFE_RELEASE( moo->meshes[a].mtris );
		SAFE_RELEASE( moo->meshes[a].tris );
	}

	SAFE_RELEASE( moo->tags );
	SAFE_RELEASE( moo->meshes );
	SAFE_RELEASE( moo );
}

/* ------------
moo_ReleaseSphereTreeNode - frees all memory allocated below this sphere tree
------------ */
static void moo_ReleaseSphereTreeNode( stNode_t *root )
{
	int a;

	// assert
	if( root == NULL ) return;

	// free the children
	for( a = 0 ; a < root->numChildren ; a++ )
	{
		moo_ReleaseSphereTreeNode( root->children[a] );
	}

	// release allocated memory
	SAFE_RELEASE( root->children );
	SAFE_RELEASE( root->inFaces );
	SAFE_RELEASE( root->inPoints );
	SAFE_RELEASE( root );
}
