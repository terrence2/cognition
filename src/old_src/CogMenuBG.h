#ifndef _COGMENUBG_H_
#define _COGMENUBG_H_

/*
	The Main Menu Class for Base Cognition
*/

#include "CogMenuGlobal.h"

class CogMenuBG
{
public:
	CogMenuBG();
	~CogMenuBG();

	// Events
	void Frame( exec_state_t *state ); // a single animation step

private:
	entity_t *m_eSkybox;
	camera_t m_Camera;
	light_t *m_Light;
	entity_t *m_particles[2];
};

#endif // _COGMENUBG_H_

