// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

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
engine_interface_t *ei(); // CogMenu.cpp
void SetCurrentScreen( CognitionWidgets::Screen *scr ); // CogMenu.cpp

#endif // _COGGLOBAL_H_
