// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

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
