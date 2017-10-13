// Cognition
// mathlib.c
// Created 2-25-02 @ 0024 by Terrence Cole

// Includes
/////////////
#include "../cognition/cog_global.h"

// Definitions
////////////////
// Global Prototypes
//////////////////////
// Local Prototypes
/////////////////////
// Header Variables
/////////////////////
// see mathlib.h

// *********** FUNCTIONALITY ***********
/* ------------
m_Initialize
------------ */
int m_Initialize(void)
{
	int a;
	float b;
	double tStart, tEnd;

	ei->con_Print( "\n<BLUE>Building</BLUE> Trig Tables..." );
	tStart = ei->t_GetSaneTime();

	for( a=0 ; a<NUM_TRIG_STEPS ; a++ )
	{
		b = a*( fPI / ( NUM_TRIG_STEPS / 2 ) );
		sin_tbl_rad[a] = (float)sin( b );
		cos_tbl_rad[a] = (float)cos( b );
		tan_tbl_rad[a] = (float)tan( b );
	}

	for( a=0 ; a<NUM_TRIG_STEPS ; a++ )
	{
		b = a * ( 180.0f / (NUM_TRIG_STEPS / 2) );
		sin_tbl_deg[a] = (float)sin( DEGTORAD(b) );
		cos_tbl_deg[a] = (float)cos( DEGTORAD(b) );
		tan_tbl_deg[a] = (float)tan( DEGTORAD(b) );
	}

	tEnd = ei->t_GetSaneTime();
	ei->con_Print( "\tDone:  %.3f sec.", tEnd - tStart ); 

	return 1;
}

/* ------------
m_Terminate
------------ */
void m_Terminate(void)
{
}

/********** 3D Float **********/
#define DEGPERRAD 57.29577951f // 180 / PI
#define RADPERDEG 0.017453293f // PI / 180

/* ------------
------------ */
void m3f_RadToDeg( vec3 vAng, vec3 vOut )
{
	vOut[YAW] = vAng[YAW] * DEGPERRAD;
	vOut[PITCH] = vAng[PITCH] * DEGPERRAD;
}

/* ------------
------------ */
void m3f_DegToRad( vec3 vAng, vec3 vOut )
{
	vOut[YAW] = vAng[YAW] * RADPERDEG;
	vOut[PITCH] = vAng[PITCH] * RADPERDEG;
}

/* ------------
------------ */
void m3f_SphereToCartR( vec3 vAng, vec3 vDirOut )
{
/*
	z = r*cos(theta)*sin(phi)
	x = r*sin(theta)*sin(phi)
	y = r*cos(phi)
	theta = YAW;
	phi = PITCH;
*/
	float sinphi = (float)sin(vAng[PITCH]);
	vDirOut[Z_AXIS] = vAng[R_SP] * (float)cos( vAng[YAW] ) * sinphi;
	vDirOut[X_AXIS] = vAng[R_SP] * (float)sin( vAng[YAW] ) * sinphi;
	vDirOut[Y_AXIS] = vAng[R_SP] * (float)cos( vAng[PITCH] );
}

/* ------------
------------ */
void m3f_SphereToCartD( vec3 vAng, vec3 vDirOut )
{
	vec3 vTmp;
	m3f_DegToRad( vAng, vTmp );
	m3f_SphereToCartR( vTmp, vDirOut );
}

/* ------------
------------ */
void m3f_CartToSphereR( vec3 vDir, vec3 vAngOut )
{
	vAngOut[R_SP] = m3f_VecLength(vDir);
	vAngOut[YAW] = (float)atan2( vDir[X_AXIS], vDir[Z_AXIS] );
	vAngOut[PITCH] = (float)acos( vDir[Y_AXIS] / vAngOut[R_SP] );
}

/* ------------
------------ */
void m3f_CartToSphereD( vec3 vDir, vec3 vAngOut )
{
	vec3 vTmp;
	m3f_CartToSphereR( vDir, vTmp );
	m3f_RadToDeg( vTmp, vAngOut );
}

/* ------------
m3f_AngleFromVec
// computes the angles of a given vector (transforms to spherical)
------------ */
void m3f_AngleFromVec( vec2 vAngOut, vec3 vDir )
{
	float r = m3f_VecLength( vDir );
	vAngOut[PITCH] = (float)asin( (float)sqrt( vDir[X_AXIS] * vDir[X_AXIS] + vDir[Z_AXIS] * vDir[Z_AXIS] ) / r );
	if( vDir[Z_AXIS] ) vAngOut[YAW] = (float)atan( vDir[Y_AXIS] / -vDir[Z_AXIS] );
	else vAngOut[YAW] = 90.0f;
}

