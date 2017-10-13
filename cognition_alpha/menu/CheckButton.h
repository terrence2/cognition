#ifndef _CHECKBUTTON_H_
#define _CHECKBUTTON_H_

// INCLUDES
/////////////
#include "CogMenuGlobal.h"
#include "Button.h"
#include "Widget.h"

// NAMESPACE
//////////////
namespace CW {

// BUTTON
///////////
class CheckButton : public Widget
{
public:
	CheckButton();
	CheckButton( const string &newLabel, const string &newName, Widget *newParent, 
						const int &newX, const int &newY );
	~CheckButton();
	bool Create( const string &newLabel, const string &newName, Widget *newParent, 
						const int &newX, const int &newY );

	// Event Handling
	void KeyUpEvent( const byte &key );
	void KeyDownEvent( const byte &key );
	void MouseUpEvent( const byte &button, const int &x, const int &y );
	void MouseDownEvent( const byte &button, const int &x, const int &y );
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

	void SetChecked() { m_IsChecked = true; m_pmCheck.Enable(); }
	void SetUnchecked() { m_IsChecked = false; m_pmCheck.Disable(); }
	bool IsChecked() const { return m_IsChecked; }

private:
	bool Initialize( const string &newLabel, const int &newX, const int &newY );
	void ToggleChecked();

private:
	// widgets
	Button m_bBox;
	TextLabel m_tlLabel;
	Pixmap m_pmCheck;

	// properties	
	bool m_IsChecked;
};

}

#endif // _CHECKBUTTON_H_
