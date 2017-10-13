#ifndef _RADIOBUTTONGROUP_H_
#define _RADIOBUTTONGROUP_H_

// INCLUDES
/////////////
#include "CogMenuGlobal.h"
#include "RadioButton.h"
#include <vector>

// NAMESPACE
//////////////
namespace CW {

// RADIOBUTTONGROUP
//////////////////////
class RadioButtonGroup : public Widget
{
public:
	RadioButtonGroup();
	RadioButtonGroup( const string &newName, Widget *newParent );
	~RadioButtonGroup();
	bool Create( const string &newName, Widget *newParent );

	// interface
	void AddButton( RadioButton &rb );
	void RemoveButton( RadioButton &rb );

	// events
	void KeyUpEvent( const byte &key );
	void KeyDownEvent( const byte &key );
	void MouseUpEvent( const byte &button, const int &x, const int &y );
	void MouseDownEvent( const byte &button, const int &x, const int &y );
	void EnabledEvent();
	void DisabledEvent();

	// logical abstraction only
	void Draw() {}

private:
	bool Initialize();
	RadioButton *GetClickedButton( const int &x, const int &y );
	RadioButton *GetKeyedButton();
	void SetCheckedButton( RadioButton *rb );
	void SetMinimalArea();

private:
	vector<RadioButton*> m_rbList;
};

}

#endif // _RADIOBUTTONGROUP_H_

