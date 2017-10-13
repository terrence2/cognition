#ifndef _PICTUREBUTTON_H_
#define _PICTUREBUTTON_H_

// INCLUDES
/////////////
#include "CogMenuGlobal.h"
#include "Widget.h"
#include "Pixmap.h"

// NAMESPACE
//////////////
namespace CW {

// DEFINITIONS
////////////////
#define PBUTTON_DEF_UP_IMG24 "buttonUp24"
#define PBUTTON_DEF_UP_IMG8 "buttonUp8"
#define PBUTTON_DEF_OVER_IMG24 "buttonOver24"
#define PBUTTON_DEF_OVER_IMG8 "buttonUp8"
#define PBUTTON_DEF_DOWN_IMG24 "buttonDown24"
#define PBUTTON_DEF_DOWN_IMG8 "buttonUp8"
#define PBUTTON_DEF_DISABLED_IMG24 "buttonDisabled24"
#define PBUTTON_DEF_DISABLED_IMG8 "buttonUp8"

// BUTTON
///////////
class PictureButton : public Widget  
{
public:
	PictureButton();
	PictureButton( const string &colorMapUp, const string &alphaMapUp,
					const int &newPicW, const int &newPicH,
					const string &newName, Widget *newParent, 
					const int &newX, const int &newY, 
					const int &newW, const int &newH );
	~PictureButton();
	bool Create(  const string &colorMapUp, const string &alphaMapUp,
					const int &newPicW, const int &newPicH,
					const string &newName, Widget *newParent, 
					const int &newX, const int &newY, 
					const int &newW, const int &newH );

	// Drawing Handler
	void Draw();
	
	// Accessors
	void SetUpPixmap( const string &colorMap, const string &alphaMap ); 
	void SetDownPixmap( const string &colorMap, const string &alphaMap );
	void SetOverPixmap( const string &colorMap, const string &alphaMap );
	void SetDisabledPixmap( const string &colorMap, const string &alphaMap );

	void SetLabelUpPixmap( const string &colorMap, const string &alphaMap );
	void SetLabelDownPixmap( const string &colorMap, const string &alphaMap );
	void SetLabelOverPixmap( const string &colorMap, const string &alphaMap );
	void SetLabelDisabledPixmap( const string &colorMap, const string &alphaMap );

private:
	// Widgets
	Pixmap m_pmUp;
	Pixmap m_pmDown;
	Pixmap m_pmOver;
	Pixmap m_pmDisabled;
	Pixmap m_pmLabelUp;
	Pixmap m_pmLabelDown;
	Pixmap m_pmLabelOver;
	Pixmap m_pmLabelDisabled;
};

} 

#endif // _PICTUREBUTTON_H_
