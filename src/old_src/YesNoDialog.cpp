// INCLUDES
////////////
#include "YesNoDialog.h"

// DEFINITIONS
///////////////
#define YN_DIALOG_FONT_SIZE 14
#define YN_DIALOG_MIN_WIDTH 255
#define YN_DIALOG_MIN_HEIGHT 128
#define YN_DIALOG_TEXT_OFFSET 20

// NAMESPACE
//////////////
using namespace CW;
	
/* ------------
Constructor
------------ */
YesNoDialog::YesNoDialog( const string &msg, const string &title, const string &newName, Widget *newParent )
	: Widget()
{
	int lines = 1;
	float len, maxLen = 0.0f;
	
	// Compute the size of the new dialog
	for( int a = 0 ; a < msg.length() ; a++ )
	{
		if( msg[a] == '\n' ) 
		{
			lines++;
			if( len > maxLen ) maxLen = len;
			len = 0.0f;
		}
		len += ei()->d_GetCharWidth( msg[a], YN_DIALOG_FONT_SIZE );
	}

	// get the best possible value
	float fTxtHt = ei()->d_GetTextHeight( YN_DIALOG_FONT_SIZE );
	int h = lines * fTxtHt;
	if( h < YN_DIALOG_MIN_HEIGHT ) h = YN_DIALOG_MIN_HEIGHT;
	int w = (int)maxLen;
	if( w < YN_DIALOG_MIN_WIDTH ) w = YN_DIALOG_MIN_WIDTH;

	// center it on screen
	int x = (SCR_WIDTH - w) / 2;
	if( x < 0 )  x = SCR_WIDTH;
	int y = (SCR_HEIGHT - h) / 2;
	if( y < 0 ) y = SCR_HEIGHT;

	// create the labels for this dialog
	m_tlMessage.Create( msg, "YNMessage", this, YN_DIALOG_TEXT_OFFSET, y + h - YN_DIALOG_MIN_HEIGHT - fTxtHt );

	// create the buttons for this dialog
}

/* ------------
Destructor
------------ */
YesNoDialog::~YesNoDialog()
{
}

/* ------------
Draw
------------ */
void YesNoDialog::Draw()
{
	// draw the parent frame

	// draw the text

	// draw the yes no buttons
}


