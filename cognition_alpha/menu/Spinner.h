#ifndef _SPINNER_H_
#define _SPINNER_H_

// INCLUDES
/////////////
#include "CogMenuGlobal.h"
#include "Widget.h"
#include "TextEdit.h"
#include "Button.h"

// DEFINITIONS
///////////////
#define SPINNER_WIDTH 150
#define SPINNER_HEIGHT 40

// NAMESPACE
//////////////
namespace CW {

// SPINNER
////////////
class Spinner : public Widget
{
public:
	Spinner();
	Spinner( const string &newName, Widget *newParent, 
				const int &newX, const int &newY );
	~Spinner();
	bool Create( const string &newName, Widget *newParent, 
				const int &newX, const int &newY );

	// Accessors
	double GetDelta() { return m_dDelta; }
	double GetValue();
	double GetMinimum() { return m_dMinValue; }
	double GetMaximum() { return m_dMaxValue; } 
	void Increment() { SetValue( m_dValue + m_dDelta ); }
	void Decrement() { SetValue( m_dValue - m_dDelta ); }
	
	// Manipulators
	void SetDelta( const double &newDelta );
	void SetValue( const double &newValue );
	void SetMinimum( const double &newMin );
	void SetMaximum( const double &newMax );
	void SetInteger( const bool &newVal ) { m_bIsInteger = newVal; }

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
	bool Initialize( const int &newX, const int &newY );
	
private:
	// widgets
	TextEdit m_teEdit;
	Button m_bUp;
	Button m_bDown;

	// properties
	bool m_bIsInteger;
	double m_dValue;
	double m_dDelta;
	double m_dMinValue;
	double m_dMaxValue;
};

}

#endif // _SPINNER_H_


