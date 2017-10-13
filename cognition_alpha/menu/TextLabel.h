#ifndef _TEXTLABEL_H_
#define _TEXTLABEL_H_

// INCLUDES
/////////////
#include "Widget.h"

// NAMESPACE
//////////////
namespace CW {

// TEXTLABEL
//////////////
class TextLabel : public Widget  
{
public:
	TextLabel();
	TextLabel( const string &newLabel, const string &newName, Widget *newParent, 
		    const int &newX, const int &newY );
	~TextLabel();
	bool Create( const string &newLabel, const string &newName, 
				Widget *newParent, const int &newX, const int &newY );

	// Drawing Handler
	void Draw();

	// accessors
	void SetText( const string &newLabel ) { m_label = newLabel; }
	string GetText() const { return m_label; } 
	void SetTextSize( const float &newSize ) { m_size = newSize; }
	float GetTextSize() const { return m_size; }
	void SetColor( const color &newColor ) { memcpy(m_clr,newColor,sizeof(color)); }
	void GetColor( color &thisColor ) { memcpy(thisColor,m_clr,sizeof(color)); }

	// operators
	TextLabel &operator =( const TextLabel &tl ) 
		{	m_label = tl.m_label; 
			m_size = tl.m_size; 
			memcpy(m_clr, tl.m_clr, sizeof(color)); 
		}

private:
	string m_label;
	float m_size;
	color m_clr;
};

}

#endif // _TEXTLABEL_H_
