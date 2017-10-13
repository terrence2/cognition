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

// shared_queue.h
// General Queue
// by Terrence Cole 1/11/04

#ifndef _SHARED_QUEUE_H_
#define _SHARED_QUEUE_H_

// Definitions
////////////////
typedef int queue_t;
#define NULL_QUEUE -1

// PROTOTYPES
///////////////
int q_Initialize(void);
void q_Terminate(void);
queue_t q_Create( int size );
void q_Delete( queue_t q );
int q_Enqueue( queue_t q, void *data );
void *q_Dequeue( queue_t q );
void *q_LookAtHead( queue_t q );
void *q_GetAt( queue_t q, int offset );
int q_Count( queue_t q );
void **q_GetBase( queue_t q );


#endif // _SHARED_QUEUE_H_