/* ------------
m3f_VecFromAngles - takes vAngs in the coordinate system of vAxisForward, and computes a forward vector in that angle coordinate system
				- useful for taking the players move angles and position to start a ray for collisions and intersections
------------ */
void m3f_VecFromAngles( vec3 vAngs, vec3 vAxisForward, vec3 vForward )
{

	float cy, sy, cx, sx, cz, sz;
	float vx, vy, vz;

	vx = vAxisForward[X_AXIS];
	vy = vAxisForward[Y_AXIS];
	vz = vAxisForward[Z_AXIS];

	cy = COS( vAngs[YAW] );
	sy = SIN( vAngs[YAW] );
	cx = COS( vAngs[PITCH] );
	sx = SIN( vAngs[PITCH] );
	cz = COS( vAngs[ROLL] );
	sz = SIN( vAngs[ROLL] );
	
	// do the computation... courtesy of my TI-89 (worth it's weight in gold)
	// Rz * Ry * Rx * Vin
	vForward[X_AXIS] = cx * (vz * sy * cz - vy * sz) + sx * ( vy * sy * cz + vz * sz) + vx * cy * cz;
	vForward[Y_AXIS] = cx * (vz * sy * sz + vy * cz) + sx * (vy * sy * sz - vz * cz) + vx * cy * sz;
	vForward[Z_AXIS] = vz * cx * cy + vy * sx* cy - vx * sy;
}

/* ------------
m_VecDot
------------ */
float m3f_VecDot( float *vec3_1, float *vec3_2 )
{
	return vec3_1[X_AXIS] * vec3_2[X_AXIS] +
		   vec3_1[Y_AXIS] * vec3_2[Y_AXIS] +
		   vec3_1[Z_AXIS] * vec3_2[Z_AXIS];
}

/* ------------
m_VecCross
------------ */
void m3f_VecCross( float *out, float *vec3_1, float *vec3_2 )
{
	out[X_AXIS] = ( vec3_1[Y_AXIS] * vec3_2[Z_AXIS] ) - ( vec3_1[Z_AXIS] * vec3_2[Y_AXIS] );
	out[Y_AXIS] = ( vec3_1[Z_AXIS] * vec3_2[X_AXIS] ) - ( vec3_1[X_AXIS] * vec3_2[Z_AXIS] );
	out[Z_AXIS] = ( vec3_1[X_AXIS] * vec3_2[Y_AXIS] ) - ( vec3_1[Y_AXIS] * vec3_2[X_AXIS] );
}

/* ------------
m_VecLength
------------ */
float m3f_VecLength( float *vec3in )
{
	return (float)sqrt( vec3in[X_AXIS] * vec3in[X_AXIS] +
						vec3in[Y_AXIS] * vec3in[Y_AXIS] + 
						vec3in[Z_AXIS] * vec3in[Z_AXIS] );
}

/* ------------
m_VecDistance
------------ */
float m3f_VecDistance( float *vec3p1, float *vec3p2 )
{
	vec3 out;

	m3f_VecSubtract( out, vec3p1, vec3p2 );
	return m3f_VecLength( out );
}

/* ------------
m_VecNormalize
------------ */
void m3f_VecUnitize( float *vec3in )
{
	float len, dlen;
	len = m3f_VecLength( vec3in );

	if( len ) {
		dlen = 1 / len;

		vec3in[X_AXIS] *= dlen;
		vec3in[Y_AXIS] *= dlen;
		vec3in[Z_AXIS] *= dlen;
	}
}

/* ------------
m_VecAdd
------------ */
void m3f_VecAdd( float *vec3out, float *vec3_1, float *vec3_2 )
{
	vec3out[X_AXIS] = vec3_1[X_AXIS] + vec3_2[X_AXIS];
	vec3out[Y_AXIS] = vec3_1[Y_AXIS] + vec3_2[Y_AXIS];
	vec3out[Z_AXIS] = vec3_1[Z_AXIS] + vec3_2[Z_AXIS];
}

