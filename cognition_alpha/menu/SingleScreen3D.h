#ifndef _SINGLESCREEN3D_H_
#define _SINGLESCREEN3D_H_

// INCLUDES
/////////////
#include "CogMenuGlobal.h"
#include "Screen.h"
#include "TextLabel.h"
#include "Pixmap.h"
#include "Button.h"
#include "CheckButton.h"
#include "RadioButton.h"
#include "RadioButtonGroup.h"
#include "Slider.h"

// NAMESPACE
//////////////
using namespace CW;

// SINGLESCREEN
///////////////
class SingleScreen3D : public Screen
{
// c++ mem management
public:
	SingleScreen3D( Widget *newParent );
	~SingleScreen3D();

	// Event Handlers
	void KeyUpEvent( const byte &key );
	void MouseUpEvent( const byte &button, const int &x, const int &y );

	// Virtual Events
	virtual void Frame( exec_state_t *state );
	
	// Drawing Handler
	void Draw();

// workers
private:
	void UpdateVars();
	void UpdateUseQuestions();
	void UpdateGameType();
	void UpdateWidth();
	void UpdateHeight();
	void UpdateDepth();
	void UpdateMineCount();
	void UpdateMaxMines();
	void StartGame();
	void EndGame();

// object memory
private:
	// memory map to file and engine
	varlatch m_vlGameType;
	varlatch m_vlHeight;
	varlatch m_vlWidth;
	varlatch m_vlDepth;
	varlatch m_vlUseQuestion;
	varlatch m_vlNumMines;

	// widgets
	TextLabel m_tlTitle;
	CheckButton m_ckUseQs;
	RadioButton m_rbGame3;
	RadioButton m_rbGame2;
	RadioButton m_rbGame1;
	RadioButtonGroup m_rbgGameType;	
	TextLabel m_tlWidth;
	Slider m_sliderWidth;
	TextLabel m_tlHeight;
	Slider m_sliderHeight;
	TextLabel m_tlDepth;
	Slider m_sliderDepth;
	TextLabel m_tlMines;
	Slider m_sliderMines;
	Button m_bLaunch;
	Button m_bEnd;
	Button m_bBack;
};

#endif // _SINGLESCREEN3D_H_

