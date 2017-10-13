// Cognition
// sal_input_win_win32.h
// Created by Terrence Cole

// input system global prototypes for the win32 input driver

#ifndef SAL_INPUT_WIN_WIN32_H
#define SAL_INPUT_WIN_WIN32_H

int in_w32_Initialize(void);
void in_w32_Terminate(void);
void in_w32_ProcessEvents();
void in_w32_Event( unsigned char key, unsigned short down );
byte in_w32_MapKey( int key );

#endif // SAL_INPUT_WIN_WIN32_H