/* ------------
m_VecSubtract
------------ */
void m3f_VecSubtract( float *vec3out, float *vec3_1, float *vec3_2 )
{
	vec3out[X_AXIS] = vec3_1[X_AXIS] - vec3_2[X_AXIS];
	vec3out[Y_AXIS] = vec3_1[Y_AXIS] - vec3_2[Y_AXIS];
	vec3out[Z_AXIS] = vec3_1[Z_AXIS] - vec3_2[Z_AXIS];
}

/* ------------
m_VecScale
------------ */
void m3f_VecScale( float *vec3out, float scalar, float *vec3in )
{
	vec3out[X_AXIS] = vec3in[X_AXIS] * scalar;
	vec3out[Y_AXIS] = vec3in[Y_AXIS] * scalar;
	vec3out[Z_AXIS] = vec3in[Z_AXIS] * scalar;
}

/* ------------
m_VecInterpolate
------------ */
void  m3f_VecInterpolate( float *vec3out, float *vec3_1, float *vec3_2, float position )
{
	vec3out[X_AXIS] = vec3_1[X_AXIS] - vec3_2[X_AXIS];
	vec3out[X_AXIS] = vec3_2[X_AXIS] + ( position * vec3out[X_AXIS] );

	vec3out[Y_AXIS] = vec3_1[Y_AXIS] - vec3_2[Y_AXIS];
	vec3out[Y_AXIS] = vec3_2[Y_AXIS] + ( position * vec3out[Y_AXIS] );

	vec3out[Z_AXIS] = vec3_1[Z_AXIS] - vec3_2[Z_AXIS];
	vec3out[Z_AXIS] = vec3_2[Z_AXIS] + ( position * vec3out[Z_AXIS] );
}

// specified ccw
/* ------------
m_CalcNormal
------------ */
void m3f_CalcNormal( float *vec3p1, float *vec3p2, float *vec3p3, float *vec3out )
{
	vec3 r1, r2;

	// ray 1 = *p2 -> *p1
	r1[X_AXIS] = vec3p1[X_AXIS] - vec3p2[X_AXIS];
	r1[Y_AXIS] = vec3p1[Y_AXIS] - vec3p2[Y_AXIS];
	r1[Z_AXIS] = vec3p1[Z_AXIS] - vec3p2[Z_AXIS];

	// ray 2 = *p2 -> *p3
	r2[X_AXIS] = vec3p2[X_AXIS] - vec3p3[X_AXIS];
	r2[Y_AXIS] = vec3p2[Y_AXIS] - vec3p3[Y_AXIS];
	r2[Z_AXIS] = vec3p2[Z_AXIS] - vec3p3[Z_AXIS];

	// cross the two rays
	vec3out[X_AXIS] = r1[Y_AXIS]*r2[Z_AXIS] - r1[Z_AXIS]*r2[Y_AXIS];
	vec3out[Y_AXIS] = r1[Z_AXIS]*r2[X_AXIS] - r1[X_AXIS]*r2[Z_AXIS];
	vec3out[Z_AXIS] = r1[X_AXIS]*r2[Y_AXIS] - r1[Y_AXIS]*r2[X_AXIS];

	m3f_VecUnitize( vec3out );
}

/* ------------
m3f_VecAngle - returns the angle between v1 and v2 in radians
------------ */
float m3f_VecAngle( vec3 v1, vec3 v2 )
{
	return (float)acos( m3f_VecDot( v1, v2 ) / ( m3f_VecLength(v1) * m3f_VecLength(v2) ) );
}

/* ------------
m3f_TransformToGlobal - transforms v1 with pos and rot and stores it to out
					  - all vectors except for out should be initialized
------------ */
void m3f_TransformToGlobal( vec3 out, const vec3 v1, const vec3 pos, const vec2 rot )
{
	// Mn = MryMrpMt
	// ...I love my TI-89

	float sinx = SINd(rot[X_AXIS]);
	float siny = SINd(rot[Y_AXIS]);
	float cosx = COSd(rot[X_AXIS]);
	float cosy = COSd(rot[Y_AXIS]);

	out[X_AXIS] = v1[X_AXIS]*cosy + v1[Z_AXIS]*siny + pos[X_AXIS];
	out[Y_AXIS] = v1[Y_AXIS]*cosx + sinx * (v1[X_AXIS]*siny - v1[Z_AXIS]*cosy) + pos[Y_AXIS];
	out[Z_AXIS] = cosx * (v1[Z_AXIS]*cosy - v1[X_AXIS]*siny) + v1[Y_AXIS]*sinx + pos[Z_AXIS];
}


