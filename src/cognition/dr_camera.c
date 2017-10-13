// Cognition
// dr_camera.c
// by Terrence Cole 12/3/02

// Includes
/////////////
#include "cog_global.h"
#include "cog_opengl.h"

// Definitions
////////////////

// Structures
///////////////

// Global Prototypes
//////////////////////
/*
int d_CamInitialize(void);
void d_CamTerminate(void);
void d_SetCamera( camera_t *cam );
camera_t *d_GetCamera();
void d_MoveCamera( float movespeed, float *vAng,  float *vMvReq, float *vOut );
void d_ComputeFrameFrustum( camera_t *cam );
void d_CameraIn();
void d_CameraOut();
*/

// Local Prototypes
/////////////////////

// Local Variables
////////////////////
static camera_t camBase;
static camera_t *camDefault = &camBase;
static camera_t *camCurrent = NULL;

// *********** FUNCTIONALITY ***********
/* ------------
d_CamInitialize
------------ */
int d_CamInitialize(void)
{
	// set a camera for the player
	memset( &camBase, 0, sizeof(camera_t) );
	camBase.type = CAMERA_PROJECTION;
	camBase.vPos[0] = camBase.vPos[1] = camBase.vPos[2] = 0.0f;
	camBase.vAng[0] = camBase.vAng[1] = 0.0f;
	camBase.fnear = 1.0f;
	camBase.ffar = 20000.0f;
	camBase.ffov = 45.0f;
	d_SetCamera( &camBase );
	return 1;
}

/* ------------
d_CamTerminate
------------ */
void d_CamTerminate(void)
{
}

/* ------------
d_SetCamera - sets the current camera, which is used to track frustum position and angles, draw pos, etc
			- also sets the viewport for the camera's view of the scene
------------ */
void d_SetCamera( camera_t *cam )
{
	float fTan, fbottom, ftop, fleft, fright;

	// set the current camera
	camCurrent = cam;

	// if cam is NULL, use the base camera instead
	if( cam == NULL )
	{
		cam = &camBase;
	}
	
	// Clear the projection transformation
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	
	if( cam->type == CAMERA_PROJECTION )
	{
		fTan = TANd( cam->ffov );

		// compute the primary angle left and right
		fleft = -cam->fnear * fTan;
		fright = -fleft;

		// use the aspect to get the top and bottom
		fbottom = fleft / gl_state.gl_aspect;
		ftop = fright / gl_state.gl_aspect;

		// set the view volume
		glFrustum( fleft, fright, fbottom, ftop, cam->fnear, cam->ffar );
	}
	else
	{
		glOrtho( 0.0f, cam->fright, 0.0f, cam->ftop, cam->fnear, cam->ffar );
	}
}

/* ------------
d_GetCamera - returns a pointer to the current camera, from set, or default if unavailable
------------ */
camera_t *d_GetCamera()
{
	// never return a null camera
	if( camCurrent == NULL ) return camDefault;
	
	return camCurrent;
}

/* ------------
d_MoveCamera - takes a player angle and a move request with respect to that system
				- angles are PITCH and YAW clamped [0,360)
------------ */
void d_MoveCamera( float movespeed, float *vAng,  float *vMvReq, float *vOut )
{
	// rotation ordering: Yaw, Pitch, Roll
	float move[3];
	float cx, sx, cy, sy;

	// assert
	if( vAng == NULL ) return;
	if( vMvReq == NULL ) return;
	if( vOut == NULL ) return;

	// zero the output, and make a copy of the move so it is preserved between frames
	memcpy( move, vMvReq, sizeof(vec3) );
	memset( vOut, 0, sizeof(vec3) );

	// set up the request vector
	m3f_VecUnitize( move );
	m3f_VecScale( move, ((float)state.delta_time / 1000.0f) * movespeed, move );

	// trade some memory for processor cycles... a bunch of processor cycles for a little memory
	cy = COS( vAng[YAW] );
	sy = SIN( vAng[YAW] );
	cx = COS( vAng[PITCH] );
	sx = SIN( vAng[PITCH] );

	// do the computation... courtesy of my TI-89 (worth it's weight in gold)
	vOut[X_AXIS] = (move[Z_AXIS] * cx * sy) + (move[Y_AXIS] * sx * sy) + (move[X_AXIS] * cy);
	vOut[Y_AXIS] = (move[Y_AXIS] * cx) - (move[Z_AXIS] * sx);
	vOut[Z_AXIS] = (move[Z_AXIS] * cx * cy) + (move[Y_AXIS] * sx * cy) - (move[X_AXIS] * sy);
}

