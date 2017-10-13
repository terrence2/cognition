#ifndef _ROOTSCREEN_H_
#define _ROOTSCREEN_H_

// INCLUDES
/////////////
#include "Screen.h"
#include "TextLabel.h"
#include "Pixmap.h"
#include "Button.h"
#include "SingleScreen.h"
#include "OptionScreenRoot.h"

// NAMESPACE
//////////////
using namespace CW;

// ROOTSCREEN
///////////////
class RootScreen : public Screen  
{
public:
	RootScreen();
	~RootScreen();
	
	// Event Handlers
	void KeyUpEvent( const byte &key );
	void MouseUpEvent( const byte &button, const int &x, const int &y );

	// Drawing Handler
	void Draw();

private:
	void DescendSingle();
	void DescendMulti();
	void DescendOptions();
	void DescendExit();

private:
	Pixmap m_pmTitle;
	TextLabel m_tlSingle;
	TextLabel m_tlMulti;
	TextLabel m_tlOptions;
	TextLabel m_tlExit;
	Button m_bSingle;
	Button m_bMulti;
	Button m_bOptions;
	Button m_bExit;

	SingleScreen *m_SingleScreen;
	// MultiScreen *m_MultiScreen;
	OptionScreenRoot *m_OptionsScreen;
};

#endif // _ROOTSCREEN_H_