/********** MATRIX MATH **********/
/* ------------
m44f_MatMultiply
------------ */
void m44f_MatMultiply( matrix44f mat1, matrix44f mat2, matrix44f out )
{
	int r, c;

	memset( out, 0, sizeof(matrix44f) );

	for( r=0 ; r<4 ; r++ )
	{
		for( c=0 ; c<4 ; c++ )
		{
			out[r][c] += mat1[r][0] * mat2[0][c];
			out[r][c] += mat1[r][1] * mat2[1][c];
			out[r][c] += mat1[r][2] * mat2[2][c];
			out[r][c] += mat1[r][3] * mat2[3][c];
		}
	}
}

/* ------------
m44f_MatVec3Multiply
------------ */
void m44f_MatVec3Multiply( matrix44f mat, vec3 vec, vec3 out )
{
	// a[i]*b[i][0] + b[3]&1, a[i]*b[i][1] + b[3]&2, a[i]*b[i][2] + b[3]&3 )

	out[X_AXIS] = vec[X_AXIS] * mat[0][0] \
				+ vec[Y_AXIS] * mat[0][1] \
				+ vec[Z_AXIS] * mat[0][2] \
	 			+ mat[0][3];

	out[Y_AXIS] = vec[X_AXIS] * mat[1][0] \
				+ vec[Y_AXIS] * mat[1][1] \
				+ vec[Z_AXIS] * mat[1][2] \
				+ mat[1][3];

	out[Z_AXIS] = vec[X_AXIS] * mat[2][0] \
				+ vec[Y_AXIS] * mat[2][1] \
				+ vec[Z_AXIS] * mat[2][2] \
				+ mat[2][3];
}

/* ------------
m44f_MatVec4Multiply
------------ */
void m44f_MatVec4Multiply( matrix44f mat, vec4 vec, vec4 out )
{
	out[X_AXIS] = vec[X_AXIS] * mat[0][0] \
				+ vec[Y_AXIS] * mat[0][1] \
				+ vec[Z_AXIS] * mat[0][2] \
	 			+ vec[W_AXIS] * mat[0][3];

	out[Y_AXIS] = vec[X_AXIS] * mat[1][0] \
				+ vec[Y_AXIS] * mat[1][1] \
				+ vec[Z_AXIS] * mat[1][2] \
				+ vec[W_AXIS] * mat[1][3];

	out[Z_AXIS] = vec[X_AXIS] * mat[2][0] \
				+ vec[Y_AXIS] * mat[2][1] \
				+ vec[Z_AXIS] * mat[2][2] \
				+ vec[W_AXIS] * mat[2][3];

	out[W_AXIS] = vec[X_AXIS] * mat[3][0] \
				+ vec[Y_AXIS] * mat[3][1] \
				+ vec[Z_AXIS] * mat[3][2] \
				+ vec[W_AXIS] * mat[3][3];
}

/********** INTERSECTION MATH **********/

/* ------------
m_PointInSphere - returns 1 if 'pt' is inside the sphere with 'center' and 'radius' and 0 if it is outside
------------ */
byte m_PointInSphere( float *pt, float *center, float radius )
{
	vec3 vPtToRad;

	m3f_VecSubtract( vPtToRad, center, pt );
	if( m3f_VecLength(vPtToRad) > radius )
		return 0;

	return 1;
}

