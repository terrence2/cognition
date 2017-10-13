// INCLUDES
/////////////
#include "CogMenuGlobal.h"
#include "Slider.h"
#include <gl/gl.h>
#include <string>

// DEFINITIONS
///////////////
#define SLIDER_MIN_WIDTH 300
#define SLIDER_MAX_WIDTH 1024
#define SLIDER_HEIGHT 40

#define PM_POS24 "sliderPos24"
#define PM_POS8 "sliderPos8"
#define PM_POS_WIDTH 15
#define PM_POS_HEIGHT 30
#define PM_BAR24 "sliderBar24"
#define PM_BAR8 "sliderBar8"
#define CM_MARK24 "sliderMark24"
#define CM_MARK8 "sliderMark8"
#define CM_MAX_MARKS 640
#define CM_BAR_OFFSETX 50
#define CM_MARK_WIDTH 2.0000
#define CM_MARK_OFFSETX -1
#define CM_MARK_OFFSETY 5
#define CM_MARK_HEIGHT 30

// NAMESPACE
//////////////
namespace CW {

/* ------------
Constructor
------------ */
Slider::Slider() : Widget() {}

/* ------------
Constructor
------------ */
Slider::Slider( const string &newName, Widget *newParent, 
				const int &newX, const int &newY, const int &newW )
	: Widget( newName, newParent, newX, newY, newW, SLIDER_HEIGHT )
{
	// Init
	Initialize( newX, newY, newW );
}

/* ------------
Destructor
------------ */
Slider::~Slider()
{
}

/* ------------
Destructor
------------ */
bool Slider::Create( const string &newName, Widget *newParent, 
				const int &newX, const int &newY, const int &newW )
{
	// Set Widget stuff
	SetName( newName );
	SetParent( newParent );
	SetPosition( newX, newY );
	SetSize( newW, SLIDER_HEIGHT );

	// init
	return Initialize( newX, newY, newW );
}

/* ------------
Construction Worker
------------ */
bool Slider::Initialize( const int &newX, const int &newY, const int &newW )
{
	// widget properties
	SetFocusable();
	SetPushable();
	
	// properties
	m_bDraggingPos = false;
	
	// adjust for w
	int w = newW;
	if( w < SLIDER_MIN_WIDTH ) 
	{
		w = SLIDER_MIN_WIDTH;
		SetSize( SLIDER_MIN_WIDTH, SLIDER_HEIGHT );
	}
	else if( w > SLIDER_MAX_WIDTH )
	{
		w = SLIDER_MAX_WIDTH;
		SetSize( SLIDER_MAX_WIDTH, SLIDER_HEIGHT );
	}
	
	// create widgets
	m_spin.Create( "SliderSpinner", this, newX + w - SPINNER_WIDTH, newY );
	m_pmBar.Create( PM_BAR24, PM_BAR8, "SliderBar", this, newX, newY, w - SPINNER_WIDTH, SLIDER_HEIGHT );
	m_pmPos.Create( PM_POS24, PM_POS8, "SliderMarker", this, newX, newY, PM_POS_WIDTH, PM_POS_HEIGHT );

	// set sane spin defaults
	m_spin.SetDelta( 1.0 );
	m_spin.SetMinimum( 0.0 );
	m_spin.SetMaximum( 20.0 );
	m_spin.SetValue( 0.0 );
	PositionIndicator();

	// setup the new parameters
	colormap_params_t cmParams;
	cmParams.bMipmap = 1; // (look pretty at all resolutions / scales)
	cmParams.bNoRescale = 0; // allow the engine to save texture memory
	cmParams.iEnv = GL_MODULATE; // standard blending
	cmParams.iFilter = ei()->cm_GetGlobalFilter(); // user level effects choice 
	cmParams.iScaleBias = 0; // no default biasing on rescale
	cmParams.iWrap = GL_CLAMP;

	// do the load
	m_cmMark = ei()->cm_LoadFromFiles( "SliderMark", CM_MARK24, CM_MARK8, &cmParams );

	return true;
}

/* ------------
PositionIndicator
// position the position indicator on the bar
------------ */
void Slider::PositionIndicator()
{
	// % offset on bar
	double percentOff = (m_spin.GetValue() - m_spin.GetMinimum()) / (m_spin.GetMaximum() - m_spin.GetMinimum());
	
	// actual offset
	double offset = percentOff * (double)(m_pmBar.Width() - (2 * CM_BAR_OFFSETX));
	int scr = (int)offset + CM_BAR_OFFSETX - (PM_POS_WIDTH / 2);

	// set the position
	m_pmPos.SetPosition( m_pmBar.x() + scr, m_pmBar.y() );
}

/* ------------
SetDragValue
// set the value of the widget according to the drag location
------------ */
void Slider::SetDragValue( const int &newX )
{
	// in screen coords
	int max = m_pmBar.x() + m_pmBar.Width() - CM_BAR_OFFSETX;
	int min = m_pmBar.x() + CM_BAR_OFFSETX;

	// lock inside
	if( newX > max )
	{
		m_spin.SetValue( m_spin.GetMaximum() );
		return;
	}
	if( newX < min ) 
	{
		m_spin.SetValue( m_spin.GetMinimum() );
		return;
	}

	// find the value of the current position
	double percent = (double)(newX - min) / (double)(max - min);
	double offset = (percent * (m_spin.GetMaximum() - m_spin.GetMinimum()));
	double value =  offset + m_spin.GetMinimum();

	// look for the near delta values
	int lowMarker = (int)((value - m_spin.GetMinimum()) / m_spin.GetDelta());
	int lowVal = lowMarker * m_spin.GetDelta() + m_spin.GetMinimum();
	int highVal = lowVal + m_spin.GetDelta();

	// find the closest delta value to value
	if( (value - lowVal) < (highVal - value) )
	{
		// snap to small
		m_spin.SetValue( lowVal );
	}
	else
	{
		// snap to large
		m_spin.SetValue( highVal );
	}
}

/* ------------
SetDelta
------------ */
void Slider::SetDelta( const double &newDelta ) 
{ 
	m_spin.SetDelta(newDelta); 
	PositionIndicator();
}

/* ------------
SetValue
------------ */
void Slider::SetValue( const double &newValue ) 
{ 
	m_spin.SetValue(newValue); 
	PositionIndicator();
}

/* ------------
SetMinimum
------------ */
void Slider::SetMinimum( const double &newMin ) 
{ 
	m_spin.SetMinimum(newMin); 
	PositionIndicator();
}

/* ------------
SetMaximum
------------ */
void Slider::SetMaximum( const double &newMax ) 
{ 
	m_spin.SetMaximum(newMax);
	PositionIndicator();
}

/* ------------
SetInteger
------------ */
void Slider::SetInteger( const bool &newVal ) 
{ 
	m_spin.SetInteger(newVal); 
	PositionIndicator();
}

/* ------------
KeyUpEvent
------------ */
void Slider:: KeyUpEvent( const byte &key )
{
	Widget::KeyUpEvent(key);
	m_spin.KeyUpEvent(key);
	PositionIndicator();
}

/* ------------
KeyDownEvent
------------ */
void Slider::KeyDownEvent( const byte &key )
{
	Widget::KeyDownEvent(key);
	m_spin.KeyDownEvent(key);
	PositionIndicator();
}

/* ------------
MouseUpEvent
------------ */
void Slider::MouseUpEvent( const byte &button, const int &x, const int &y )
{
	Widget::MouseUpEvent(button,x,y);
	if( m_spin.Contains(x,y) )
	{
		m_spin.MouseUpEvent(button,x,y);
	}
	m_bDraggingPos = false;
	PositionIndicator();
}

/* ------------
MouseDownEvent
------------ */
void Slider::MouseDownEvent( const byte &button, const int &x, const int &y )
{
	Widget::MouseDownEvent(button,x,y);
	if( m_spin.Contains(x,y) )
	{
		m_spin.MouseDownEvent(button,x,y);
	}
	else if( m_pmBar.Contains(x,y) )
	{
		m_bDraggingPos = true;
		MouseMoveEvent( x, y );
	}
	PositionIndicator();
}

/* ------------
MouseMoveEvent
------------ */
void Slider::MouseMoveEvent( const int &x, const int &y )
{
	Widget::MouseMoveEvent(x,y);
	if( m_spin.Contains(x,y) )
	{
		m_spin.MouseMoveEvent(x,y);
	}
	if( m_bDraggingPos )
	{
		SetDragValue( x );
		PositionIndicator();
	}
}

/* ------------
PositionChangeEvent
------------ */
void Slider::PositionChangeEvent( const int &x, const int &y )
{
	Widget::PositionChangeEvent( x, y );
	m_spin.SetPosition( x + Width() - SPINNER_WIDTH, y );
	m_pmBar.SetPosition( x, y );
	PositionIndicator();
}

/* ------------
SizeChangeEvent
------------ */
void Slider::SizeChangeEvent( const int &w, const int &h ) 
{
	// workers
	int width = w;
	int height = h;

	// lock to size
	if( w < SLIDER_MIN_WIDTH ) width = SLIDER_MIN_WIDTH;
	if( w > SLIDER_MAX_WIDTH ) width = SLIDER_MAX_WIDTH;

	// reposition
	m_spin.SetPosition( x() + width - SPINNER_WIDTH, SPINNER_HEIGHT );
	m_pmBar.SetSize( width - SPINNER_WIDTH, SLIDER_HEIGHT );
	PositionIndicator();
}

/* ------------
FocusedEvent
------------ */
void Slider::FocusedEvent() 
{
	Widget::FocusedEvent();
	m_spin.FocusedEvent();
}

/* ------------
UnfocusedEvent
------------ */
void Slider::UnfocusedEvent() 
{
	Widget::UnfocusedEvent();
	m_spin.UnfocusedEvent();
}

/* ------------
Draw
------------ */
void Slider::Draw()
{
	color white = { 255, 255, 255, 200 };
	double fLeft, fRight, fTop, fBottom;
	
	// draw the basis
	m_spin.Draw();
	m_pmBar.Draw();

	// get the mark count
	double cnt = ((m_spin.GetMaximum() - m_spin.GetMinimum()) / m_spin.GetDelta()) + 1.0 ;

	// get the draw mark interval
	double interval = (double)(m_pmBar.Width() - (2 * CM_BAR_OFFSETX))  / (double)(cnt - 1);

	// space out the interval
	while( interval < 6.0 )
	{
		interval *= 2;
		cnt /= 2;
	}

	// draw the marks
	fLeft = (double)m_pmBar.x() + (double)CM_BAR_OFFSETX - (double)CM_MARK_OFFSETX;
	fRight = fLeft + (double)CM_MARK_WIDTH;
	fBottom = (double)y() + (double)CM_MARK_OFFSETY;
	fTop = fBottom + (double)CM_MARK_HEIGHT;
	for( int a = 0 ; a < (int)cnt ; a++ )
	{	
		// draw a mark
		ei()->d_MenuRect( fLeft, fRight, fBottom, fTop, white, m_cmMark );

		// advance
		fLeft += interval;
		fRight = fLeft + (double)CM_MARK_WIDTH;
	}

	// draw the position indicator
	m_pmPos.Draw();
}


}
