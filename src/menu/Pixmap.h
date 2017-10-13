// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

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
      return *this;
		}

private:
	colormap_t *m_cm;
	color m_clr;
};

}

#endif // _PIXMAP_H_