/* ------------
m_RaySphereIntersect - returns 1 if the ray described by 'start' and 'dir' intersects the sphere of 'center' and 'radius'
					 - 'hit', 'norm', and 'reflection' must point to buffers > 3, and will contain information about the
					 - intersection, if return > 0.  Returns 0 if the ray and sphere do not intersect
------------ */
byte m_RaySphereIntersect( float *hit, float *norm, float *reflection, float *start, float *dir, float *center, float radius )
{
	float A, B, C, discriminant, t0, invRad;
	vec3 vLight, vTemp;
	float fTemp;

	A = (dir[X_AXIS] * dir[X_AXIS]) + 
		(dir[Y_AXIS] * dir[Y_AXIS]) + 
		(dir[Z_AXIS] * dir[Z_AXIS]);  // 3 mult, 2 add
	B = (2 * dir[X_AXIS] * (start[X_AXIS] - center[X_AXIS])) + 
		(2 * dir[Y_AXIS] * (start[Y_AXIS] - center[Y_AXIS])) +
		(2 * dir[Z_AXIS] * (start[Z_AXIS] - center[Z_AXIS])); // 6 mult, 3 subtr
	C = (center[X_AXIS] * center[X_AXIS]) +
		(center[Y_AXIS] * center[Y_AXIS]) +
		(center[Z_AXIS] * center[Z_AXIS]) +
		(start[X_AXIS] * start[X_AXIS]) +
		(start[Y_AXIS] * start[Y_AXIS]) +
		(start[Z_AXIS] * start[Z_AXIS]) -
		( 2 * ( (center[X_AXIS] * start[X_AXIS]) + 
				(center[Y_AXIS] * start[Y_AXIS]) + 
				(center[Z_AXIS] * start[Z_AXIS]) +
				(radius * radius) ) ); // 11 mult, 8 add, 1 subtr

	discriminant = (B * B) + (4 * A * C);

	if( discriminant < 0 ) return 0;

	t0 = ((-B) - (float)sqrt(discriminant)) / (2 * A);

	if( t0 < 0 ) return 0;

	hit[X_AXIS] = start[X_AXIS] + dir[X_AXIS] * t0;
	hit[Y_AXIS] = start[Y_AXIS] + dir[Y_AXIS] * t0;
	hit[Z_AXIS] = start[Z_AXIS] + dir[Z_AXIS] * t0;

	invRad = 1 / radius;

	norm[X_AXIS] = (hit[X_AXIS] - center[X_AXIS]) * invRad;
	norm[Y_AXIS] = (hit[Y_AXIS] - center[Y_AXIS]) * invRad;
	norm[Z_AXIS] = (hit[Z_AXIS] - center[Z_AXIS]) * invRad;	
	
	//		   L	N    R
	//			\	|   /
	//			  \	| /
	//			_________
	// reflection vector is R = 2(N*L)N - L

	m3f_VecSubtract( vLight, start, hit );
	m3f_VecUnitize( vLight );

	fTemp = 2 * m3f_VecDot( norm, vLight );
	m3f_VecScale( vTemp, fTemp, norm );

	m3f_VecSubtract( reflection, vTemp, vLight );

	return 1;
}

/* ------------
m_PointInBox - includes box bounds
------------ */
byte m_PointInBox( float *pt, aabb_t *bbox )
{
	if( (pt[0] >= bbox->lobounds[0]) && (pt[0] <= bbox->hibounds[0]) &&
		(pt[1] >= bbox->lobounds[1]) && (pt[1] <= bbox->hibounds[1]) &&
		(pt[2] >= bbox->lobounds[2]) && (pt[2] <= bbox->hibounds[2]) )
	{
		return 1;
	}

	return 0;
}

/* ------------
m_RayBoxIntersect
tNear/Far == pointers to a single float to take the value of the nearest/farthest t
start/dir == 3-float arrays; start/direction of the ray
bbox == the itersection box
------------ */
byte m_RayBoxIntersect( float *tNear, float *tFar, vec3 start, vec3 dir, aabb_t *bbox )
{
	byte a;
	float t1, t2, temp;
	float tnear = -999999.9f;
	float tfar = 999999.9f;
	*tNear = 0.0f;
	*tFar = 0.0f;

	if( m_PointInBox( start, bbox ) ) return 1;
	
	for( a=0 ; a<3 ; a++ )
	{
		// check for simple exclusions
		if( (dir[a] == 0) && ( (start[a] > bbox->hibounds[a]) || (start[a] < bbox->lobounds[a]) ) )
			return 0;

		// get a distance to each plane
		t1 = ( bbox->lobounds[a] - start[a] ) / dir[a];
		t2 = ( bbox->hibounds[a] - start[a] ) / dir[a];

		// the farther plane is always t2
		if( t1 > t2 ) // swap
		{
			temp = t1;
			t1 = t2;
			t2 = temp;
		}

		// we need to cross the near plane first on EVERY axis
		if( t1 > tnear ) tnear = t1;
		if( t2 < tfar ) tfar = t2;

		if( tnear > tfar ) return 0;
		if( tfar < 0 ) return 0;
	}

	*tNear = tnear;
	*tFar = tfar;
	return 1;
}

