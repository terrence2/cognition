// Cognition
// Menu
// menu_tree.c
// Created  by Terrence Cole

/*
	This module defines how we move through the menu system.
	Each node represents a separate menu; it holds a pointer to
	the Initialization and Termination routines for each level.
*/

// Includes
/////////////
#include "menu_global.h"

// Definitions
////////////////

// Local Structures
/////////////////////
typedef struct menu_tree_node_s
{
	int (*init)(void);
	void (*term)(void);
	int32_t iNumChildren;
	struct menu_tree_node_s **children; // a numChildren sized block of pointers
	struct menu_tree_node_s *parent;
} menu_tree_node_t;

// Global Prototypes
//////////////////////
/*
int tree_Initialize( void );
void tree_Terminate( void );
void tree_Ascend();
void tree_Descend( int child );
*/

// Local Prototypes
/////////////////////

// Local Variables
////////////////////
static menu_tree_node_t *mtCurrent = NULL;

#define ROOT_MENU 0
#define SINGLE_MENU 1
#define MULTI_MENU 2
#define OPTIONS_MENU 3

static menu_tree_node_t menus[] =
{
/*0*/	{ root_Initialize, root_Terminate, ROOT_NUM_CHILDREN, NULL, NULL },
/*1*/	{ single_Initialize, single_Terminate, SINGLE_NUM_CHILDREN, NULL, &menus[ROOT_MENU] },
/*2*/	{ multi_Initialize, multi_Terminate, MULTI_NUM_CHILDREN, NULL, &menus[ROOT_MENU] },
};
static int iNumMenus = sizeof(menus) / sizeof(menu_tree_node_t);

// *********** FUNCTIONALITY ***********
/* ------------
tree_Initialize
------------ */
int tree_Initialize( void )
{
	menu_tree_node_t *mtTmp;

	ei->con_Print( "\n<BLUE>Initializing</BLUE> Menu Traversal Tree..." );

	// SINGLE
	mtTmp = &menus[SINGLE_MENU];
	mtTmp->children =  (menu_tree_node_t**)ei->mem_alloc( sizeof(menu_tree_node_t*) * mtTmp->iNumChildren );
	mtTmp->children[SINGLE_GAME] = NULL;
	mtTmp->children[SINGLE_GAME2D] = NULL;
	mtTmp->children[SINGLE_BACK] = &menus[ROOT_MENU];


	// MULTI
	mtTmp = &menus[MULTI_MENU];
	mtTmp->children =  (menu_tree_node_t**)ei->mem_alloc( sizeof(menu_tree_node_t*) * mtTmp->iNumChildren );
//	mtTmp->children[] = NULL;
//	mtTmp->children[] = NULL;
	mtTmp->children[MULTI_BACK] = &menus[ROOT_MENU];

	// OPTIONS
	mtTmp = &menus[OPTIONS_MENU];

	// ROOT
	mtTmp = &menus[ROOT_MENU];
	mtTmp->children = (menu_tree_node_t**)ei->mem_alloc( sizeof(menu_tree_node_t*) * mtTmp->iNumChildren );
	mtTmp->children[ROOT_SINGLE_PLAYER] = &menus[SINGLE_MENU];
	mtTmp->children[ROOT_MULTI_PLAYER] = &menus[ROOT_MENU];
	mtTmp->children[ROOT_OPTIONS] = NULL;

	// CURRENT
	mtCurrent = &menus[ROOT_MENU];

	// start
	if( !mtCurrent->init() )
	{
		ei->con_Print( "\t<RED>Failure." );
		return 0;
	}

	ei->con_Print( "\t<GREEN>Success.</GREEN>" );

	return 1;
}

/* ------------
tree_Terminate
------------ */
void tree_Terminate( void )
{
	int a;
	
	ei->con_Print( "\n<RED>Terminating</RED> Menu Traversal Tree..." );

	for( a = 0 ; a < iNumMenus ; a++ )
	{
		SAFE_RELEASE( menus[a].children );
		menus[a].term();
	}

	ei->con_Print( "\t<GREEN>Success." );
}

/* ------------
tree_Ascend
// should be called by the back button or escape key, etc
------------ */
void tree_Ascend()
{
	menu_tree_node_t *cur = mtCurrent;
	
	if( mtCurrent == NULL ) return;

	// ascend
	mtCurrent = mtCurrent->parent;

	// re-init or exit
	if( (mtCurrent == NULL) || (mtCurrent->init == NULL) )
	{
		// that was the last layer of menus (or we're broken)
		ei->eng_Stop( NULL );
	}
	else
	{
		mtCurrent->init();
	}

	// term
	if( cur->term != NULL ) cur->term();
}

/* ------------
tree_Descend
// called with different args by buttons on a menu, arg determines offset into children
------------ */
void tree_Descend( int child )
{
	if( mtCurrent == NULL ) return;
	if( mtCurrent->iNumChildren <= 0 ) return;
	if( mtCurrent->children == NULL ) return;
	if( child < 0 ) return;
	if( child >= mtCurrent->iNumChildren ) return;
	if( mtCurrent->children[child] == NULL ) return;
	if( mtCurrent->children[child]->init == NULL ) return;
	if( !mtCurrent->children[child]->init() ) return;

	mtCurrent->term();
	mtCurrent = mtCurrent->children[child];
}

