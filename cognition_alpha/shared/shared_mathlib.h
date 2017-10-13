// Cognition
// mathlib.h
// Created 2-25-02 @ 0032 by Terrence Cole

#ifndef _MATHLIB_H
#define _MATHLIB_H

// Includes
/////////////
#include "shared_misc.h"
#include "shared_structures.h"

// Definitions
////////////////
#define dPI 3.14159265359
#define fPI 3.14159265359f
#define PI  3.14159265359

#define dTWOPI 6.28318530718
#define fTWOPI 6.28318530718f
#define TWOPI 6.28318530718

#define NUM_TRIG_STEPS 8192

#define TRIG_TRANSFORM_RAD 1303.797f
#define TRIG_TRANSFORM_DEG 22.755556f

#define FLOAT_ZERO_TOLERANCE 0.0001

// ABSOLUTE VALUE 
#ifdef ABS
#undef ABS
#endif // ABS

#define ABS(x) (((x) < 0) ? -(x) : (x))

// SINE 
	// degrees
#ifdef SIN
#undef SIN
#endif // SIN

#ifdef dSIN
#undef dSIN
#endif // dSIN

#ifdef SINd
#undef SINd
#endif // SINd

#define  SIN(x) sin_tbl_deg[ ABS( (int)( x * TRIG_TRANSFORM_DEG ) ) ]
#define dSIN(x) sin_tbl_deg[ ABS( (int)( x * TRIG_TRANSFORM_DEG ) ) ]
#define SINd(x) sin_tbl_deg[ ABS( (int)( x * TRIG_TRANSFORM_DEG ) ) ]

	// radians
#ifdef rSIN
#undef rSIN
#endif // rSIN

#ifdef SINr
#undef SINr
#endif // SINr

#define rSIN(x) sin_tbl_rad[ ABS( (int)( x * TRIG_TRANSFORM_RAD ) ) ]
#define SINr(x) sin_tbl_rad[ ABS( (int)( x * TRIG_TRANSFORM_RAD ) ) ]

// COSINE 
	// degrees
#ifdef COS
#undef COS
#endif // COS

#ifdef dCOS
#undef dCOS
#endif // dCOS

#ifdef COSd
#undef COSd
#endif // COSd
	
#define  COS(x) cos_tbl_deg[ ABS( (int)( x * TRIG_TRANSFORM_DEG ) ) ]
#define dCOS(x) cos_tbl_deg[ ABS( (int)( x * TRIG_TRANSFORM_DEG ) ) ]
#define COSd(x) cos_tbl_deg[ ABS( (int)( x * TRIG_TRANSFORM_DEG ) ) ]

	// radians
#ifdef rCOS
#undef rCOS
#endif // rCOS

#ifdef COSr
#undef COSr
#endif // COSr


#define rCOS(x) cos_tbl_rad[ ABS( (int)( x * TRIG_TRANSFORM_RAD ) ) ]
#define COSr(x) cos_tbl_rad[ ABS( (int)( x * TRIG_TRANSFORM_RAD ) ) ]

// TANGENT 
	// degrees
#ifdef TAN
#undef TAN
#endif // TAN

#ifdef dTAN
#undef dTAN
#endif // dTAN

#ifdef TANd
#undef TANd
#endif // TANd

#define  TAN(x) tan_tbl_deg[ ABS( (int)( x * TRIG_TRANSFORM_DEG) ) ]
#define dTAN(x) tan_tbl_deg[ ABS( (int)( x * TRIG_TRANSFORM_DEG) ) ]
#define TANd(x) tan_tbl_deg[ ABS( (int)( x * TRIG_TRANSFORM_DEG) ) ]

	// radians
#ifdef rTAN
#undef rTAN
#endif // rTAN

#ifdef TANr
#undef TANr
#endif // TANr

#define rTAN(x) tan_tbl_rad[ ABS( (int)( x * TRIG_TRANSFORM_RAD) ) ]
#define TANr(x) tan_tbl_rad[ ABS( (int)( x * TRIG_TRANSFORM_RAD) ) ]

// CONVERSIONS 
#ifdef DEGTORAD
#undef DEGTORAD
#endif // DEGTORAD

#define DEGTORAD(x) ( x * fPI / 180.0f )

#ifdef RADTODEG
#undef RADTODEG
#endif // RADTODEG

#define RADTODEG(x) ( x * 180.0f / fPI )

// Header Variables
/////////////////////
float sin_tbl_rad[NUM_TRIG_STEPS];
float cos_tbl_rad[NUM_TRIG_STEPS];
float tan_tbl_rad[NUM_TRIG_STEPS];

float sin_tbl_deg[NUM_TRIG_STEPS];
float cos_tbl_deg[NUM_TRIG_STEPS];
float tan_tbl_deg[NUM_TRIG_STEPS];

// Prototypes
///////////////
int m_Initialize(void);
void m_Terminate(void);

// 2D

// 3D
void m3f_RadToDeg( vec3 vAng, vec3 vOut );
void m3f_DegToRad( vec3 vAng, vec3 vOut );
void m3f_SphereToCartR( vec3 vAng, vec3 vDirOut );
void m3f_SphereToCartD( vec3 vAng, vec3 vDirOut );
void m3f_CartToSphereR( vec3 vDir, vec3 vAngOut );
void m3f_CartToSphereD( vec3 vDir, vec3 vAngOut );
void m3f_AngleFromVec( vec2 vAngOut, vec3 vDir );
void m3f_VecFromAngles( vec3 vAngs, vec3 vAxisForward, vec3 vForward );
float m3f_VecDot( float *vec3_1, float *vec3_2 );
void m3f_VecCross( float *out, float *vec3_1, float *vec3_2 );
float m3f_VecLength( float *vec3in );
float m3f_VecDistance( float *vec3p1, float *vec3p2 );
void m3f_VecUnitize( float *vec3in );
void m3f_VecAdd( float *vec3out, float *vec3_1, float *vec3_2 );
void m3f_VecSubtract( float *vec3out, float *vec3_1, float *vec3_2 );
void m3f_VecScale( float *vec3out, float scalar, float *vec3in );
void m3f_VecInterpolate( float *vec3out, float *vec3_1, float *vec3_2, float position );
void m3f_CalcNormal( float *vec3p1, float *vec3p2, float *vec3p3, float *vec3out );
float m3f_VecAngle( vec3 v1, vec3 v2 );
void m3f_TransformToGlobal( vec3 out, const vec3 v1, const vec3 pos, const vec2 rot );

// matrix math
void m44f_MatMultiply( matrix44f mat1, matrix44f mat2, matrix44f out );
void m44f_MatVec3Multiply( matrix44f mat, vec3 vec, vec3 out );
void m44f_MatVec4Multiply( matrix44f mat, vec4 vec, vec4 out );

// collision math
byte m_PointInSphere( float *pt, float *center, float radius );
byte m_RaySphereIntersect( float *hit, float *norm, float *reflection, float *start, float *dir, float *center, float radius );
byte m_PointInBox( float *pt, aabb_t *bbox );
byte m_RayBoxIntersect( float *tNear, float *tFar, vec3 start, vec3 dir, aabb_t *bbox );
byte m_RayPlaneIntersect( float *hit, float *reflection, float *start, float *dir, float *norm, float d );
byte m_RayTriIntersect( float *hit, float *reflection, float *start, float *dir, float *v0, float *v1, float *v2, float *norm, float d );

#endif // _MATHLIB_H
