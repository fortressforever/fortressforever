//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef TEXTIMAGE_H
#define TEXTIMAGE_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/Image.h>

#include <UtlVector.h>

class KeyValues;

namespace vgui
{

//-----------------------------------------------------------------------------
// Purpose: Image that handles drawing of a text string
//-----------------------------------------------------------------------------
class TextImage : public Image
{
public:	
	TextImage(const char *text);
	TextImage(const wchar_t *wszText);
	~TextImage();

public:
	// takes the string and looks it up in the localization file to convert it to unicode
	virtual void SetText(const char *text);
	// sets unicode text directly
	virtual void SetText(const wchar_t *text);
	// get the full text in the image
	virtual void GetText(char *buffer, int bufferSize);
	virtual void GetText(wchar_t *buffer, int bufferLength);
	// get the text in it's unlocalized form
	virtual void GetUnlocalizedText(char *buffer, int bufferSize);
	virtual StringIndex_t GetUnlocalizedTextSymbol();

	// set the font of the text
	virtual void SetFont(vgui::HFont font);
	// get the font of the text
	virtual vgui::HFont GetFont();

	// set the width of the text to be drawn
	// use this function if the textImage is in another window to cause 
	// the text to be truncated to the width of the window (elipsis added)
	void SetDrawWidth(int width);
	// get the width of the text to be drawn
	void GetDrawWidth(int &width); 

    void ResizeImageToContent();

	// set the size of the image
	virtual void SetSize(int wide,int tall);

	// get the full size of a text string
	virtual void GetContentSize(int &wide, int &tall);

	// draws the text
	virtual void Paint();

	void SetWrap( bool bWrap );
	void RecalculateNewLinePositions();

protected:
	// truncate the _text string to fit into the draw width
	void SizeText(wchar_t *tempText, int stringLength);
	// gets the size of a specified piece of text
	virtual void GetTextSize(int &wide, int &tall);

private:
	void RecalculateEllipsesPosition();

	wchar_t *_utext;	// unicode version of the text
	short _textBufferLen;	// size of the text buffer
	short _textLen;		// length of the text string
	vgui::HFont _font;	// font of the text string
	int _drawWidth;		// this is the width of the window we are drawing into. 
						// if there is not enough room truncate the txt	and add an elipsis

	StringIndex_t _unlocalizedTextSymbol;	// store off the unlocalized text index for build mode
	bool m_bRecalculateTruncation;
	wchar_t *m_pwszEllipsesPosition;

	bool m_bWrap;
	CUtlVector<wchar_t *>	   m_LineBreaks;		// an array that holds the index in the buffer to wrap lines at
};

} // namespace vgui

#endif // TEXTIMAGE_H
