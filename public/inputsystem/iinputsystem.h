//===== Copyright � 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
//===========================================================================//

#ifndef IINPUTSYSTEM_H
#define IINPUTSYSTEM_H
#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"
#include "appframework/IAppSystem.h"

#include "inputsystem/InputEnums.h"
#include "inputsystem/ButtonCode.h"
#include "inputsystem/AnalogCode.h"

//-----------------------------------------------------------------------------
// Main interface for input. This is a low-level interface
//-----------------------------------------------------------------------------
#define INPUTSYSTEM_INTERFACE_VERSION	"InputSystemVersion001"
abstract_class IInputSystem : public IAppSystem
{
public:
	// Attach, detach input system from a particular window
	// This window should be the root window for the application
	// Only 1 window should be attached at any given time.
	virtual void AttachToWindow( void* hWnd ) = 0;
	virtual void DetachFromWindow( void* hWnd ) = 0;

	// Enables/disables input. PollInputState will not update current 
	// button/analog states when it is called if the system is disabled.
	virtual void EnableInput( bool bEnable ) = 0;

	// Enables/disables the windows message pump. PollInputState will not
	// Peek/Dispatch messages if this is disabled
	virtual void EnableMessagePump( bool bEnable ) = 0;

	// Polls the current input state
	virtual void PollInputState() = 0;

	// Gets the time of the last polling in ms
	virtual int GetPollTick() const = 0;

	// Is a button down? "Buttons" are binary-state input devices (mouse buttons, keyboard keys)
	virtual bool IsButtonDown( ButtonCode_t code ) const = 0;

	// Returns the tick at which the button was pressed and released
	virtual int GetButtonPressedTick( ButtonCode_t code ) const = 0;
	virtual int GetButtonReleasedTick( ButtonCode_t code ) const = 0;

	// Gets the value of an analog input device this frame
	// Includes joysticks, mousewheel, mouse
	virtual int GetAnalogValue( AnalogCode_t code ) const = 0;

	// Gets the change in a particular analog input device this frame
	// Includes joysticks, mousewheel, mouse
	virtual int GetAnalogDelta( AnalogCode_t code ) const = 0;

	// Returns the input events since the last poll
	virtual int GetEventCount() const = 0;
	virtual const InputEvent_t* GetEventData( ) const = 0;

	// Posts a user-defined event into the event queue; this is expected
	// to be called in overridden wndprocs connected to the root panel.
	virtual void PostUserEvent( const InputEvent_t &event ) = 0;

	// Returns the number of joysticks
	virtual int GetJoystickCount() const = 0;

	// Enable/disable joystick, it has perf costs
	virtual void EnableJoystickInput( int nJoystick, bool bEnable ) = 0;

	// Enable/disable diagonal joystick POV (simultaneous POV buttons down)
	virtual void EnableJoystickDiagonalPOV( int nJoystick, bool bEnable ) = 0;

	// Sample the joystick and append events to the input queue
	virtual void SampleDevices( void ) = 0;

	// FIXME: Should force-feedback occur here also?
	virtual void SetRumble( float fLeftMotor, float fRightMotor ) = 0;
	virtual void StopRumble( void ) = 0;

	// Resets the input state
	virtual void ResetInputState() = 0;

};


#endif // IINPUTSYSTEM_H
