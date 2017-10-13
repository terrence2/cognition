// Cognition
// sal_input_win_di.h
// Created by Terrence Cole

// input system global prototypes for the DirectInput input driver

#ifndef SAL_INPUT_WIN_DI_H
#define SAL_INPUT_WIN_DI_H

// the current version in this release
#define DIRECTINPUT_VERSION 0x0800

int in_di_Initialize(void);
void in_di_Terminate(void);
void in_di_ProcessEvents();

#endif // SAL_INPUT_WIN_DI_H