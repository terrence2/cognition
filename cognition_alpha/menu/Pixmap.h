#ifndef _PIXMAP_H_
#define _PIXMAP_H_

// INCLUDES
/////////////
#include "Widget.h"

// NAMESPACE
//////////////
namespace CW {

// PIXMAP
///////////
class Pixmap : public Widget  
{
public:
	Pixmap();
	Pixmap( const string &colorMap, const string &alphaMap,
			const string &newName, Widget *newParent, 
			const int &newX, const int &newY, 
			const int &newW, const int &newH );
	~Pixmap();
	bool Create( const string &colorMap, const string &alphaMap,
				const string &newName, Widget *newParent, 
				const int &newX, const int &newY, 
				const int &newW, const int &newH );

	// Drawing Handler
	void Draw();

	// accessors
	void SetPixmap( const string &colorMap, const string &alphaMap );
	colormap_t *GetPixmap() const { return m_cm; }
	void SetColor( const color &newColor ) { memcpy(m_clr,newColor,sizeof(color)); }
	void GetColor( color &thisColor ) { memcpy(thisColor,m_clr,sizeof(color)); }

	// operators
	Pixmap &operator =( const Pixmap &tl ) 
		{	 
			m_cm = tl.m_cm; 
			memcpy(m_clr, tl.m_clr, sizeof(color)); 
		}

private:
	colormap_t *m_cm;
	color m_clr;
};

}

#endif // _PIXMAP_H_

