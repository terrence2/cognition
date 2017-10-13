// Cognition
// dr_lighting.c
// by Terrence Cole 12/3/02

// Includes
/////////////
#include "cog_global.h"
#include "cog_opengl.h"

// Definitions
////////////////
/*
#define LIGHT_NORMAL 0
#define LIGHT_SPOT 1
#define LIGHT_DIRECTIONAL 2
*/

// Structures
///////////////

// Global Prototypes
//////////////////////
/*
int l_Initialize(void);
void l_Terminate(void);
void l_KillAll();
light_t *l_Spawn( byte type, float *position, float *ambient, float *diffuse, float *specular, 
							 float spot_exp, float spot_cutoff, float *spot_dir, 
							 float const_atten, float linear_atten, float quad_atten );
void l_Kill( light_t *l );
void l_Draw();
void l_PrintInfo( char *null );
*/

// Local Prototypes
/////////////////////

// Local Variables
////////////////////
static light_t lights[GL_MAX_LIGHTS];
static byte l_usage[GL_MAX_LIGHTS];

// *********** FUNCTIONALITY ***********
/* ------------
l_Initialize - sets all lights to thier default state
------------ */
int l_Initialize(void)
{
	int a;

	con_Print( "\nInitializing Lights..." );

	for( a = 0 ; a < GL_MAX_LIGHTS ; a++ )
	{
		lights[a].type = LIGHT_NORMAL;
		lights[a].state = 0;
		lights[a].lightNum = a;
		lights[a].position[R_AXIS] = 0.0f;
		lights[a].position[G_AXIS] = 0.0f;
		lights[a].position[B_AXIS] = 0.0f;
		lights[a].position[A_AXIS] = 0.0f;
		lights[a].ambient[R_AXIS] = 0.5f;
		lights[a].ambient[G_AXIS] = 0.5f;
		lights[a].ambient[B_AXIS] = 0.5f;
		lights[a].ambient[A_AXIS] = 0.5f;
		lights[a].diffuse[R_AXIS] = 0.5f;
		lights[a].diffuse[G_AXIS] = 0.5f;
		lights[a].diffuse[B_AXIS] = 0.5f;
		lights[a].diffuse[A_AXIS] = 0.5f;
		lights[a].specular[R_AXIS] = 0.5f;
		lights[a].specular[G_AXIS] = 0.5f;
		lights[a].specular[B_AXIS] = 0.5f;
		lights[a].specular[A_AXIS] = 0.5f;
		lights[a].spot_exponent = 0;
		lights[a].spot_cutoff = 25;
		lights[a].spot_direction[X_AXIS] = 0.0f;
		lights[a].spot_direction[Y_AXIS] = 0.0f;
		lights[a].spot_direction[Z_AXIS] = -1.0f;
		lights[a].constant_attenuation = 1.0f;
		lights[a].linear_attenuation = 0.0f;
		lights[a].quadratic_attenuation = 0.0f;

		l_usage[a] = 0;
	}

	return 1;
}

/* ------------
l_Terminate
------------ */
void l_Terminate(void)
{
	con_Print( "\nTerminating Lighting System..." );
	l_KillAll();
}

/* ------------
l_KillAll - shuts off all lights
------------ */
void l_KillAll()
{
	int a;
	for( a = 0 ; a < GL_MAX_LIGHTS ; a++ )
	{
		lights[a].state = 0;
		l_usage[a] = 0;
	}
}

