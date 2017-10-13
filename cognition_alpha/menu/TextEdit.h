#ifndef _TEXTEDIT_H_
#define _TEXTEDIT_H_

// INCLUDES
/////////////
#include "CogMenuGlobal.h"
#include "Widget.h"
#include "TextLabel.h"
#include "Pixmap.h"
#include <string>

// NAMESPACE
//////////////
namespace CW {

// TEXTEDIT
/////////////
class TextEdit : public Widget
{
public:
	TextEdit();
	TextEdit( const string &newName, Widget *newParent, 
				const int &newX, const int &newY, const int &newW );
	~TextEdit();
	bool Create( const string &newName, Widget *newParent, 
				const int &newX, const int &newY, const int &newW );

	// Manipulators
	string GetText() const { return m_tlText.GetText(); }
	void SetText( const string &newText ); 

	// Event Handlers
	void KeyUpEvent( const byte &key );
	void KeyDownEvent( const byte &key );
	void MouseUpEvent( const byte &button, const int &x, const int &y );
	void MouseDownEvent( const byte &button, const int &x, const int &y );
	void MouseMoveEvent( const int &x, const int &y );
	void PositionChangeEvent( const int &x, const int &y );
	void SizeChangeEvent( const int &w, const int &h );
	void FocusedEvent();
	void UnfocusedEvent();

	// Drawing handler
	void Draw();

private:
	bool Initialize( const int &newX, const int &newY, const int &newW );
	void PositionCursor();
	int GetClickPosition( const float &fx );
	float GetOffsetPosition( const int &off );
	void RemoveSelection();

private:
	// widgets
	TextLabel m_tlText;
	Pixmap m_pmCursor;
	Pixmap m_pmBackground;
	Pixmap m_pmSelection;

	// states
	float m_fCurLength;
	float m_fMaxLength;
	string m_text;
	int m_iCursorPos;
	bool m_bShifted;
	bool m_bInDrag;
	bool m_bHaveSelection;
	int m_iStartDrag;
	int m_iEndDrag;
		
};

}

#endif // _TEXTEDIT_H_

