#ifndef _SINGLESCREEN_H_
#define _SINGLESCREEN_H_

// INCLUDES
/////////////
#include "CogMenuGlobal.h"
#include "Screen.h"
#include "TextLabel.h"
#include "Pixmap.h"
#include "Button.h"
#include "Single2DOptionsScreen.h"
#include "SingleScreen3D.h"

// NAMESPACE
//////////////
using namespace CW;

// SINGLESCREEN
///////////////
class SingleScreen : public Screen
{
public:
	SingleScreen( Widget *newParent );
	~SingleScreen();

	// Event Handlers
	void KeyUpEvent( const byte &key );
	void MouseUpEvent( const byte &button, const int &x, const int &y );

	// Drawing Handler
	void Draw();

private:
	void Descend2D();
	void Descend3D();

private:
	TextLabel m_tlTitle;
	TextLabel m_tl2D;
	TextLabel m_tl3D;
	Button m_b2D;
	Button m_b3D;
	Button m_bBack;

	Single2DOptionsScreen *m_Screen2D;
	SingleScreen3D *m_Screen3D;

};

#endif // _SINGLESCREEN_H_