/* ------------
m_RayPlaneIntersect
------------ */
byte m_RayPlaneIntersect( float *hit, float *reflection, float *start, float *dir, float *norm, float d )
{
	float V0, Vd, t, fTemp;
	vec3 vLight, vTemp;

	Vd = m3f_VecDot( norm, dir );
	if( Vd >= 0 )  
		return 0; // parallel to or back facing

	V0 = -( m3f_VecDot( norm, start ) + d );
	if( ABS(V0) < FLOAT_ZERO_TOLERANCE ) V0 = 0;

	t = V0 / Vd;
	if( t <= 0 ) // plane is behind the ray
		return 0;

	hit[X_AXIS] = start[X_AXIS] + dir[X_AXIS] * t;
	hit[Y_AXIS] = start[Y_AXIS] + dir[Y_AXIS] * t;
	hit[Z_AXIS] = start[Z_AXIS] + dir[Z_AXIS] * t;

//		   L	N    R
//			\	|   /
//			  \	| /
//			_________
// reflection vector is R = 2(N*L)N - L

	m3f_VecSubtract( vLight, start, hit );
	m3f_VecUnitize( vLight );

	fTemp = 2 * m3f_VecDot( norm, vLight );
	m3f_VecScale( vTemp, fTemp, norm );

	m3f_VecSubtract( reflection, vTemp, vLight );

	return 1;
}

/* ------------
m_RayTriIntersect
------------ */
byte m_RayTriIntersect( float *hit, float *reflection, float *start, float *dir, float *v0, float *v1, float *v2, float *norm, float d )
{
	byte u, v, a, b;
	vec2 p[3];
	signed char sv1, sv2;
	signed char su1, su2;
	float m, t;
	byte nhits;

	if( !m_RayPlaneIntersect( hit, reflection, start, dir, norm, d ) ) return 0;

	// find the dominate plane axis
	if( (norm[Y_AXIS] >= norm[X_AXIS]) && (norm[Y_AXIS] >= norm[Z_AXIS]) )
	{
		u = X_AXIS;
		v = Z_AXIS;
	}
	else if( (norm[X_AXIS] >= norm[Y_AXIS]) && (norm[X_AXIS] >= norm[Z_AXIS]) )
	{
		u = Y_AXIS;
		v = Z_AXIS;
	}
	else
	{
		u = X_AXIS;
		v = Y_AXIS;
	}

	// translate the poly to the hit
	p[0][0] = v0[u] - hit[u];
	p[0][1] = v0[v] - hit[v];
	p[1][0] = v1[u] - hit[u];
	p[1][1] = v1[v] - hit[v];
	p[2][0] = v2[u] - hit[u];
	p[2][1] = v2[v] - hit[v];

	// this gives us three lines p1->p2, p2->p3, p3->p1
	nhits = 0;

	for( a=0, b=1 ; a<3 ; a++, b++ )
	{
		if( b >= 3 ) b = 0;

		// check for failure on either side of the line
		if( p[a][1] > 0 ) { sv1 = 1; }
		else { sv1 = -1; }
		if( p[b][1] > 0 ) { sv2 = 1; }
		else { sv2 = -1; }
		if( sv1 == sv2 ) continue; // on either side of the line

		// check for behind, infront of, or indeterminate
		if( p[a][0] < 0 ) { su1 = -1; }
		else { su1 = 1; }
		if( p[b][0] < 0 ) { su2 = -1; }
		else { su2 = 1; }

		if( su1 == su2  )  // line is definatly one way or the other
		{
			if( su1 < 0 )
			{
				continue;  // behind the point
			}
			else
			{
				nhits++; // add a hit
				continue;
			}
		}
		else
		{
			// no obvious solution so get an intersect
			m = ( p[a][1] - p[b][1] ) / ( p[a][0] - p[b][0] );
			t = p[a][0] - ( p[a][1] / m );
			if( t < 0 )
			{
				continue;
			}
			else
			{
				nhits++;
				continue;
			}
		}
	}

	// odd hits == in poly
	if( nhits == 0 ) return 0;
	if( nhits == 1 ) return 1;

	// check for the rare, but annoying, vertex hit
	for( a=0 ; a<3 ; a++ )
	{
		t = p[a][0] - hit[u];
		if( ABS(t) < FLOAT_ZERO_TOLERANCE ) // hit on u axis
		{
			t = p[a][1] - hit[v];
			if( ABS(t) < FLOAT_ZERO_TOLERANCE ) // hit on v axis
				return 1;
		}
	}

	return 0;
}
