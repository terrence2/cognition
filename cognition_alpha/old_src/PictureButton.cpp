// INCLUDES
/////////////
#include "PictureButton.h"

// NAMESPACE
//////////////
namespace CW {

/* ------------
Constructor
------------ */
PictureButton::PictureButton() : Widget()
{
}

/* ------------
Constructor
------------ */
PictureButton::PictureButton(  const string &colorMapUp, const string &alphaMapUp,
								const int &newPicW, const int &newPicH,
								const string &newName, Widget *newParent, 
								const int &newX, const int &newY, 
								const int &newW, const int &newH )
				: Widget( newName, newParent, newX, newY, newW, newH )
{
	// set basic properties
	SetFocusable();
	SetPushable();
	SetUp();
	
	// load images
	m_pmUp.Create( PBUTTON_DEF_UP_IMG24, PBUTTON_DEF_UP_IMG8, 
					"PictureButtonUpPM", this, newX, newY, newW, newH );
	m_pmDown.Create( PBUTTON_DEF_DOWN_IMG24, PBUTTON_DEF_DOWN_IMG8,
					"PictureButtonDownPM", this, newX, newY, newW, newH );
	m_pmOver.Create( PBUTTON_DEF_OVER_IMG24, PBUTTON_DEF_OVER_IMG8, 
					"PictureButtonOverPM", this, newX, newY, newW, newH );
	m_pmDisabled.Create( PBUTTON_DEF_DISABLED_IMG24, PBUTTON_DEF_DISABLED_IMG8,
					"PictureButtonDisabledPM", this, newX, newY, newW, newH );

	// compute a centering for the picture
	int XOff = (newW - newPicW) / 2;
	int YOff = (newH - newPicH) / 2;
	m_pmLabelUp.Create( colorMapUp, alphaMapUp, "PictureButtonLabelUpPM", 
						this, newX + XOff, newY + YOff, newPicW, newPicH );
	m_pmLabelDown.Create( colorMapUp, alphaMapUp, "PictureButtonLabelDownPM", 
						this, newX + XOff, newY + YOff, newPicW, newPicH );
	m_pmLabelOver.Create( colorMapUp, alphaMapUp, "PictureButtonLabelOverPM", 
						this, newX + XOff, newY + YOff, newPicW, newPicH );
	m_pmLabelDisabled.Create( colorMapUp, alphaMapUp, "PictureButtonLabelDisabledPM", 
						this, newX + XOff, newY + YOff, newPicW, newPicH );
}

/* ------------
Destructor
------------ */
PictureButton::~PictureButton()
{
}

/* ------------
Create
------------ */
bool PictureButton::Create( const string &colorMapUp, const string &alphaMapUp,
							const int &newPicW, const int &newPicH,
							const string &newName, Widget *newParent, 
							const int &newX, const int &newY, 
							const int &newW, const int &newH )
{
	// Set Widget Properties
	SetName( newName );
	SetParent( newParent );
	SetPosition( newX, newY );
	SetSize( newW, newH );
	SetFocusable();
	SetPushable();
	SetUp();

	// load images
	m_pmUp.Create( PBUTTON_DEF_UP_IMG24, PBUTTON_DEF_UP_IMG8, 
					"PictureButtonUpPM", this, newX, newY, newW, newH );
	m_pmDown.Create( PBUTTON_DEF_DOWN_IMG24, PBUTTON_DEF_DOWN_IMG8,
					"PictureButtonDownPM", this, newX, newY, newW, newH );
	m_pmOver.Create( PBUTTON_DEF_OVER_IMG24, PBUTTON_DEF_OVER_IMG8, 
					"PictureButtonOverPM", this, newX, newY, newW, newH );
	m_pmDisabled.Create( PBUTTON_DEF_DISABLED_IMG24, PBUTTON_DEF_DISABLED_IMG8,
					"PictureButtonDisabledPM", this, newX, newY, newW, newH );

	// compute a centering for the picture
	int XOff = (newW - newPicW) / 2;
	int YOff = (newH - newPicH) / 2;
	m_pmLabelUp.Create( colorMapUp, alphaMapUp, "PictureButtonLabelUpPM", 
						this, newX + XOff, newY + YOff, newPicW, newPicH );
	m_pmLabelDown.Create( colorMapUp, alphaMapUp, "PictureButtonLabelDownPM", 
						this, newX + XOff, newY + YOff, newPicW, newPicH );
	m_pmLabelOver.Create( colorMapUp, alphaMapUp, "PictureButtonLabelOverPM", 
						this, newX + XOff, newY + YOff, newPicW, newPicH );
	m_pmLabelDisabled.Create( colorMapUp, alphaMapUp, "PictureButtonLabelDisabledPM", 
						this, newX + XOff, newY + YOff, newPicW, newPicH );

	// Return Success
	Enable();
	return true;
}

/* ------------
Draw
------------ */
void PictureButton::Draw()
{
	Pixmap *pm;
	Pixmap *pmLabel;
	
	if( !IsEnabled() )
	{
		pm = &m_pmDisabled;
		pmLabel = &m_pmLabelDisabled;
	}

	else if( IsDown() )
	{
		pm = &m_pmDown;
		pmLabel = &m_pmLabelDown;
	}
	
	else if( IsFocused() )
	{
		pm = &m_pmOver;
		pmLabel = &m_pmLabelOver;
	}

	// otherwise, draw normally
	else
	{
		pm = &m_pmUp;
		pmLabel = &m_pmLabelUp;
	}

	// do the pixmap draw
	if( pm ) pm->Draw();

	// do the label draw
	if( pmLabel ) pmLabel->Draw();
}

/* ------------
SetUpPixmap
------------ */
void PictureButton::SetUpPixmap( const string &colorMap, const string &alphaMap )
{
	m_pmUp.SetPixmap( colorMap, alphaMap );
}

/* ------------
SetDownPixmap
------------ */
void PictureButton::SetDownPixmap( const string &colorMap, const string &alphaMap )
{
	m_pmDown.SetPixmap( colorMap, alphaMap );
}

/* ------------
SetOverPixmap
------------ */
void PictureButton::SetOverPixmap( const string &colorMap, const string &alphaMap )
{
	m_pmOver.SetPixmap( colorMap, alphaMap );
}

/* ------------
SetDisabledPixmap
------------ */
void PictureButton::SetDisabledPixmap( const string &colorMap, const string &alphaMap )
{
	m_pmDisabled.SetPixmap( colorMap, alphaMap );
}

/* ------------
SetLabelUpPixmap
------------ */
void PictureButton::SetLabelUpPixmap( const string &colorMap, const string &alphaMap )
{
	m_pmLabelUp.SetPixmap( colorMap, alphaMap );
}

/* ------------
SetLabelDownPixmap
------------ */
void PictureButton::SetLabelDownPixmap( const string &colorMap, const string &alphaMap )
{
	m_pmLabelDown.SetPixmap( colorMap, alphaMap );
}

/* ------------
SetLabelOverPixmap
------------ */
void PictureButton::SetLabelOverPixmap( const string &colorMap, const string &alphaMap )
{
	m_pmLabelOver.SetPixmap( colorMap, alphaMap );
}

/* ------------
SetLabelDisabledPixmap
------------ */
void PictureButton::SetLabelDisabledPixmap( const string &colorMap, const string &alphaMap )
{
	m_pmLabelDisabled.SetPixmap( colorMap, alphaMap );
}

}
