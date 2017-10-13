#ifndef _RADIOBUTTON_H_
#define _RADIOBUTTON_H_

// INCLUDES
/////////////
#include "CogMenuGlobal.h"
#include "Pixmap.h"
#include "TextLabel.h"
#include "Button.h"
#include "Widget.h"

// NAMESPACE
//////////////
namespace CW {

// RADIOBUTTON
////////////////
class RadioButton : public Widget
{
public:
	RadioButton();
	RadioButton( const string &newLabel, const string &newName, Widget *newParent, 
						const int &newX, const int &newY );
	~RadioButton();
	bool Create( const string &newLabel, const string &newName, Widget *newParent, 
						const int &newX, const int &newY );

	// Event Handling
	void PositionChangeEvent( const int &x, const int &y );

	void PushedDownEvent();
	void PushedUpEvent();
	void EnabledEvent();
	void DisabledEvent();
	void FocusedEvent();
	void UnfocusedEvent();
	
	// Drawing Handler
	void Draw();
	
	// accessors
	void SetText( const string &newLabel ) { m_tlLabel.SetText(newLabel); }
	string GetText() const { return m_tlLabel.GetText(); }
	void SetColor( const color &newColor ) { m_tlLabel.SetColor(newColor); }
	void GetColor( color &thisColor ) { m_tlLabel.GetColor(thisColor); }

	void SetChecked() { m_IsSelected = true; m_pmCenter.Enable(); }
	void SetUnchecked() { m_IsSelected = false; m_pmCenter.Disable(); }
	bool IsChecked() const { return m_IsSelected; }
	
private:
	bool Initialize( const string &newLabel, const int &newX, const int &newY );

private:
	// widgets
	Button m_bButton;
	Pixmap m_pmCenter;
	TextLabel m_tlLabel;

	// properties	
	bool m_IsSelected;
};

}

#endif // _RADIOBUTTON_H_

