#ifndef _WIDGET_H_
#define _WIDGET_H_

/*
	Base class for All widgets
*/
// INCLUDES
/////////////
extern "C"
{
#include "../shared/shared_misc.h"
#include "../shared/shared_structures.h"
}
#include <string>

// DEFINITIONS
////////////////
#define CW CognitionWidgets

// NAMESPACE
//////////////
using namespace std;
namespace CW {

// WIDGET
///////////
class Widget  
{
public:
	Widget();
	Widget( const string &newName, Widget *newParent, 
		    const int &newX, const int &newY, const int &newW, const int &newH );
	virtual ~Widget();

	// Event Handlers
	virtual void KeyUpEvent( const byte &key ) {}
	virtual void KeyDownEvent( const byte &key ) {}
	virtual void MouseUpEvent( const byte &button, const int &x, const int &y ) {}
	virtual void MouseDownEvent( const byte &button, const int &x, const int &y ) {}
	virtual void MouseInEvent( const int &x, const int &y ) { m_bMouseIsOver = true; }
	virtual void MouseOutEvent( const int &x, const int &y ) { m_bMouseIsOver = false; }
	virtual void MouseMoveEvent( const int &x, const int &y ) {}

	virtual void PositionChangeEvent( const int &x, const int &y ) {}
	virtual void SizeChangeEvent( const int &w, const int &h ) {}
	virtual void NameChangeEvent( const string &newName ) {}
	virtual void ParentChangeEvent( Widget *newParent ) {}

	virtual void PushedDownEvent() {}
	virtual void PushedUpEvent() {}
	virtual void EnabledEvent() {}
	virtual void DisabledEvent() {}
	virtual void FocusedEvent() {}
	virtual void UnfocusedEvent() {}

	// Drawing Handler
	virtual void Draw() = 0;

	// State Machine
	bool MouseIsOver() const { return m_bMouseIsOver; } 

	// Base Functionality
	void SetPosition( const int &x, const int &y );
	void SetSize( const int &w, const int &h );
	bool Contains( const int &x, const int &y ) const;
	bool Contains( const float &x, const float &y ) const;
	
	// Properties
	void SetName( const string &newName );
	string GetName() const { return m_Name; }
	
	void SetParent( Widget *&newParent );
	Widget *GetParent() const { return m_Parent; }

	// Accessors
	int Height() const { return m_H; }
	int Width() const { return m_W; }
	int x() const { return m_X; }
	int y() const { return m_Y; }

	void Enable() { m_Enabled = true; EnabledEvent(); }
	void Disable() { m_Enabled = false; DisabledEvent(); }
	bool IsEnabled() const { return m_Enabled; }

	void Focus() { m_Focused = true; FocusedEvent(); }
	void Unfocus() { m_Focused = false; UnfocusedEvent(); }
	bool IsFocused() const { return m_Focused; }
	void SetFocusable() { m_IsFocusable = true; }
	void SetUnfocusable() { m_IsFocusable = false; }
	bool IsFocusable() const { return m_IsFocusable; }

	void SetPushable() { m_IsPushable = true; }
	void SetUnpushable() { m_IsPushable = false; }
	bool IsPushable() const { return m_IsPushable; }
	void SetDown() { m_IsDown = true; PushedDownEvent(); }
	void SetUp() { m_IsDown = false; PushedUpEvent(); }
	bool IsDown() const { return m_IsDown; }

	// Operators
	Widget &operator =( const Widget &w );

protected:
	// Bottom Left Centric
	int m_X;
	int m_Y;
	int m_W;
	int m_H;
	int m_Top;
	int m_Right;

	// Properties
	string m_Name;
	Widget *m_Parent;

	// Flags
	bool m_Enabled;
	bool m_IsFocusable;
	bool m_Focused;
	bool m_IsPushable;
	bool m_IsDown;

	// State Machine
	bool m_bMouseIsOver;
};

}

#endif // _WIDGET_H_


