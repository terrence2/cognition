#ifndef _COGGLOBAL_H_
#define _COGGLOBAL_H_

extern "C"
{
#include "../shared/shared_structures.h"
#include "../shared/shared_misc.h"
#include "../shared/shared_keycodes.h"
}
#include "Screen.h"

// Global DEFS
////////////////
#define SAFE_DELETE( a ) if( a != NULL ) { delete a; a = NULL; }
#define SAFE_DELETEv( a ) if( a != NULL ) { delete [] a; a = NULL; }

// Global Prototypes
//////////////////////
inline engine_interface_t *ei(); // CogMenu.cpp
void SetCurrentScreen( CognitionWidgets::Screen *scr ); // CogMenu.cpp

#endif // _COGGLOBAL_H_