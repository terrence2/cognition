#ifndef _BUTTON_H_
#define _BUTTON_H_

// INCLUDES
/////////////
#include "CogMenuGlobal.h"
#include "Widget.h"
#include "Pixmap.h"
#include "TextLabel.h"

// NAMESPACE
//////////////
namespace CW {

// BUTTON
///////////
class Button : public Widget  
{
public:
	Button();
	Button( const string &newLabel, const string &newName, Widget *newParent, 
			const int &newX, const int &newY, const int &newW, const int &newH );
	~Button();
	bool Create( const string &newLabel, const string &newName, Widget *newParent, 
			const int &newX, const int &newY, const int &newW, const int &newH );

	// Event Handlers
	void PositionChangeEvent( const int &x, const int &y );
	void SizeChangeEvent( const int &w, const int &h );
	void PushedDownEvent();
	void PushedUpEvent();

	// Drawing Handler
	void Draw();
	
	// Accessors
	void SetUpPixmap( const string &colorMap, const string &alphaMap ); 
	void SetDownPixmap( const string &colorMap, const string &alphaMap );
	void SetOverPixmap( const string &colorMap, const string &alphaMap );
	void SetDisabledPixmap( const string &colorMap, const string &alphaMap );
	void SetText( const string &newLabel );
	void EnableSounds() { m_bSndEnabled = true; }
	void DisableSounds() { m_bSndEnabled = false; }

private:
	bool Initialize( const string &newLabel, const int &newX, const int &newY, const int &newW, const int &newH );

private:
	// Widgets
	Pixmap m_pmUp;
	Pixmap m_pmDown;
	Pixmap m_pmOver;
	Pixmap m_pmDisabled;
	TextLabel m_tlLabel;

	// sounds
	bool m_bSndEnabled;
	sound_t *m_sndDown;
};

} 

#endif // _BUTTON_H_
