#ifndef _OPTIONSCREENROOT_H_
#define _OPTIONSCREENROOT_H_

// INCLUDES
/////////////
#include "CogMenuGlobal.h"
#include "Screen.h"
#include "TextLabel.h"
#include "Pixmap.h"
#include "Button.h"
#include "SingleScreen2D.h"

// NAMESPACE
//////////////
using namespace CW;

// OPTIONSCREENROOT
//////////////////////
class OptionScreenRoot : public Screen
{
public:
	OptionScreenRoot( Widget *newParent );
	~OptionScreenRoot();

	// Event Handlers
	void KeyUpEvent( const byte &key );
	void MouseUpEvent( const byte &button, const int &x, const int &y );

	// Drawing Handler
	void Draw();

private:
	void DescendPlayer();
	void DescendGame();
	void DescendControls();
	void DescendVideo();
	void DescendSound();

private:
	TextLabel m_tlTitle;
	TextLabel m_tlPlayer;
	TextLabel m_tlGame;
	TextLabel m_tlControls;
	TextLabel m_tlVideo;
	TextLabel m_tlSound;
	Button m_bPlayer;
	Button m_bGame;
	Button m_bControls;
	Button m_bVideo;
	Button m_bSound;
	Button m_bBack;

//	SingleScreen2D *m_Screen2D;
//	SingleScreen3D *m_Screen3D;


};

#endif // _OPTIONSCREENROOT_H_

