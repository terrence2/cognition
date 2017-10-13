// INCLUDES
/////////////
#include "CogMenuGlobal.h"
#include "Pixmap.h"
#include <gl/gl.h>

// NAMESPACE
//////////////
namespace CW {

/* ------------
Constructor
------------ */
Pixmap::Pixmap() : Widget()
{
	m_cm = NULL;
}

/* ------------
Constructor
------------ */
Pixmap::Pixmap( const string &colorMap, const string &alphaMap,
			    const string &newName, Widget *newParent, 
				const int &newX, const int &newY, 
				const int &newW, const int &newH )
	: Widget( newName, newParent, newX, newY, newW, newH )
{
	// Set Widget Properties
	SetUnfocusable();
	SetUnpushable();

	// Set Pixmap Properties
	m_clr[0] = m_clr[1] = m_clr[2] = m_clr[3] = 255;
	m_cm = NULL;
	SetPixmap( colorMap, alphaMap );
}

/* ------------
Create
------------ */
bool Pixmap::Create( const string &colorMap, const string &alphaMap,
			const string &newName, Widget *newParent, 
			const int &newX, const int &newY, 
			const int &newW, const int &newH )
{
	// Do Constructor Stuff
	SetName( newName );
	SetParent( newParent );
	SetPosition( newX, newY );
	SetSize( newW, newH );

	// Set Widget Properties
	SetUnfocusable();
	SetUnpushable();
	
	// Set Pixmap Properties
	m_clr[0] = m_clr[1] = m_clr[2] = m_clr[3] = 255;
	m_cm = NULL;
	SetPixmap( colorMap, alphaMap );

	// return success
	Enable();
	return true;
}

/* ------------
Destructor
------------ */
Pixmap::~Pixmap()
{
   	if( m_cm ) 
	{
		ei()->cm_Unload( m_cm );
		m_cm = NULL;
	}
}

/* ------------
Draw
------------ */
void Pixmap::Draw()
{
	if( !IsEnabled() ) return;
	if( m_cm ) ei()->d_MenuRect( x(), m_Right, y(), m_Top, m_clr, m_cm );
}

/* ------------
SetPixmap
------------ */
void Pixmap::SetPixmap( const string &colorMap, const string &alphaMap )
{
	// delay unloading to optimize for cached images
	colormap_t *cmTmp = m_cm;

	// setup the new parameters
	colormap_params_t cmParams;
	cmParams.bMipmap = 1; // (look pretty at all resolutions / scales)
	cmParams.bNoRescale = 0; // allow the engine to save texture memory
	cmParams.iEnv = GL_MODULATE; // standard blending
	cmParams.iFilter = ei()->cm_GetGlobalFilter(); // user level effects choice 
	cmParams.iScaleBias = 0; // no default biasing on rescale
	cmParams.iWrap = GL_CLAMP; // these are generally being used as labels
	string sName = colorMap + "_WITH_" + alphaMap;
	
	// do the load
	m_cm = ei()->cm_LoadFromFiles( sName.c_str(), colorMap.c_str(), alphaMap.c_str(), &cmParams );

	// check the load / unload previous
	if( !m_cm )
	{
		m_cm = cmTmp;
	}
	if( cmTmp )
	{
		ei()->cm_Unload( cmTmp );
	}
}

}
