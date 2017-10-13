// Cognition
// menu_global.h
// created by Terrence Cole 4-1-02

#ifndef _COG_MENU_GLOBAL_H
#define _COG_MENU_GLOBAL_H

// Includes
/////////////
#include <string.h> // for memset
#include <stdio.h> // sprintf
#include "../shared/shared_misc.h"
#include "../shared/shared_structures.h"
#include "../shared/shared_keycodes.h"

// Definitions
////////////////
#define SAFE_RELEASE(a) if( (a) != NULL ) { ei->mem_free( (a) ); (a) = NULL; }

/*
#define MENU_ROOT 0
#define MENU_SINGLEPLAYER 1
#define MENU_MULTIPLAYER 2
#define MENU_OPTIONS 3
#define MENU_EXIT 0xFFFFFFFF
*/

// MENU TREE DEFS
#define ROOT_SINGLE_PLAYER 0
#define ROOT_MULTI_PLAYER 1
#define ROOT_OPTIONS 2
#define ROOT_NUM_CHILDREN 3

#define SINGLE_GAME 0
#define SINGLE_GAME2D 1
#define SINGLE_BACK 2
#define SINGLE_NUM_CHILDREN 3

#define MULTI_BACK 0
#define MULTI_NUM_CHILDREN 1

// Global Variables
/////////////////////
menu_interface_t mi;
engine_interface_t *ei;

/* ------------
menu_background.c
------------ */
int bg_LoadMenuBG();
void bg_UnloadMenuBG();
void bg_Animate( exec_state_t *state );

/* ------------
menu_main.c
------------ */
menu_interface_t *GetMenuAPI( engine_interface_t *engine_interface );
int menu_Initialize(void);
void menu_Terminate(void);
void menu_Animate( exec_state_t *state );
void menu_KeyPress( byte key, byte key_state );

/* ------------
menu_multi.c
------------ */
int multi_Initialize(void);
void multi_Terminate(void);

/* ------------
menu_root.c
------------ */
int root_Initialize(void);
void root_Terminate(void);

/* ------------
menu_single.c
------------ */
int single_Initialize(void);
void single_Terminate(void);

/* ------------
menu_tree.c
------------ */
int tree_Initialize( void );
void tree_Terminate( void );
void tree_Ascend();
void tree_Descend( int child );

/* ------------
shared_mathlib.c
------------ */
#include "../shared/shared_mathlib.h"

#endif // _COG_MENU_GLOBAL_H
