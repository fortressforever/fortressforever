//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Basic button control
//
// $NoKeywords: $
//=============================================================================//

#include <stdio.h>
#include <UtlSymbol.h>

#include <vgui/IBorder.h>
#include <vgui/IInput.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/IVGui.h>
#include <vgui/MouseCode.h>
#include <vgui/KeyCode.h>
#include <KeyValues.h>

#include <vgui_controls/Button.h>
#include <vgui_controls/FocusNavGroup.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

// global list of all the names of all the sounds played by buttons
CUtlSymbolTable g_ButtonSoundNames;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
Button::Button(Panel *parent, const char *panelName, const char *text, Panel *pActionSignalTarget, const char *pCmd ) : Label(parent, panelName, text)
{
	Init();
	if ( pActionSignalTarget && pCmd )
	{
		AddActionSignalTarget( pActionSignalTarget );
		SetCommand( pCmd );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
Button::Button(Panel *parent, const char *panelName, const wchar_t *wszText, Panel *pActionSignalTarget, const char *pCmd ) : Label(parent, panelName, wszText)
{
	Init();
	if ( pActionSignalTarget && pCmd )
	{
		AddActionSignalTarget( pActionSignalTarget );
		SetCommand( pCmd );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Button::Init()
{
	_armed = false;
	_selected = false;
	_depressed = false;
	_forceDepressed = false;
	_buttonBorderEnabled = true;
	_useCaptureMouse = true;
	_keyDown = false;
	_mouseClickMask = 0;
	_actionMessage = NULL;
	_defaultBorder = NULL;
	_depressedBorder = NULL;
	_keyFocusBorder = NULL;
	_defaultButton = false;
	m_sArmedSoundName = UTL_INVAL_SYMBOL;
	m_sDepressedSoundName = UTL_INVAL_SYMBOL;
	m_sReleasedSoundName = UTL_INVAL_SYMBOL;
	SetTextInset(6, 0);
	SetMouseClickEnabled( MOUSE_LEFT, true );
	SetButtonActivationType(ACTIVATE_ONPRESSEDANDRELEASED);

	// labels have this off by default, but we need it on
	SetPaintBackgroundEnabled(true);
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
Button::~Button()
{
	if (_actionMessage)
	{
		_actionMessage->deleteThis();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Button::SetButtonActivationType(ActivationType_t activationType)
{
	_activationType = activationType;
}

//-----------------------------------------------------------------------------
// Purpose: Set button border attribute enabled.
//-----------------------------------------------------------------------------
void Button::SetButtonBorderEnabled( bool state )
{
	if (state != _buttonBorderEnabled)
	{
		_buttonBorderEnabled = state;
		InvalidateLayout(false);
	}
}

//-----------------------------------------------------------------------------
// Purpose:	Set button selected state.
//-----------------------------------------------------------------------------
void Button::SetSelected( bool state )
{
	if (_selected != state)
	{
		_selected = state;
		RecalculateDepressedState();
		InvalidateLayout(false);
	}
}

//-----------------------------------------------------------------------------
// Purpose:	Set button force depressed state.
//-----------------------------------------------------------------------------
void Button::ForceDepressed(bool state)
{
	if (_forceDepressed != state)
	{
		_forceDepressed = state;
		RecalculateDepressedState();
		InvalidateLayout(false);
	}
}

//-----------------------------------------------------------------------------
// Purpose:	Set button depressed state with respect to the force depressed state.
//-----------------------------------------------------------------------------
void Button::RecalculateDepressedState( void )
{
	bool newState;
	if (!IsEnabled())
	{
		newState = false;
	}
	else
	{
		newState = _forceDepressed ? true : (_armed && _selected);
	}

	_depressed = newState;
}

//-----------------------------------------------------------------------------
// Purpose: Sets whether or not the button captures all mouse input when depressed
//			Defaults to true
//			Should be set to false for things like menu items where there is a higher-level mouse capture
//-----------------------------------------------------------------------------
void Button::SetUseCaptureMouse( bool state )
{
	_useCaptureMouse = state;
}

//-----------------------------------------------------------------------------
// Purpose: Check if mouse capture is enabled.
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool Button::IsUseCaptureMouseEnabled( void )
{
	return _useCaptureMouse;
}

//-----------------------------------------------------------------------------
// Purpose:	Set armed state.
//-----------------------------------------------------------------------------
void Button::SetArmed(bool state)
{
	if (_armed != state)
	{
		_armed = state;
		RecalculateDepressedState();
		InvalidateLayout(false);

		// play any sounds specified
		if (_armed && m_sArmedSoundName != UTL_INVAL_SYMBOL)
		{
			surface()->PlaySound(g_ButtonSoundNames.String(m_sArmedSoundName));
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:	Check armed state
//-----------------------------------------------------------------------------
bool Button::IsArmed()
{
	return _armed;
}

//-----------------------------------------------------------------------------
// Purpose:	Activate a button click.
//-----------------------------------------------------------------------------
void Button::DoClick()
{
	SetSelected(true);
	FireActionSignal();

	// check for playing a transition sound
	if (m_sReleasedSoundName != UTL_INVAL_SYMBOL)
	{
		surface()->PlaySound(g_ButtonSoundNames.String(m_sReleasedSoundName));
	}

	SetSelected(false);
}

//-----------------------------------------------------------------------------
// Purpose: Check selected state
//-----------------------------------------------------------------------------
bool Button::IsSelected()
{
	return _selected;
}

//-----------------------------------------------------------------------------
// Purpose:	Check depressed state
//-----------------------------------------------------------------------------
bool Button::IsDepressed()
{
	return _depressed;
}

//-----------------------------------------------------------------------------
// Purpose:	Paint button on screen
//-----------------------------------------------------------------------------
void Button::Paint(void)
{
	BaseClass::Paint();

	if (HasFocus()  && IsEnabled() )
	{
		int x0, y0, x1, y1;
		int wide, tall;
		GetSize(wide, tall);
		x0 = 3, y0 = 3, x1 = wide - 4 , y1 = tall - 2;
		DrawFocusBorder(x0, y0, x1, y1);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Perform graphical layout of button.
//-----------------------------------------------------------------------------
void Button::PerformLayout()
{
	// reset our border
	SetBorder(GetBorder(_depressed, _armed, _selected, HasFocus()));

	// set our color
	SetFgColor(GetButtonFgColor());
	SetBgColor(GetButtonBgColor());

	BaseClass::PerformLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Get button foreground color
// Output : Color
//-----------------------------------------------------------------------------
Color Button::GetButtonFgColor()
{
	if (_depressed)
	{
		return _depressedFgColor;
	}
	else if (_armed)
	{
		return _armedFgColor;
	}
	else
	{
		return _defaultFgColor;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Get button background color
//-----------------------------------------------------------------------------
Color Button::GetButtonBgColor()
{
	if (_depressed)
	{
		return _depressedBgColor;
	}
	else if (_armed)
	{
		return _armedBgColor;
	}
	else
	{
		return _defaultBgColor;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called when key focus is received
//-----------------------------------------------------------------------------
void Button::OnSetFocus()
{
	InvalidateLayout(false);
	BaseClass::OnSetFocus();
}

//-----------------------------------------------------------------------------
// Purpose: Respond when focus is killed
//-----------------------------------------------------------------------------
void Button::OnKillFocus()
{
	InvalidateLayout(false);
	BaseClass::OnKillFocus();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Button::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	// get the borders we need
	_defaultBorder = pScheme->GetBorder("ButtonBorder");
	_depressedBorder = pScheme->GetBorder("ButtonDepressedBorder");
	_keyFocusBorder = pScheme->GetBorder("ButtonKeyFocusBorder");

	_defaultFgColor = GetSchemeColor("Button.TextColor", Color(255, 255, 255, 255), pScheme);
	_defaultBgColor = GetSchemeColor("Button.BgColor", Color(0, 0, 0, 255), pScheme);

	_armedFgColor = GetSchemeColor("Button.ArmedTextColor", _defaultFgColor, pScheme);
	_armedBgColor = GetSchemeColor("Button.ArmedBgColor", _defaultBgColor, pScheme);

	_depressedFgColor = GetSchemeColor("Button.DepressedTextColor", _defaultFgColor, pScheme);
	_depressedBgColor = GetSchemeColor("Button.DepressedBgColor", _defaultBgColor, pScheme);
	_keyboardFocusColor = GetSchemeColor("Button.FocusBorderColor", Color(0,0,0,255), pScheme);
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Set default button colors.
//-----------------------------------------------------------------------------
void Button::SetDefaultColor(Color fgColor, Color bgColor)
{
	if (!(_defaultFgColor == fgColor && _defaultBgColor == bgColor))
	{
		_defaultFgColor = fgColor;
		_defaultBgColor = bgColor;

		InvalidateLayout(false);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set armed button colors
//-----------------------------------------------------------------------------
void Button::SetArmedColor(Color fgColor, Color bgColor)
{
	if (!(_armedFgColor == fgColor && _armedBgColor == bgColor))
	{
		_armedFgColor = fgColor;
		_armedBgColor = bgColor;

		InvalidateLayout(false);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set depressed button colors
//-----------------------------------------------------------------------------
void Button::SetDepressedColor(Color fgColor, Color bgColor)
{
	if (!(_depressedFgColor == fgColor && _depressedBgColor == bgColor))
	{
		_depressedFgColor = fgColor;
		_depressedBgColor = bgColor;

		InvalidateLayout(false);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set default button border attributes.
//-----------------------------------------------------------------------------
void Button::SetDefaultBorder(IBorder *border)
{
	_defaultBorder = border;
	InvalidateLayout(false);
}

//-----------------------------------------------------------------------------
// Purpose: Set depressed button border attributes.
//-----------------------------------------------------------------------------
void Button::SetDepressedBorder(IBorder *border)
{
	_depressedBorder = border;
	InvalidateLayout(false);
}

//-----------------------------------------------------------------------------
// Purpose: Set key focus button border attributes.
//-----------------------------------------------------------------------------
void Button::SetKeyFocusBorder(IBorder *border)
{
	_keyFocusBorder = border;
	InvalidateLayout(false);
}


//-----------------------------------------------------------------------------
// Purpose: Get button border attributes.
//-----------------------------------------------------------------------------
IBorder *Button::GetBorder(bool depressed, bool armed, bool selected, bool keyfocus)
{
	if (_buttonBorderEnabled)
	{
		// raised buttons with no armed state
		if (depressed)
		{
			return _depressedBorder;
		}
		else if (keyfocus)
		{
			return _keyFocusBorder;
		}
		else if (IsEnabled() && _defaultButton)
		{
			return _keyFocusBorder;
		}
		else
		{
			return _defaultBorder;
		}
	}
	else
	{
		// flat buttons that raise
		if (depressed)
		{
			return _depressedBorder;
		}
		else if (armed)
		{
			return _defaultBorder;
		}
	}

	return _defaultBorder;
}

//-----------------------------------------------------------------------------
// Purpose: sets this button to be the button that is accessed by default 
//			when the user hits ENTER or SPACE
//-----------------------------------------------------------------------------
void Button::SetAsCurrentDefaultButton(int state)
{
	if (_defaultButton != (bool)state)
	{
		_defaultButton = state;
		if (state)
		{
			// post a message up notifying our nav group that we're now the default button
			if (GetVParent())
			{
				KeyValues *msg = new KeyValues("CurrentDefaultButtonSet");
				msg->SetPtr("button", this);

				ivgui()->PostMessage(GetVParent(), msg, GetVPanel());
			}
		}

		InvalidateLayout();
		Repaint();
	}
}

//-----------------------------------------------------------------------------
// Purpose: sets this button to be the button that is accessed by default 
//			when the user hits ENTER or SPACE
//-----------------------------------------------------------------------------
void Button::SetAsDefaultButton(int state)
{
	if (_defaultButton != (bool)state)
	{
		_defaultButton = state;
		if (state)
		{
			// post a message up notifying our nav group that we're now the default button
			if (GetVParent())
			{
				KeyValues *msg = new KeyValues("DefaultButtonSet");
				msg->SetPtr("button", this);

				ivgui()->PostMessage(GetVParent(), msg, GetVPanel());
			}
		}

		InvalidateLayout();
		Repaint();
	}
}

//-----------------------------------------------------------------------------
// Purpose: sets rollover sound
//-----------------------------------------------------------------------------
void Button::SetArmedSound(const char *sound)
{
	if (sound)
	{
		m_sArmedSoundName = g_ButtonSoundNames.AddString(sound);
	}
	else
	{
		m_sArmedSoundName = UTL_INVAL_SYMBOL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Button::SetDepressedSound(const char *sound)
{
	if (sound)
	{
		m_sDepressedSoundName = g_ButtonSoundNames.AddString(sound);
	}
	else
	{
		m_sDepressedSoundName = UTL_INVAL_SYMBOL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Button::SetReleasedSound(const char *sound)
{
	if (sound)
	{
		m_sReleasedSoundName = g_ButtonSoundNames.AddString(sound);
	}
	else
	{
		m_sReleasedSoundName = UTL_INVAL_SYMBOL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set button to be mouse clickable or not.
//-----------------------------------------------------------------------------
void Button::SetMouseClickEnabled(MouseCode code,bool state)
{
	if(state)
	{
		//set bit to 1
		_mouseClickMask|=1<<((int)(code+1));
	}
	else
	{
		//set bit to 0
		_mouseClickMask&=~(1<<((int)(code+1)));
	}	
}

//-----------------------------------------------------------------------------
// Purpose: Check if button is mouse clickable
//-----------------------------------------------------------------------------
bool Button::IsMouseClickEnabled(MouseCode code)
{
	if(_mouseClickMask&(1<<((int)(code+1))))
	{
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: sets the command to send when the button is pressed
//-----------------------------------------------------------------------------
void Button::SetCommand( const char *command )
{
	SetCommand(new KeyValues("Command", "command", command));
}

//-----------------------------------------------------------------------------
// Purpose: sets the message to send when the button is pressed
//-----------------------------------------------------------------------------
void Button::SetCommand( KeyValues *message )
{
	// delete the old message
	if (_actionMessage)
	{
		_actionMessage->deleteThis();
	}

	_actionMessage = message;
}

//-----------------------------------------------------------------------------
// Purpose: Message targets that the button has been pressed
//-----------------------------------------------------------------------------
void Button::FireActionSignal()
{
	// message-based action signal
	if (_actionMessage)
	{
		// see if it's a url
		if (!stricmp(_actionMessage->GetName(), "command")
			&& !strnicmp(_actionMessage->GetString("command", ""), "url ", strlen("url "))
			&& strstr(_actionMessage->GetString("command", ""), "://"))
		{
			// it's a command to launch a url, run it
			system()->ShellExecute("open", _actionMessage->GetString("command", "      ") + 4);
		}
		PostActionSignal(_actionMessage->MakeCopy());
	}
}

//-----------------------------------------------------------------------------
// Purpose: gets info about the button
//-----------------------------------------------------------------------------
bool Button::RequestInfo(KeyValues *outputData)
{
	if (!stricmp(outputData->GetName(), "CanBeDefaultButton"))
	{
		outputData->SetInt("result", CanBeDefaultButton() ? 1 : 0);
		return true;
	}
	else if (!stricmp(outputData->GetName(), "GetState"))
	{
		outputData->SetInt("state", IsSelected());
		return true;
	}
	else if ( !stricmp( outputData->GetName(), "GetCommand" ))
	{
		if ( _actionMessage )
		{
			outputData->SetString( "command", _actionMessage->GetString( "command", "" ) );
		}
		else
		{
			outputData->SetString( "command", "" );
		}
		return true;
	}


	return BaseClass::RequestInfo(outputData);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool Button::CanBeDefaultButton(void)
{
    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Get control settings for editing
//-----------------------------------------------------------------------------
void Button::GetSettings( KeyValues *outResourceData )
{
	BaseClass::GetSettings(outResourceData);

	if (_actionMessage)
	{
		outResourceData->SetString("command", _actionMessage->GetString("command", ""));
	}
	outResourceData->SetInt("default", _defaultButton);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Button::ApplySettings( KeyValues *inResourceData )
{
	BaseClass::ApplySettings(inResourceData);

	const char *cmd = inResourceData->GetString("command", "");
	if (*cmd)
	{
		// add in the command
		SetCommand(cmd);
	}

	// set default button state
	int defaultButton = inResourceData->GetInt("default");
	if (defaultButton && CanBeDefaultButton())
	{
		SetAsDefaultButton(true);
	}
}


//-----------------------------------------------------------------------------
// Purpose: Describes editing details
//-----------------------------------------------------------------------------
const char *Button::GetDescription( void )
{
	static char buf[1024];
	Q_snprintf(buf, sizeof(buf), "%s, string command, int default", BaseClass::GetDescription());
	return buf;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Button::OnSetState(int state)
{
	SetSelected((bool)state);
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Button::OnCursorEntered()
{
	if (IsEnabled())
	{
		SetArmed(true);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Button::OnCursorExited()
{
	if (!_keyDown)
	{
		SetArmed(false);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Button::OnMousePressed(MouseCode code)
{
	if (!IsEnabled())
		return;
	
	if (!IsMouseClickEnabled(code))
		return;

	if (_activationType == ACTIVATE_ONPRESSED)
	{
		DoClick();
		return;
	}

	// play activation sound
	if (m_sDepressedSoundName != UTL_INVAL_SYMBOL)
	{
		surface()->PlaySound(g_ButtonSoundNames.String(m_sDepressedSoundName));
	}

	if (IsUseCaptureMouseEnabled() && _activationType == ACTIVATE_ONPRESSEDANDRELEASED)
	{
		{
			RequestFocus();
			SetSelected(true);
			Repaint();
		}

		// lock mouse input to going to this button
		input()->SetMouseCapture(GetVPanel());
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Button::OnMouseDoublePressed(MouseCode code)
{
	OnMousePressed(code);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Button::OnMouseReleased(MouseCode code)
{
	// ensure mouse capture gets released
	if (IsUseCaptureMouseEnabled())
	{
		input()->SetMouseCapture(NULL);
	}

	if (_activationType == ACTIVATE_ONPRESSED)
		return;

	if (!IsMouseClickEnabled(code))
		return;

	if (!IsSelected() && _activationType == ACTIVATE_ONPRESSEDANDRELEASED)
		return;

	// it has to be both enabled and (mouse over the button or using a key) to fire
	if (IsEnabled() && (GetVPanel() == input()->GetMouseOver() || _keyDown))
	{
		DoClick();
	}
	else
	{
		SetSelected(false);
	}

	// make sure the button gets unselected
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Button::OnKeyCodePressed(KeyCode code)
{
	if (code == KEY_SPACE || code == KEY_ENTER)
	{
		SetArmed(true);
		_keyDown = true;
		OnMousePressed(MOUSE_LEFT);
		if (IsUseCaptureMouseEnabled()) // undo the mouse capture since its a fake mouse click!
		{
			input()->SetMouseCapture(NULL);
		}
	}
	else
	{
		_keyDown = false;
		BaseClass::OnKeyCodePressed(code);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Button::OnKeyCodeReleased(KeyCode code)
{
	if (_keyDown && (code == KEY_SPACE || code == KEY_ENTER))
	{
		SetArmed(true);
		OnMouseReleased(MOUSE_LEFT);
	}
	else
	{
		BaseClass::OnKeyCodeReleased(code);
	}
	_keyDown = false;
	SetArmed(false);
}

//-----------------------------------------------------------------------------
// Purpose: Override this to draw different focus border
//-----------------------------------------------------------------------------
void Button::DrawFocusBorder(int tx0, int ty0, int tx1, int ty1)
{
	surface()->DrawSetColor(_keyboardFocusColor);
	DrawDashedLine(tx0, ty0, tx1, ty0+1, 1, 1);		// top
	DrawDashedLine(tx0, ty0, tx0+1, ty1, 1, 1);		// left
	DrawDashedLine(tx0, ty1-1, tx1, ty1, 1, 1);		// bottom
	DrawDashedLine(tx1-1, ty0, tx1, ty1, 1, 1);		// right
}

//-----------------------------------------------------------------------------
// Purpose: Size the object to its button and text.  - only works from in ApplySchemeSettings or PerformLayout()
//-----------------------------------------------------------------------------
void Button::SizeToContents()
{
	int wide, tall;
	GetContentSize(wide, tall);
	SetSize(wide + Label::Content, tall + Label::Content);
}

