//===== Copyright � 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
//===========================================================================//

#ifndef INPUTENUMS_H
#define INPUTENUMS_H
#ifdef _WIN32
#pragma once
#endif

// Standard maximum +/- value of a joystick axis
#define MAX_BUTTONSAMPLE			32768

//-----------------------------------------------------------------------------
// Forward declarations: 
//-----------------------------------------------------------------------------

enum
{
	MAX_JOYSTICKS = 1,
	MOUSE_BUTTON_COUNT = 5,
};

enum JoystickAxis_t
{
	JOY_AXIS_X = 0,
 	JOY_AXIS_Y,
	JOY_AXIS_Z,
	JOY_AXIS_R,
	JOY_AXIS_U,
	JOY_AXIS_V,
	MAX_JOYSTICK_AXES,
};



//-----------------------------------------------------------------------------
// Extra mouse codes
//-----------------------------------------------------------------------------
enum
{
	MS_WM_XBUTTONDOWN	= 0x020B,
	MS_WM_XBUTTONUP		= 0x020C,
	MS_WM_XBUTTONDBLCLK	= 0x020D,
	MS_MK_BUTTON4		= 0x0020,
	MS_MK_BUTTON5		= 0x0040,
};

//-----------------------------------------------------------------------------
// Events
//-----------------------------------------------------------------------------
enum InputEventType_t
{
	IE_ButtonPressed = 0,	// m_nData contains a ButtonCode_t
	IE_ButtonReleased,		// m_nData contains a ButtonCode_t
	IE_ButtonDoubleClicked,	// m_nData contains a ButtonCode_t
	IE_JoyButtonPressed,	// m_nData contains a ButtonCode_t
	IE_JoyButtonReleased,	// m_nData contains a ButtonCode_t
	IE_AnalogValueChanged,	// m_nData contains an AnalogCode_t, m_nData2 contains the value

	IE_EventTypeCount,
};

struct InputEvent_t
{
	int m_nType;				// Type of the event (see InputEventType_t)
	int m_nTick;				// Tick on which the event occurred
	int m_nData;				// Generic 32-bit data, what it contains depends on the event
	int m_nData2;				// Generic 32-bit data, what it contains depends on the event
	int m_nData3;				// Generic 32-bit data, what it contains depends on the event
};

#endif // INPUTENUMS_H
