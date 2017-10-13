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