/* ------------
l_Spawn - returns a light with the given properties if an unused light exists or returns null if all lights are unavailable
------------ */
light_t *l_Spawn( byte type, float *position, float *ambient, float *diffuse, float *specular, 
							 float spot_exp, float spot_cutoff, float *spot_dir, 
							 float const_atten, float linear_atten, float quad_atten )
{
	int a;

	for( a = 0 ; a < GL_MAX_LIGHTS ; a++ )
	{
		// look for an unused light
		if( !l_usage[a] )
		{
			lights[a].type = type;
			lights[a].state = 1;
			lights[a].position[R_AXIS] = position[X_AXIS];
			lights[a].position[G_AXIS] = position[Y_AXIS];
			lights[a].position[B_AXIS] = position[Z_AXIS];
			lights[a].position[A_AXIS] = position[W_AXIS];
			lights[a].ambient[R_AXIS] = ambient[R_AXIS];
			lights[a].ambient[G_AXIS] = ambient[G_AXIS];
			lights[a].ambient[B_AXIS] = ambient[B_AXIS];
			lights[a].ambient[A_AXIS] = ambient[A_AXIS];
			lights[a].diffuse[R_AXIS] = diffuse[R_AXIS];
			lights[a].diffuse[G_AXIS] = diffuse[G_AXIS];
			lights[a].diffuse[B_AXIS] = diffuse[B_AXIS];
			lights[a].diffuse[A_AXIS] = diffuse[A_AXIS];
			lights[a].specular[R_AXIS] = specular[R_AXIS];
			lights[a].specular[G_AXIS] = specular[G_AXIS];
			lights[a].specular[B_AXIS] = specular[B_AXIS];
			lights[a].specular[A_AXIS] = specular[A_AXIS];
			lights[a].spot_exponent = spot_exp;
			lights[a].spot_cutoff = spot_cutoff;
			lights[a].spot_direction[X_AXIS] = spot_dir[X_AXIS];
			lights[a].spot_direction[Y_AXIS] = spot_dir[Y_AXIS];
			lights[a].spot_direction[Z_AXIS] = spot_dir[Z_AXIS];
			lights[a].constant_attenuation = const_atten;
			lights[a].linear_attenuation = linear_atten;
			lights[a].quadratic_attenuation = quad_atten;

			l_usage[a] = 1;
			
			return &(lights[a]);
		}
	}

	return NULL;
}

/* ------------
l_Kill - turns off light l and marks it as unused
------------ */
void l_Kill( light_t *l )
{
	l->state = 0;
	l_usage[l->lightNum] = 0;
}

/* ------------
l_Draw - send the lights to ogl for a frame
------------ */
void l_Draw()
{
	int a;

	for( a = 0 ; a < GL_MAX_LIGHTS ; a++ )
	{
		if( lights[a].state > 0 )
		{
			glEnable( GL_LIGHT0 + a );
			glLightfv( GL_LIGHT0 + a, GL_POSITION, lights[a].position );
			glLightfv( GL_LIGHT0 + a, GL_AMBIENT, lights[a].ambient );
			glLightfv( GL_LIGHT0 + a, GL_DIFFUSE, lights[a].diffuse );
			glLightfv( GL_LIGHT0 + a, GL_SPECULAR, lights[a].specular );
			glLightf( GL_LIGHT0 + a, GL_CONSTANT_ATTENUATION, lights[a].constant_attenuation );
			glLightf( GL_LIGHT0 + a, GL_LINEAR_ATTENUATION, lights[a].linear_attenuation );
			glLightf( GL_LIGHT0 + a, GL_QUADRATIC_ATTENUATION, lights[a].quadratic_attenuation );

			if( lights[a].type == LIGHT_SPOT )
			{
				glLightfv( GL_LIGHT0 + a, GL_SPOT_DIRECTION, lights[a].spot_direction );
				glLightf( GL_LIGHT0 + a, GL_SPOT_EXPONENT, lights[a].spot_exponent );
				glLightf( GL_LIGHT0 + a, GL_SPOT_CUTOFF, lights[a].spot_cutoff );
			}
		}
		else
		{
			glDisable( GL_LIGHT0 + a );
		}
	}
}

/* ------------
l_PrintInfo - prints all light states to the console
------------ */
void l_PrintInfo( char *null )
{
	int a;
	
	con_Print( "Syntax: Type, Position, Ambient, Diffuse, Specular" );

	for( a = 0 ; a < GL_MAX_LIGHTS ; a++ )
	{
		if( lights[a].state > 0 )
		{
			con_Print( "Light %d: %d, ( %f, %f, %f, %f ), ( %f, %f, %f ), ( %f, %f, %f ), ( %f, %f, %f )",
						a, lights[a].type, lights[a].position[0], lights[a].position[1], lights[a].position[2], lights[a].position[3],
						lights[a].ambient[0], lights[a].ambient[1], lights[a].ambient[2],
						lights[a].diffuse[0], lights[a].diffuse[1], lights[a].diffuse[2], 
						lights[a].specular[0], lights[a].specular[1], lights[a].specular[2] );
		}
		else
		{
			con_Print( "Light %d: <RED>OFF", a );
		}
	}
}
