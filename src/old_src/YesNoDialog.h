#ifndef _YESNODIALOG_H_
#define _YESNODIALOG_H_

// INCLUDES
/////////////
#include "CogMenuGlobal.h"
#include "Widget.h"
#include "TextLabel.h"

// NAMESPACE
//////////////
using namespace CW;

// ROOTSCREEN
///////////////
class YesNoDialog : public Widget
{
public:
	YesNoDialog();
	YesNoDialog( const string &msg, const string &title, const string &newName, Widget *newParent );
	~YesNoDialog();
	Create( const string &msg, const string &title, const string &newName, Widget *newParent );

	// Events
	void Draw();

private:
	TextLabel m_tlMessage;
};

#endif // _YESNODIALOG_H_
