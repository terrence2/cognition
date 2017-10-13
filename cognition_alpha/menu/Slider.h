#ifndef _SLIDER_H_
#define _SLIDER_H_

// INCLUDES
/////////////
#include "CogMenuGlobal.h"
#include "Widget.h"
#include "Pixmap.h"
#include "Spinner.h"

// NAMESPACE
//////////////
namespace CW {

// SLIDER
///////////
class Slider : public Widget
{
public:
	Slider();
	Slider( const string &newName, Widget *newParent, 
				const int &newX, const int &newY, const int &newW );
	~Slider();
	bool Create( const string &newName, Widget *newParent, 
				const int &newX, const int &newY, const int &newW );

	// Accessors
	double GetDelta() { return m_spin.GetDelta(); }
	double GetValue() { return m_spin.GetValue(); }
	double GetMinimum() { return m_spin.GetMinimum(); }
	double GetMaximum() { return m_spin.GetMinimum(); } 
	
	// Manipulators
	void SetDelta( const double &newDelta );
	void SetValue( const double &newValue );
	void SetMinimum( const double &newMin );
	void SetMaximum( const double &newMax ); 
	void SetInteger( const bool &newVal );

	// Event Handlers
	void KeyUpEvent( const byte &key );
	void KeyDownEvent( const byte &key );
	void MouseUpEvent( const byte &button, const int &x, const int &y );
	void MouseDownEvent( const byte &button, const int &x, const int &y );
	void MouseMoveEvent( const int &x, const int &y );
	void PositionChangeEvent( const int &x, const int &y );
	void SizeChangeEvent( const int &w, const int &h );
	void FocusedEvent();
	void UnfocusedEvent();
	void Draw();

private:
	bool Initialize( const int &newX, const int &newY, const int &newW );
	void PositionIndicator();
	void SetDragValue( const int &newX );
	
private:
	// widgets
	Spinner m_spin;
	Pixmap m_pmBar;
	Pixmap m_pmPos;
	colormap_t *m_cmMark;

	// states
	bool m_bDraggingPos;
};

}

#endif // _SLIDER_H_