/* ------------
d_ComputeFrameFrustum - takes the position and angle of the camera, and computes frustum planes on which to cull entities 
------------ */
void d_ComputeFrameFrustum( camera_t *cam )
{
	// counters
	int a;
	
	// axis in
	vec3 vForward = { 0.0f, 0.0f, -1.0f };
	vec3 vRight = { 1.0f, 0.0f, 0.0f };
	vec3 vHead = { 0.0f, 1.0f, 0.0f };
	
	// distances
	float fNearWidth, fNearHeight;
	float fFarWidth, fFarHeight;
	
	// transfer vectors
	vec3 vNear, vFar;
	vec3 vNearWidth, vFarWidth;
	vec3 vNearHeight, vFarHeight;
	
	// necessary frustum corners
	vec3 vFarTopRight, vFarTopLeft;
	vec3 vFarBottomRight, vFarBottomLeft;
	vec3 vNearTopRight, vNearBottomLeft;
	vec3 vNearTopLeft;
	
	// move our axis into the angled coordinate system
	m3f_VecFromAngles( cam->vAng, vForward, cam->vFwd );
	m3f_VecFromAngles( cam->vAng, vRight, cam->vRt );
	m3f_VecFromAngles( cam->vAng, vHead, cam->vUp );

	// work out the near and far frustum vectors directly
	m3f_VecScale( vNear, cam->fnear, cam->vFwd );
	m3f_VecScale( vFar, cam->ffar, cam->vFwd );
	
	// find the width and height of the viewport at the near and far distance
	fNearWidth = cam->fnear * TAN( cam->ffov );
	fNearHeight = fNearWidth / gl_state.gl_aspect;
	fFarWidth = cam->ffar * TAN( cam->ffov );
	fFarHeight = fFarWidth / gl_state.gl_aspect;
	
	// find the vector widths / heights
	m3f_VecScale( vNearWidth, fNearWidth, cam->vRt );
	m3f_VecScale( vNearHeight, fNearHeight, cam->vUp );
	m3f_VecScale( vFarWidth, fFarWidth, cam->vRt );
	m3f_VecScale( vFarHeight, fFarHeight, cam->vUp );
	
	// compute frustum corners so that we can compute planes
	for( a = 0 ; a < 3 ; a++ )
	{
		// necessary
		vFarTopRight[a] = vFar[a] + vFarHeight[a] + vFarWidth[a];
		vFarTopLeft[a] = vFar[a] + vFarHeight[a] - vFarWidth[a];
		vFarBottomLeft[a] = vFar[a] - vFarHeight[a] - vFarWidth[a];
		vFarBottomRight[a] = vFar[a] - vFarHeight[a] + vFarWidth[a];
		vNearTopRight[a] = vNear[a] + vNearHeight[a] + vNearWidth[a];
		vNearBottomLeft[a] = vNear[a] - vNearHeight[a] - vNearWidth[a];
		vNearTopLeft[a] = vNear[a] + vNearHeight[a] - vNearWidth[a];
	}

	// compute the left plane normal
	m3f_CalcNormal( vFarBottomLeft, vFarTopLeft, vNearTopLeft, cam->pLeft.norm );
	// compute the right plane normal
	m3f_CalcNormal( vNearTopRight, vFarTopRight, vFarBottomRight, cam->pRight.norm );
	// compute the top plane normal
	m3f_CalcNormal( vNearTopLeft, vFarTopLeft, vFarTopRight, cam->pTop.norm );
	// compute the bottom plane normal
	m3f_CalcNormal( vFarBottomRight, vFarBottomLeft, vNearBottomLeft, cam->pBottom.norm );
	// compute the near plane normal
	m3f_CalcNormal( vNearBottomLeft, vNearTopLeft, vNearTopRight, cam->pNear.norm );
	// compute the far plane normal
	m3f_CalcNormal( vFarTopRight, vFarTopLeft, vFarBottomLeft, cam->pFar.norm );

	// we need to add our extended frustum points with our view position to get points on the real frustum
	m3f_VecAdd( cam->pLeft.V, vFarTopLeft, cam->vPos );
	m3f_VecAdd( cam->pRight.V, vFarBottomRight, cam->vPos );
	m3f_VecAdd( cam->pTop.V, vFarTopLeft, cam->vPos );
	m3f_VecAdd( cam->pBottom.V, vFarBottomRight, cam->vPos );
	m3f_VecAdd( cam->pNear.V, vNearTopLeft, cam->vPos );
	m3f_VecAdd( cam->pFar.V, vFarTopLeft, cam->vPos );

	// compute D for each plane
	cam->pLeft.D = -m3f_VecDot( cam->pLeft.V, cam->pLeft.norm );
	cam->pRight.D = -m3f_VecDot( cam->pRight.V, cam->pRight.norm );
	cam->pTop.D = -m3f_VecDot( cam->pTop.V, cam->pTop.norm );
	cam->pBottom.D = -m3f_VecDot( cam->pBottom.V, cam->pBottom.norm );
	cam->pNear.D = -m3f_VecDot( cam->pNear.V, cam->pNear.norm );
	cam->pFar.D = -m3f_VecDot( cam->pFar.V, cam->pFar.norm );
}

/* ------------
d_CameraIn - preframe call, to setup all needed camera states, and set the modelview to the current camera
------------ */
void d_CameraIn()
{
	camera_t *camTmp;

	// select the best camera for this frame
	camTmp = d_GetCamera();

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
		glLoadIdentity();
		glRotatef( -camTmp->vAng[PITCH], 1.0f, 0.0f, 0.0f );
		glRotatef( -camTmp->vAng[YAW], 0.0f, 1.0f, 0.0f );
		glTranslatef( -camTmp->vPos[X_AXIS], -camTmp->vPos[Y_AXIS], -camTmp->vPos[Z_AXIS] );

	// set the current frustum
	d_ComputeFrameFrustum( camTmp );
}

/* ------------
d_CameraOut - postframe call to handle all camera related tasks and take the modelview out of the current camera
------------ */
void d_CameraOut()
{
	glPopMatrix();
}

